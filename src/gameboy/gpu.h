#pragma once

#include "defines.h"

#include "gameboy\memory_module.h"

namespace gameboy
{
	namespace gpu
	{
		const bool green_palette = true;

		u8* scanline = 0;
		u8* coincidence_scanline = 0;
		u8* lcd_control = 0;
		u8* lcd_status = 0;
		u8* scroll_y = 0;
		u8* scroll_x = 0;
		u8* window_x = 0;
		u8* window_y = 0;
		u8* palette_bg = 0;
		u8* sprite_attr = 0;

		const u8 width = 160;
		const u8 height = 144;
		u8 framebuffer[width * height * 4];
		u8 bg_palette_indices[width * height];
		bool sprite_pixels[160];
		bool lcd_enabling = false;
		bool lcd_enabled = false;
		bool scanline_inc = false;
		s32 horz_cycle_count = 0;
		bool vblank_occurred = false;
		u8 window_scanline_counter = 0;
		
		inline u8 get_lcd_control_flag(u8 flag)
		{
			return ((*lcd_control & (1 << flag)) >> flag);
		}

		//Bit 0 - BG and Window enable/priority (for CGB bg and window lose priorty. object display on top) (0 = Off, 1 = On)
		//Bit 1 - OBJ(Sprite) Display Enable(0 = Off, 1 = On)
		//Bit 2 - OBJ(Sprite) Size(0 = 8x8, 1 = 8x16)
		//Bit 3 - BG Tile Map Display Select(0 = 9800 - 9BFF, 1 = 9C00 - 9FFF)
		//Bit 4 - BG & Window Tile Data Select(0 = 8800 - 97FF, 1 = 8000 - 8FFF)
		//Bit 5 - Window Display Enable(0 = Off, 1 = On)
		//Bit 6 - Window Tile Map Display Select(0 = 9800 - 9BFF, 1 = 9C00 - 9FFF)
		//Bit 7 - LCD Display Enable(0 = Off, 1 = On)

		enum LCD_CONTROL_FLAGS
		{
			FLAG_BG_WINDOW_ENABLE_PRIORITY= 0,
			FLAG_OBJ_DISPLAY_ENABLED,
			FLAG_OBJ_SIZE,
			FLAG_BG_TILEMAP_DISPLAY_SELECT,
			FLAG_BG_WINDOW_TILE_DISPLAY_SELECT,
			FLAG_WINDOW_DISPLAY_ENABLED,
			FLAG_WINDOW_TILEMAP_DISPLAY_SELECT,
			FLAG_LCD_DISPLAY_ENABLED,
		};

		// set and get lcd status mode
		inline void set_lcd_status_mode(u8 mode)
		{
			mode &= 0x3; // just incase
			*lcd_status &= 0xFC; // clear old mode bits
			*lcd_status |= mode;
			*lcd_status |= 0x80; // turn bit 7 on
		}

		inline u8 get_lcd_status_mode()
		{
			return *lcd_status & 0x3;
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
			*lcd_status &= 0x87; // take all but bits 3, 4, 5, 6
		}

		enum LCD_INTERRUPT_FLAGS
		{
			FLAG_HBLANK = 3,
			FLAG_VBLANK,
			FLAG_OAM_ACCESS,
			FLAG_COINCIDENCE
		};

		// get sprite attributes
		inline u8 get_sprite_attribute(u8 attribute, u8 flag)
		{
			return ((attribute & (1 << flag)) >> flag);
		}
		
		//Bit 0 - Not Used
		//Bit 1 - Not Used
		//Bit 2 - Not Used
		//Bit 3 - Not Used
		//Bit 4 - Palette Number (0 = 0xFF48, 1 = 0xFF49)
		//Bit 5 - X Flip (0 = None, 1 = Horizontal Flip)
		//Bit 6 - Y Flip (0 = None, 1 = Vertical Flip)
		//Bit 7 - Sprite to Background Priority

		enum SPRITE_ATTRIBUTE_FLAGS
		{
			FLAG_BIT_0 = 0,
			FLAG_BIT_1,
			FLAG_BIT_2,
			FLAG_BIT_3,
			FLAG_SPRITE_PALETTE,
			FLAG_SPRITE_FLIP_X,
			FLAG_SPRITE_FLIP_Y,
			FLAG_SPRITE_PRIORITY,
		};

		int reset()
		{
			horz_cycle_count = 0;
			lcd_enabling = false;
			lcd_enabled = false;
			window_scanline_counter = 0;
			memset(framebuffer, 0x0, sizeof(framebuffer));
			memset(bg_palette_indices, 0x0, sizeof(bg_palette_indices));
			
			return 0;
		}

		int initialize()
		{
			// setup memory ptrs
			scanline = memory_module::get_memory(0xFF44, true);
			coincidence_scanline = memory_module::get_memory(0xFF45, true);
			lcd_control = memory_module::get_memory(0xFF40, true);
			lcd_status = memory_module::get_memory(0xFF41, true);
			scroll_y =  memory_module::get_memory(0xFF42, true);
			scroll_x = memory_module::get_memory(0xFF43, true);
			window_y = memory_module::get_memory(0xFF4A, true);
			window_x = memory_module::get_memory(0xFF4B, true);
			palette_bg = memory_module::get_memory(0xFF47, true);
			sprite_attr = memory_module::get_memory(0xFE00, true);

			reset();

			return 0;
		}

		u32 get_palette_color(u8 palette_color, u8 palette)
		{
			palette >>= (palette_color << 1);
			palette &= 0x3;

			u32 color = 0xFF; // alpha

			if (green_palette)
			{
				switch (palette)
				{
				case 0x00: // white
					color = 0xE0F8D0FF;
					break;
				case 0x1: // light grey
					color = 0x88C070FF;
					break;
				case 0x2: // dark grey
					color = 0x346856FF;
					break;
				case 0x3: // black
					color = 0x081820FF;
					break;
				}
			}
			else
			{
				switch (palette)
				{
				case 0x00: // white
					color = 0xFFFFFFFF;
					break;
				case 0x1: // light grey
					color = 0xCCCCCCFF;
					break;
				case 0x2: // dark grey
					color = 0x777777FF;
					break;
				case 0x3: // black
					color = 0x000000FF;
					break;
				}
			}

			return color;
		}

		u32 get_palette_color(u8 palette_color)
		{
			return get_palette_color(palette_color, memory_module::read_memory(0xFF47, true));
		}

		int draw_scanline()
		{
			if (get_lcd_control_flag(FLAG_LCD_DISPLAY_ENABLED) == false)
			{
				return 0;
			}

			bool is_drawing_bg = true;
			bool is_drawing_window = get_lcd_control_flag(FLAG_WINDOW_DISPLAY_ENABLED) > 0;
			bool window_visible_this_line = false; // this tracks and increments the window scaline position

			// lcd control bit 0 == 0 means nothing is rendered. different for CGB. objects take priorty over bg and window
			if (get_lcd_control_flag(FLAG_BG_WINDOW_ENABLE_PRIORITY) == 0)
			{
				is_drawing_bg = false;
				is_drawing_window = false;
			}
			
			if (is_drawing_bg)
			{
				s32 tileset_offset = 128; // offset depending on tileset used
				s32 tilesize = 16; // each tile is 16 bytes. 2 x 8 rows of a tile
				u16 tileset_addr = 0x8800; // addr of tileset
				if (get_lcd_control_flag(FLAG_BG_WINDOW_TILE_DISPLAY_SELECT))
				{
					tileset_addr = 0x8000;
					tileset_offset = 0;
				}

				u8 y_pos = 0;
				u8 x_pos = 0;
				u32 tile_x = 0;
				u32 tile_y = 0;
				u8 tile_y_pixel = 0;
				u8 tile_x_pixel = 0;
				u8 window_pixel_x = 0;
				u8 window_pixel_y = window_scanline_counter;
				s16 tile_id = 0;
				u16 tilemap_addr = 0;

				// draw the 160 horz pixels
				for (u32 pixel = 0; pixel < 160; pixel++)
				{
					// if window is drawing, scaline it >= window_y and horz pixel is >= window_x
					if (is_drawing_window &&
						*scanline >= *window_y &&
						pixel >= *window_x - 7)
					{
						window_visible_this_line = true; // we rendered a window scanline, increment counter

						// Window doesn't scroll - it starts at (0,0) in its tilemap
						window_pixel_x = pixel - (*window_x - 7);  // Pixel within window
						tile_x = window_pixel_x / 8;
						tile_y = (window_pixel_y / 8) * 32;
						tile_x_pixel = window_pixel_x % 8;
						tile_y_pixel = window_pixel_y % 8;

						if (get_lcd_control_flag(FLAG_WINDOW_TILEMAP_DISPLAY_SELECT))
						{
							tilemap_addr = 0x9C00;
						}
						else
						{
							tilemap_addr = 0x9800;
						}
					}
					else
					{
						y_pos = *scroll_y + *scanline;
						tile_x = 0;
						tile_y = (y_pos / 8) * 32; // calc tile offset based on y_pos. map is 32 tiles wide
						tile_y_pixel = y_pos % 8; // the row of the specific tile the scanline is on
						x_pos = *scroll_x + pixel;
						tile_x = x_pos / 8; // calc tile offset base on xPos
						tile_x_pixel = x_pos % 8; // the column of the speific tile to draw

						if (get_lcd_control_flag(FLAG_BG_TILEMAP_DISPLAY_SELECT))
						{
							tilemap_addr = 0x9C00;
						}
						else
						{
							tilemap_addr = 0x9800;
						}
					}

					if (tileset_offset != 0)
					{
						tile_id = (s8)memory_module::read_memory(tilemap_addr + tile_x + tile_y, true);
						tile_id += tileset_offset;
					}
					else
					{
						tile_id = memory_module::read_memory(tilemap_addr + tile_x + tile_y, true);
					}

					// we have the tile id. lets draw pixel
					u8* tileset = memory_module::get_memory(tileset_addr + (tile_id * tilesize) + (tile_y_pixel * 2), true);
					u8 dataA = tileset[0];
					u8 dataB = tileset[1];
					u8 bit = 7 - tile_x_pixel; // the bits and pixels are inversed
					u8 palette_color = ((dataA & (1 << bit)) >> bit) | (((dataB & (1 << bit)) >> bit) << 1);

					u32 color = gpu::get_palette_color(palette_color);

					u32 pixel_index = (*scanline) * 160 + pixel;
					bg_palette_indices[pixel_index] = palette_color; // store palette index for priority
					u32 pixel_pos = pixel_index * 4; // the pixel we are drawing * 4 bytes per pixel
					framebuffer[pixel_pos++] = (color >> 24) & 0xFF;
					framebuffer[pixel_pos++] = (color >> 16) & 0xFF;
					framebuffer[pixel_pos++] = (color >> 8) & 0xFF;
					framebuffer[pixel_pos++] = 0xFF;
				}
			}

			// Increment window line counter only if window was visible on this scanline
			if (window_visible_this_line)
			{
				window_scanline_counter++;
			}

			return 0;
		}

		int draw_sprites()
		{
			if (get_lcd_control_flag(FLAG_LCD_DISPLAY_ENABLED) == false)
			{
				return 0;
			}

			if (get_lcd_control_flag(FLAG_OBJ_DISPLAY_ENABLED) == false)
			{
				return 0; // dont render sprites with obj disabled
			}

			// Clear sprite priority tracking for this scanline
			memset(sprite_pixels, 0, sizeof(sprite_pixels));

			u8 sprite_height = (get_lcd_control_flag(FLAG_OBJ_SIZE) == 0 ? 8 : 16);
			u8* sprite_ptr = sprite_attr;
			u8 sprite_count = 0;
			u8 sprite_line_count = 0;
			u8* sprites_on_line[10];

			// collect a ptr to first 10 sprites on scanline
			while (sprite_count < 40 && sprite_line_count < 10)
			{
				u8* current_sprite = sprite_ptr;
				u8 y_pos = *(sprite_ptr++) - 16;
				sprite_ptr += 3; // Skip X, tile ID, attr for now

				// Check if sprite is on this scanline
				if (*scanline >= y_pos && *scanline < y_pos + sprite_height)
				{
					sprites_on_line[sprite_line_count++] = current_sprite;
				}

				sprite_count++;
			}

			// sort by x coord. lower x has higher prio
			for (u8 i = 0; i < sprite_line_count - 1; i++)
			{
				for (u8 j = i + 1; j < sprite_line_count; j++)
				{
					u8 x_i = sprites_on_line[i][1]; // X is at offset 1
					u8 x_j = sprites_on_line[j][1];

					// Sort by X coordinate (lower X wins)
					if (x_j < x_i)
					{
						u8* temp = sprites_on_line[i];
						sprites_on_line[i] = sprites_on_line[j];
						sprites_on_line[j] = temp;
					}
				}
			}

			s32 tile_size = 16; // each tile is 16 bytes. 2 x 8 rows of a tile
			for (u8 i = 0; i < sprite_line_count; i++)
			{
				u8* sprite_ptr = sprites_on_line[i];
				u8 y_pos = *(sprite_ptr++) - 16;
				u8 x_pos = *(sprite_ptr++) - 8;
				u8 tile_id = *(sprite_ptr++);
				u8 attr = *(sprite_ptr++);
				u8 sprite_prio = get_sprite_attribute(attr, FLAG_SPRITE_PRIORITY);

				// For 8x16 sprites, ignore LSB of tile ID
				if (sprite_height == 16)
				{
					tile_id &= 0xFE; // Force bit 0 to 0
				}

				// check if scanline within y_min y_max
				if (*scanline >= y_pos && *scanline < y_pos + sprite_height)
				{
					s16 tile_y = *scanline - y_pos;

					if (get_sprite_attribute(attr, FLAG_SPRITE_FLIP_Y))
					{
						tile_y = sprite_height - 1 - tile_y;
					}

					// For 8x16 sprites, bottom half uses next tile
					u8 current_tile = tile_id;
					if (sprite_height == 16 && tile_y >= 8)
					{
						current_tile = tile_id | 0x01; // Bottom tile (tile_id + 1)
						tile_y -= 8; // Offset within the bottom tile
					}

					u8* tileset = memory_module::get_memory(0x8000 + (current_tile * tile_size) + (tile_y * 2));
					u8 data_a = tileset[0];
					u8 data_b = tileset[1];

					// render the 8 pixels of the tiles scanline
					for (u8 pixel = 0; pixel < 8; pixel++)
					{
						s16 screen_x = x_pos + pixel;

						// check screen bounds
						if (screen_x < 0 || screen_x >= 160)
						{
							continue;
						}

						// sprite-to-sprite priority: earlier sprites have priority
						if (sprite_pixels[screen_x])
						{
							continue;
						}

						s8 bit = 7 - pixel;

						if (get_sprite_attribute(attr, FLAG_SPRITE_FLIP_X))
						{
							bit -= 7;
							bit *= -1;
						}

						u8 palette_color = ((data_a & (1 << bit)) >> bit) | (((data_b & (1 << bit)) >> bit) << 1);

						if (palette_color == 0x0)
						{
							// pixel is transparent
							continue;
						}

						u8 palette = get_sprite_attribute(attr, FLAG_SPRITE_PALETTE);

						if (palette == 0)
						{
							palette = memory_module::read_memory(0xFF48, true);
						}
						else
						{
							palette = memory_module::read_memory(0xFF49, true);
						}

						u32 color = gpu::get_palette_color(palette_color, palette);
						u32 pixel_index = (*scanline) * 160 + x_pos + pixel;
						u32 pixel_pos = pixel_index * 4; // the pixel we are drawing * 4 bytes per pixel

						if (sprite_prio == 1) // check bg color
						{
							// if colors match then priority is on bg
							if (bg_palette_indices[pixel_index] != 0)
							{
								continue;
							}
						}

						framebuffer[pixel_pos++] = (color >> 24) & 0xFF;
						framebuffer[pixel_pos++] = (color >> 16) & 0xFF;
						framebuffer[pixel_pos++] = (color >> 8) & 0xFF;
						framebuffer[pixel_pos++] = 0xFF;

						sprite_pixels[screen_x] = true; // mark pixel as drawn
					}
				}
			}

			return 0;
		}

		int increment_scanline()
		{
			if (*scanline < 144)
			{
				draw_scanline();
				draw_sprites();
			}

			(*scanline)++; // inc scanline interrupt

			if (*coincidence_scanline == *scanline)
			{
				if (get_lcd_interrupt_flag(FLAG_COINCIDENCE))
				{
					cpu::set_request_interrupt_flag(cpu::INTERRUPT_LCD);
				}
			}

			return 0;
		}

		int switch_lcd_mode(u8 lcd_mode)
		{
			set_lcd_status_mode(lcd_mode);
			scanline_inc = false;

			switch (lcd_mode)
			{
			case MODE_HBLANK:
				memory_module::set_memory_access(memory_module::MEMORY_OAM, 0x3);
				memory_module::set_memory_access(memory_module::MEMORY_VRAM, 0x3);

				if (get_lcd_interrupt_flag(FLAG_HBLANK))
				{
					cpu::set_request_interrupt_flag(cpu::INTERRUPT_LCD);
				}

				horz_cycle_count += 204;
				break;
			case MODE_VBLANK:
				memory_module::set_memory_access(memory_module::MEMORY_OAM, 0x3);
				memory_module::set_memory_access(memory_module::MEMORY_VRAM, 0x3);

				// draw the scan line
				draw_scanline();
				//draw_sprites();

				cpu::set_request_interrupt_flag(cpu::INTERRUPT_VBLANK);

				if (get_lcd_interrupt_flag(FLAG_VBLANK))
				{
					cpu::set_request_interrupt_flag(cpu::INTERRUPT_LCD);
				}

				horz_cycle_count += 456;
				vblank_occurred = true;
				break;
			case MODE_OAM_ACCESS:
				memory_module::set_memory_access(memory_module::MEMORY_OAM, 0);

				if (get_lcd_interrupt_flag(FLAG_OAM_ACCESS))
				{
					cpu::set_request_interrupt_flag(cpu::INTERRUPT_LCD);
				}

				horz_cycle_count += 80;
				break;
			case MODE_VRAM_ACCESS:
				memory_module::set_memory_access(memory_module::MEMORY_OAM, 0);
				memory_module::set_memory_access(memory_module::MEMORY_VRAM, 0);

				horz_cycle_count += 172;
				break;
			}

			return 0;
		}

		int update_lcd_scanline_lcd_enabling()
		{
			u8 lcd_mode = get_lcd_status_mode();
			bool req_lcd_interrupt = false;

			switch (lcd_mode)
			{
			case MODE_HBLANK:
				if (horz_cycle_count < 0)
				{
					memory_module::set_memory_access(memory_module::MEMORY_OAM, 0);
					memory_module::set_memory_access(memory_module::MEMORY_VRAM, 0);

					set_lcd_status_mode(MODE_VRAM_ACCESS);
					horz_cycle_count += 172;
				}
				break;
			case MODE_VRAM_ACCESS:
				if (horz_cycle_count < 0)
				{
					lcd_enabling = false;
					switch_lcd_mode(MODE_HBLANK);
				}
				break;
			case MODE_VBLANK:
			case MODE_OAM_ACCESS:
				assert("lcd mode should not be set when enabling");
				break;
			}

			return 0;
		}

		int update_lcd_scanline()
		{
			u8 lcd_mode = get_lcd_status_mode();
			bool req_lcd_interrupt = false;

			switch (lcd_mode)
			{
			case MODE_HBLANK:
				if (!scanline_inc && horz_cycle_count <= 0)
				{
					// draw the scan line
					increment_scanline();
					scanline_inc = true;
					memory_module::set_memory_access(memory_module::MEMORY_OAM, 0);
				}

				if (horz_cycle_count < 0)
				{
					switch_lcd_mode(MODE_OAM_ACCESS);
				}
				break;
			case MODE_VBLANK:
				if (horz_cycle_count < 0) // restart screen refresh
				{
					if (*scanline < 153)
					{
						increment_scanline();
					}
					else
					{
						*scanline = 0;
						window_scanline_counter = 0; // reset window scanline
						switch_lcd_mode(MODE_OAM_ACCESS);
					}

					horz_cycle_count += 456;
				}
				break;
			case MODE_OAM_ACCESS:
				if (horz_cycle_count <= 0)
				{
					memory_module::set_memory_access(memory_module::MEMORY_VRAM, 0);
				}

				if (horz_cycle_count < 0)
				{
					switch_lcd_mode(MODE_VRAM_ACCESS);
				}
				break;
			case MODE_VRAM_ACCESS:
				if (horz_cycle_count < 0)
				{
					if (*scanline < 143)
					{
						switch_lcd_mode(MODE_HBLANK);
					}
					else // enter vblank
					{
						switch_lcd_mode(MODE_VBLANK);
					}
				}
				break;
			}

			return 0;
		}
		
		int update(u8 cycles)
		{
			if (get_lcd_control_flag(FLAG_LCD_DISPLAY_ENABLED) == false)
			{
				if (lcd_enabled)
				{
					u8* framebuffer_ptr = (u8*)framebuffer;
					u32 color = gpu::get_palette_color(0, 0);

					for (u32 i = 0; i < (width * height); i++)
					{
						*framebuffer_ptr++ = (color >> 24) & 0xFF;
						*framebuffer_ptr++ = (color >> 16) & 0xFF;
						*framebuffer_ptr++ = (color >> 8) & 0xFF;
						*framebuffer_ptr++ = 0xFF;
					}
				}

				lcd_enabled = false;
				set_lcd_status_mode(MODE_HBLANK);
				*scanline = 0;
				window_scanline_counter = 0;

				return 0;
			}

			if (!lcd_enabled)
			{
				// lcd being re enabled. reset scanline and horz cycle count. lcd mode set to hblank
				lcd_enabled = true;
				lcd_enabling = true;
				*scanline = 0;
				window_scanline_counter = 0;
				horz_cycle_count = 68;

				set_lcd_status_mode(MODE_HBLANK);
			}
			else
			{
				horz_cycle_count -= cycles;
			}

			if (lcd_enabling)
			{
				update_lcd_scanline_lcd_enabling();
			}
			else
			{
				update_lcd_scanline();
			}

			return 0;
		}

		void check_coincidence_flag()
		{
			if (*coincidence_scanline != *scanline)
			{
				*lcd_status &= ~(1 << 2); // clear bit 2 for coincidence
			}

			if (scanline_inc)
			{
				return;
			}

			// check for coincidence flag
			if (*coincidence_scanline == *scanline)
			{
				*lcd_status |= (1 << 2); // set bit 2 for coincidence
			}
		}
	}
}