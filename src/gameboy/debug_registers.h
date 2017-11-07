#pragma once

#include "defines.h"
#include "debug_window.h"

#include <SFML/Graphics.hpp>

namespace gameboy
{
	struct debug_registers : public debug_window
	{
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
			stream << "R.pc: 0x" << std::hex << cpu::R.pc << std::endl;
			stream << "R.a: 0x" << std::hex << (int)cpu::R.a << std::endl;
			stream << "R.f: 0x" << std::hex << (int)cpu::R.f << std::endl;
			stream << "R.b: 0x" << std::hex << (int)cpu::R.b << std::endl;
			stream << "R.c: 0x" << std::hex << (int)cpu::R.c << std::endl;
			stream << "R.d: 0x" << std::hex << (int)cpu::R.d << std::endl;
			stream << "R.e: 0x" << std::hex << (int)cpu::R.e << std::endl;
			stream << "R.h: 0x" << std::hex << (int)cpu::R.h << std::endl;
			stream << "R.l: 0x" << std::hex << (int)cpu::R.l << std::endl;
			stream << "R.af: 0x" << std::hex << cpu::R.af << std::endl;
			stream << "R.bc: 0x" << std::hex << cpu::R.bc << std::endl;
			stream << "R.de: 0x" << std::hex << cpu::R.de << std::endl;
			stream << "R.hl: 0x" << std::hex << cpu::R.hl << std::endl;
			stream << "R.sp: 0x" << std::hex << cpu::R.sp << std::endl;

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