#ifndef HEATMAP_HPP
#define HEATMAP_HPP

#include <SFML/Graphics.hpp>
#include <memory>
#include <string>
#include <vector>

#include "utils/AscParser.hpp"

class Heatmap
{
public:
    Heatmap();

    void loadData(int fileIndex);

    const std::unique_ptr<AscParser>& getAscData() const;
    const std::vector<std::string>&   getDataFiles() const;
    int                               getSelectedFileIndex() const;

private:
    std::unique_ptr<AscParser> m_ascData;
    std::vector<std::string>   m_dataFiles;
    int                        m_selectedFileIndex = -1;
    int                        m_currentColormapID = 0;

    void scanDataDirectory();
};

#endif