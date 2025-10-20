#include <filesystem>
#include <iostream>

#include "heatmap.hpp"

#define GL_R32F 0x822E // Should be imported by glad. Decide later to import the whole lib or not

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

Heatmap::~Heatmap()
{
    // Handle this because of float data custom implementation
    if (m_heatmapTexture.getNativeHandle() != 0)
    {
        GLuint handle = m_heatmapTexture.getNativeHandle();
        glDeleteTextures(1, &handle);
    }
}

void Heatmap::draw(const sf::View& view)
{
    if (!m_ascData)
    {
        return;
    }

    calculateAutoClamp(view);
}

void Heatmap::loadData(int fileIndex)
{
    if (fileIndex < 0 || fileIndex >= m_dataFiles.size())
    {
        return;
    }

    m_selectedFileIndex         = fileIndex;
    const std::string& filename = m_dataFiles[m_selectedFileIndex];

    try
    {
        const std::string dataFolderPath = DATA_PATH;
        std::string       filepath       = dataFolderPath + "/" + filename;
        m_ascData                        = std::make_unique<AscParser>(filepath);

        m_globalMin = static_cast<float>(m_ascData->getMinValue());
        m_globalMax = static_cast<float>(m_ascData->getMaxValue());

        m_currentClampMin = m_globalMin;
        m_currentClampMax = m_globalMax;
        m_manualClampMin  = m_globalMin;
        m_manualClampMax  = m_globalMax;

        m_heatmapShader.setUniform("uFloatTexture", sf::Shader::CurrentTexture);

        updateHeatmapTexture();
    } catch (const std::runtime_error& e)
    {
        std::cerr << "Failed to load ASC data: " << e.what() << std::endl;
        m_ascData.reset();
    }
}

void Heatmap::setCurrentColormapID(int id)
{
    m_currentColormapID = id;
    m_heatmapShader.setUniform("uColormapID", m_currentColormapID);
}

void Heatmap::updateHeatmapView(sf::View view)
{
    if (!m_ascData)
    {
        return;
    }

    const sf::Vector2f textureSize(m_heatmapTexture.getSize());
    const sf::Vector2f viewSize = view.getSize();

    // Calculate the scale factor to fit the texture within the view while preserving aspect ratio
    float scaleX = viewSize.x / textureSize.x;
    float scaleY = viewSize.y / textureSize.y;
    float scale  = std::min(scaleX, scaleY);

    m_heatmapSprite.setScale({scale, scale});

    // Center the sprite within the view
    const sf::FloatRect spriteBounds = m_heatmapSprite.getGlobalBounds();
    const float         posX         = (viewSize.x - spriteBounds.size.x) * 0.5f;
    const float         posY         = (viewSize.y - spriteBounds.size.y) * 0.5f;

    m_heatmapSprite.setPosition({posX, posY});
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

bool Heatmap::isAutoClamping() const
{
    return m_isAutoClamping;
}

float Heatmap::getGlobalMin() const
{
    return m_globalMin;
}

float Heatmap::getGlobalMax() const
{
    return m_globalMax;
}

float Heatmap::getManualClampMin() const
{
    return m_manualClampMin;
}

float Heatmap::getManualClampMax() const
{
    return m_manualClampMax;
}

float Heatmap::getCurrentClampMin() const
{
    return m_currentClampMin;
}

float Heatmap::getCurrentClampMax() const
{
    return m_currentClampMax;
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

        std::sort(m_dataFiles.begin(), m_dataFiles.end());
    } catch (const std::filesystem::filesystem_error& e)
    {
        std::cerr << "Heatmap - error scanning directory: " << e.what() << std::endl;
        throw;
    }
}

void Heatmap::updateHeatmapTexture()
{
    if (!m_ascData)
    {
        return;
    }

    const auto&        header     = m_ascData->getHeader();
    const auto&        doubleData = m_ascData->getData();
    std::vector<float> floatData(doubleData.begin(), doubleData.end());

    if (m_heatmapTexture.getNativeHandle() == 0)
    {
        const bool result = m_heatmapTexture.resize(
            {static_cast<unsigned int>(header.ncols), static_cast<unsigned int>(header.nrows)});

        if (!result)
        {
            throw std::runtime_error("Failed to resize heatmap texture");
        }
    }

    GLuint handle = m_heatmapTexture.getNativeHandle();
    glBindTexture(GL_TEXTURE_2D, handle);

    // Upload the float data. GL_R32F is the internal format for a single 32bit float channel.
    // Reference at https://registry.khronos.org/OpenGL-Refpages/gl4/html/glTexImage2D.xhtml
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, header.ncols, header.nrows, 0, GL_RED, GL_FLOAT, floatData.data());

    glBindTexture(GL_TEXTURE_2D, 0);

    m_heatmapSprite.setTexture(m_heatmapTexture, true);
}

void Heatmap::setAutoClamp(bool enabled)
{
    m_isAutoClamping = enabled;

    // When auto clamping is off revert to previous manual values
    if (!enabled)
    {
        m_currentClampMin = m_manualClampMin;
        m_currentClampMax = m_manualClampMax;
    }
}

void Heatmap::setManualClampRange(float min, float max)
{
    constexpr float minGap = 1.0f;

    // Ensure min is never greater than (max - minGap) otherwise the shader will start flickering
    if (min > max - minGap)
    {
        min = max - minGap;
    }

    m_manualClampMin = min;
    m_manualClampMax = max;

    if (!m_isAutoClamping)
    {
        m_currentClampMin = m_manualClampMin;
        m_currentClampMax = m_manualClampMax;
    }
}

void Heatmap::calculateAutoClamp(const sf::View& view)
{
    if (!m_ascData || !m_isAutoClamping)
    {
        return;
    }

    const sf::Vector2f  viewCenter = view.getCenter();
    const sf::Vector2f  viewSize   = view.getSize();
    const sf::FloatRect viewRect(viewCenter - viewSize * 0.5f, viewSize);
    const sf::FloatRect spriteRect = m_heatmapSprite.getGlobalBounds();

    // AABB Intersection between viewRect and spriteRect (world coords)
    const sf::Vector2f aMin = spriteRect.position;
    const sf::Vector2f aMax = spriteRect.position + spriteRect.size;
    const sf::Vector2f bMin = viewRect.position;
    const sf::Vector2f bMax = viewRect.position + viewRect.size;

    sf::Vector2f intersectionMin{std::max(aMin.x, bMin.x), std::max(aMin.y, bMin.y)};
    sf::Vector2f intersectionMax{std::min(aMax.x, bMax.x), std::min(aMax.y, bMax.y)};

    // No overlap so fall back to global clamp default behavior
    if (intersectionMax.x <= intersectionMin.x || intersectionMax.y <= intersectionMin.y)
    {
        m_currentClampMin = m_globalMin;
        m_currentClampMax = m_globalMax;

        return;
    }

    sf::FloatRect intersectionRect(intersectionMin, intersectionMax - intersectionMin);

    // Get intersected (visible) sprite local coords (texture space)
    const sf::Transform inv              = m_heatmapSprite.getInverseTransform();
    const sf::Vector2f  topLeftLocal     = inv.transformPoint(intersectionRect.position);
    const sf::Vector2f  bottomRightLocal = inv.transformPoint(intersectionRect.position + intersectionRect.size);

    // TODO: Local coords might not be ordered if there is a negative scale. Assume that there will never be a negative scale
    const float left   = topLeftLocal.x;
    const float right  = bottomRightLocal.x;
    const float top    = topLeftLocal.y;
    const float bottom = bottomRightLocal.y;

    const sf::Vector2u textureSize = m_heatmapTexture.getSize(); // ncols x nrows
    const float        maxX        = static_cast<float>(textureSize.x);
    const float        maxY        = static_cast<float>(textureSize.y);

    if (right <= left || bottom <= top)
    {
        m_currentClampMin = m_globalMin;
        m_currentClampMax = m_globalMax;

        return;
    }

    const auto& header = m_ascData->getHeader();
    const int   ncols  = header.ncols;
    const int   nrows  = header.nrows;

    int startCol = std::max(0, static_cast<int>(std::floor(left)));
    int endCol   = std::min(ncols, static_cast<int>(std::ceil(right)));
    int startRow = std::max(0, static_cast<int>(std::floor(top)));
    int endRow   = std::min(nrows, static_cast<int>(std::ceil(bottom)));

    if (startCol >= endCol || startRow >= endRow)
    {
        m_currentClampMin = m_globalMin;
        m_currentClampMax = m_globalMax;

        return;
    }

    const auto&  data   = m_ascData->getData();
    const double nodata = header.nodata_value;

    float localMin    = std::numeric_limits<float>::max();
    float localMax    = std::numeric_limits<float>::lowest();
    bool  hasAnyValue = false;

    // Scan the visible cells to find local min/max
    for (int r = startRow; r < endRow; ++r)
    {
        const std::size_t base = static_cast<std::size_t>(r) * ncols;

        for (int c = startCol; c < endCol; ++c)
        {
            const double value = data[base + c];
            if (value == nodata)
            {
                continue;
            }

            hasAnyValue            = true;
            const float floatValue = static_cast<float>(value);

            if (floatValue < localMin)
            {
                localMin = floatValue;
            }

            if (floatValue > localMax)
            {
                localMax = floatValue;
            }
        }
    }

    if (!hasAnyValue)
    {
        m_currentClampMin = m_globalMin;
        m_currentClampMax = m_globalMax;

        return;
    }

    if (localMax <= localMin)
    {
        localMax = localMin + std::numeric_limits<float>::epsilon();
    }

    m_currentClampMin = localMin;
    m_currentClampMax = localMax;
}