#include <imgui.h>

#include "uiManager.hpp"

void UIManager::update(Heatmap& heatmap)
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
    ImGui::Text("Dataset Selection");

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

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Display loaded file info
    if (heatmap.getAscData())
    {
        ImGui::Text("File Information:");

        const auto& header = heatmap.getAscData()->getHeader();
        ImGui::Text("  - Dimensions: %d x %d", header.ncols, header.nrows);
        ImGui::Text("  - Cell Size: %.3f", header.cellsize);
        ImGui::Text("  - Min/Max: %.3f / %.3f", heatmap.getAscData()->getMinValue(), heatmap.getAscData()->getMaxValue());
    }
    else
    {
        ImGui::Text("No data loaded.");
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Display colormap selection
    ImGui::Text("Colormap Selection");

    const auto& colormapNames = Heatmap::COLORMAP_NAMES;
    int         colormapID    = heatmap.getCurrentColormapID();

    // Since colormapID is initialized to 0 this will always be valid
    const char* colormap_preview_value = colormapNames[colormapID].c_str();

    if (ImGui::BeginCombo("Colormap", colormap_preview_value))
    {
        for (int i = 0; i < colormapNames.size(); ++i)
        {
            const bool is_selected = (colormapID == i);
            if (ImGui::Selectable(colormapNames[i].c_str(), is_selected))
            {
                if (colormapID != i)
                {
                    heatmap.setCurrentColormapID(i);
                }
            }

            if (is_selected)
            {
                ImGui::SetItemDefaultFocus();
            }
        }

        ImGui::EndCombo();
    }

    ImGui::End();
}