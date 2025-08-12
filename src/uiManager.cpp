#include <imgui.h>

#include "uiManager.hpp"

void UIManager::draw(Heatmap& heatmap)
{
    const float          initialWidth = 350.0f;
    const ImGuiViewport* viewport     = ImGui::GetMainViewport();
    ImVec2               window_pos(viewport->WorkPos.x + viewport->WorkSize.x, viewport->WorkPos.y);
    ImVec2               window_pos_pivot(1.0f, 0.0f); // Pivot to top right

    // ImGuiCond_FirstUseEver to make the ui position and size persistent
    ImGui::SetNextWindowPos(window_pos, ImGuiCond_FirstUseEver, window_pos_pivot);
    ImGui::SetNextWindowSize(ImVec2(initialWidth, 0.0f), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowBgAlpha(0.75f);

    ImGui::Begin("Control Panel");

    // File selection dropdown
    const auto& dataFiles         = heatmap.getDataFiles();
    int         selectedFileIndex = heatmap.getSelectedFileIndex();
    const char* combo_preview_value = (selectedFileIndex >= 0) ? dataFiles[selectedFileIndex].c_str() : "Select a file...";

    if (ImGui::BeginCombo("Dataset", combo_preview_value))
    {
        for (int i = 0; i < dataFiles.size(); ++i)
        {
            const bool is_selected = (selectedFileIndex == i);

            if (ImGui::Selectable(dataFiles[i].c_str(), is_selected))
            {
                if (selectedFileIndex != i)
                {
                    heatmap.loadData(i);
                }
            }

            if (is_selected)
            {
                ImGui::SetItemDefaultFocus();
            }
        }

        ImGui::EndCombo();
    }

    ImGui::Separator();

    // Display loaded file info
    if (heatmap.getAscData())
    {
        ImGui::Text("File Information:");

        const auto& header = heatmap.getAscData()->getHeader();
        ImGui::Text("  - Dimensions: %d x %d", header.ncols, header.nrows);
        ImGui::Text("  - Cell Size: %.4f", header.cellsize);
        ImGui::Text("  - Min/Max: %.2f / %.2f", heatmap.getAscData()->getMinValue(), heatmap.getAscData()->getMaxValue());
    }
    else
    {
        ImGui::Text("No data loaded.");
    }

    ImGui::End();
}