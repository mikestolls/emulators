#pragma once

#include "defines.h"
#include "debug_window.h"
#include "disassembler.h"

#include <SFML/Graphics.hpp>

namespace gameboy
{
	class debug_disassembler : public debug_window
	{
	public:
		#define WRITE_HEX_16(x) "0x" << std::setfill('0') << std::setw(4) << std::uppercase << std::hex << x
		#define WRITE_HEX_8(x) "0x" << std::setfill('0') << std::setw(2) << std::uppercase << std::hex << x

		#define LINE_HEIGHT		20
		#define LINE_XPOS		32
		#define LINE_COUNT		16

		sf::Text disassembler_text;

		sf::RectangleShape inner_border;
		sf::RectangleShape line_border;

		sf::CircleShape breakpoint_marker;

		s16 active_line;
		u16 active_addr;
		u16 pc_start;
		std::vector<u16> program_addr;

		debug_disassembler() : debug_window(916, 320)
		{
			disassembler_text.setString("");
			disassembler_text.setFont(font);
			disassembler_text.setCharacterSize(16);
			disassembler_text.setPosition(BORDER_SIZE, BORDER_SIZE + TITLEBAR_SIZE);

			inner_border.setSize(sf::Vector2f(916, 320));
			inner_border.setFillColor(sf::Color(0, 0, 0, 255));
			inner_border.setPosition(BORDER_SIZE, BORDER_SIZE + TITLEBAR_SIZE);

			line_border.setSize(sf::Vector2f(916, LINE_HEIGHT));
			line_border.setFillColor(sf::Color(100, 100, 100, 255));
			line_border.setPosition(BORDER_SIZE, BORDER_SIZE + TITLEBAR_SIZE);
			
			title_text.setString("Disassembler");

			bottom_text.setString("(Up / Down) Change Line");

			breakpoint_marker.setFillColor(sf::Color(255, 0, 0, 255));
			breakpoint_marker.setRadius(5);

			active_line = 0;
			pc_start = 0;
			program_addr.push_back(0x0);
		}
		
		u16 find_next_instr(u16 pc)
		{
			u16 next_pc = disassembler::disassemble_instr(pc); // this is the true next pc

			// try to find in our list
			auto itr = std::find(program_addr.begin(), program_addr.end(), pc);

			if (itr == program_addr.end())
			{
				// pc is not in list. add to end
				program_addr.push_back(pc);
			}
			else
			{
				// found pc in the list. check that the next itr matches the next_pc
				if (*(itr + 1) == next_pc)
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
						pc = disassembler::disassemble_instr(pc); // this is the true next pc
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
			 // try to find pc in our list
			auto itr = std::find(program_addr.begin(), program_addr.end(), pc);

			if (itr == program_addr.end())
			{
				// pc is not in list. start from begin and walk to pc
				u16 i = 0;
				while (i != pc)
				{
					i = disassembler::disassemble_instr(i); // this is the true next pc
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
				u16 next_pc = disassembler::disassemble_instr(prev_pc);

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
						next_pc = disassembler::disassemble_instr(next_pc); // this is the true next pc
						program_addr.push_back(next_pc);
						itr = program_addr.end() - 1;
					} while (next_pc != *(itr + 1));

					return prev_pc;
				}
			}

			assert(0);

			return 0;
		}

		void update()
		{
			// draw to the window texture. 
			window_texture.draw(outer_border);
			window_texture.draw(inner_border);
			window_texture.draw(title_text);
			
			line_border.setPosition(BORDER_SIZE, BORDER_SIZE + TITLEBAR_SIZE);
			disassembler_text.setPosition(BORDER_SIZE + LINE_XPOS, BORDER_SIZE + TITLEBAR_SIZE);
			breakpoint_marker.setPosition(BORDER_SIZE + 10, BORDER_SIZE + TITLEBAR_SIZE + 5);
			
			// draw foreground of each line
			u16 pc = pc_start;
			u8 color = 30;
			for (unsigned int i = 0; i < LINE_COUNT; i++)
			{
				disassembler::symbol sym;
				pc = disassembler::disassemble_instr(pc, sym);

				if (sym.addr > program_addr.back())
				{
					program_addr.push_back(sym.addr);
				}
				
				// draw addr
				std::stringstream stream;
				stream << WRITE_HEX_16(sym.addr);
				stream << "\t\t" << WRITE_HEX_16((int)sym.opcode);
				stream << "\t\t" << WRITE_HEX_16((int)sym.cb_opcode);
				stream << "\t\t" << std::setfill(' ') << std::setw(4) << sym.mnemonic;
				stream << "\t\t" << sym.operands;

				disassembler_text.setString(stream.str());
				
				// draw the background line
				if (i == active_line)
				{
					active_addr = sym.addr;
					line_border.setFillColor(sf::Color(150, 150, 150, 255));
				}

				// check if breakpoint is set.
				auto breakpoint_itr = std::find(cpu::breakpoints.begin(), cpu::breakpoints.end(), sym.addr);
				if (breakpoint_itr != cpu::breakpoints.end())
				{
					line_border.setFillColor(line_border.getFillColor() + sf::Color(50, 0, 0, 0));
				}

				window_texture.draw(line_border);

				// draw breakpoint marker
				if (breakpoint_itr != cpu::breakpoints.end())
				{
					window_texture.draw(breakpoint_marker);
				}

				// draw the line
				window_texture.draw(disassembler_text);

				// increase position
				sf::Vector2f pos = disassembler_text.getPosition();
				pos.y += LINE_HEIGHT;
				disassembler_text.setPosition(pos);

				pos = line_border.getPosition();
				pos.y += LINE_HEIGHT;
				line_border.setPosition(pos);

				pos = breakpoint_marker.getPosition();
				pos.y += LINE_HEIGHT;
				breakpoint_marker.setPosition(pos);

				line_border.setFillColor(sf::Color(color, color, color, 255));
				color = (color == 30 ? 50 : 30);
			}

			window_texture.display();
		}

		void on_keypressed(sf::Keyboard::Key key)
		{
			// handle up and down
			if (key == sf::Keyboard::Down)
			{
				active_line++;
			}
			else if (key == sf::Keyboard::Up)
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
			if (key == sf::Keyboard::F9)
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
		}
	};
}