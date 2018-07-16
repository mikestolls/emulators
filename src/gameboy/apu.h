#pragma once

#include "defines.h"

#include "memory_module.h"

#include <portaudio/portaudio.h>

#define _USE_MATH_DEFINES
#include <math.h>

namespace gameboy
{
	namespace apu
	{
		struct sound_channel
		{
			u8* on_off;
			u8* control;
			u8* frequency;
			u8* volume;
			u8* length;
			u8* sweep;
		};

		enum SOUND_CHANNELS
		{
			CHANNEL_PULSE_A = 0,
			CHANNEL_PULSE_B,
			CHANNEL_WAVE,
			CHANNEL_NOISE,
			CHANNEL_COUNT
		};

		bool is_started = false;
		std::atomic<bool> tick_frame;

		// gameboy channels and control info
		sound_channel channels[CHANNEL_COUNT];
		u8* control;
		u8* balance;
		u8* power;

		// port audio
		PaHostApiIndex audio_host;
		PaDeviceIndex audio_device;
		PaStream* audio_stream;

		u32 sample_rate;
		u32 num_samples_per_frame;
		u32 num_cycles_per_sample;
		s32 cycle_count;
		s32 cycle_count_frame;
		u32 fill_buffer_pos;
		bool is_sound_on;
		s16 volume;

		// sound buffers
		s16* sound_buffer = 0;
		s16* play_buffer;
		s16* fill_buffer;

		inline s16 generate_sample();

		auto cur_time = std::chrono::high_resolution_clock::now();
		auto last_time = cur_time;

		static int audio_callback(const void *inputBuffer, void *outputBuffer, unsigned long framesPerBuffer, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void *userData)
		{
			s16 *out = (s16*)outputBuffer;

			s16* buffer = play_buffer;
			for (unsigned int i = 0; i < framesPerBuffer; i++)
			{
				*out++ = (*buffer)++;
				buffer++;
			}

			return 0;
		}

		int initialize_audio_device()
		{
			// initialize port audio
			PaError err;
			err = Pa_Initialize();

			if (err != paNoError)
			{
				return -1;
			}

			audio_host = Pa_GetDefaultHostApi();
			audio_device = Pa_GetDefaultOutputDevice();

			// Output information about the audio system
			const PaHostApiInfo* hostInfo = Pa_GetHostApiInfo(audio_host);
			const PaDeviceInfo* deviceInfo = Pa_GetDeviceInfo(audio_device);
			sample_rate = (int)deviceInfo->defaultSampleRate;

			num_samples_per_frame = sample_rate / cpu::fps;
			num_cycles_per_sample = cpu::num_cycles_per_frame / num_samples_per_frame;
			cycle_count = num_cycles_per_sample;
			cycle_count_frame = cpu::num_cycles_per_frame;
			fill_buffer_pos = 0;

			printf("Audio host: %s\n", hostInfo->name);
			printf("Audio device: %s\n", deviceInfo->name);
			printf("rate: %g\n", deviceInfo->defaultSampleRate);
			printf("latency: %g\n", deviceInfo->defaultLowOutputLatency);

			PaStreamParameters output;
			output.channelCount = 1;
			output.device = audio_device;
			output.hostApiSpecificStreamInfo = nullptr;
			output.sampleFormat = paInt16;
			output.suggestedLatency = deviceInfo->defaultLowInputLatency;

			err = Pa_OpenStream(&audio_stream,
				nullptr,
				&output,
				deviceInfo->defaultSampleRate,
				num_samples_per_frame,
				0,
				&audio_callback,
				0);

			err = Pa_StartStream(audio_stream);

			if (err != paNoError)
			{
				return -1;
			}

			tick_frame = true;
			is_started = true;

			return 0;
		}

		enum POWER_FLAG
		{
			POWER_SOUND1 = 0,
			POWER_SOUND2,
			POWER_SOUND3,
			POWER_SOUND4,
			POWER_SOUNDALL = 7,
		};

		inline void set_power_flag(u8 flag)
		{
			flag = (1 << flag);
			*power |= flag;
		}

		inline void clear_power_flag(u8 flag)
		{
			flag = (1 << flag);
			*power &= ~flag; // clear the bit
		}

		inline u8 get_power_flag(u8 flag)
		{
			return ((*power & (1 << flag)) >> flag);
		}

		s32 frequency = 3000;
		float time = 0.0f;
		inline s16 generate_sample()
		{
			float time_inc = (float)(frequency / cpu::fps) / num_samples_per_frame;

			float val = sgn<float>(sinf(time * 2 * (float)M_PI));
			time += time_inc;

			if (time >= 1.0f)
			{
				time -= 1.0f;
			}

			return (s32)(val * volume);
		}

		int reset()
		{
			if (is_started)
			{
				// terminate if its been created
				Pa_StopStream(audio_stream);
				Pa_CloseStream(audio_stream);
				Pa_Terminate();
			}

			initialize_audio_device();

			// initialize the apu
			channels[CHANNEL_PULSE_A].on_off = 0x0;
			channels[CHANNEL_PULSE_A].sweep = memory_module::get_memory(0xFF10);
			channels[CHANNEL_PULSE_A].length = memory_module::get_memory(0xFF11);
			channels[CHANNEL_PULSE_A].volume = memory_module::get_memory(0xFF12);
			channels[CHANNEL_PULSE_A].frequency = memory_module::get_memory(0xFF13);
			channels[CHANNEL_PULSE_A].control = memory_module::get_memory(0xFF14);

			channels[CHANNEL_PULSE_B].on_off = 0x0;
			channels[CHANNEL_PULSE_B].sweep = 0x0;
			channels[CHANNEL_PULSE_B].length = memory_module::get_memory(0xFF16);
			channels[CHANNEL_PULSE_B].volume = memory_module::get_memory(0xFF17);
			channels[CHANNEL_PULSE_B].frequency = memory_module::get_memory(0xFF18);
			channels[CHANNEL_PULSE_B].control = memory_module::get_memory(0xFF19);

			channels[CHANNEL_WAVE].on_off = memory_module::get_memory(0xFF1A);
			channels[CHANNEL_WAVE].sweep = 0x0;
			channels[CHANNEL_WAVE].length = memory_module::get_memory(0xFF1B);
			channels[CHANNEL_WAVE].volume = memory_module::get_memory(0xFF1C);
			channels[CHANNEL_WAVE].frequency = memory_module::get_memory(0xFF1D);
			channels[CHANNEL_WAVE].control = memory_module::get_memory(0xFF1E);

			channels[CHANNEL_NOISE].on_off = 0x0;
			channels[CHANNEL_NOISE].sweep = 0x0;
			channels[CHANNEL_NOISE].length = memory_module::get_memory(0xFF20);
			channels[CHANNEL_NOISE].volume = memory_module::get_memory(0xFF21);
			channels[CHANNEL_NOISE].frequency = memory_module::get_memory(0xFF22);
			channels[CHANNEL_NOISE].control = memory_module::get_memory(0xFF23);

			control = memory_module::get_memory(0xFF24);
			balance = memory_module::get_memory(0xFF25);
			power = memory_module::get_memory(0xFF26);

			is_sound_on = false;
			volume = 500;

			// initialize sounds buffer
			if (sound_buffer)
			{
				delete[] sound_buffer;
				sound_buffer = 0;
			}

			sound_buffer = new s16[num_samples_per_frame * 2]; // double buffer sound per frame
			memset(sound_buffer, 0x0, sizeof(s16) * num_samples_per_frame * 2);
			play_buffer = sound_buffer;
			fill_buffer = &sound_buffer[num_samples_per_frame];
			
			return 0;
		}

		int initialize()
		{
			reset();

			return 0;
		}

		int update(u8 cycles)
		{
			/*if (get_power_flag(POWER_SOUNDALL) == 0) // all sound off
			{
				volume = 0;
				is_sound_on = false;
				return 0;
			}

			if (!is_sound_on)
			{
				is_sound_on = true;
				volume = 1000;
			}

			// check which sound channels are on
			if ((*channels[CHANNEL_PULSE_A].control & 0x80) != 0)
			{
				return 0;
			}*/

			if (fill_buffer_pos < num_samples_per_frame)
			{
				cycle_count -= cycles;

				// still more smaples to fill for the frame
				if (cycle_count <= 0)
				{
					// put the next sample into the buffer
					fill_buffer[fill_buffer_pos++] = generate_sample();

					cycle_count += num_cycles_per_sample;
				}
			}

			cycle_count_frame -= cycles;
			if (cycle_count_frame < 0)
			{
				// swap the buffers for the next callback
				fill_buffer_pos = 0;
				std::swap(fill_buffer, play_buffer);
				cycle_count_frame += cpu::num_cycles_per_frame;
			}

			return 0;
		}
	}
}
