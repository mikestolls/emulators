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
            #define MEM_LINE_COUNT				16

            #define ROW_COLOR_0                 0.12f
            #define ROW_COLOR_1                 0.22f

            s16 active_line;
            s16 active_column;
            u16 mem_start;

            s32 memory_breakpoint_last_addr;

            bool is_goto_prompt;
            bool is_mem_prompt;

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
                is_goto_prompt = false;
                is_mem_prompt = false;

                memory_breakpoint_last_addr = -1;

                return 0;
            }

            int update()
            {
                if (cpu::paused && cpu::memory_breakpoint_last_addr != memory_breakpoint_last_addr)
                {
                    memory_breakpoint_last_addr = cpu::memory_breakpoint_last_addr;
                    goto_memory_address(cpu::memory_breakpoint_last_addr);
                }
                else if (!cpu::paused)
                {
                    memory_breakpoint_last_addr = -1;
                }

                return 0;
            }

            int draw(bool is_focused)
            {
                if (debugger::debugger_panel_begin("Memory", ImVec2(1120, 335), is_focused, 0.7f))
                {
                    ImGui::SetWindowFontScale(0.9f);
                    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8, 0)); // 0 vertical spacing

                    // Draw goto/edit prompt if active
                    if (is_goto_prompt || is_mem_prompt)
                    {
                        // Center popup in the memory panel
                        ImVec2 childPos = ImGui::GetWindowPos();
                        ImVec2 childSize = ImGui::GetWindowSize();

                        ImVec2 popupSize(300, 100);
                        ImVec2 popupPos(
                            childPos.x + (childSize.x - popupSize.x) * 0.5f,
                            childPos.y + (childSize.y - popupSize.y) * 0.5f
                        );

                        ImGui::SetNextWindowPos(popupPos, ImGuiCond_Always);
                        ImGui::SetNextWindowSize(popupSize);

                        const char* title = is_goto_prompt ? "Go To Address" : "Enter Value";
                        bool* open_flag = is_goto_prompt ? &is_goto_prompt : &is_mem_prompt;

                        if (ImGui::Begin(title, open_flag, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse))
                        {
                            static char input_buf[5] = ""; // 4 hex digits + null

                            ImGui::Text(is_goto_prompt ? "Enter address:" : "Enter byte value:");
                            ImGui::Text("0x");
                            ImGui::SameLine();
                            ImGui::SetKeyboardFocusHere();

                            int max_chars = is_goto_prompt ? 4 : 2;

                            if (ImGui::InputText("##input", input_buf, max_chars + 1,
                                ImGuiInputTextFlags_CharsHexadecimal |
                                ImGuiInputTextFlags_CharsUppercase |
                                ImGuiInputTextFlags_EnterReturnsTrue))
                            {
                                if (is_goto_prompt)
                                {
                                    // Parse address and goto
                                    u16 addr = 0;
                                    if (sscanf(input_buf, "%hx", &addr) == 1)
                                    {
                                        goto_memory_address(addr);
                                    }
                                    is_goto_prompt = false;
                                }
                                else // is_mem_prompt
                                {
                                    // Parse value and write to memory
                                    u8 value = 0;
                                    if (sscanf(input_buf, "%hhx", &value) == 1)
                                    {
                                        u16 addr = mem_start + (active_line * MEM_PER_LINE) + active_column;
                                        memory_module::write_memory(addr, value, true);
                                    }
                                    is_mem_prompt = false;
                                }

                                input_buf[0] = '\0'; // Clear buffer
                            }

                            // Pre-fill with current value if editing memory
                            if (is_mem_prompt && input_buf[0] == '\0')
                            {
                                u16 addr = mem_start + (active_line * MEM_PER_LINE) + active_column;
                                u8 val = memory_module::read_memory(addr, true);
                                snprintf(input_buf, sizeof(input_buf), "%02X", val);
                            }

                            ImGui::End();
                        }
                    }

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
                            char displayStr[5] = { 0 }; // UTF-8 can be up to 4 bytes + null
                            
                            if (val < 0x20 || val == 0x7F) // Control characters
                            {
                                displayStr[0] = '.';
                            }
                            else if (val < 0x80) // Standard ASCII (0x20-0x7E)
                            {
                                displayStr[0] = (char)val;
                            }
                            else // Extended ASCII (0x80-0xFF) - convert to UTF-8
                            {
                                // Convert Windows-1252/Latin-1 to UTF-8
                                displayStr[0] = (char)(0xC0 | (val >> 6));
                                displayStr[1] = (char)(0x80 | (val & 0x3F));
                            }
                            
                            if (j > 0)
                            {
                                ImGui::SameLine(0, 0);
                            }
                            
                            // Highlight if this is the active column
                            if (i == active_line && j == active_column)
                            {
                                ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "%s", displayStr); // Red for active
                            }
                            else
                            {
                                ImGui::TextColored(ImVec4(0.5f, 1.0f, 0.5f, 1.0f), "%s", displayStr); // Light green
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
                if (is_goto_prompt || is_mem_prompt) // imgui handles prompt events
                {
                    return 0;
                }

                if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>())
                {
                    sf::Keyboard::Key key = keyPressed->code;

                    // handle up and down
                    if (key == sf::Keyboard::Key::Down)
                    {
                        active_line++;
                    }
                    else if (key == sf::Keyboard::Key::Up)
                    {
                        active_line--;
                    }
                    else if (key == sf::Keyboard::Key::Left)
                    {
                        active_column--;

                        if (active_column < 0)
                        {
                            active_column += MEM_PER_LINE;
                        }
                    }
                    else if (key == sf::Keyboard::Key::Right)
                    {
                        active_column++;

                        if (active_column >= MEM_PER_LINE)
                        {
                            active_column -= MEM_PER_LINE;
                        }
                    }
                    else if (key == sf::Keyboard::Key::G)
                    {
                        is_goto_prompt = true;
                    }
                    else if (key == sf::Keyboard::Key::Enter)
                    {
                        is_mem_prompt = true;
                    }
                    else if (key == sf::Keyboard::Key::F9) // handle debugging. same as the handlers in disassembly
                    {
                        u16 addr = mem_start + (active_line * MEM_PER_LINE) + active_column;
                        auto itr = std::find(cpu::memory_breakpoints.begin(), cpu::memory_breakpoints.end(), addr);

                        if (itr != cpu::memory_breakpoints.end())
                        {
                            cpu::memory_breakpoints.erase(itr);
                        }
                        else
                        {
                            cpu::memory_breakpoints.push_back(addr);
                        }
                    }
                    else if (key == sf::Keyboard::Key::F5)
                    {
                        // resume the cpu
                        cpu::paused = false;

                        if (cpu::memory_breakpoint_last_addr == 0x0) // not a memory breakpoint. yes i know. 0x0 is a valid addr, but no one needs to watch it
                        {
                            cpu::breakpoint_disable_one_instr = true;
                        }
                    }
                    else if (key == sf::Keyboard::Key::F10)
                    {
                        /*if (cpu::paused)
                        {
                            disassembler::symbol sym;
                            disassembler::disassemble_instr(cpu::R.pc, sym);

                            if (sym.mnemonic.compare("CALL") == 0)
                            {
                                cpu::soft_breakpoints.push_back(find_next_instr(cpu::R.pc));
                                cpu::paused = false;
                                cpu::breakpoint_disable_one_instr = true;
                            }
                            else
                            {
                                cpu::breakpoint_disable_one_instr = true;
                            }
                        }*/
                    }
                    else if (key == sf::Keyboard::Key::F11)
                    {
                        if (cpu::paused)
                        {
                            //cpu::paused = false;
                            cpu::breakpoint_disable_one_instr = true;
                        }
                    }

                    if (active_line > MEM_LINE_COUNT - 1)
                    {
                        active_line = MEM_LINE_COUNT - 1;
                        s32 temp = mem_start + MEM_PER_LINE;
                        u16 max_addr = 0xFFFF - (MEM_LINE_COUNT * MEM_PER_LINE) + 1;

                        if (temp > max_addr)
                        {
                            temp = max_addr;
                        }

                        mem_start = (u16)temp;
                    }
                    else if (active_line < 0)
                    {
                        active_line = 0;
                        s32 temp = mem_start - MEM_PER_LINE;

                        if (temp < 0x0)
                        {
                            temp = 0x0;
                        }

                        mem_start = (u16)temp;
                    }
                }

                return 0;
            }
        }
	}
}
