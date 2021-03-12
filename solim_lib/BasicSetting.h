/*!
 * @brief Basic setting of raster layer data.
 * @version 1.0
 * @deprecated This class is redundant and should be removed in the future version. ZHULJ
 * @revision  17-11-21 zhanglei - initial version
 *            17-11-27 lj       - code revision and format
 */
#ifndef BASICSETTING_HPP_
#define BASICSETTING_HPP_

namespace solim {
class BasicSetting {
public:
	int Width;
	int Height;
	double Xmin;
	double Ymin;
	double Xmax;
	double Ymax;
	double CellSize;
	double NoDataValue;

public:
	BasicSetting()
		: Width(0), Height(0), Xmin(0.), Ymin(0.),
		  Xmax(0.), Ymax(0.), CellSize(10.), NoDataValue(-9999.) {}

	~BasicSetting() {};
};
}

#endif
