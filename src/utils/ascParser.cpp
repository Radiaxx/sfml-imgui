#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>

#include "AscParser.hpp"

static std::string toLower(const std::string& str)
{
    std::string lower_str = str;

    for (char& c : lower_str)
    {
        c = std::tolower(c);
    }

    return lower_str;
}

AscParser::AscParser(const std::string& filepath)
{
    loadFile(filepath);
    findMinMax();
}

void AscParser::loadFile(const std::string& filepath)
{
    std::ifstream file(filepath);
    if (!file.is_open())
    {
        throw std::runtime_error("Could not open file: " + filepath);
    }

    std::string   line;
    std::string   key;
    const uint8_t header_lines = 6;

    for (int i = 0; i < header_lines; ++i)
    {
        if (!std::getline(file, line))
        {
            throw std::runtime_error("Error reading header from file: " + filepath);
        }

        std::stringstream ss(line);
        ss >> key;
        std::string lowerKey = toLower(key);

        if (lowerKey == HeaderKeys::NCOLS)
        {
            ss >> m_header.ncols;
        }
        else if (lowerKey == HeaderKeys::NROWS)
        {
            ss >> m_header.nrows;
        }
        else if (lowerKey == HeaderKeys::XLLCORNER)
        {
            ss >> m_header.xllcorner;
        }
        else if (lowerKey == HeaderKeys::YLLCORNER)
        {
            ss >> m_header.yllcorner;
        }
        else if (lowerKey == HeaderKeys::CELLSIZE)
        {
            ss >> m_header.cellsize;
        }
        else if (lowerKey == HeaderKeys::NODATA_VALUE)
        {
            ss >> m_header.nodata_value;
        }
        else
        {
            throw std::runtime_error("Unknown header key: " + key);
        }
    }

    if (m_header.ncols <= 0 || m_header.nrows <= 0)
    {
        throw std::runtime_error("Invalid header data: ncols or nrows is zero or negative.");
    }

    m_data.reserve(m_header.ncols * m_header.nrows);

    double value;
    while (file >> value)
    {
        m_data.push_back(value);
    }

    if (m_data.size() != static_cast<size_t>(m_header.ncols * m_header.nrows))
    {
        throw std::runtime_error("Data size mismatch. Expected " + std::to_string(m_header.ncols * m_header.nrows) +
                                 " values, but found " + std::to_string(m_data.size()));
    }
}

void AscParser::findMinMax()
{
    m_minValue = std::numeric_limits<double>::max();
    m_maxValue = std::numeric_limits<double>::lowest();

    for (const double value : m_data)
    {
        if (value != m_header.nodata_value)
        {
            if (value < m_minValue)
            {
                m_minValue = value;
            }

            if (value > m_maxValue)
            {
                m_maxValue = value;
            }
        }
    }
}

const AscParser::Header& AscParser::getHeader() const
{
    return m_header;
}

const std::vector<double>& AscParser::getData() const
{
    return m_data;
}

double AscParser::getMinValue() const
{
    return m_minValue;
}

double AscParser::getMaxValue() const
{
    return m_maxValue;
}