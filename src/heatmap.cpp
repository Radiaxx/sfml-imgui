#include "heatmap.hpp"
#include <filesystem>
#include <iostream>

Heatmap::Heatmap()
{
    scanDataDirectory();
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