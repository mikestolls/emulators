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

		sf::Font font;
		sf::Text disassembler_text;

		sf::RectangleShape inner_border;

		debug_disassembler() : debug_window(916, 256)
		{
			font.loadFromFile("courbd.ttf");

			disassembler_text.setString("");
			disassembler_text.setFont(font);
			disassembler_text.setCharacterSize(16);
			disassembler_text.setPosition(BORDER_SIZE, BORDER_SIZE + TITLEBAR_SIZE);

			inner_border.setSize(sf::Vector2f(916, 256));
			inner_border.setFillColor(sf::Color(0, 0, 0, 255));
			inner_border.setPosition(BORDER_SIZE, BORDER_SIZE + TITLEBAR_SIZE);

			title_text.setString("Disassembler");
		}

		void update()
		{
			std::stringstream stream;

			for (unsigned int i = 0; i < 16; i++)
			{
				stream << WRITE_HEX_16(disassembler::disassembled_program[i].addr);
				stream << WRITE_HEX_16((int)disassembler::disassembled_program[i].opcode);
				stream << WRITE_HEX_16((int)disassembler::disassembled_program[i].cb_opcode);
				stream << disassembler::disassembled_program[i].mnemonic;
				stream << disassembler::disassembled_program[i].operands;
				stream << std::endl;
			}
			
			disassembler_text.setString(stream.str());

			// draw to the window texture
			window_texture.draw(outer_border);
			window_texture.draw(inner_border);
			window_texture.draw(disassembler_text);
			window_texture.draw(title_text);

			window_texture.display();
		}
	};
}