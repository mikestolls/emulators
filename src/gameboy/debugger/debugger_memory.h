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
        namespace memory
        {
			#define LINE_COUNT					16
            #define MEM_PER_LINE				16

            #define ROW_COLOR_0                 0.12f
            #define ROW_COLOR_1                 0.22f

            s16 active_line;
            s16 active_column;
            u16 mem_start;

            void goto_memory_address(u16 address)
            {
                u16 addr = address;
                u16 max_addr = 0xFFFF - ((LINE_COUNT - 1) * MEM_PER_LINE);

                if (addr > max_addr)
                {
                    addr = max_addr;
                }

                mem_start = addr - (addr % MEM_PER_LINE);

                // select addr
                addr = address;
                addr -= mem_start;
                active_column = addr % MEM_PER_LINE;
                active_line = (addr - active_column) / MEM_PER_LINE;
            }

            int init()
            {
                active_line = 0;
                active_column = 0;
                mem_start = 0xFF00;

                return 0;
            }

            int update()
            {
                return 0;
            }

            int draw(bool is_focused)
            {
                if (debugger::debugger_panel_begin("Memory", ImVec2(1120, 335), is_focused, 0.7f))
                {
                    ImGui::SetWindowFontScale(0.9f);
                    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8, 0)); // 0 vertical spacing

                    // draw foreground of each line
                    float row_color = ROW_COLOR_0;
                    for (unsigned int i = 0; i < LINE_COUNT; i++)
                    {
                        u16 addr = mem_start + (i * MEM_PER_LINE);

                        // draw memory line
                        memory_module::memory_map_object* map = memory_module::find_map(addr);

                        // Determine background color
                        ImVec4 bgColor(row_color, row_color, row_color, 1.0f);

                        if (i == active_line)
                        {
                            bgColor = ImVec4(0.59f, 0.59f, 0.59f, 1.0f);
                        }

                        // Draw background for the line
                        ImVec2 lineStart = ImGui::GetCursorScreenPos();
                        ImVec2 lineEnd(lineStart.x + ImGui::GetContentRegionAvail().x, lineStart.y + ImGui::GetTextLineHeight());
                        ImGui::GetWindowDrawList()->AddRectFilled(lineStart, lineEnd, ImGui::ColorConvertFloat4ToU32(bgColor));

                        // Draw memory map name in cyan
                        if (map->map_name.compare("ROMS") == 0)
                        {
                            ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "ROM%d", mbc::mbc_get_rom_bank_idx());
                        }
                        else
                        {
                            ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "%s", map->map_name.c_str());
                        }
                        
                        ImGui::SameLine();
                        ImGui::SetCursorPosX(60);
                        
                        // Draw address in cyan
                        ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), " : 0x%04X\t", addr);
                        
                        ImGui::SameLine();

                        // Draw hex values in yellow
                        for (unsigned int j = 0; j < MEM_PER_LINE; j++)
                        {
                            if (j > 0)
                            {
                                ImGui::SameLine(0, 4);
                            }
                            
                            u8 val = memory_module::read_memory(addr + j, true);
                            
                            // Highlight if this is the active column
                            if (i == active_line && j == active_column)
                            {
                                ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "%02X ", val); // Red for active
                            }
                            else
                            {
                                ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "%02X ", val); // Yellow
                            }
                            
                            // Draw breakpoint marker if set
                            auto memory_breakpoint_itr = std::find(cpu::memory_breakpoints.begin(), 
                                                               cpu::memory_breakpoints.end(), 
                                                               addr + j);
                            if (memory_breakpoint_itr != cpu::memory_breakpoints.end())
                            {
                                ImVec2 markerPos = ImGui::GetCursorScreenPos();
                                markerPos.y -= ImGui::GetTextLineHeight();
                                ImGui::GetWindowDrawList()->AddCircleFilled(
                                    ImVec2(markerPos.x + 5, markerPos.y + ImGui::GetTextLineHeight() * 0.5f),
                                    3.0f,
                                    IM_COL32(255, 0, 0, 255)
                                );
                            }
                        }

                        ImGui::SameLine();
                        ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "\t");
                        ImGui::SameLine();

                        // Draw ASCII representation in light green
                        for (unsigned int j = 0; j < MEM_PER_LINE; j++)
                        {
                            u8 val = memory_module::read_memory(addr + j, true);
                            char displayChar;
                            
                            if (val < 0x20 || (val > 0x7E && val < 0xA0))
                            {
                                displayChar = '.';
                            }
                            else
                            {
                                displayChar = (char)val;
                            }
                            
                            if (j > 0)
                            {
                                ImGui::SameLine(0, 0);
                            }
                            
                            // Highlight if this is the active column
                            if (i == active_line && j == active_column)
                            {
                                ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "%c", displayChar); // Red for active
                            }
                            else
                            {
                                ImGui::TextColored(ImVec4(0.5f, 1.0f, 0.5f, 1.0f), "%c", displayChar); // Light green
                            }
                        }

                        row_color = (row_color == ROW_COLOR_0 ? ROW_COLOR_1 : ROW_COLOR_0);
                    }

                    ImGui::PopStyleVar();
                    ImGui::SetWindowFontScale(1.0f);

                    debugger::debugger_panel_end();
                }

                return 0;
            }

            int process_event(const sf::Event* event)
            {
                return 0;
            }
        }
	}
}
