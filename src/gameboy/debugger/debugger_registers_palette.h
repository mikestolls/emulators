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
        namespace registers_palette
        {
            std::string flag_title[4] = { "C", "H", "N", "Z" };

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
				if (debugger_panel_begin("Registers & Palette & Flags", ImVec2(512, 256), is_focused, 0.9f))
				{
                    ImGui::SetWindowFontScale(0.85f);
                    ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(8, 8)); // Horizontal, Vertical padding

                    if (ImGui::BeginTable("Info", 2))
                    {
                        ImGui::TableSetupColumn("Column1", ImGuiTableColumnFlags_WidthFixed, 350.0f);
                        ImGui::TableSetupColumn("Column2", ImGuiTableColumnFlags_WidthStretch); // Takes remaining space

                        ImGui::TableNextRow();
                        ImGui::TableNextColumn();

                        // draw registers
                        ImGui::Text("R.af: 0x%04X", cpu::R.af);
                        ImGui::Text("R.bc: 0x%04X", cpu::R.bc);
                        ImGui::Text("R.de: 0x%04X", cpu::R.de);
                        ImGui::Text("R.hl: 0x%04X", cpu::R.hl);
                        ImGui::Text("R.sp: 0x%04X", cpu::R.sp);
                        ImGui::Text("R.pc: 0x%04X", cpu::R.pc);

                        ImGui::TableNextColumn();

                        // draw gpu info
                        ImGui::Text(" LCDC: 0x%02X", *gpu::lcd_control);
                        ImGui::Text(" LCDS: 0x%02X", (0x80 | *gpu::lcd_status));
                        ImGui::Text(" SCAN: 0x%02X", *gpu::scanline);
                        ImGui::Text("CSCAN: 0x%02X", *gpu::coincidence_scanline);
                        ImGui::Text("   IE: 0x%02X", *cpu::interrupt_enable_flag);
                        ImGui::Text("   IF: 0x%02X", *cpu::interrupt_request_flag);
                        ImGui::Text("  IME: %d", (cpu::interrupt_master ? 1 : 0));
                        ImGui::Text("  CNT: %d%s", gpu::horz_cycle_count, (gpu::get_lcd_control_flag(gpu::FLAG_LCD_DISPLAY_ENABLED) == 0 ? " - " : ""));

                        ImGui::TableNextRow();
                        ImGui::TableNextColumn();

                        // draw flags
                        ImGui::BeginDisabled();
                        for (u8 i = cpu::FLAG_CARRY; i <= cpu::FLAG_ZERO; i++)
                        {
                            bool flag_value = cpu::get_flag(i);
                            ImGui::Checkbox(flag_title[i - cpu::FLAG_CARRY].c_str(), &flag_value);
                            ImGui::SameLine(0.0f, 4.0f);
                        }
                        ImGui::EndDisabled();

                        ImGui::TableNextColumn();

                        // draw palette
                        float checkboxSize = ImGui::GetFrameHeight();
                        ImVec2 boxSize(checkboxSize, checkboxSize);

                        for (u8 i = 0; i < 4; i++)
                        {
                            u32 color = gpu::get_palette_color(i);

                            // Convert u32 color to ImVec4 (assuming color is RGBA format 0xRRGGBBAA)
                            float r = ((color >> 24) & 0xFF) / 255.0f;
                            float g = ((color >> 16) & 0xFF) / 255.0f;
                            float b = ((color >> 8) & 0xFF) / 255.0f;
                            float a = ((color >> 0) & 0xFF) / 255.0f;

                            // Draw colored box (non-interactive)
                            ImGui::ColorButton(("##palette" + std::to_string(i)).c_str(), ImVec4(r, g, b, a),
                                ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoBorder, boxSize);
                            ImGui::SameLine(0.0f, 8.0f);
                        }

                        ImGui::EndTable();
                        ImGui::PopStyleVar();
                        ImGui::SetWindowFontScale(1.0f);
                    }

                    debugger::debugger_panel_end();
				}

                return 0;
            }

            void on_keypressed(sf::Keyboard::Key key)
            {

            }
        }
	}
}
