#include "StringUtil.h"

#include <string>
#include <cstdarg>
#include <memory>
#include <locale>
#include <iomanip>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include "util/Log.h"

namespace StringUtil {

std::string Format(const std::string fmt_str, ...) {
	int final_n, n = std::max(1024, ((int) fmt_str.size()) * 2); /* Reserve two times as much as the length of the fmt_str or 1k whichever is bigger. This really sucks and should be fixed properly */
	std::unique_ptr<char[]> formatted;
	va_list ap;
	while (1) {
		formatted.reset(new char[n]); /* Wrap the plain char array into the unique_ptr */
		strcpy(&formatted[0], fmt_str.c_str());
		va_start(ap, fmt_str);
		final_n = vsnprintf(&formatted[0], n, fmt_str.c_str(), ap);
		va_end(ap);
		if (final_n < 0 || final_n >= n)
			n += abs(final_n - n + 1);
		else
			break;
	}
	std::string x = std::string(formatted.get());
	return x;
}

std::string FormatTimeHHMM(unsigned long ms) {
	unsigned int hh = ms / HOUR_MS;
	unsigned int mm = (ms - (hh * HOUR_MS)) / MINUTE_MS;
	return Format("%02d:%02d", hh, mm);
}

std::string FormatTimeHHMMSS(unsigned long ms) {
	unsigned int hh = ms / HOUR_MS;
	unsigned int mm = (ms - (hh * HOUR_MS)) / MINUTE_MS;
	unsigned int ss = (ms - ((hh * HOUR_MS) + (mm * MINUTE_MS))) / SECOND_MS;
	return Format("%02d:%02d:%02d", hh, mm, ss);
}

std::string FormatTimeHHMMSSmm(unsigned long ms) {
	unsigned int hh = ms / HOUR_MS;
	unsigned int mm = (ms - (hh * HOUR_MS)) / MINUTE_MS;
	unsigned int ss = (ms - ((hh * HOUR_MS) + (mm * MINUTE_MS))) / SECOND_MS;
	unsigned int mms = (ms - ((hh * HOUR_MS) + (mm * MINUTE_MS) + (ss * SECOND_MS)));
	return Format("%02d:%02d:%02d:%02d", hh, mm, ss, mms);
}

int SafeParseInt(const std::string& str) {
	return SafeParseInt(str, 0);
}

int SafeParseInt(const std::string& str, int defaultValue) {
	char *end;
	long  l;
	const char *s = str.c_str();
	l = strtol(s, &end, 0);
	if ((errno == ERANGE && l == LONG_MAX) || l > INT_MAX) {
		return defaultValue;
	}
	if ((errno == ERANGE && l == LONG_MIN) || l < INT_MIN) {
		return defaultValue;
	}
	if (*s == '\0' || *end != '\0') {
		return defaultValue;
	}
	return l;
}

void LTrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
        return !std::isspace(ch);
    }));
}

void RTrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
        return !std::isspace(ch);
    }).base(), s.end());
}

void Trim(std::string &s) {
    LTrim(s);
    RTrim(s);
}

std::string LTrimCopy(std::string s) {
    LTrim(s);
    return s;
}

std::string RTrimCopy(std::string s) {
    RTrim(s);
    return s;
}

std::string TrimCopy(std::string s) {
    Trim(s);
    return s;
}

unsigned long ParseTimeHHMM(const std::string& timeString) {
	std::tm t = { };
	std::istringstream ss(timeString);
	ss >> std::get_time(&t, "%H:%M");
	return (t.tm_hour * HOUR_MS) + (t.tm_min * MINUTE_MS);
}

unsigned long ParseTimeHHMMSS(const std::string& timeString) {
	std::tm t = { };
	std::istringstream ss(timeString);
	ss >> std::get_time(&t, "%H:%M:%SS");
	return (t.tm_hour * HOUR_MS) + (t.tm_min * MINUTE_MS)
			+ (t.tm_sec * SECOND_MS);
}

std::string ReplaceAll(std::string str, const std::string& from,
		const std::string& to) {
	size_t start_pos = 0;
	while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
		str.replace(start_pos, from.length(), to);
		start_pos += to.length();
	}
	return str;
}

std::string LowerCase(const std::string& in) {
	std::string a = in;
	std::transform(a.begin(), a.end(), a.begin(), ::tolower);
	return a;
}

std::string UpperCase(const std::string& in) {
	std::string a = in;
	std::transform(a.begin(), a.end(), a.begin(), ::toupper);
	return a;
}
}
