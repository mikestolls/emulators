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
			MODE_SEARCHING_SPRITE_ATTR,
			MODE_TRANSFER_TO_LCD,
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
			FLAG_SEARCHING_SPRITE_ATTR,
			FLAG_COINCIDENCE
		};

		int reset()
		{
			horz_cycle_count = horz_cycles;

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

			reset();

			return 0;
		}

		int draw_scanline()
		{
			// only doing background. but will need to merge this with window
			if (get_lcd_control_flag(FLAG_BG_DISPLAY_ENABLED))
			{
				// draw the scanline tiles
				u8* bgTilemapPtr = memory_module->get_memory(0x9800);
				if (get_lcd_control_flag(FLAG_BG_TILEMAP_DISPLAY_SELECT))
				{
					bgTilemapPtr = memory_module->get_memory(0x9C00);
				}

				u8* tilemapPtr = memory_module->get_memory(0x8800);
				s32 tilemapOffset = 128;
				s32 tileSize = 16;
				if (get_lcd_control_flag(FLAG_BG_WINDOW_TILE_DISPLAY_SELECT))
				{
					tilemapPtr = memory_module->get_memory(0x8000);
					tilemapOffset = 0;
				}

				u8 yPos = *scrollY + *scanline;
				u8 tileY = (yPos / 8) * 32; // calc tile offset based on yPos

				// draw the 160 horz pixels
				for (u32 pixel = 0; pixel < 160; pixel++)
				{
					u8 xPos = *scrollX + pixel;
					u8 tileX = xPos / 8; // calc tile offset base on xPos

					u8* tileAddr = bgTilemapPtr + tileX + tileY;
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
				new_lcd_mode = MODE_SEARCHING_SPRITE_ATTR;

				if (get_lcd_interrupt_flag(FLAG_SEARCHING_SPRITE_ATTR))
				{
					req_lcd_interrupt = true;
				}
			}
			else if (horz_cycle_count >= horz_mode_transfer) // mode 3 - transfer data is next 172 cycles
			{
				new_lcd_mode = MODE_TRANSFER_TO_LCD;
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