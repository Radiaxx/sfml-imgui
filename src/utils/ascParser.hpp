#ifndef ASC_PARSER_HPP
#define ASC_PARSER_HPP

#include <stdexcept>
#include <string>
#include <vector>

class AscParser
{
public:
    struct HeaderKeys
    {
        static constexpr const char* NCOLS        = "ncols";
        static constexpr const char* NROWS        = "nrows";
        static constexpr const char* XLLCORNER    = "xllcorner";
        static constexpr const char* YLLCORNER    = "yllcorner";
        static constexpr const char* CELLSIZE     = "cellsize";
        static constexpr const char* NODATA_VALUE = "nodata_value";
    };

    struct Header
    {
        int    ncols        = 0;
        int    nrows        = 0;
        double xllcorner    = 0.0;
        double yllcorner    = 0.0;
        double cellsize     = 0.0;
        double nodata_value = -9999.0;
    };

    AscParser(const std::string& filepath);

    const Header&              getHeader() const;
    const std::vector<double>& getData() const;
    double                     getMinValue() const;
    double                     getMaxValue() const;

private:
    Header              m_header;
    std::vector<double> m_data;
    double              m_minValue;
    double              m_maxValue;

    void loadFile(const std::string& filepath);
    void findMinMax();
};

#endif