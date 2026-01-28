#pragma once

#include "defines.h"

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>
#include <imgui.h>
#include <imgui-SFML.h>

// start with some helper functions
namespace gameboy
{
    namespace debugger
    {
        void debugger_colored_text(const char* text, ImVec4 backgroundColor, ImVec4 textColor)
        {
            // 1. Calculate the size of the text
            ImVec2 textSize = ImGui::CalcTextSize(text);

            // Optional: Add padding if desired (e.g., style.FramePadding * 2)
            // Here we will use 0 padding for simplicity.

            // 2. Get current cursor position and draw list
            ImVec2 textPos = ImGui::GetCursorScreenPos();
            ImDrawList* drawList = ImGui::GetWindowDrawList();

            // 3. Draw the background filled rectangle *before* the text
            // The min point is the current position, max point is current position + text size
            drawList->AddRectFilled(textPos, ImVec2(textPos.x + textSize.x, textPos.y + textSize.y), ImGui::ColorConvertFloat4ToU32(backgroundColor));

            // 4. Render the text on top
            // Push the text color style
            ImGui::PushStyleColor(ImGuiCol_Text, textColor);
            // Move the cursor back to the original position so Text() draws on top of the rect
            ImGui::SetCursorScreenPos(textPos);
            ImGui::Text("%s", text);
            ImGui::PopStyleColor();
        }

        bool debugger_panel_begin(const char* title, ImVec2& size)
        {
            // draw emulator display with imgui layout
            ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoResize |
                ImGuiWindowFlags_NoMove |
                ImGuiWindowFlags_NoCollapse |
                ImGuiWindowFlags_NoScrollbar;

            float headerHeight = ImGui::GetFrameHeight() + ImGui::GetStyle().ItemSpacing.y * 2;
            float separatorHeight = 1.0f;
            ImVec2 windowPadding = ImGui::GetStyle().WindowPadding;
            float childBorderSize = ImGui::GetStyle().ChildBorderSize;

            float totalWidth = size.x + (windowPadding.x * 2) + (childBorderSize * 2);
            float totalHeight = size.y + headerHeight + separatorHeight + (windowPadding.y * 2) + (childBorderSize * 2);

            if (ImGui::BeginChild(title, ImVec2(totalWidth, totalHeight), true, window_flags))
            {
                // Get current cursor position and available width for header background
                ImVec2 startPos = ImGui::GetCursorScreenPos();
                float availableWidth = ImGui::GetContentRegionAvail().x;
                float textHeight = ImGui::GetFrameHeight();
                
                // Draw grey background for header
                ImU32 headerBgColor = ImGui::GetColorU32(ImGuiCol_TitleBgActive);
                ImDrawList* drawList = ImGui::GetWindowDrawList();
                drawList->AddRectFilled(
                    startPos,
                    ImVec2(startPos.x + availableWidth, startPos.y + textHeight),
                    headerBgColor
                );
                
                // Draw header text on top of background
                ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8, 4));
                ImVec2 textSize = ImGui::CalcTextSize(title);
                float textOffsetY = (textHeight - textSize.y) * 0.5f;
                
                ImGui::SetCursorPosY(ImGui::GetCursorPosY() + textOffsetY);
                ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), title);
                
                ImGui::PopStyleVar();
                
                // Move cursor to after the header
                ImGui::SetCursorScreenPos(ImVec2(startPos.x, startPos.y + textHeight));

                ImGui::Separator();
                ImGui::Spacing();

                return true;
            }

            return false;
        }

        void debugger_panel_end()
        {
            ImGui::EndChild();
        }
    }
}
