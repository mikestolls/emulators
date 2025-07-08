#include "defines.h"
#include <iostream>

#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>
#include <imgui.h>
#include <imgui-SFML.h>
#include <ImGuiFileDialog.h>

namespace common
{
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

    int main_window()
    {
        sf::RenderWindow window(sf::VideoMode({ 1024, 768 }), "Emulators");
        window.setFramerateLimit(60);
        bool success = ImGui::SFML::Init(window);

        if (!success)
        {
            std::cout << "Error - Create window failed" << std::endl;
            return -1;
        }

        //DemoWindow(window);
        //return 0;

        sf::Clock deltaClock;
        while (window.isOpen()) 
        {
            while (const auto event = window.pollEvent()) 
            {
                ImGui::SFML::ProcessEvent(window, *event);

                if (event->is<sf::Event::Closed>()) 
                {
                    window.close();
                }
            }

            ImGui::SFML::Update(window, deltaClock.restart());

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
                            ImGuiFileDialog::Instance()->OpenDialog("LoadChip8Rom", "Load Chip8 Rom", ".ch8,*.*", config);
						}
                        else if (ImGui::MenuItem("Gameboy Rom"))
                        {
                            IGFD::FileDialogConfig config;
                            config.path = ".";
                            ImGuiFileDialog::Instance()->OpenDialog("LoadGameboyRom", "Load Gameboy Rom", ".gb,*.*", config);
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

            window.clear();
            ImGui::SFML::Render(window);
            window.display();
        }

        ImGui::SFML::Shutdown();

        return 0;
    }
}
