#pragma once

#include "defines.h"

#include <SFML/Graphics.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>
#include <imgui.h>
#include <imgui-SFML.h>

#include "common/debugger/debugger_helper.h"

namespace nes
{
	namespace debugger
	{
        namespace tileset
        {
            int init()
            {
                return 0;
            }

            int update()
            {
                return 0;
            }

            int draw(bool is_focused)
            {
                return 0;
            }

            int process_event(const sf::Event* event)
            {
                return 0;
            }
        }
	}
}
