#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <assert.h>
#include <ctime>
#include <chrono>
#include <thread>
#include <fstream>
#include <algorithm>
#include <vector>
#include <map>
#include <cctype>
#include <functional>

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long u64;

typedef char s8;
typedef short s16;
typedef int s32;
typedef long s64;

#define warning(x)					__pragma(message("[warning] - " x));
#define warning_assert(x)			warning(x); assert(0); 

namespace string_helpers
{
	std::string replaceAll(const std::string& str, const std::string& replace, const std::string& with)
	{
		std::string ret = str;

		size_t pos = ret.find(replace, 0);

		while (pos != std::string::npos)
		{
			ret.erase(pos, replace.length());
			ret.insert(pos, with);

			pos = ret.find(replace, pos + 1);
		}

		return ret;
	}

	std::string trim(const std::string& str)
	{
		std::string ret = str;

		size_t first = ret.find_first_not_of(' ');
		if (std::string::npos == first)
		{
			return ret;
		}
		size_t last = ret.find_last_not_of(' ');
		return ret.substr(first, (last - first + 1));
	}

	int split(std::vector<std::string>& split, const std::string& str, const char delim)
	{
		split.clear();
		std::string ret = str;
		size_t pos = ret.find(delim);
	
		while (pos != std::string::npos)
		{
			if (pos > 0)
			{
				split.push_back(ret.substr(0, pos));
			}

			ret = ret.substr(pos + 1);
			pos = ret.find(delim);
		}

		split.push_back(ret);

		return (int)split.size();;
	}
}