#ifndef STRINGUTIL_H
#define STRINGUTIL_H

#define DAY_MS 86400000
#define HOUR_MS 3600000
#define MINUTE_MS 60000
#define SECOND_MS 1000

#include <string>

namespace StringUtil
{
	std::string Format(const std::string fmt_str, ...);

	std::string FormatTimeHHMM(unsigned long ms);

	std::string FormatTimeHHMMSS(unsigned long ms);

	std::string FormatTimeHHMMSSmm(unsigned long ms);

	int SafeParseInt(const std::string& str);

	int SafeParseInt(const std::string& str, int defaultValue);

	unsigned long ParseTimeHHMM(const std::string& timeString);

	unsigned long ParseTimeHHMMSS(const std::string& timeString);

	std::string ReplaceAll(std::string str, const std::string& from,
			const std::string& to);

	std::string LowerCase(const std::string& in);

	std::string UpperCase(const std::string& in);

	void LTrim(std::string &s);
	void RTrim(std::string &s);
	void Trim(std::string &s);
	std::string LTrimCopy(std::string s);
	std::string RTrimCopy(std::string s);
	std::string TrimCopy(std::string s);

}

#endif //STRINGUTIL_H
