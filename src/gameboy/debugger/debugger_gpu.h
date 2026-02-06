#pragma once

#include "defines.h"

#include <SFML/Graphics.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>
#include <imgui.h>
#include <imgui-SFML.h>

#include "debugger_helper.h"

namespace gameboy
{
    namespace debugger
    {
        namespace gpu
        {
            int init()
            {
                return 0;
            }

            int update()
            {
                return 0;
            }

            int draw(bool is_focused)
            {
                if (debugger_panel_begin("GPU", ImVec2(500, 800), is_focused, 0.7f))
                {
                    ImGui::SetWindowFontScale(0.85f);
                    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 8));
                    
                    // LCD Control Register
                    u8 lcdc = *gameboy::gpu::lcd_control;
                    
                    ImGui::Text("LCD Control (0xFF40): 0x%02X", lcdc);
                    ImGui::Separator();
                    
                    ImGui::BeginDisabled();
                    
                    bool bit7 = (lcdc & 0x80) != 0;
                    ImGui::Checkbox("Bit 7: LCD Display Enable", &bit7);
                    
                    bool bit6 = (lcdc & 0x40) != 0;
                    ImGui::Checkbox("Bit 6: Window Tilemap (9C00=1, 9800=0)", &bit6);
                    
                    bool bit5 = (lcdc & 0x20) != 0;
                    ImGui::Checkbox("Bit 5: Window Display Enable", &bit5);
                    
                    bool bit4 = (lcdc & 0x10) != 0;
                    ImGui::Checkbox("Bit 4: BG/Win Tileset (8000=1, 8800=0)", &bit4);
                    
                    bool bit3 = (lcdc & 0x08) != 0;
                    ImGui::Checkbox("Bit 3: BG Tilemap (9C00=1, 9800=0)", &bit3);
                    
                    bool bit2 = (lcdc & 0x04) != 0;
                    ImGui::Checkbox("Bit 2: OBJ Size (8x16=1, 8x8=0)", &bit2);
                    
                    bool bit1 = (lcdc & 0x02) != 0;
                    ImGui::Checkbox("Bit 1: OBJ Display Enable", &bit1);
                    
                    bool bit0 = (lcdc & 0x01) != 0;
                    ImGui::Checkbox("Bit 0: BG/Window Enable/Priority", &bit0);
                    
                    ImGui::EndDisabled();
                    
                    ImGui::Spacing();
                    ImGui::Separator();
                    ImGui::Spacing();
                    
                    // LCD Status Register
                    u8 lcds = *gameboy::gpu::lcd_status;
                    ImGui::Text("LCD Status (0xFF41): 0x%02X", lcds);
                    ImGui::Indent();
                    ImGui::Text("Mode: %d (%s)", lcds & 0x3, 
                        (lcds & 0x3) == 0 ? "HBlank" :
                        (lcds & 0x3) == 1 ? "VBlank" :
                        (lcds & 0x3) == 2 ? "OAM Search" : "VRAM Transfer");
                    ImGui::BeginDisabled();
                    bool coincidence = (lcds & 0x04) != 0;
                    ImGui::Checkbox("Coincidence Flag", &coincidence);
                    ImGui::EndDisabled();
                    ImGui::Unindent();
                    
                    ImGui::Spacing();
                    ImGui::Separator();
                    ImGui::Spacing();
                    
                    // Scroll Position
                    ImGui::Text("Scroll Position");
                    ImGui::Indent();
                    ImGui::Text("Scroll X (0xFF43): 0x%02X (%d)", *gameboy::gpu::scroll_x, *gameboy::gpu::scroll_x);
                    ImGui::Text("Scroll Y (0xFF42): 0x%02X (%d)", *gameboy::gpu::scroll_y, *gameboy::gpu::scroll_y);
                    ImGui::Unindent();
                    
                    ImGui::Spacing();
                    ImGui::Separator();
                    ImGui::Spacing();
                    
                    // Window Position
                    ImGui::Text("Window Position");
                    ImGui::Indent();
                    ImGui::Text("Window X (0xFF4B): 0x%02X (%d) [Screen: %d]", 
                        *gameboy::gpu::window_x, *gameboy::gpu::window_x, 
                        *gameboy::gpu::window_x - 7);
                    ImGui::Text("Window Y (0xFF4A): 0x%02X (%d)", *gameboy::gpu::window_y, *gameboy::gpu::window_y);
                    ImGui::Unindent();
                    
                    ImGui::Spacing();
                    ImGui::Separator();
                    ImGui::Spacing();
                    
                    // Scanline Info
                    ImGui::Text("Scanline (0xFF44): 0x%02X (%d)", *gameboy::gpu::scanline, *gameboy::gpu::scanline);
                    ImGui::Text("LY Compare (0xFF45): 0x%02X (%d)", *gameboy::gpu::coincidence_scanline, *gameboy::gpu::coincidence_scanline);
                    ImGui::Checkbox("LCD Enabled", &gameboy::gpu::lcd_enabled);
                    ImGui::Checkbox("LCD Enabling", &gameboy::gpu::lcd_enabling);
                    
                    ImGui::PopStyleVar();
                    ImGui::SetWindowFontScale(1.0f);
                    debugger::debugger_panel_end();
                }

                return 0;
            }

            int process_event(const sf::Event * event)
            {
                return 0;
            }
        }
    }
}
