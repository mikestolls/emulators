#pragma once

#include "defines.h"

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>
#include <imgui.h>
#include <imgui-SFML.h>

#include "debugger_tileset.h"
#include "debugger_tilemap.h"
#include "debugger_registers_palette.h"
#include "debugger_disassembler.h"
#include "debugger_memory.h"

namespace gameboy
{
	namespace debugger
	{
        typedef int (*DebuggerPanelInit)();
        typedef int (*DebuggerPanelUpdate)();
        typedef int (*DebuggerPanelDraw)();

        struct DebuggerPanel
        {
            DebuggerPanelInit init = nullptr;
            DebuggerPanelUpdate update = nullptr;
            DebuggerPanelDraw draw = nullptr;
        };

        ImGuiContext* context = nullptr;
        sf::RenderWindow window;

        DebuggerPanel panel_tileset;
        DebuggerPanel panel_tilemap;
        DebuggerPanel panel_registers_palette;
        DebuggerPanel panel_disassembler;
        DebuggerPanel panel_memory;

        int setup_debugger_panels()
        {
            // setup tileset panel;
            panel_tileset.init = tileset::init;
            panel_tileset.update = tileset::update;
            panel_tileset.draw = tileset::draw;
            panel_tileset.init();

            // setup tilemap panel;
            panel_tilemap.init = tilemap::init;
            panel_tilemap.update = tilemap::update;
            panel_tilemap.draw = tilemap::draw;
            panel_tilemap.init();

            // setup registers and palette panel
            panel_registers_palette.init = registers_palette::init;
            panel_registers_palette.update = registers_palette::update;
            panel_registers_palette.draw = registers_palette::draw;
            panel_registers_palette.init();

            // setup disassembler panel
            panel_disassembler.init = disassembler::init;
            panel_disassembler.update = disassembler::update;
            panel_disassembler.draw = disassembler::draw;
            panel_disassembler.init();

            // setup memory panel
            panel_memory.init = memory::init;
            panel_memory.update = memory::update;
            panel_memory.draw = memory::draw;
            panel_memory.init();

            return 0;
        }

		int init_debugger()
		{
            ImGuiContext* old_context = ImGui::GetCurrentContext();

            // init sfml window
            context = ImGui::CreateContext();

            window = sf::RenderWindow(sf::VideoMode({ 1920, 1080 }), "Debugger");

            ImGui::SetCurrentContext(context);
            bool success = ImGui::SFML::Init(window, false);

            // Setup font for context
            ImGuiIO& io = ImGui::GetIO();
            io.Fonts->Clear();
            io.Fonts->AddFontFromFileTTF("courbd.ttf", 24);
            success = ImGui::SFML::UpdateFontTexture();

            // restore old context state
            ImGui::SetCurrentContext(old_context);

            setup_debugger_panels();

            return 0;
		}

		int update_debugger(const sf::Time& deltaTime)
		{
            ImGuiContext* old_context = ImGui::GetCurrentContext();

            ImGui::SetCurrentContext(context);

            // process debugger events
            while (const auto event = window.pollEvent())
            {
                if (event.has_value())
                {
                    ImGui::SFML::ProcessEvent(window, *event);
                    if (event->is<sf::Event::Closed>())
                    {
                        window.close();
                    }
                    if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>())
                    {
                        if (keyPressed->code == sf::Keyboard::Key::F1)
                        {
                            // close debugger
                        }
                    }
                }
            }

            // update the panels
            panel_tileset.update();
            panel_tilemap.update();
            panel_registers_palette.update();
            panel_disassembler.update();
            panel_memory.update();

            // imgui updates and drawing
            window.clear();
            ImGui::SFML::Update(window, deltaTime);

            ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar |
                ImGuiWindowFlags_NoResize |
                ImGuiWindowFlags_NoMove |
                ImGuiWindowFlags_NoCollapse |
                ImGuiWindowFlags_NoBackground;

            // Set window to fill entire display, starting at top-left
            ImGui::SetNextWindowPos(ImVec2(0, 0));
            ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);

            if (ImGui::Begin("Gameboy Debugger", nullptr, window_flags))
            {
                panel_tileset.draw();
                ImGui::SameLine(0.0f, 46.0f);
                panel_tilemap.draw();
                ImGui::SameLine(0.0f, 46.0f);
                panel_registers_palette.draw();

                ImGui::Spacing();
                panel_disassembler.draw();

                ImGui::Spacing();
                panel_memory.draw();

                ImGui::End();
            }
                                    
            // clear and display updated sfml window
            ImGui::SFML::Render(window);
            window.display();

            // restore old context state
            ImGui::SetCurrentContext(old_context);

            return 0;
		}

        int cleanup_debugger()
        {
            ImGuiContext* old_context = ImGui::GetCurrentContext();

            // have a cleanup method for each emulator that calls this
            window.close();

            ImGui::SetCurrentContext(context);
            ImGui::SFML::Shutdown(window);

            ImGui::DestroyContext(context);

            // restore old context state
            ImGui::SetCurrentContext(old_context);

            return 0;
        }
	}
}
