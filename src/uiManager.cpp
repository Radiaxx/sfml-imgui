#include <imgui.h>

#include "geoData.hpp"
#include "uiManager.hpp"

static ImVec4 toImVec4FromColor(sf::Color color)
{
    return ImVec4(color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f);
}

static sf::Color fromImVec4ToColor(const ImVec4& vec)
{
    auto toU8 = [](float x) -> std::uint8_t { return static_cast<std::uint8_t>(std::roundf(x * 255.0f)); };

    return sf::Color(toU8(vec.x), toU8(vec.y), toU8(vec.z), toU8(vec.w));
}

void UIManager::draw(Heatmap& heatmap, GeoData& geoData, GridOverlay& gridOverlay)
{
    const float          initialWidth = 380.0f;
    const ImGuiViewport* viewport     = ImGui::GetMainViewport();
    ImVec2               window_pos(viewport->WorkPos.x + viewport->WorkSize.x, viewport->WorkPos.y);
    ImVec2               window_pos_pivot(1.0f, 0.0f);

    ImGui::SetNextWindowPos(window_pos, ImGuiCond_None, window_pos_pivot);
    ImGui::SetNextWindowSize(ImVec2(initialWidth, 0.0f), ImGuiCond_None);
    ImGui::SetNextWindowBgAlpha(0.75f);

    ImGui::Begin("Control Panel");

    ImGui::Text("Dataset Selection");
    const auto& dataFiles         = heatmap.getDataFiles();
    int         selectedFileIndex = heatmap.getSelectedFileIndex();
    const char* combo_preview_value = (selectedFileIndex >= 0) ? dataFiles[selectedFileIndex].c_str() : "Select a file...";

    if (ImGui::BeginCombo("Dataset", combo_preview_value))
    {
        for (int i = 0; i < static_cast<int>(dataFiles.size()); ++i)
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

    // Display geo data info and category toggles
    if (geoData.getGeoData())
    {
        // Display loaded geo data info
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::Text("Geo Data Info:");
        ImGui::Text("  - Entities: %zu", geoData.getGeoData()->getEntities().size());
        ImGui::Text("  - Life Range: %.3f / %.3f", geoData.getLifeMin(), geoData.getLifeMax());

        // Display life filter
        float lifeMin = static_cast<float>(geoData.getLifeFilterMin());
        float lifeMax = static_cast<float>(geoData.getLifeFilterMax());
        ImGui::Text("Filter by life:");
        if (ImGui::DragFloatRange2("life range",
                                   &lifeMin,
                                   &lifeMax,
                                   1.0f,
                                   static_cast<float>(geoData.getLifeMin()),
                                   static_cast<float>(geoData.getLifeMax())))
        {
            geoData.setLifeFilterRange(lifeMin, lifeMax);
        }

        // Display mode (lines or areas)
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        ImGui::Text("Display Mode:");

        int displayMode = (geoData.getDisplayMode() == GeoData::DisplayMode::Lines) ? 0 : 1;
        if (ImGui::RadioButton("Lines", displayMode == 0))
        {
            displayMode = 0;
            geoData.setDisplayMode(GeoData::DisplayMode::Lines);
        }

        ImGui::SameLine();
        if (ImGui::RadioButton("Areas", displayMode == 1))
        {
            displayMode = 1;
            geoData.setDisplayMode(GeoData::DisplayMode::Areas);
        }

        // Entity toggles
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        ImGui::Text("Entity Selection");

        // Points
        ImGui::Text("Points");
        bool isMaximumVisible = geoData.getShowMaximum();
        bool isMinimumVisible = geoData.getShowMinimum();
        bool isSaddlesVisible = geoData.getShowSaddles();

        bool allPoints = (isMaximumVisible && isMinimumVisible && isSaddlesVisible);
        if (ImGui::Checkbox("All points", &allPoints))
        {
            geoData.setShowMaximum(allPoints);
            geoData.setShowMinimum(allPoints);
            geoData.setShowSaddles(allPoints);
            isMaximumVisible = isMinimumVisible = isSaddlesVisible = allPoints;
        }

        const std::string maximumCheckBoxText = std::string("Maximum (") + std::to_string(geoData.maximumInRange().size()) +
                                                " / " + std::to_string(geoData.maximum().size()) + ")";

        if (ImGui::Checkbox(maximumCheckBoxText.c_str(), &isMaximumVisible))
        {
            geoData.setShowMaximum(isMaximumVisible);
        }

        const std::string minimumCheckBoxText = std::string("Minimum (") + std::to_string(geoData.minimumInRange().size()) +
                                                " / " + std::to_string(geoData.minimum().size()) + ")";

        if (ImGui::Checkbox(minimumCheckBoxText.c_str(), &isMinimumVisible))
        {
            geoData.setShowMinimum(isMinimumVisible);
        }

        const std::string saddlesCheckBoxText = std::string("Saddle (") + std::to_string(geoData.saddlesInRange().size()) +
                                                " / " + std::to_string(geoData.saddles().size()) + ")";

        if (ImGui::Checkbox(saddlesCheckBoxText.c_str(), &isSaddlesVisible))
        {
            geoData.setShowSaddles(isSaddlesVisible);
        }

        // Lines (disabled if Areas mode)
        const bool linesDisabled = (geoData.getDisplayMode() == GeoData::DisplayMode::Areas);
        if (linesDisabled)
        {
            ImGui::BeginDisabled();
        }

        ImGui::Spacing();
        ImGui::Text("Lines");
        bool isLinesAscending  = geoData.getShowLinesAscending();
        bool isLinesDescending = geoData.getShowLinesDescending();

        const std::string ascCheckBoxText = std::string("Ascending (") +
                                            std::to_string(geoData.linesAscendingInRange().size()) + " / " +
                                            std::to_string(geoData.linesAscending().size()) + ")";

        if (ImGui::Checkbox(ascCheckBoxText.c_str(), &isLinesAscending))
        {
            geoData.setShowLinesAscending(isLinesAscending);
        }

        const std::string descCheckBoxText = std::string("Descending (") +
                                             std::to_string(geoData.linesDescendingInRange().size()) + " / " +
                                             std::to_string(geoData.linesDescending().size()) + ")";

        if (ImGui::Checkbox(descCheckBoxText.c_str(), &isLinesDescending))
        {
            geoData.setShowLinesDescending(isLinesDescending);
        }

        if (linesDisabled)
        {
            ImGui::EndDisabled();
        }

        // Areas (disabled if Lines mode)
        const bool areasDisabled = (geoData.getDisplayMode() == GeoData::DisplayMode::Lines);
        if (areasDisabled)
        {
            ImGui::BeginDisabled();
        }

        ImGui::Spacing();
        ImGui::Text("Areas");

        bool              showAreas         = geoData.getShowAreas();
        const std::string areasCheckBoxText = std::string("Areas (") + std::to_string(geoData.areasInRange().size()) +
                                              " / " + std::to_string(geoData.areas().size()) + ")";

        if (ImGui::Checkbox(areasCheckBoxText.c_str(), &showAreas))
        {
            geoData.setShowAreas(showAreas);
        }

        if (areasDisabled)
        {
            ImGui::EndDisabled();
        }

        // Display colors
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        ImGui::Text("Colors");

        ImVec4 maximumColor = toImVec4FromColor(geoData.getMaximumColor());
        if (ImGui::ColorEdit4("Maximum", &maximumColor.x))
        {
            geoData.setMaximumColor(fromImVec4ToColor(maximumColor));
        }

        ImVec4 minimumColor = toImVec4FromColor(geoData.getMinimumColor());
        if (ImGui::ColorEdit4("Minimum", &minimumColor.x))
        {
            geoData.setMinimumColor(fromImVec4ToColor(minimumColor));
        }

        ImVec4 saddlesColor = toImVec4FromColor(geoData.getSaddlesColor());
        if (ImGui::ColorEdit4("Saddles", &saddlesColor.x))
        {
            geoData.setSaddlesColor(fromImVec4ToColor(saddlesColor));
        }

        ImVec4 lineAscColor = toImVec4FromColor(geoData.getLineAscColor());
        if (ImGui::ColorEdit4("Line Ascending", &lineAscColor.x))
        {
            geoData.setLineAscColor(fromImVec4ToColor(lineAscColor));
        }

        ImVec4 lineDescColor = toImVec4FromColor(geoData.getLineDescColor());
        if (ImGui::ColorEdit4("Line Descending", &lineDescColor.x))
        {
            geoData.setLineDescColor(fromImVec4ToColor(lineDescColor));
        }

        ImVec4 areasColor = toImVec4FromColor(geoData.getAreasColor());
        if (ImGui::ColorEdit4("Areas", &areasColor.x))
        {
            geoData.setAreasColor(fromImVec4ToColor(areasColor));
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        ImGui::Text("Points Scaling");

        // Points: size scaling toggle + ranges
        bool pointSizeScale = geoData.getPointSizeScaleByLife();
        if (ImGui::Checkbox("Scale marker size by life", &pointSizeScale))
        {
            geoData.setPointSizeScaleByLife(pointSizeScale);
        }

        float pointSizeBase = geoData.getPointSizeBase();
        if (ImGui::DragFloat("Base size", &pointSizeBase, 0.1f, 1.0f, 64.0f))
        {
            geoData.setPointSizeBase(pointSizeBase);
        }

        float pointSizeMin = geoData.getPointSizeMin();
        float pointSizeMax = geoData.getPointSizeMax();
        if (ImGui::DragFloatRange2("Size range", &pointSizeMin, &pointSizeMax, 0.1f, 1.0f, 64.0f))
        {
            geoData.setPointSizeRange(pointSizeMin, pointSizeMax);
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        ImGui::Text("Lines/Areas Scaling");

        // Lines/Areas: choose color route (brightness). One toggle applies to whichever is visible now.
        bool isLineColorScaledByLife = geoData.getLineColorScaleByLife();
        if (ImGui::Checkbox("Scale color BRIGHTNESS by life", &isLineColorScaledByLife))
        {
            geoData.setLineColorScaleByLife(isLineColorScaledByLife);
        }

        // TODO: Choose if use thick lines or mono pixel ones
        // float defaultLineThickness = geoData.getLineThicknessBase();
        // if (ImGui::DragFloat("Base Thickness", &defaultLineThickness, 1.0f, 1.0f, 32.0f))
        // {
        //     geoData.setLineThicknessBase(defaultLineThickness);
        // }
    }
    else
    {
        ImGui::TextDisabled("Load geo data to enable controls below.");
    }


    ImGui::End();
}

void UIManager::setView(sf::View view)
{
    m_view = view;
}