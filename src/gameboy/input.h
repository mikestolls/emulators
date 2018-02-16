#pragma once

namespace gameboy
{
	namespace cpu
	{
		extern void request_joypad_interrupt();
	}

	u8 input_buttons = 0xFF;
	u8 input_directional = 0xFF;

	// set and get input helpers
	inline void set_button_pressed(u8 button, bool is_directional)
	{
		u8* input = 0;
		if (is_directional)
		{
			input = &input_directional;
		}
		else
		{
			input = &input_buttons;
		}

		u8 temp = *input;
		*input &= ~(1 << button);

		if ((temp & 0xF) != (*input & 0xF))
		{
			// input changed
			cpu::request_joypad_interrupt();
		}
	}

	inline void set_button_released(u8 button, bool is_directional)
	{
		if (is_directional)
		{
			input_directional |= (1 << button);
		}
		else
		{
			input_buttons |= (1 << button);
		}
	}
	
	inline u8 get_button_state(u8 button, bool is_directional)
	{
		if (is_directional)
		{
			return ((input_directional & (1 << button)) >> button);
		}
		else
		{
			return ((input_buttons & (1 << button)) >> button);
		}
	}

	inline u8 get_button_register(bool is_directional)
	{
		if (is_directional)
		{
			return input_directional;
		}
		else
		{
			return input_buttons;
		}
	}

	enum BUTTONS
	{
		BUTTON_A,
		BUTTON_B,
		BUTTON_SELECT,
		BUTTON_START,
	};
	
	enum DIRECTION
	{
		DIRECTION_RIGHT,
		DIRECTION_LEFT,
		DIRECTION_UP,
		DIRECTION_DOWN,
	};
}