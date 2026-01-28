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

            int init()
            {
				active_line = 0;
				pc_start = 0;
				program_addr.push_back(0x0);

                return 0;
            }

            int update()
            {
                return 0;
            }

            int draw()
            {
                if (debugger::debugger_panel_begin("Disassembler", ImVec2(1120, 380)))
                {
					ImGui::SetWindowFontScale(1.0f);
                    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8, 0)); // 0 vertical spacing

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
						ImVec4 bgColor(row_color, row_color, row_color, 1.0f); // default alternate row color

						// Draw background for the line
						ImVec2 lineStart = ImGui::GetCursorScreenPos();
						ImVec2 lineEnd(lineStart.x + ImGui::GetContentRegionAvail().x, lineStart.y + ImGui::GetTextLineHeight());
						ImGui::GetWindowDrawList()->AddRectFilled(lineStart, lineEnd, ImGui::ColorConvertFloat4ToU32(bgColor));

                        // Address in cyan
                        ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "\t0x%04X", sym.addr);
                        ImGui::SameLine(0, 20);

                        // Opcode in yellow
                        ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "\t\t0x%02X", sym.opcode);

                        if (sym.opcode == 0xCB)
                        {
                            ImGui::SameLine(0, 5);
                            ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "0x%02X", sym.cb_opcode);
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

            void on_keypressed(sf::Keyboard::Key key)
            {

            }
        }
	}
}
