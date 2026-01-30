#pragma once

#include <SFML/Graphics.hpp>
#include <argparse.h>


#include "cpu.h"
#include "rom.h"
#include "disassembler.h"
#include "assembler.h"

namespace chip8
{
	static const std::pair<sf::Keyboard::Key, u8> keyboard[] = {
		{sf::Keyboard::Key::Num0, 0x0},
		{sf::Keyboard::Key::Num2, 0x1},
		{sf::Keyboard::Key::Num3, 0x2},
		{sf::Keyboard::Key::Num4, 0x3},
		{sf::Keyboard::Key::Q, 0x4},
		{sf::Keyboard::Key::W, 0x5},
		{sf::Keyboard::Key::E, 0x6},
		{sf::Keyboard::Key::R, 0x7},
		{sf::Keyboard::Key::A, 0x8},
		{sf::Keyboard::Key::S, 0x9},
		{sf::Keyboard::Key::D, 0xA},
		{sf::Keyboard::Key::F, 0xB},
		{sf::Keyboard::Key::Z, 0xC},
		{sf::Keyboard::Key::X, 0xD},
		{sf::Keyboard::Key::C, 0xE},
		{sf::Keyboard::Key::V, 0xF},
	};

	rom loaded_rom;

	// init sfml
	u8 pixel_size = 16;
	sf::RectangleShape white_rect(sf::Vector2f(pixel_size, pixel_size));
	sf::RenderTexture render_texture;

	int init_emulator(const std::string& rom_filename)
	{
		// init cpu and load rom
		cpu::initialize();
		loaded_rom.load(rom_filename);
		cpu::load_rom(loaded_rom.romdata, loaded_rom.romsize & 0xFFFF);

		white_rect.setFillColor(sf::Color::White);

		// init main screen texture
		render_texture.resize(sf::Vector2u(cpu::width * pixel_size, cpu::height * pixel_size));
		render_texture.clear(sf::Color::Black);

		return 0;
	}

	int process_event(const sf::Event* event)
	{
		if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>())
		{
			// check if key is pressed and set the corresponding key in the cpu
			for (const auto& key : keyboard)
			{
				if (keyPressed->code == key.first)
				{
					cpu::set_keys(key.second, true);
					break;
				}
			}

			if (keyPressed->code == sf::Keyboard::Key::Space) // resetting here because chip8 doesnt have debugger
			{
				cpu::reset();
			}
		}
		else if (const auto* keyReleased = event->getIf<sf::Event::KeyReleased>())
		{
			// check if key is released and unset the corresponding key in the cpu
			for (const auto& key : keyboard)
			{
				if (keyReleased->code == key.first)
				{
					cpu::set_keys(key.second, false);
					break;
				}
			}
		}

		return 0;
	}

	int update()
	{
		// update the cpu emulation
		const int INSTRUCTIONS_PER_FRAME = 10;

		for (int i = 0; i < INSTRUCTIONS_PER_FRAME; i++)
		{
			cpu::update_cycle();
		}

		cpu::update_timers();

		if (cpu::drawFlag)
		{
			cpu::drawFlag = false;

			// clear window
			render_texture.clear(sf::Color::Black);

			// draw screen
			u16 pixel = 0;
			for (u8 y = 0; y < cpu::height; y++)
			{
				for (u8 x = 0; x < cpu::width; x++)
				{
					if (cpu::gfx[pixel++] != 0)
					{
						white_rect.setPosition(sf::Vector2f((float)(x * pixel_size), (float)(y * pixel_size)));
						render_texture.draw(white_rect);
					}

				}
			}

			render_texture.display();

			return 1;
		}

		return 0;
	}

	const sf::Texture* get_emulator_texture()
	{
		return &render_texture.getTexture();
	}
}
