#include "defines.h"
#include <iostream>

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

    struct EmulatorDisplay
    {
        int fps;
        float display_scale;
        const sf::Texture* display_texture = nullptr;
    };

    typedef int (*EmulatorInitFunction)(const std::string&);
    typedef int (*EmulatorDestroyFunction)();
    typedef int (*EmulatorUpdateFunction)(const sf::Time&);
    typedef int (*EmulatorProcessEventFunction)(const sf::Event*);
    typedef int (*EmulatorAssemblerFunction)(const std::string&);
    typedef int (*EmulatorDisassemblerFunction)(const std::string&);

    sf::RenderWindow window;
    u8 emulator_type = -1;
    
    EmulatorInitFunction emulator_function_init = nullptr;
    EmulatorDestroyFunction emulator_function_destroy = nullptr;
    EmulatorUpdateFunction emulator_function_update = nullptr;
    EmulatorProcessEventFunction emulator_function_process_event = nullptr;
    EmulatorAssemblerFunction emulator_function_assembler = nullptr;
    EmulatorDisassemblerFunction emulator_function_disassembler = nullptr;

    EmulatorDisplay emulator_display;

    // debug variables
    namespace debug
    {
        u32 fps = 0;
        u32 frame_count = 0;
        sf::Clock fps_clock;

        int init()
        {
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

    int setup_emulator(u8 emulator_type)
    {
        switch (emulator_type)
        {
        case EMULATOR_TYPE_CHIP8:
            emulator_function_init = chip8::init_emulator;
            emulator_function_destroy = chip8::destroy_emulator;
            emulator_function_update = chip8::update;
            emulator_function_process_event = chip8::process_event;
            emulator_function_assembler = chip8::assembler::assemble_to_file;
            emulator_function_disassembler = chip8::disassembler::disassemble_to_file;

            emulator_display.display_texture = chip8::get_emulator_texture();
            emulator_display.fps = 60;
            emulator_display.display_scale = 1.0f;
            break;
        case EMULATOR_TYPE_GAMEBOY:
            emulator_function_init = gameboy::init_emulator;
            emulator_function_destroy = gameboy::destroy_emulator;
            emulator_function_update = gameboy::update;
            emulator_function_process_event = gameboy::process_event;
            emulator_function_assembler = gameboy::assembler::assemble_to_file;
            emulator_function_disassembler = gameboy::disassembler::disassemble_to_file;

            emulator_display.display_texture = gameboy::get_emulator_texture();
            emulator_display.fps = 60;
            emulator_display.display_scale = 4.0f;
            break;
        default:
            return -1;
        }

        // set any window settings
        window.setFramerateLimit(emulator_display.fps);

        return 0;
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

                setup_emulator(EMULATOR_TYPE_CHIP8);
                emulator_function_init(filePath);
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

                setup_emulator(EMULATOR_TYPE_GAMEBOY);
                emulator_function_init(filePath);
            }

            // close
            ImGuiFileDialog::Instance()->Close();
        }

        return 0;
    }

    void update_main_display(const sf::RenderWindow& window)
    {
        float menuHeight = ImGui::GetFrameHeight();
        float debugHeight = 64.0f;
        float windowWidth = ImGui::GetIO().DisplaySize.x;
        float windowHeight = ImGui::GetIO().DisplaySize.y;

        // Position it right below the menu bar. and to fill width
        ImGui::SetNextWindowPos(ImVec2(0, menuHeight));
        ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x, 0), ImGuiCond_Always);

        // draw emulator display with imgui layout
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoCollapse;

        ImGui::Begin("Emulator Display", nullptr, window_flags);

        if (emulator_display.display_texture != nullptr)
        {
            // Get the texture from the sprite
            sf::Vector2u textureSize = emulator_display.display_texture->getSize();
            textureSize.x *= emulator_display.display_scale;
            textureSize.y *= emulator_display.display_scale;

            // Display as ImGui image
            ImGui::Image(*emulator_display.display_texture, sf::Vector2f(textureSize.x, textureSize.y));

            // Show controls below - TODO: make emulator specific control
            ImGui::Separator();
            ImGui::Text("Controls: QWER/ASDF/ZXCV + 0234");
            ImGui::Text("Space: Reset");
        }
        else
        {
            ImGui::Text("No emulator loaded");
            ImGui::Text("Use File -> Load Rom to load a ROM");
        }

        ImGui::End();

        // Position it right below the menu bar. and to fill width
        ImGui::SetNextWindowPos(ImVec2(0, ImGui::GetFrameHeight()));
        ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x, 0), ImGuiCond_Always);

        // Show FPS in a separate window
        ImGui::SetNextWindowPos(ImVec2(0, windowHeight - debugHeight));
        ImGui::SetNextWindowSize(ImVec2(windowWidth, debugHeight), ImGuiCond_Always);

        ImGui::Begin("Debug Info", nullptr, window_flags);
        debug::update();
        ImGui::Text("FPS: %d", debug::fps);
        ImGui::End();
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
        ImGuiContext* main_context = ImGui::CreateContext();
        window = sf::RenderWindow(sf::VideoMode({ 1920, 1080 }), "Emulators");

        ImGui::SetCurrentContext(main_context);
        bool success = ImGui::SFML::Init(window, false);

        // Setup font for MAIN context
        ImGuiIO& io = ImGui::GetIO();
        io.Fonts->Clear();
        io.Fonts->AddFontFromFileTTF("courbd.ttf", 24);
        ImGui::SFML::UpdateFontTexture();

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
        setup_emulator(emulator_type);
        emulator_function_init(rom_filename);

        debug::init();

        sf::Clock clock;
        while (window.isOpen()) 
        {
            // update on the main imgui context
            ImGui::SetCurrentContext(main_context);

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
                    else if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>())
                    {
                        if (keyPressed->code == sf::Keyboard::Key::F1)
                        {
                            // open debugger
                        }
                    }

                    // send event first to emulator to process. only if emulator returns != 0 is it consumed
                    emulator_function_process_event(const_cast<sf::Event*>(&(*event)));
                }
            }

            // handle updating the emulator
            emulator_function_update(deltaTime);

            // imgui updates and drawing
            window.clear();
            ImGui::SFML::Update(window, deltaTime);

            // handle the main menu
            update_mainmenu(window);

            // layout the main display
            update_main_display(window);

            // clear and display updated sfml window
            ImGui::SFML::Render(window);
            window.display();
        }

        // destory the emulator. may need to call this before we init a new emulator
        emulator_function_destroy(); 

        window.close();

        ImGui::SetCurrentContext(main_context);
        ImGui::SFML::Shutdown(window);

        ImGui::DestroyContext(main_context);

        return 0;
    }
}
