#pragma once

#include "defines.h"

#include "gameboy\memory_module.h"

namespace gameboy
{
	namespace gpu
	{
		// main memory module pointer
		gameboy::memory_module* memory_module;

		u8* scanline = 0;
		u8* coincidence_scanline = 0;
		u8* lcd_control = 0;
		u8* lcd_status = 0;
		u8* scrollY = 0;
		u8* scrollX = 0;
		u8* windowX = 0;
		u8* windowY = 0;
		u8* palette_bg = 0;

		const u8 width = 160;
		const u8 height = 144;
		u8 framebuffer[width * height * 4];
		
		const s32 horz_cycles = 456; // cycles per horz scanline
		s32 horz_cycle_count = horz_cycles;

		const u32 horz_mode_searchspriteattr = horz_cycles - 80;
		const u32 horz_mode_transfer = horz_mode_searchspriteattr - 172;
		
		// set and get lcd control flag helpers
		inline void set_lcd_control_flag(u8 flag)
		{
			flag = (1 << flag);
			*lcd_control |= flag;
		}

		inline void clear_lcd_control_flag(u8 flag)
		{
			flag = (1 << flag);
			*lcd_control &= ~flag; // clear the bit
		}

		inline u8 get_lcd_control_flag(u8 flag)
		{
			return ((*lcd_control & (1 << flag)) >> flag);
		}

		inline void clear_all_lcd_control_flags()
		{
			*lcd_control = 0x0;
		}

		//Bit 0 - BG Display(for CGB see below) (0 = Off, 1 = On)
		//Bit 1 - OBJ(Sprite) Display Enable(0 = Off, 1 = On)
		//Bit 2 - OBJ(Sprite) Size(0 = 8x8, 1 = 8x16)
		//Bit 3 - BG Tile Map Display Select(0 = 9800 - 9BFF, 1 = 9C00 - 9FFF)
		//Bit 4 - BG & Window Tile Data Select(0 = 8800 - 97FF, 1 = 8000 - 8FFF)
		//Bit 5 - Window Display Enable(0 = Off, 1 = On)
		//Bit 6 - Window Tile Map Display Select(0 = 9800 - 9BFF, 1 = 9C00 - 9FFF)
		//Bit 7 - LCD Display Enable(0 = Off, 1 = On)

		enum LCD_CONTROL_FLAGS
		{
			FLAG_BG_DISPLAY_ENABLED = 0,
			FLAG_OBJ_DISPLAY_ENABLED,
			FLAG_OBJ_SIZE,
			FLAG_BG_TILEMAP_DISPLAY_SELECT,
			FLAG_BG_WINDOW_TILE_DISPLAY_SELECT,
			FLAG_WINDOW_DISPLAY_ENABLED,
			FLAG_WINDOW_TILE_DISPLAY_SELECT,
			FLAG_LCD_DISPLAY_ENABLED,
		};

		// set and get lcd status mode
		inline void set_lcd_status_mode(u8 mode)
		{
			mode &= 0x3; // just incase
			*lcd_status &= 0xFC; // clear old mode bits
			*lcd_status |= mode;
		}

		inline u8 get_lcd_status_mode()
		{
			return *lcd_status &= 0x3;
		}
		
		enum LCD_STATUS_MODES
		{
			MODE_HBLANK = 0,
			MODE_VBLANK,
			MODE_OAM_ACCESS,
			MODE_VRAM_ACCESS,
		};

		// lcd status interrupt flags
		inline void set_lcd_interrupt_flag(u8 flag)
		{
			flag = (1 << flag);
			*lcd_status |= flag;
		}

		inline void clear_lcd_interrupt_flag(u8 flag)
		{
			flag = (1 << flag);
			*lcd_status &= ~flag; // clear the bit
		}

		inline u8 get_lcd_interrupt_flag(u8 flag)
		{
			return ((*lcd_status & (1 << flag)) >> flag);
		}

		inline void clear_all_lcd_interrupt_flags()
		{
			*lcd_status &= ~0x78; // take all but bits 3, 4, 5, 6
		}

		enum LCD_INTERRUPT_FLAGS
		{
			FLAG_HBLANK = 3,
			FLAG_VBLANK,
			FLAG_OAM_ACCESS,
			FLAG_COINCIDENCE
		};

		int reset()
		{
			horz_cycle_count = horz_cycles;
			memset(framebuffer, 0x0, sizeof(framebuffer));

			return 0;
		}

		int initialize(gameboy::memory_module* memory)
		{
			memory_module = memory;

			// setup memory ptrs
			scanline = memory_module->get_memory(0xFF44);
			coincidence_scanline = memory_module->get_memory(0xFF45);
			lcd_control = memory_module->get_memory(0xFF40);
			lcd_status = memory_module->get_memory(0xFF41);
			scrollY = memory_module->get_memory(0xFF42);
			scrollX = memory_module->get_memory(0xFF43);
			windowY = memory_module->get_memory(0xFF4A);
			windowX = memory_module->get_memory(0xFF4B);
			palette_bg = memory_module->get_memory(0xFF47);
			reset();

			return 0;
		}

		u8 get_palette_color(u8 color)
		{
			u8 palette = *palette_bg;
			palette &= (0x3 << (color * 2)); // take two bits from palette depending on the passed in color id
			palette >>= (color * 2); // shift palette color back down

			switch (palette)
			{
			case 0x00: // white
				color = 0xFF;
				break;
			case 0x1: // light grey
				color = 0xCC;
				break;
			case 0x2: // dark grey
				color = 0x77;
				break;
			case 0x3: // black
				color = 0x0;
				break;
			}

			return color;
		}

		int draw_scanline()
		{
			// only doing background. but will need to merge this with window
			if (get_lcd_control_flag(FLAG_BG_DISPLAY_ENABLED))
			{
				// tilemap starting location. tilemap is 32 x 32 bytes that map to a tile
				u16 tilemapAddr = 0x9800;
				if (get_lcd_control_flag(FLAG_BG_TILEMAP_DISPLAY_SELECT))
				{
					tilemapAddr = 0x9C00;
				}

				s32 tilesetOffset = 128; // offset depending on tileset used
				s32 tileSize = 16; // each tile is 16 bytes. 2 x 8 rows of a tile
				u16 tilesetAddr = 0x8800; // addr of tileset
				if (get_lcd_control_flag(FLAG_BG_WINDOW_TILE_DISPLAY_SELECT))
				{
					tilesetAddr = 0x8000;
					tilesetOffset = 0;
				}

				u8 yPos = *scrollY + *scanline;
				u8 tileY = (yPos / 8) * 32; // calc tile offset based on yPos
				u8 tileYPixel = yPos % 8; // the row of the specific tile the scanline is on

				// draw the 160 horz pixels
				for (u32 pixel = 0; pixel < 160; pixel++)
				{
					u8 xPos = *scrollX + pixel;
					u8 tileX = xPos / 8; // calc tile offset base on xPos
					u8 tileXPixel = xPos % 8; // the column of the speific tile to draw
					s16 tileId = 0;
					
					if (tilesetOffset != 0) // its a signed value
					{
						tileId = (s8)memory_module->read_memory(tilemapAddr + tileX + tileY);
						tileId += tilesetOffset; // apply offset to the id
					}
					else
					{
						tileId = memory_module->read_memory(tilemapAddr + tileX + tileY);
					}

					// we have the tile id. lets draw pixel
					u8 tileOffset = (tileId * tileSize) + (tileYPixel * 2); // get tile, then add offset to y pos of tile, each row is 2 bytes
					u8 dataA = memory_module->read_memory(tilesetAddr + tileOffset);
					u8 dataB = memory_module->read_memory(tilesetAddr + tileOffset + 1);
					u8 bit = 7 - tileXPixel; // the bits and pixels are inversed
					u8 color = ((dataA & (1 << bit)) >> bit)| (((dataB & (1 << bit)) >> bit) << 1);

					color = gpu::get_palette_color(color);

					u32 pixelPos = (*scanline * 160 + pixel) * 4; // the pixel we are drawing * 4 bytes per pixel
					framebuffer[pixelPos++] = color;
					framebuffer[pixelPos++] = color;
					framebuffer[pixelPos++] = color;
					framebuffer[pixelPos++] = 0xFF;
				}
			}

			return 0;
		}

		int update_lcd_status()
		{
			// handle the new lcd status mode
			u8 new_lcd_mode = get_lcd_status_mode();
			bool req_lcd_interrupt = false;

			if (*scanline >= 144) // vblank mode
			{
				new_lcd_mode = MODE_VBLANK;

				if (get_lcd_interrupt_flag(FLAG_VBLANK))
				{
					req_lcd_interrupt = true;
				}
			}
			else if (horz_cycle_count >= horz_mode_searchspriteattr) // mode 2 - searching sprite attr is first 80 cycles
			{
				new_lcd_mode = MODE_OAM_ACCESS;

				if (get_lcd_interrupt_flag(FLAG_OAM_ACCESS))
				{
					req_lcd_interrupt = true;
				}
			}
			else if (horz_cycle_count >= horz_mode_transfer) // mode 3 - transfer data is next 172 cycles
			{
				new_lcd_mode = MODE_VRAM_ACCESS;
			}
			else // mode 0 - hblank is the remaining cycles before new scanline
			{
				new_lcd_mode = MODE_HBLANK;

				if (get_lcd_interrupt_flag(FLAG_HBLANK))
				{
					req_lcd_interrupt = true;
				}
			}

			if (*lcd_status != new_lcd_mode)
			{
				if (req_lcd_interrupt)
				{
					// trigger an lcd interrupt
					cpu::set_request_interrupt_flag(cpu::INTERRUPT_LCD);
				}

				// set the new mode
				*lcd_status = new_lcd_mode;
			}

			// check coincidence interrupt. scanline == coincidence scanline
			if (*coincidence_scanline == *scanline)
			{
				*lcd_status |= (1 << 2); // set bit 2 for coincidence

				if (get_lcd_interrupt_flag(FLAG_COINCIDENCE))
				{
					cpu::set_request_interrupt_flag(cpu::INTERRUPT_LCD);
				}
			}
			else
			{
				*lcd_status &= 0xFB; // take all but bit 2
			}

			return 0;
		}

		int update_scanline()
		{
			// handle the scanline update
			if (horz_cycle_count <= 0)
			{
				// continue to next scanline
				horz_cycle_count = horz_cycles;

				(*scanline)++;

				if (*scanline < 144)
				{
					// draw the scan line
					draw_scanline();
				}
				else if (*scanline == 144)
				{
					// start of vblank
					cpu::set_request_interrupt_flag(cpu::INTERRUPT_VBLANK);
				}
				else if (*scanline > 153)
				{
					// reset scanline to 0
					*scanline = 0;
				}
			}

			return 0;
		}

		int update(u8 cycles)
		{
			if (get_lcd_control_flag(FLAG_LCD_DISPLAY_ENABLED) == 0)
			{
				// lcd not enabled. reset scanline and horz cycle count. lcd mode set to 1 (VBlank)
				*scanline = 0;
				horz_cycle_count = 0;
				set_lcd_status_mode(MODE_VBLANK);
				return 0;
			}

			horz_cycle_count -= cycles; // dec the horz cycles

			update_lcd_status();
			update_scanline();

			return 0;
		}
	}
}