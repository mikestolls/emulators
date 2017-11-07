#pragma once

#include "defines.h"
#include "debug_window.h"

#include <SFML/Graphics.hpp>

namespace gameboy
{
	struct debug_registers : public debug_window
	{
		#define WRITE_HEX_16(x) "0x" << std::setfill('0') << std::setw(4) << std::uppercase << std::hex << x
		#define WRITE_HEX_8(x) "0x" << std::setfill('0') << std::setw(2) << std::uppercase << std::hex << x

		sf::Font font;
		sf::Text registers_text;

		sf::RectangleShape inner_border;

		debug_registers() : debug_window(256, 256)
		{
			font.loadFromFile("arial.ttf");

			registers_text.setString("");
			registers_text.setFont(font);
			registers_text.setCharacterSize(16);
			registers_text.setPosition(BORDER_SIZE, BORDER_SIZE + TITLEBAR_SIZE);

			inner_border.setSize(sf::Vector2f(256, 256));
			inner_border.setFillColor(sf::Color(0, 0, 0, 255));
			inner_border.setPosition(BORDER_SIZE, BORDER_SIZE + TITLEBAR_SIZE);

			title_text.setString("Registers");
		}

		void update()
		{
			std::stringstream stream;
			stream << "R.pc: " << WRITE_HEX_16(cpu::R.pc) << std::endl;
			stream << "R.a: " << WRITE_HEX_8((int)cpu::R.a) << std::endl;
			stream << "R.f: " << WRITE_HEX_8((int)cpu::R.f) << std::endl;
			stream << "R.b: " << WRITE_HEX_8((int)cpu::R.b) << std::endl;
			stream << "R.c: " << WRITE_HEX_8((int)cpu::R.c) << std::endl;
			stream << "R.d: " << WRITE_HEX_8((int)cpu::R.d) << std::endl;
			stream << "R.e: " << WRITE_HEX_8((int)cpu::R.e) << std::endl;
			stream << "R.h: " << WRITE_HEX_8((int)cpu::R.h) << std::endl;
			stream << "R.l: " << WRITE_HEX_8((int)cpu::R.l) << std::endl;
			stream << "R.af: " << WRITE_HEX_16(cpu::R.af) << std::endl;
			stream << "R.bc: " << WRITE_HEX_16(cpu::R.bc) << std::endl;
			stream << "R.de: " << WRITE_HEX_16(cpu::R.de) << std::endl;
			stream << "R.hl: " << WRITE_HEX_16(cpu::R.hl) << std::endl;
			stream << "R.sp: " << WRITE_HEX_16(cpu::R.sp) << std::endl;

			registers_text.setString(stream.str());

			// draw to the window texture
			window_texture.draw(outer_border);
			window_texture.draw(inner_border);
			window_texture.draw(registers_text);
			window_texture.draw(title_text);

			window_texture.display();
		}
	};
}