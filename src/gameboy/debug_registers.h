#pragma once

#include "defines.h"
#include "debug_window.h"

#include <SFML/Graphics.hpp>

namespace gameboy
{
	class debug_registers : public debug_window
	{
	public:
		#define CHECKBOX_SIZE		16

		#define FLAG_X				210
		#define FLAG_Y				24
		#define FLAG_GAP			24

		#define GPU_REG_X			138
		#define GPU_REG_Y			132

		sf::Text registers_text;

		sf::RectangleShape inner_border;

		sf::RectangleShape checkbox_outer_border;
		sf::RectangleShape checkbox_inner_border;
		sf::RectangleShape checkbox_inner_check;
		sf::Text flag_text;

		sf::Text gpu_registers_text;

		const char* flag_title[8] = { "", "", "", "", "C", "H", "N", "Z" };

		debug_registers() : debug_window(256, 256)
		{
            is_selectable = false;

			registers_text.setString("");
			registers_text.setFont(font);
			registers_text.setCharacterSize(16);
			registers_text.setPosition(BORDER_SIZE, BORDER_SIZE + TITLEBAR_SIZE);

			inner_border.setSize(sf::Vector2f(256, 256));
			inner_border.setFillColor(sf::Color(0, 0, 0, 255));
			inner_border.setPosition(BORDER_SIZE, BORDER_SIZE + TITLEBAR_SIZE);

			title_text.setString("Registers");

			checkbox_outer_border.setSize(sf::Vector2f(CHECKBOX_SIZE + BORDER_SIZE * 2, CHECKBOX_SIZE + BORDER_SIZE * 2));
			checkbox_outer_border.setFillColor(sf::Color(200, 200, 200, 255));
			checkbox_outer_border.setPosition(0, 0);

			checkbox_inner_border.setSize(sf::Vector2f(CHECKBOX_SIZE, CHECKBOX_SIZE));
			checkbox_inner_border.setFillColor(sf::Color(0, 0, 0, 255));
			checkbox_inner_border.setPosition(BORDER_SIZE, BORDER_SIZE);

			checkbox_inner_check.setSize(sf::Vector2f(CHECKBOX_SIZE - BORDER_SIZE * 2, CHECKBOX_SIZE - BORDER_SIZE * 2));
			checkbox_inner_check.setFillColor(sf::Color(200, 200, 200, 255));
			checkbox_inner_check.setPosition(BORDER_SIZE * 2, BORDER_SIZE * 2);

			flag_text.setString("");
			flag_text.setFont(font);
			flag_text.setCharacterSize(16);
			flag_text.setPosition(0, 0);

			gpu_registers_text.setString("");
			gpu_registers_text.setFont(font);
			gpu_registers_text.setCharacterSize(16);
			gpu_registers_text.setPosition(GPU_REG_X, GPU_REG_Y);
		}

		void update()
		{
			std::stringstream stream;
			//stream << "R.a : " << WRITE_HEX_8((int)cpu::R.a) << std::endl;
			//stream << "R.f : " << WRITE_HEX_8((int)cpu::R.f) << std::endl;
			//stream << "R.b : " << WRITE_HEX_8((int)cpu::R.b) << std::endl;
			//stream << "R.c : " << WRITE_HEX_8((int)cpu::R.c) << std::endl;
			//stream << "R.d : " << WRITE_HEX_8((int)cpu::R.d) << std::endl;
			//stream << "R.e : " << WRITE_HEX_8((int)cpu::R.e) << std::endl;
			//stream << "R.h : " << WRITE_HEX_8((int)cpu::R.h) << std::endl;
			//stream << "R.l : " << WRITE_HEX_8((int)cpu::R.l) << std::endl;
			stream << "R.af: " << WRITE_HEX_16(cpu::R.af) << std::endl;
			stream << "R.bc: " << WRITE_HEX_16(cpu::R.bc) << std::endl;
			stream << "R.de: " << WRITE_HEX_16(cpu::R.de) << std::endl;
			stream << "R.hl: " << WRITE_HEX_16(cpu::R.hl) << std::endl;
			stream << "R.sp: " << WRITE_HEX_16(cpu::R.sp) << std::endl;
			stream << "R.pc: " << WRITE_HEX_16(cpu::R.pc) << std::endl;

			registers_text.setString(stream.str());

			// draw to the window texture
			window_texture.draw(outer_border);
			window_texture.draw(inner_border);
			window_texture.draw(registers_text);
			window_texture.draw(title_text);

			// draw the flag checkboxes
			float x = FLAG_X;
			float y = FLAG_Y;
			for (unsigned i = cpu::FLAG_CARRY; i <= cpu::FLAG_ZERO; i++)
			{
				checkbox_outer_border.setPosition(x, y);
				checkbox_inner_border.setPosition(x + BORDER_SIZE, y + BORDER_SIZE);
				checkbox_inner_check.setPosition(x + BORDER_SIZE * 2, y + BORDER_SIZE * 2);

				flag_text.setString(flag_title[i]);
				flag_text.setPosition(x + CHECKBOX_SIZE + BORDER_SIZE * 2 + 2, y - 1);

				window_texture.draw(checkbox_outer_border);
				window_texture.draw(checkbox_inner_border);
				window_texture.draw(flag_text);


				if (cpu::get_flag(i))
				{
					window_texture.draw(checkbox_inner_check);
				}

				y += FLAG_GAP;
			}

			// draw the gpu 
			stream.str("");
			stream << " LCDC: " << WRITE_HEX_8(*gpu::lcd_control) << std::endl;
			stream << " LCDS: " << WRITE_HEX_8((0x80 | *gpu::lcd_status)) << std::endl;
			stream << " SCAN: " << WRITE_HEX_8(*gpu::scanline) << std::endl;
			stream << "CSCAN: " << WRITE_HEX_8(*gpu::coincidence_scanline) << std::endl;
			stream << "   IE: " << WRITE_HEX_8(*cpu::interrupt_enable_flag) << std::endl;
			stream << "   IF: " << WRITE_HEX_8(*cpu::interrupt_request_flag) << std::endl;
			stream << "  IME: " << std::dec << (cpu::interrupt_master ? 1 : 0) << std::endl;

			gpu_registers_text.setString(stream.str());

			window_texture.draw(gpu_registers_text);

			window_texture.display();
		}
	};
}