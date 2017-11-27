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
		#define LINE_HEIGHT					20
		#define LINE_XPOS					32
		#define LINE_COUNT					16

		#define GOTO_PROMPT_X				256
		#define GOTO_PROMPT_Y				128
		#define GOTO_PROMPT_WIDTH			188
		#define GOTO_PROMPT_HEIGHT			48

		#define GOTO_PROMPT_INPUT_X			32
		#define GOTO_PROMPT_INPUT_Y			30
		#define GOTO_PROMPT_INPUT_WIDTH		124
		#define GOTO_PROMPT_INPUT_HEIGHT	24
		
		#define BREAKPOINT_PAUSE_TRIANGLE_SIZE		10

		sf::Text disassembler_text;

		sf::RectangleShape inner_border;
		sf::RectangleShape line_border;

		sf::CircleShape breakpoint_marker;
		sf::ConvexShape breakpoint_paused_marker;

		sf::RectangleShape goto_outer_border;
		sf::RectangleShape goto_inner_border;
		sf::RectangleShape goto_outer_input_border;
		sf::RectangleShape goto_inner_input_border;
		sf::Text goto_title_text;
		sf::Text goto_input_text;
		std::stringstream goto_input_stream;

		s16 active_line;
		u16 active_addr;
		u16 pc_start;
		std::vector<u16> program_addr;

		bool is_goto_prompt;

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

			bottom_text.setString("(Up / Down) Change Line\t(G) Goto Address\tF5 Continue\tF9 Set Breakpoint\tF10 Step Once");

			breakpoint_marker.setFillColor(sf::Color(255, 0, 0, 255));
			breakpoint_marker.setRadius(5);

			breakpoint_paused_marker.setPointCount(3);
			breakpoint_paused_marker.setPoint(0, sf::Vector2f(0, -BREAKPOINT_PAUSE_TRIANGLE_SIZE));
			breakpoint_paused_marker.setPoint(1, sf::Vector2f(0, BREAKPOINT_PAUSE_TRIANGLE_SIZE));
			breakpoint_paused_marker.setPoint(2, sf::Vector2f(BREAKPOINT_PAUSE_TRIANGLE_SIZE, 0));
			breakpoint_paused_marker.setFillColor(sf::Color(0, 255, 0, 255));

			// setup the goto prompt visual
			goto_outer_border.setSize(sf::Vector2f(GOTO_PROMPT_WIDTH + BORDER_SIZE * 2, GOTO_PROMPT_HEIGHT + TITLEBAR_SIZE + BORDER_SIZE * 2));
			goto_outer_border.setPosition(sf::Vector2f(GOTO_PROMPT_X, GOTO_PROMPT_Y));
			goto_outer_border.setFillColor(sf::Color(200, 200, 200, 255));

			goto_inner_border.setSize(sf::Vector2f(GOTO_PROMPT_WIDTH, GOTO_PROMPT_HEIGHT));
			goto_inner_border.setPosition(sf::Vector2f(GOTO_PROMPT_X + BORDER_SIZE, GOTO_PROMPT_Y + BORDER_SIZE + TITLEBAR_SIZE));
			goto_inner_border.setFillColor(sf::Color(0, 0, 0, 255));

			goto_outer_input_border.setSize(sf::Vector2f(GOTO_PROMPT_INPUT_WIDTH + BORDER_SIZE * 2, GOTO_PROMPT_INPUT_HEIGHT + BORDER_SIZE * 2));
			goto_outer_input_border.setPosition(sf::Vector2f(GOTO_PROMPT_X + GOTO_PROMPT_INPUT_X, GOTO_PROMPT_Y + GOTO_PROMPT_INPUT_Y));
			goto_outer_input_border.setFillColor(sf::Color(200, 200, 200, 255));

			goto_inner_input_border.setSize(sf::Vector2f(GOTO_PROMPT_INPUT_WIDTH, GOTO_PROMPT_INPUT_HEIGHT));
			goto_inner_input_border.setPosition(sf::Vector2f(GOTO_PROMPT_X + GOTO_PROMPT_INPUT_X + BORDER_SIZE, GOTO_PROMPT_Y + GOTO_PROMPT_INPUT_Y + BORDER_SIZE));
			goto_inner_input_border.setFillColor(sf::Color(0, 0, 0, 255));

			goto_title_text.setString("Goto Address");
			goto_title_text.setFillColor(sf::Color(0, 0, 0, 255));
			goto_title_text.setFont(font);
			goto_title_text.setCharacterSize(16);
			goto_title_text.setPosition(sf::Vector2f(GOTO_PROMPT_X + BORDER_SIZE, GOTO_PROMPT_Y));

			goto_input_text.setString("");
			goto_input_text.setFont(font);
			goto_input_text.setCharacterSize(16);
			goto_input_text.setPosition(sf::Vector2f(GOTO_PROMPT_X + GOTO_PROMPT_INPUT_X + BORDER_SIZE * 2, GOTO_PROMPT_Y + GOTO_PROMPT_INPUT_Y + BORDER_SIZE));

			goto_input_stream.clear();

			active_line = 0;
			pc_start = 0;
			program_addr.push_back(0x0);

			is_goto_prompt = false;
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
						prev_pc = next_pc;
						next_pc = disassembler::disassemble_instr(next_pc); // this is the true next pc
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

		void update_goto_prompt()
		{
			window_texture.draw(goto_outer_border);
			window_texture.draw(goto_inner_border);
			window_texture.draw(goto_title_text);
			window_texture.draw(goto_outer_input_border);
			window_texture.draw(goto_inner_input_border);

			// update the input text
			std::stringstream input_stream;
			input_stream <<"0x";
			input_stream << goto_input_stream.str();
			goto_input_text.setString(input_stream.str());

			window_texture.draw(goto_input_text);
		}

		void update()
		{
			// check if cpu hit a breakpoint
			if (cpu::breakpoint_hit)
			{
				pc_start = cpu::R.pc;
				active_line = 0;
				cpu::breakpoint_hit = false;
			}

			// draw to the window texture. 
			window_texture.draw(outer_border);
			window_texture.draw(inner_border);
			window_texture.draw(title_text);
			
			line_border.setPosition(BORDER_SIZE, BORDER_SIZE + TITLEBAR_SIZE);
			disassembler_text.setPosition(BORDER_SIZE + LINE_XPOS, BORDER_SIZE + TITLEBAR_SIZE);
			breakpoint_marker.setPosition(BORDER_SIZE + 10, BORDER_SIZE + TITLEBAR_SIZE + 5);
			breakpoint_paused_marker.setPosition(BORDER_SIZE + 5, BORDER_SIZE + TITLEBAR_SIZE + 10);

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
				stream << "\t\t" << WRITE_HEX_8((int)sym.opcode);

				if (sym.opcode == 0xCB)
				{
					stream << " " << WRITE_HEX_8((int)sym.cb_opcode);
				}

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

				// if cpu is paused. draw where its paused
				if (cpu::paused && sym.addr == cpu::R.pc)
				{
					window_texture.draw(breakpoint_paused_marker);
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

				pos = breakpoint_paused_marker.getPosition();
				pos.y += LINE_HEIGHT;
				breakpoint_paused_marker.setPosition(pos);

				line_border.setFillColor(sf::Color(color, color, color, 255));
				color = (color == 30 ? 50 : 30);
			}

			if (is_goto_prompt)
			{
				update_goto_prompt();
			}

			window_texture.display();
		}

		void on_keypressed_goto(sf::Keyboard::Key key)
		{
			if (key >= sf::Keyboard::Num0 && key <= sf::Keyboard::Num9)
			{
				// add the number to the input stream
				goto_input_stream << (int)(key - sf::Keyboard::Num0);
			}
			else if (key >= sf::Keyboard::Numpad0 && key <= sf::Keyboard::Numpad9)
			{
				// add the number to the input stream
				goto_input_stream << (int)(key - sf::Keyboard::Numpad0);
			}
			else if (key >= sf::Keyboard::A && key <= sf::Keyboard::F)
			{
				goto_input_stream << (char)(0x41 + (int)(key - sf::Keyboard::A));
			}
			else if (key == sf::Keyboard::BackSpace)
			{
				if (goto_input_stream.str().length() > 0)
				{
					std::string temp = goto_input_stream.str().erase(goto_input_stream.str().length() - 1);
					goto_input_stream.str("");
					goto_input_stream << temp;
				}
			}
			else if (key == sf::Keyboard::Return)
			{
				is_goto_prompt = false;

				if (goto_input_stream.str().length() == 0)
				{
					return;
				}

				//get the instruction int
				u16 addr = (u16)std::stoul(goto_input_stream.str(), nullptr, 16);

				goto_instr(addr);
			}
			else if (key == sf::Keyboard::Escape)
			{
				is_goto_prompt = false;
			}

			// cap size
			if (goto_input_stream.tellp() > 4)
			{
				std::string temp = goto_input_stream.str().erase(4);
				goto_input_stream.str("");
				goto_input_stream << temp;
			}
		}

		void on_keypressed(sf::Keyboard::Key key)
		{
			if (is_goto_prompt)
			{
				on_keypressed_goto(key);
				return;
			}

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
			else if (key == sf::Keyboard::F5)
			{
				// resume the cpu
				cpu::paused = false;
				cpu::breakpoint_disable_one_instr = true;
			}
			else if (key == sf::Keyboard::F10)
			{
				if (cpu::paused)
				{
					//cpu::paused = false;
					cpu::breakpoint_disable_one_instr = true;
				}
			}
			else if (key == sf::Keyboard::G)
			{
				is_goto_prompt = true;
				goto_input_stream.str("");
			}
		}
	};
}