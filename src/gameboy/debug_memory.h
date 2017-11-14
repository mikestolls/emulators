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

		#define MEM_PER_LINE				16

		sf::Text memory_text;

		sf::RectangleShape inner_border;
		sf::RectangleShape line_border;
		
		sf::RectangleShape goto_outer_border;
		sf::RectangleShape goto_inner_border;
		sf::RectangleShape goto_outer_input_border;
		sf::RectangleShape goto_inner_input_border;
		sf::Text goto_title_text;
		sf::Text goto_input_text;
		std::stringstream goto_input_stream;

		s16 active_line;
		u16 active_addr;
		u16 mem_start;

		bool is_goto_prompt;

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
			
			title_text.setString("Memory Viewer");

			bottom_text.setString("(Up / Down) Change Line\t(G) Goto Address");
			
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
			mem_start = 0;

			is_goto_prompt = false;
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
			// draw to the window texture. 
			window_texture.draw(outer_border);
			window_texture.draw(inner_border);
			window_texture.draw(title_text);
			
			line_border.setPosition(BORDER_SIZE, BORDER_SIZE + TITLEBAR_SIZE);
			memory_text.setPosition(BORDER_SIZE + LINE_XPOS, BORDER_SIZE + TITLEBAR_SIZE);
			
			// draw foreground of each line
			u16 addr = mem_start;
			u8 color = 30;
			for (unsigned int i = 0; i < LINE_COUNT; i++)
			{
				// draw memory line
				std::stringstream stream;
				stream << WRITE_HEX_16(addr) << "\t";
				
				for (unsigned int j = 0; j < MEM_PER_LINE; j++)
				{
					u32 val = (u32)memory_module::memory[addr + (i * MEM_PER_LINE) + j];
					stream << std::setfill('0') << std::setw(2) << std::uppercase << std::hex << val << " ";
				}

				stream << "\t\t";

				for (unsigned int j = 0; j < MEM_PER_LINE; j++)
				{
					u32 val = (u32)memory_module::memory[addr + (i * MEM_PER_LINE) + j];
					if (val < 0x20 || (val > 0x7E && val < 0xA0))
					{
						stream << ".";
					}
					else
					{
						stream << (char)val;
					}
				}

				memory_text.setString(stream.str());
				
				// draw the background line
				if (i == active_line)
				{
					line_border.setFillColor(sf::Color(150, 150, 150, 255));
				}

				window_texture.draw(line_border);
				
				// draw the line
				window_texture.draw(memory_text);

				// increase position
				sf::Vector2f pos = memory_text.getPosition();
				pos.y += LINE_HEIGHT;
				memory_text.setPosition(pos);

				pos = line_border.getPosition();
				pos.y += LINE_HEIGHT;
				line_border.setPosition(pos);

				line_border.setFillColor(sf::Color(color, color, color, 255));
				color = (color == 30 ? 50 : 30);

				addr += MEM_PER_LINE;
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
				//get the instruction int
				u16 addr = (u16)std::stoul(goto_input_stream.str(), nullptr, 16);
				u16 max_addr = 0xFFFF - ((LINE_COUNT - 1) * MEM_PER_LINE);

				if (addr > max_addr)
				{
					addr = max_addr;
				}

				mem_start = addr - (addr % MEM_PER_LINE);

				is_goto_prompt = false;
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
				s32 temp = mem_start + MEM_PER_LINE;
				u16 max_addr = 0xFFFF - (LINE_COUNT * MEM_PER_LINE) + 1;

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

			// handle goto
			if (key == sf::Keyboard::G)
			{
				is_goto_prompt = true;
				goto_input_stream.str("");
			}
		}
	};
}