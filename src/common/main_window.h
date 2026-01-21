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

namespace common
{
	std::string rom_filename;

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

                chip8::init_emulator(filePath);
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
            }

            // close
            ImGuiFileDialog::Instance()->Close();
        }

        return 0;
    }

	int update_emulator(sf::RenderWindow& window)
    {
		return chip8::update(window);
	}

    int process_event(sf::Event* event)
    {
        return chip8::process_event(event);
    }

    int main_window(int argc, const char* argv[])
    {
        // do some arg parsing
        argparse::ArgumentParser parser("Argument parser for Emulators");
        parser.add_argument("-d", "--disassemble", "Disassemble the rom", false);
        parser.add_argument("-a", "--assemble", "Assemble the rom", false);
        parser.add_argument("-r", "--rom_file", "Rom file", false);

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
			std::cout << "Error - Disassembler not yet implemented" << std::endl;
            return -1;
        }
        else if (parser.exists("a"))
        {
            std::string rom_filename = parser.get<std::string>("r");
            std::cout << "Error - Assembler not yet implemented" << std::endl;
            return -1;
        }

        // TODO - need to add support for the unit tests from gameboy

		// init sfml window
        sf::RenderWindow window(sf::VideoMode({ 1024, 768 }), "Emulators");
        window.setFramerateLimit(60);
        bool success = ImGui::SFML::Init(window);

        // if a rom is passed we will load it right away
        rom_filename = "";
		if (parser.exists("r"))
		{
            rom_filename = parser.get<std::string>("r");
		}

		chip8::init_emulator(rom_filename);

        //DemoWindow(window);
        //return 0;

        sf::Clock deltaClock;
        while (window.isOpen()) 
        {
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
                    process_event(const_cast<sf::Event*>(&(*event)));
                }
            }

            ImGui::SFML::Update(window, deltaClock.restart());

            // handle the main menu
            update_mainmenu(window);

            // handle rendering the emulator
            bool display = update_emulator(window);

            //window.clear();
            ImGui::SFML::Render(window);
            if (display)
            {
                window.display();
            }
        }

        ImGui::SFML::Shutdown();

        return 0;
    }
}
