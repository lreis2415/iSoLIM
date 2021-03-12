/*!
 * @brief Location.
 * @version 1.0
 * @revision  17-11-21 zhanglei - initial version
 *            17-11-27 lj       - code revision and format
 */
#ifndef LOCATION_HPP_
#define LOCATION_HPP_

namespace solim {
class Location {
public:
    Location(): Row(-1), Col(-1), X(-9999.), Y(-9999.)  {
    }

    Location(const int row, const int col)
        : Row(row), Col(col), X(-9999.), Y(-9999.) {
    }

    Location(const int row, const int col, const double x, const double y)
        : Row(row), Col(col), X(x), Y(y) {
    }

    ~Location() {
    }

public:
    int Row;  // Row number of current cell
    int Col;  // Col number of current cell
    double X; // X coordinate of current cell at upper left
    double Y; // Y coordinate of current cell at upper left
};
}

#endif
