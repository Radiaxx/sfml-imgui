#include <filesystem>
#include <iostream>

#include "heatmap.hpp"

Heatmap::Heatmap() : m_heatmapSprite(m_heatmapTexture)
{
    scanDataDirectory();

    const std::string shaderFolderPath  = SHADERS_PATH;
    const std::string heatmapShaderPath = shaderFolderPath + "/heatmap.frag";

    if (!m_heatmapShader.loadFromFile(heatmapShaderPath, sf::Shader::Type::Fragment))
    {
        throw std::runtime_error("Failed to load heatmap shader.");
    }
}

void Heatmap::loadData(int fileIndex)
{
    if (fileIndex < 0 || fileIndex >= m_dataFiles.size())
        return;

    m_selectedFileIndex         = fileIndex;
    const std::string& filename = m_dataFiles[m_selectedFileIndex];

    try
    {
        const std::string dataFolderPath = DATA_PATH;
        std::string       filepath       = dataFolderPath + "/" + filename;
        m_ascData                        = std::make_unique<AscParser>(filepath);

        createHeatmapTexture();
    } catch (const std::runtime_error& e)
    {
        std::cerr << "Failed to load ASC data: " << e.what() << std::endl;
        m_ascData.reset();
    }
}

void Heatmap::scanDataDirectory()
{
    const std::string dataFolderPath   = DATA_PATH;
    const std::string ascFileExtension = ".asc";
    m_dataFiles.clear();

    try
    {
        for (const auto& entry : std::filesystem::directory_iterator(dataFolderPath))
        {
            if (entry.is_regular_file() && entry.path().extension() == ascFileExtension)
            {
                m_dataFiles.push_back(entry.path().filename().string());
            }
        }
    } catch (const std::filesystem::filesystem_error& e)
    {
        std::cerr << "Error scanning directory: " << e.what() << std::endl;
        throw;
    }
}

void Heatmap::createHeatmapTexture()
{
    if (!m_ascData)
    {
        return;
    }

    const auto&  header      = m_ascData->getHeader();
    const auto&  data        = m_ascData->getData();
    const double minVal      = m_ascData->getMinValue();
    const double maxVal      = m_ascData->getMaxValue();
    const double denominator = maxVal - minVal;

    sf::Image heatmapImage;
    heatmapImage.resize({static_cast<unsigned int>(header.ncols), static_cast<unsigned int>(header.nrows)});

    for (int y = 0; y < header.nrows; ++y)
    {
        for (int x = 0; x < header.ncols; ++x)
        {
            size_t index = static_cast<size_t>(y) * header.ncols + x;
            double value = data[index];

            auto pixelPosition = sf::Vector2u(static_cast<unsigned int>(x), static_cast<unsigned int>(y));

            if (value == header.nodata_value)
            {
                heatmapImage.setPixel(pixelPosition, sf::Color::Transparent);
            }
            else
            {
                const double  normalized = (denominator != 0.0) ? (value - minVal) / denominator : 0.0;
                const uint8_t gray       = static_cast<std::uint8_t>(std::clamp(normalized, 0.0, 1.0) * 255.0);

                heatmapImage.setPixel(pixelPosition, sf::Color(gray, gray, gray));
            }
        }
    }

    if (!m_heatmapTexture.loadFromImage(heatmapImage))
    {
        throw std::runtime_error("Failed to load heatmap image into texture.");
    }

    m_heatmapSprite.setTexture(m_heatmapTexture, true);
}

const std::unique_ptr<AscParser>& Heatmap::getAscData() const
{
    return m_ascData;
}

const std::vector<std::string>& Heatmap::getDataFiles() const
{
    return m_dataFiles;
}

int Heatmap::getSelectedFileIndex() const
{
    return m_selectedFileIndex;
}

void Heatmap::setCurrentColormapID(int id)
{
    m_currentColormapID = id;
    m_heatmapShader.setUniform("uScalarTexture", sf::Shader::CurrentTexture);
    m_heatmapShader.setUniform("uColormapID", m_currentColormapID);
}

int Heatmap::getCurrentColormapID() const
{
    return m_currentColormapID;
}

const sf::Texture& Heatmap::getHeatmapTexture() const
{
    return m_heatmapTexture;
}

const sf::Sprite& Heatmap::getHeatmapSprite() const
{
    return m_heatmapSprite;
}

sf::Shader& Heatmap::getHeatmapShader()
{
    return m_heatmapShader;
}