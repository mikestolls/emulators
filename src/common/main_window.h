#include "defines.h"
#include <iostream>

#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>
#include <imgui.h>
#include <imgui-SFML.h>
#include <ImGuiFileDialog.h>
#include <argparse.h>

#include "chip8/chip8.h"
#include "chip8/rom.h"
#include "chip8/assembler.h"
#include "chip8/disassembler.h"

#include "gameboy/gameboy.h"
#include "gameboy/rom.h"
#include "gameboy/assembler.h"
#include "gameboy/disassembler.h"

namespace common
{
    enum
    {
        EMULATOR_TYPE_CHIP8 = 0,
        EMULATOR_TYPE_GAMEBOY,
    };

    typedef int (*EmulatorInitFunction)(const std::string&);
    typedef int (*EmulatorUpdateFunction)();
    typedef int (*EmulatorProcessEventFunction)(const sf::Event*);
    typedef int (*EmulatorAssemblerFunction)(const std::string&);
    typedef int (*EmulatorDisassemblerFunction)(const std::string&);

    u8 emulator_type = -1;
    
    EmulatorInitFunction emulator_function_init = nullptr;
    EmulatorUpdateFunction emulator_function_update = nullptr;
    EmulatorProcessEventFunction emulator_function_process_event = nullptr;
    EmulatorAssemblerFunction emulator_function_assembler = nullptr;
    EmulatorDisassemblerFunction emulator_function_disassembler = nullptr;

    sf::RenderTexture* emulator_render_texture = nullptr;
    std::unique_ptr<sf::Sprite> emulator_sprite;

    // debug variables
    namespace debug
    {
        bool debug_fps = true;
        u32 fps = 0;
        u32 frame_count = 0;
        sf::Clock fps_clock;

        // fps counter and profiler
        sf::Font font;
        sf::Text fps_text(font);

        int init()
        {
            // init the debug text
            bool success = font.openFromFile("courbd.ttf");
            sf::Text fps_text(font);

            fps_text.setFillColor(sf::Color::White);
            fps_text.setPosition(sf::Vector2f(10, 10));
            fps_text.setOutlineColor(sf::Color::Black);
            fps_text.setOutlineThickness(2);
            fps_text.setCharacterSize(18);

            return 0;
        }

        int update()
        {
            frame_count++;

            // Update FPS display once per second
            if (fps_clock.getElapsedTime().asSeconds() >= 1.0f)
            {
                fps = frame_count;
                frame_count = 0;
                fps_clock.restart();
            }

            return 0;
        }

        int draw(sf::RenderWindow& window)
        {
            if (debug_fps)
            {
                // show profliler stats
                std::stringstream stream;
                stream << "FPS: " << fps << "\n";

                fps_text.setString(stream.str());
                fps_text.setPosition(sf::Vector2f(0.0f, 16.0f));

                window.draw(fps_text);
            }

            return 0;
        }
    }

    int get_emulator_type(std::string& emulator)
    {
        if (emulator == "chip8")
        {
            return EMULATOR_TYPE_CHIP8;
        }
        else if (emulator == "gameboy")
        {
            return EMULATOR_TYPE_GAMEBOY;
        }
         
        return -1;
    }

    int get_emulator_type_from_rom(std::string& filename)
    {
        if (filename.find(".ch8") != std::string::npos)
        {
            return EMULATOR_TYPE_CHIP8;
        }
        else if (filename.find(".gb") != std::string::npos)
        {
            return EMULATOR_TYPE_GAMEBOY;
        }

        return -1;
    }

    int setup_emulator(u8 emulator_type, const std::string& rom_filename)
    {
        switch (emulator_type)
        {
        case EMULATOR_TYPE_CHIP8:
            emulator_function_init = &chip8::init_emulator;
            emulator_function_update = &chip8::update;
            emulator_function_process_event = &chip8::process_event;
            emulator_function_assembler = &chip8::assembler::assemble_to_file;
            emulator_function_disassembler = &chip8::disassembler::disassemble_to_file;

            emulator_render_texture = chip8::get_render_texture();
            break;
        case EMULATOR_TYPE_GAMEBOY:
            emulator_function_init = &gameboy::init_emulator;
            emulator_function_update = &gameboy::update;
            emulator_function_process_event = &gameboy::process_event;
            emulator_function_assembler = &gameboy::assembler::assemble_to_file;
            emulator_function_disassembler = &gameboy::disassembler::disassemble_to_file;

            emulator_render_texture = nullptr;
            break;
        default:
            return -1;
        }

        // run the emulator init with the rom and setup sprite texture
        emulator_function_init(rom_filename);

        if (emulator_render_texture != nullptr)
        {
            emulator_sprite = std::make_unique<sf::Sprite>(emulator_render_texture->getTexture());
        }

        return 0;
    }

    void DemoWindow(sf::RenderWindow& window)
    {
        sf::CircleShape shape(100.f);
        shape.setFillColor(sf::Color::Green);

        sf::Clock deltaClock;
        while (window.isOpen()) {
            while (const auto event = window.pollEvent()) {
                ImGui::SFML::ProcessEvent(window, *event);

                if (event->is<sf::Event::Closed>()) {
                    window.close();
                }
            }

            ImGui::SFML::Update(window, deltaClock.restart());

            ImGui::ShowDemoWindow();

            ImGui::Begin("Hello, world!");
            ImGui::Button("Look at this pretty button");
            ImGui::End();

            window.clear();
            window.draw(shape);
            ImGui::SFML::Render(window);
            window.display();
        }
    }

    int update_mainmenu(sf::RenderWindow& window)
    {
        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::BeginMenu("Load Rom"))
                {
                    if (ImGui::MenuItem("Chip8 Rom"))
                    {
                        IGFD::FileDialogConfig config;
                        config.path = ".";
                        ImGuiFileDialog::Instance()->OpenDialog("LoadChip8Rom", "Load Chip8 Rom", ".ch8", config);
                    }
                    else if (ImGui::MenuItem("Gameboy Rom"))
                    {
                        IGFD::FileDialogConfig config;
                        config.path = ".";
                        ImGuiFileDialog::Instance()->OpenDialog("LoadGameboyRom", "Load Gameboy Rom", ".gb", config);
                    }

                    ImGui::EndMenu();
                }

                ImGui::Separator();

                if (ImGui::MenuItem("Exit"))
                {
                    window.close();
                }

                ImGui::EndMenu();
            }

            ImGui::EndMainMenuBar();
        }

        // check if file dialogs are displaying
        if (ImGuiFileDialog::Instance()->Display("LoadChip8Rom"))
        {
            if (ImGuiFileDialog::Instance()->IsOk())
            {
                std::string filePath = ImGuiFileDialog::Instance()->GetFilePathName();
                std::cout << "Selected Chip8 Rom: " << filePath << std::endl;

                setup_emulator(EMULATOR_TYPE_CHIP8, filePath);
            }

            // close
            ImGuiFileDialog::Instance()->Close();
        }
        else if (ImGuiFileDialog::Instance()->Display("LoadGameboyRom"))
        {
            if (ImGuiFileDialog::Instance()->IsOk())
            {
                std::string filePath = ImGuiFileDialog::Instance()->GetFilePathName();
                std::cout << "Selected Gameboy Rom: " << filePath << std::endl;

                setup_emulator(EMULATOR_TYPE_GAMEBOY, filePath);
            }

            // close
            ImGuiFileDialog::Instance()->Close();
        }

        return 0;
    }

    int main_window(int argc, const char* argv[])
    {
        std::string rom_filename = "";

        // do some arg parsing
        argparse::ArgumentParser parser("Argument parser for Emulators");
        parser.add_argument("-d", "--disassemble", "Disassemble the rom", false);
        parser.add_argument("-a", "--assemble", "Assemble the rom", false);
        parser.add_argument("-r", "--rom_file", "Rom file", false);
        parser.add_argument("-e", "--emulator", "Emulator Type (chip8, gameboy)", false);

        parser.enable_help();
        auto err = parser.parse(argc, argv);
        if (err)
        {
            std::cout << err << std::endl;
            return -1;
        }

        if (parser.exists("help"))
        {
            parser.print_help();
            return 0;
        }
        else if (parser.exists("d"))
        {
            std::string rom_filename = parser.get<std::string>("r");
            emulator_function_disassembler(rom_filename);
            return 0;
        }
        else if (parser.exists("a"))
        {
            std::string rom_filename = parser.get<std::string>("r");
            emulator_function_assembler(rom_filename);
            return 0;
        }

        // TODO - need to add support for the unit tests from gameboy

		// init sfml window
        sf::RenderWindow window(sf::VideoMode({ 1024, 768 }), "Emulators");
        window.setFramerateLimit(60); // TODO: set the emulator specific one
        bool success = ImGui::SFML::Init(window);

        // if a rom is passed we will load it right away
		if (parser.exists("r"))
		{
            rom_filename = parser.get<std::string>("r");
		}

        // check if emulator type is passed
        if (parser.exists("e"))
        {
            std::string emulator = parser.get<std::string>("e");
            emulator_type = get_emulator_type(emulator);
        }
        else
        {
            // try to determine emulator by rom file extension
            emulator_type = get_emulator_type_from_rom(rom_filename);
        }

        // setup sprite to draw the emulator onto main window
        setup_emulator(emulator_type, rom_filename);

        //DemoWindow(window);
        //return 0;

        debug::init();

        sf::Clock clock;
        while (window.isOpen()) 
        {
            sf::Time deltaTime = clock.restart();

            while (const auto event = window.pollEvent()) 
            {
                if (event.has_value())
                {
                    ImGui::SFML::ProcessEvent(window, *event);
                    if (event->is<sf::Event::Closed>())
                    {
                        window.close();
                    }

                    // send event first to emulator to process. only if emulator returns != 0 is it consumed
                    emulator_function_process_event(const_cast<sf::Event*>(&(*event)));
                }
            }

            ImGui::SFML::Update(window, deltaTime);

            // handle the main menu
            update_mainmenu(window);

            // handle updating the emulator
            emulator_function_update();

            window.clear();

            // render the emualtor texture
            emulator_sprite->setPosition(sf::Vector2f(0.0f, 32.0f));
            window.draw(*emulator_sprite);

            // update menu
            ImGui::SFML::Render(window);

            // debug drawing
            debug::update();
            debug::draw(window);
            
            // finally display
            window.display();
        }

        ImGui::SFML::Shutdown();

        return 0;
    }
}
