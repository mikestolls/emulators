#pragma once

#include "defines.h"
#include "debug_window.h"
#include "disassembler.h"

#include <SFML/Graphics.hpp>

namespace gameboy
{
	class debug_memory : public debug_window
	{
	public:
		#define MEM_LINE_HEIGHT					20
		#define MEM_LINE_XPOS					16
		#define MEM_LINE_COUNT					16
		#define MEM_LINE_COLUMN_XPOS			(BORDER_SIZE + MEM_LINE_XPOS + 167)
		#define MEM_LINE_COLUMN_WIDTH			26
		#define MEM_LINE_COLUMN_GAP				30

		#define MEM_BREAKPOINT_MARKER_OFFSET_X	1
		#define MEM_BREAKPOINT_MARKER_OFFSET_Y	1

		#define GOTO_PROMPT_X				256
		#define GOTO_PROMPT_Y				128
		#define GOTO_PROMPT_WIDTH			188
		#define GOTO_PROMPT_HEIGHT			48

		#define GOTO_PROMPT_INPUT_X			32
		#define GOTO_PROMPT_INPUT_Y			30
		#define GOTO_PROMPT_INPUT_WIDTH		124
		#define GOTO_PROMPT_INPUT_HEIGHT	24

		#define MEM_PER_LINE				16

		sf::Text memory_text;
		
		sf::RectangleShape inner_border;
		sf::RectangleShape line_border;
		sf::RectangleShape active_border;
		sf::CircleShape breakpoint_marker;
		
		sf::RectangleShape goto_outer_border;
		sf::RectangleShape goto_inner_border;
		sf::RectangleShape goto_outer_input_border;
		sf::RectangleShape goto_inner_input_border;
		sf::Text goto_title_text;
		sf::Text goto_input_text;
		std::stringstream goto_input_stream;

		s16 active_line;
		s16 active_column;
		u16 active_addr;
		u16 mem_start;

		s32 memory_breakpoint_last_addr;

		bool is_goto_prompt;
		bool is_mem_prompt;

		debug_memory() : debug_window(916, 320)
		{
			memory_text.setString("");
			memory_text.setFont(font);
			memory_text.setCharacterSize(16);
			memory_text.setPosition(BORDER_SIZE, BORDER_SIZE + TITLEBAR_SIZE);

			inner_border.setSize(sf::Vector2f(916, 320));
			inner_border.setFillColor(sf::Color(0, 0, 0, 255));
			inner_border.setPosition(BORDER_SIZE, BORDER_SIZE + TITLEBAR_SIZE);

			line_border.setSize(sf::Vector2f(916, LINE_HEIGHT));
			line_border.setFillColor(sf::Color(100, 100, 100, 255));
			line_border.setPosition(BORDER_SIZE, BORDER_SIZE + TITLEBAR_SIZE);
			
			active_border.setSize(sf::Vector2f(MEM_LINE_COLUMN_WIDTH, LINE_HEIGHT));
			active_border.setFillColor(sf::Color(200, 200, 200, 200));
			active_border.setPosition(BORDER_SIZE, BORDER_SIZE + TITLEBAR_SIZE);

			breakpoint_marker.setFillColor(sf::Color(255, 0, 0, 255));
			breakpoint_marker.setRadius(5);

			title_text.setString("Memory Viewer");

			bottom_text.setString("(Up / Down / Left / Right) Select Address\t(G) Goto Address\t(Enter) Modify Value");
			
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
			active_column = 0;
			mem_start = 0xFF00;

			memory_breakpoint_last_addr = -1;

			is_goto_prompt = false;
			is_mem_prompt = false;
		}

		void goto_memory_address(u16 address)
		{
			u16 addr = address;
			u16 max_addr = 0xFFFF - ((MEM_LINE_COUNT - 1) * MEM_PER_LINE);

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
			if (cpu::paused && cpu::memory_breakpoint_last_addr != memory_breakpoint_last_addr)
			{
				memory_breakpoint_last_addr = cpu::memory_breakpoint_last_addr;
				goto_memory_address(cpu::memory_breakpoint_last_addr);
			}
			else if (!cpu::paused)
			{
				memory_breakpoint_last_addr = -1;
			}

			// draw to the window texture. 
			window_texture.draw(outer_border);
			window_texture.draw(inner_border);
			window_texture.draw(title_text);
			
			line_border.setPosition(BORDER_SIZE, BORDER_SIZE + TITLEBAR_SIZE);
			memory_text.setPosition(BORDER_SIZE + MEM_LINE_XPOS, BORDER_SIZE + TITLEBAR_SIZE);
			
			// draw foreground of each line
			u8 color = 30;
			for (unsigned int i = 0; i < LINE_COUNT; i++)
			{
				// draw the background line
				if (i == active_line)
				{
					// draw active border
					sf::Vector2f pos = memory_text.getPosition();
					pos.x = (float)(MEM_LINE_COLUMN_XPOS + (active_column * (MEM_LINE_COLUMN_GAP)));
					active_border.setPosition(pos);

					line_border.setFillColor(sf::Color(150, 150, 150, 255));
					window_texture.draw(line_border);
					window_texture.draw(active_border);
				}
				else
				{
					window_texture.draw(line_border);
				}

				u16 addr = mem_start + (i * MEM_PER_LINE);

				// draw memory line
				memory_module::memory_map_object* map = memory_module::find_map(addr);

				std::stringstream stream;
				if (map->map_name.compare("ROMS") == 0)
				{
					stream << "ROM" << mbc::mbc_get_rom_bank_idx();
				}
				else
				{
					stream << map->map_name;
				}
				
				stream << " : " << WRITE_HEX_16(addr) << "\t";
				
				for (unsigned int j = 0; j < MEM_PER_LINE; j++)
				{
					u32 val = (u32)memory_module::read_memory(addr + j, true);
					stream << std::setfill('0') << std::setw(2) << std::uppercase << std::hex << val << " ";
				}

				stream << "\t\t";
				
				for (unsigned int j = 0; j < MEM_PER_LINE; j++)
				{
					u32 val = (u32)memory_module::read_memory(addr + j, true);
					if (val < 0x20 || (val > 0x7E && val < 0xA0))
					{
						stream << ".";
					}
					else
					{
						stream << (char)val;
					}

					// check if memory breakpoint is set
					auto memory_breakpoint_itr = std::find(cpu::memory_breakpoints.begin(), cpu::memory_breakpoints.end(), addr + j);
					if (memory_breakpoint_itr != cpu::memory_breakpoints.end())
					{
						sf::Vector2f pos = memory_text.getPosition();
						pos.x = (float)(MEM_LINE_COLUMN_XPOS + (j * (MEM_LINE_COLUMN_GAP))) + MEM_BREAKPOINT_MARKER_OFFSET_X;
						pos.y += MEM_BREAKPOINT_MARKER_OFFSET_Y;

						breakpoint_marker.setPosition(pos);
						window_texture.draw(breakpoint_marker);
					}
					
				}

				memory_text.setString(stream.str());

				// draw the line
				window_texture.draw(memory_text);

				// increase position
				sf::Vector2f pos = memory_text.getPosition();
				pos.y += MEM_LINE_HEIGHT;
				memory_text.setPosition(pos);

				pos = line_border.getPosition();
				pos.y += MEM_LINE_HEIGHT;
				line_border.setPosition(pos);

				line_border.setFillColor(sf::Color(color, color, color, 255));
				color = (color == 30 ? 50 : 30);
			}

			if (is_goto_prompt || is_mem_prompt)
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
				
				goto_memory_address(addr);
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

		void on_keypressed_mem(sf::Keyboard::Key key)
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
				is_mem_prompt = false;

				if (goto_input_stream.str().length() == 0)
				{
					return;
				}

				u8 value = (u8)std::stoul(goto_input_stream.str(), nullptr, 16);
				u16 addr = mem_start + (active_line * MEM_PER_LINE) + active_column;
				memory_module::write_memory(addr, value, true);
			}
			else if (key == sf::Keyboard::Escape)
			{
				is_mem_prompt = false;
			}

			// cap size
			if (goto_input_stream.tellp() > 2)
			{
				std::string temp = goto_input_stream.str().erase(2);
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

			if (is_mem_prompt)
			{
				on_keypressed_mem(key);
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
			else if (key == sf::Keyboard::Left)
			{
				active_column--;

				if (active_column < 0)
				{
					active_column += MEM_PER_LINE;
				}
			}
			else if (key == sf::Keyboard::Right)
			{
				active_column++;

				if (active_column >= MEM_PER_LINE)
				{
					active_column -= MEM_PER_LINE;
				}
			}
			else if (key == sf::Keyboard::G)
			{
				goto_title_text.setString("Goto Address");
				goto_input_stream.str("");
				is_goto_prompt = true;
			}
			else if (key == sf::Keyboard::Return)
			{
				u32 val = (u32)memory_module::read_memory(mem_start + (active_line * MEM_PER_LINE) + active_column, true);

				goto_title_text.setString("Enter Value");
				goto_input_stream.str("");
				goto_input_stream << std::setfill('0') << std::setw(2) << std::uppercase << std::hex << val << " ";

				is_mem_prompt = true;
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
	};
}