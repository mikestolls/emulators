#pragma once

#include "defines.h"

#include <SFML/Graphics.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>
#include <imgui.h>
#include <imgui-SFML.h>

#include "debugger_helper.h"
#include "../disassembler.h"

namespace gameboy
{
	namespace debugger
	{
        namespace disassembler
        {
			#define LINE_COUNT					16
            #define ROW_COLOR_0                 0.12f
            #define ROW_COLOR_1                 0.22f

			s16 active_line;
			u16 active_addr;
			u16 pc_start;
			std::vector<u16> program_addr;
			bool is_goto_prompt;
			bool is_cpu_paused;
			bool is_skip_update;

			u16 find_next_instr(u16 pc)
			{
				u16 next_pc = gameboy::disassembler::disassemble_instr(pc); // this is the true next pc

				// try to find in our list
				auto itr = std::find(program_addr.begin(), program_addr.end(), pc);

				if (itr == program_addr.end())
				{
					// pc is not in list. add to end
					program_addr.push_back(pc);
					return next_pc;
				}
				else
				{
					// found pc in the list. check that the next itr matches the next_pc
					if (itr == program_addr.end() - 1)
					{
						program_addr.push_back(next_pc);
						return next_pc;
					}
					else if (*(itr + 1) == next_pc)
					{
						// we are good
						return next_pc;
					}
					else
					{
						// clear everything beyond pc
						program_addr.erase(itr + 1, program_addr.end());

						// there is a gap. fill it
						while (pc != next_pc)
						{
							pc = gameboy::disassembler::disassemble_instr(pc); // this is the true next pc
							itr = program_addr.insert(itr, pc);
						}

						return next_pc;
					}
				}

				assert(0);

				return 0;
			}

			u16 find_prev_instr(u16 pc)
			{
				if (pc == 0x0)
				{
					return 0x0;
				}

				// try to find pc in our list
				auto itr = std::find(program_addr.begin(), program_addr.end(), pc);

				if (itr == program_addr.end() || itr == program_addr.begin())
				{
					program_addr.clear();

					// pc is not in list. start from begin and walk to pc
					u16 i = 0;
					while (i != pc)
					{
						i = gameboy::disassembler::disassemble_instr(i); // this is the true next pc
						program_addr.push_back(i);
					}

					// return the previous
					itr = std::find(program_addr.begin(), program_addr.end(), pc); // optmize this

					return *(itr - 1);
				}
				else if (*itr == 0x0) // start of program
				{
					return 0x0;
				}
				else
				{
					// found pc in the list. start from the prev entry, walk up to pc to fill gap
					u16 prev_pc = *(itr - 1);
					u16 next_pc = gameboy::disassembler::disassemble_instr(prev_pc);

					if (next_pc == *(itr)) // no gap in list
					{
						return prev_pc;
					}
					else
					{
						// clear everything beyond itr
						program_addr.erase(itr, program_addr.end());

						next_pc = prev_pc;
						do
						{
							prev_pc = next_pc;
							next_pc = gameboy::disassembler::disassemble_instr(next_pc); // this is the true next pc
							program_addr.push_back(next_pc);
						} while (next_pc != pc);

						return prev_pc;
					}
				}

				assert(0);

				return 0;
			}

			void goto_instr(u16 addr)
			{
				u16 target_addr = addr;
				addr = 0x0;
				warning("optimze this");
				while (addr < target_addr)
				{
					addr = find_next_instr(addr);
				}

				pc_start = addr;
				active_line = 0;
			}

            int init()
            {
				active_line = 0;
				pc_start = 0;
				program_addr.push_back(0x0);
				is_goto_prompt = false;
				is_cpu_paused = false;
				is_skip_update = false;

				cpu::breakpoints.push_back(0x100);

                return 0;
            }

            int update()
            {
				if (is_skip_update) // mostly used for step in and over to allow cpu to run first to update PC
				{
					is_skip_update = false;
					return 0;
				}

				if (!cpu::is_opcode_complete)
				{
					return 0;
				}

				if (cpu::paused && !is_cpu_paused)
				{
					// CPU just paused (breakpoint hit) - snap to current PC
					goto_instr(cpu::R.pc);
					is_cpu_paused = true;
				}

				is_cpu_paused = cpu::paused;

                return 0;
            }

            int draw(bool is_focused)
            {
                if (debugger::debugger_panel_begin("Disassembler", ImVec2(1120, 335), is_focused, 0.7f))
                {
                    ImGui::SetWindowFontScale(0.9f);
                    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8, 0));

                    // Draw goto prompt if active
                    if (is_goto_prompt)
                    {
						// Center popup in the disassembler panel
						ImVec2 childPos = ImGui::GetWindowPos();
						ImVec2 childSize = ImGui::GetWindowSize();

						ImVec2 popupSize(300, 100);
						ImVec2 popupPos(
							childPos.x + (childSize.x - popupSize.x) * 0.5f,
							childPos.y + (childSize.y - popupSize.y) * 0.5f
						);

						ImGui::SetNextWindowPos(popupPos, ImGuiCond_Always);
						ImGui::SetNextWindowSize(popupSize);

                        if (ImGui::Begin("Go To Address", &is_goto_prompt, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse))
                        {
                            static char address_buf[5] = ""; // 4 hex digits + null

							ImGui::Text("Enter address:");
							ImGui::Text("0x");
							ImGui::SameLine();
                            ImGui::SetKeyboardFocusHere();
                            
                            if (ImGui::InputText("##address", address_buf, sizeof(address_buf), ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_EnterReturnsTrue))
                            {
                                // Parse hex address
                                u16 addr = 0;
                                if (sscanf(address_buf, "%hx", &addr) == 1)
                                {
                                    goto_instr(addr);
                                }
                                
                                is_goto_prompt = false;
                                address_buf[0] = '\0'; // Clear buffer
                            }
                                                        
                            ImGui::End();
                        }
                    }

                    // draw foreground of each line
                    u16 pc = pc_start;
                    float row_color = ROW_COLOR_0;
                    
                    for (unsigned int i = 0; i < LINE_COUNT; i++)
                    {
                        gameboy::disassembler::symbol sym;
                        pc = gameboy::disassembler::disassemble_instr(pc, sym);

                        if (sym.addr > program_addr.back())
                        {
                            program_addr.push_back(sym.addr);
                        }

                        // Determine background color
                        ImVec4 bgColor(row_color, row_color, row_color, 1.0f);

                        if (i == active_line)
                        {
                            active_addr = sym.addr;
                            bgColor = ImVec4(0.59f, 0.59f, 0.59f, 1.0f);
                        }
                        
                        // Add red tint if breakpoint is set
                        auto breakpoint_itr = std::find(cpu::breakpoints.begin(), cpu::breakpoints.end(), sym.addr);
                        if (breakpoint_itr != cpu::breakpoints.end())
                        {
                            bgColor.x += 0.2f;
                        }

                        // Draw background for the line
                        ImVec2 lineStart = ImGui::GetCursorScreenPos();
                        ImVec2 lineEnd(lineStart.x + ImGui::GetContentRegionAvail().x, lineStart.y + ImGui::GetTextLineHeight());
                        ImGui::GetWindowDrawList()->AddRectFilled(lineStart, lineEnd, ImGui::ColorConvertFloat4ToU32(bgColor));

                        // Draw breakpoint marker
                        if (breakpoint_itr != cpu::breakpoints.end())
                        {
                            ImGui::GetWindowDrawList()->AddCircleFilled(
                                ImVec2(lineStart.x + 8, lineStart.y + ImGui::GetTextLineHeight() * 0.5f),
                                5.0f,
                                IM_COL32(255, 0, 0, 255)
                            );
                        }
                        
                        // Draw pause marker
                        if (cpu::paused && sym.addr == cpu::R.pc)
                        {
                            ImVec2 p1(lineStart.x + 18, lineStart.y + 2);
                            ImVec2 p2(lineStart.x + 18, lineStart.y + ImGui::GetTextLineHeight() - 2);
                            ImVec2 p3(lineStart.x + 28, lineStart.y + ImGui::GetTextLineHeight() * 0.5f);
                            
                            ImGui::GetWindowDrawList()->AddTriangleFilled(p1, p2, p3, IM_COL32(0, 255, 0, 255));
                        }

                        // Offset text to make room for markers
                        ImGui::SetCursorScreenPos(ImVec2(lineStart.x + 32, lineStart.y));

                        // Address in cyan
                        ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "\t0x%04X", sym.addr);
                        ImGui::SameLine(0, 20);

                        // Opcode in yellow
                        if (sym.opcode == 0xCB)
                        {
                            ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "\t\t0x%02X 0x%02X", sym.opcode, sym.cb_opcode);
                        }
                        else
                        {
                            ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "\t\t0x%02X", sym.opcode);
                        }

                        ImGui::SameLine(0, 20);

                        // Mnemonic in white
                        ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "\t\t%-4s", sym.mnemonic.c_str());

                        ImGui::SameLine(0, 20);

                        // Operands in light green
                        ImGui::TextColored(ImVec4(0.5f, 1.0f, 0.5f, 1.0f), "\t\t%s", sym.operands.c_str());

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
				if (is_goto_prompt) // prompt events handled by imgui
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

					if (active_line > LINE_COUNT - 1)
					{
						active_line = LINE_COUNT - 1;
						pc_start = find_next_instr(pc_start);
					}
					else if (active_line < 0)
					{
						active_line = 0;
						pc_start = find_prev_instr(pc_start);
					}

					// handle breakpoint
					if (key == sf::Keyboard::Key::F9)
					{
						auto itr = std::find(cpu::breakpoints.begin(), cpu::breakpoints.end(), active_addr);

						if (itr != cpu::breakpoints.end())
						{
							cpu::breakpoints.erase(itr);
						}
						else
						{
							cpu::breakpoints.push_back(active_addr);
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
						if (cpu::paused)
						{
							gameboy::disassembler::symbol sym;
							gameboy::disassembler::disassemble_instr(cpu::R.pc, sym);

							u16 next_pc = find_next_instr(cpu::R.pc);

							// handle stepping over a call
							if (sym.mnemonic.compare("CALL") == 0)
							{
								cpu::soft_breakpoints.push_back(next_pc);
								cpu::paused = false;
							}

							cpu::breakpoint_disable_one_instr = true;
							is_cpu_paused = false;
							is_skip_update = true;
						}
					}
					else if (key == sf::Keyboard::Key::F11)
					{
						if (cpu::paused)
						{
							cpu::breakpoint_disable_one_instr = true;
							is_cpu_paused = false;
							is_skip_update = true;
						}
					}
					else if (key == sf::Keyboard::Key::G)
					{
						is_goto_prompt = true;
					}
				}

				return 0;
			}
        }
	}
}
