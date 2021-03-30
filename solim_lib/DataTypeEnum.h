/*!
 * @brief Datatype of environmental variables.
 * @version 1.0
 * @revision  17-11-21 zhanglei - initial version
 */
#ifndef DATATYPEENUM_HPP_
#define DATATYPEENUM_HPP_
#include <string>
using std::string;
namespace solim {
enum DataTypeEnum {
    CATEGORICAL,
    CONTINUOUS,
    OTHER
};

enum CurveTypeEnum {
    BELL_SHAPED,
    S_SHAPED,
    Z_SHAPED,
    ORDER_SHAPED,
    NAME_SHAPED
};
enum RuleType {
	RANGE_RULE,
	FREEHAND_RULE,
	POINT_RULE,
	WORD_RULE,
	ENUMERATED_RULE,
};
enum IntegrationMethod {
	MINIMUM,
	MEAN
};
enum ComputationMode {
	SINGLE_LAYER,
	MULTI_LAYER
};
enum PrototypeSource {
	SAMPLE,
	EXPERT,
	MAP,
	UNKNOWN
};
enum ExceptionType {
	OCCURRENCE,
	EXCLUSION
};

static const char* DataTypeEnum_str[]={"CATEGORICAL","CONTINUOUS","OTHER"};
static const char* PrototypeSource_str[] = {"SAMPLE","EXPERT","MAP","UNKNOW"};
static DataTypeEnum getDatatypeFromString(string sDatatype) {
	for (int i = 0; i<sDatatype.length(); i++)
        toupper(sDatatype[i]);

	if (sDatatype == "CATEGORICAL")
		return CATEGORICAL;
	else
		return CONTINUOUS;
}
static string getDatatypeInString(DataTypeEnum datatype) {
	if (datatype == CATEGORICAL)
		return "CATEGORICAL";
	else if (datatype == OTHER)
		return "OTHER";
	else
		return "CONTINUOUS";
}
static PrototypeSource getSourceFromString(string source_str) {
    for (int i = 0; i<source_str.length(); i++)
        source_str[i] = toupper(source_str[i]);
    if (source_str == "SAMPLE")
        return SAMPLE;
    else if(source_str=="EXPERT")
        return EXPERT;
    else if(source_str=="MAP")
        return MAP;
    else
        return UNKNOWN;
}
}

#endif
