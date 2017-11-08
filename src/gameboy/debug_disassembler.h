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

		sf::Text disassembler_text;

		sf::RectangleShape inner_border;
		sf::RectangleShape line_border;

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
		}

		void update()
		{
			// draw to the window texture
			window_texture.draw(outer_border);
			window_texture.draw(inner_border);
			window_texture.draw(title_text);
			
			line_border.setPosition(BORDER_SIZE, BORDER_SIZE + TITLEBAR_SIZE);
			disassembler_text.setPosition(BORDER_SIZE, BORDER_SIZE + TITLEBAR_SIZE);

			u8 color = 30;
			u16 pc = cpu::R.pc;
			for (unsigned int i = 0; i < 16; i++)
			{
				disassembler::symbol sym;
				pc = disassembler::disassemble_instr(pc, sym);

				std::stringstream stream;
				stream << WRITE_HEX_16(sym.addr);
				stream << "\t" << WRITE_HEX_16((int)sym.opcode);
				stream << "\t" << WRITE_HEX_16((int)sym.cb_opcode);
				stream << "\t" << sym.mnemonic;
				stream << "\t" << sym.operands;

				disassembler_text.setString(stream.str());

				// draw the line
				window_texture.draw(line_border);
				window_texture.draw(disassembler_text);

				sf::Vector2f pos = line_border.getPosition();
				pos.y += LINE_HEIGHT;
				line_border.setPosition(pos);
				disassembler_text.setPosition(pos);

				line_border.setFillColor(sf::Color(color, color, color, 255));
				color = (color == 30 ? 50 : 30);
			}

			window_texture.display();
		}
	};
}