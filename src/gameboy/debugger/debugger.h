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
        typedef int (*DebuggerPanelInitFunction)();
        typedef int (*DebuggerPanelUpdateFunction)();
        typedef int (*DebuggerPanelDrawFunction)(bool);
        typedef int (*EDebuggerPanelProcessEventFunction)(const sf::Event*);

        struct DebuggerPanel
        {
            DebuggerPanelInitFunction init = nullptr;
            DebuggerPanelUpdateFunction update = nullptr;
            DebuggerPanelDrawFunction draw = nullptr;
            EDebuggerPanelProcessEventFunction process_event = nullptr;

            bool is_focused = false;
        };

        enum
        {
            DEBUGGER_PANEL_TILESET = 0,
            DEBUGGER_PANEL_TILEMAP,
            DEBUGGER_PANEL_REGISTERS_PALETTE,
            DEBUGGER_PANEL_DISASSEMBLER,
            DEBUGGER_PANEL_MEMORY,
            DEBUGGER_PANEL_COUNT,
        };

        ImGuiContext* context = nullptr;
        sf::RenderWindow window;

        std::vector<DebuggerPanel> panels;
        u8 panel_focus_index;

        int setup_debugger_panels()
        {
            panels.resize(DEBUGGER_PANEL_COUNT);

            // setup tileset panel;
            panels[DEBUGGER_PANEL_TILESET].init = tileset::init;
            panels[DEBUGGER_PANEL_TILESET].update = tileset::update;
            panels[DEBUGGER_PANEL_TILESET].draw = tileset::draw;
            panels[DEBUGGER_PANEL_TILESET].process_event = tileset::process_event;

            // setup tilemap panel;
            panels[DEBUGGER_PANEL_TILEMAP].init = tilemap::init;
            panels[DEBUGGER_PANEL_TILEMAP].update = tilemap::update;
            panels[DEBUGGER_PANEL_TILEMAP].draw = tilemap::draw;
            panels[DEBUGGER_PANEL_TILEMAP].process_event = tilemap::process_event;

            // setup registers and palette panel
            panels[DEBUGGER_PANEL_REGISTERS_PALETTE].init = registers_palette::init;
            panels[DEBUGGER_PANEL_REGISTERS_PALETTE].update = registers_palette::update;
            panels[DEBUGGER_PANEL_REGISTERS_PALETTE].draw = registers_palette::draw;
            panels[DEBUGGER_PANEL_REGISTERS_PALETTE].init();

            // setup disassembler panel
            panels[DEBUGGER_PANEL_DISASSEMBLER].init = disassembler::init;
            panels[DEBUGGER_PANEL_DISASSEMBLER].update = disassembler::update;
            panels[DEBUGGER_PANEL_DISASSEMBLER].draw = disassembler::draw;

            // setup memory panel
            panels[DEBUGGER_PANEL_MEMORY].init = memory::init;
            panels[DEBUGGER_PANEL_MEMORY].update = memory::update;
            panels[DEBUGGER_PANEL_MEMORY].draw = memory::draw;

            for (auto itr = panels.begin(); itr != panels.end(); itr++)
            {
                (*itr).init();
            }

            panel_focus_index = 0;
            panels[panel_focus_index].is_focused = true;

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
                        continue;
                    }
                    else if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>())
                    {
                        if (keyPressed->code == sf::Keyboard::Key::Tab)
                        {
                            panels[panel_focus_index].is_focused = false;
                            panel_focus_index++;

                            if (panel_focus_index >= DEBUGGER_PANEL_COUNT)
                            {
                                panel_focus_index = 0;
                            }

                            panels[panel_focus_index].is_focused = true;

                            continue;
                        }
                    }

                    // pass any other event to the panels
                    if (panels[panel_focus_index].process_event != nullptr)
                    {
                        panels[panel_focus_index].process_event(const_cast<sf::Event*>(&(*event)));
                    }
                }
            }

            // update the panels
            for (auto itr = panels.begin(); itr != panels.end(); itr++)
            {
                (*itr).update();
            }

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
                panels[DEBUGGER_PANEL_TILESET].draw(panels[DEBUGGER_PANEL_TILESET].is_focused);
                ImGui::SameLine(0.0f, 46.0f);
                panels[DEBUGGER_PANEL_TILEMAP].draw(panels[DEBUGGER_PANEL_TILEMAP].is_focused);
                ImGui::SameLine(0.0f, 46.0f);
                panels[DEBUGGER_PANEL_REGISTERS_PALETTE].draw(panels[DEBUGGER_PANEL_REGISTERS_PALETTE].is_focused);

                ImGui::Spacing();
                panels[DEBUGGER_PANEL_DISASSEMBLER].draw(panels[DEBUGGER_PANEL_DISASSEMBLER].is_focused);

                ImGui::Spacing();
                panels[DEBUGGER_PANEL_MEMORY].draw(panels[DEBUGGER_PANEL_MEMORY].is_focused);

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
