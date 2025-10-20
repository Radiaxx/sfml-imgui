#include "uiManager.hpp"
#include <imgui.h>

void UIManager::draw(Heatmap& heatmap, GeoData& geoData, GridOverlay& gridOverlay)
{
    const float          initialWidth = 350.0f;
    const ImGuiViewport* viewport     = ImGui::GetMainViewport();
    ImVec2               window_pos(viewport->WorkPos.x + viewport->WorkSize.x, viewport->WorkPos.y);
    ImVec2               window_pos_pivot(1.0f, 0.0f); // Pivot to top right

    ImGui::SetNextWindowPos(window_pos, ImGuiCond_None, window_pos_pivot);
    ImGui::SetNextWindowSize(ImVec2(initialWidth, 0.0f), ImGuiCond_None);
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
                    heatmap.updateHeatmapView(m_view);
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

        const auto& header         = heatmap.getAscData()->getHeader();
        const auto  globalMinValue = heatmap.getAscData()->getMinValue();
        const auto  globalMaxValue = heatmap.getAscData()->getMaxValue();

        ImGui::Text("  - Dimensions: %d x %d", header.ncols, header.nrows);
        ImGui::Text("  - Cell Size: %.3f", header.cellsize);
        ImGui::Text("  - Min/Max: %.3f / %.3f", globalMinValue, globalMaxValue);

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::Text("Settings:");

        bool showValues = gridOverlay.isShowingValues();
        if (ImGui::Checkbox("Show Values", &showValues))
        {
            gridOverlay.setShowValues(showValues);
        }

        bool isAuto = heatmap.isAutoClamping();
        if (ImGui::Checkbox("Auto clamp min/max to View", &isAuto))
        {
            heatmap.setAutoClamp(isAuto);
        }

        // Disable the manual slider if auto clamping is on
        if (isAuto)
        {
            ImGui::BeginDisabled();
        }

        float           minVal    = heatmap.getManualClampMin();
        float           maxVal    = heatmap.getManualClampMax();
        constexpr float dragSpeed = 1.0f;

        ImGui::DragFloatRange2("min/max Range", &minVal, &maxVal, dragSpeed, heatmap.getGlobalMin(), heatmap.getGlobalMax());

        if (minVal != heatmap.getManualClampMin() || maxVal != heatmap.getManualClampMax())
        {
            if (minVal < globalMinValue)
            {
                minVal = globalMinValue;
            }

            if (maxVal > globalMaxValue)
            {
                maxVal = globalMaxValue;
            }

            heatmap.setManualClampRange(minVal, maxVal);
        }

        if (isAuto)
        {
            ImGui::EndDisabled();
        }
    }
    else
    {
        ImGui::TextDisabled("Load data to enable clamping.");
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

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Display geo data selection
    ImGui::Text("Geo Data Selection");

    const auto& geoFiles             = geoData.getGeoFiles();
    int         selectedGeoFileIndex = geoData.getSelectedFileIndex();
    const char* geo_combo_preview    = (selectedGeoFileIndex >= 0) ? geoFiles[selectedGeoFileIndex].c_str()
                                                                   : "Select a geo data file...";

    if (ImGui::BeginCombo("Geo Data", geo_combo_preview))
    {
        for (int i = 0; i < static_cast<int>(geoFiles.size()); ++i)
        {
            const bool is_selected = (selectedGeoFileIndex == i);

            if (ImGui::Selectable(geoFiles[i].c_str(), is_selected))
            {
                if (selectedGeoFileIndex != i)
                {
                    geoData.loadData(i);
                }
            }

            if (is_selected)
            {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }

    // Display geo data info and category toggles (no rendering yet)
    if (geoData.getGeoData())
    {
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::Text("Geo Data Info:");
        ImGui::Text("  - Entities: %zu", geoData.getGeoData()->getEntities().size());
        ImGui::Text("  - Life Range: %.3f / %.3f", geoData.getLifeMin(), geoData.getLifeMax());

        ImGui::Spacing();
        ImGui::Text("Categories:");
        bool value;

        value = geoData.getShowMaxima();
        if (ImGui::Checkbox(("Maxima (" + std::to_string(geoData.maxima().size()) + ")").c_str(), &value))
        {
            geoData.setShowMaxima(value);
        }

        value = geoData.getShowMinima();
        if (ImGui::Checkbox(("Minima (" + std::to_string(geoData.minima().size()) + ")").c_str(), &value))
        {
            geoData.setShowMinima(value);
        }

        value = geoData.getShowSaddles();
        if (ImGui::Checkbox(("Saddles (" + std::to_string(geoData.saddles().size()) + ")").c_str(), &value))
        {
            geoData.setShowSaddles(value);
        }

        value = geoData.getShowLinesAscending();
        if (ImGui::Checkbox(("Lines Asc (" + std::to_string(geoData.linesAscending().size()) + ")").c_str(), &value))
        {
            geoData.setShowLinesAscending(value);
        }

        value = geoData.getShowLinesDescending();
        if (ImGui::Checkbox(("Lines Desc (" + std::to_string(geoData.linesDescending().size()) + ")").c_str(), &value))
        {
            geoData.setShowLinesDescending(value);
        }

        value = geoData.getShowAreas();
        if (ImGui::Checkbox(("Areas (" + std::to_string(geoData.areas().size()) + ")").c_str(), &value))
        {
            geoData.setShowAreas(value);
        }
    }
    else
    {
        ImGui::TextDisabled("Load geo data to enable categories.");
    }

    ImGui::End();
}

void UIManager::setView(sf::View view)
{
    m_view = view;
}