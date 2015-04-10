#include "ConfigString.h"
#include "Util.h"
#include <stdlib.h>

ConfigString :: ConfigString()
{
}

ConfigString :: ~ConfigString()
{
}

ConfigString :: ConfigString(const std::string &data)
{
	Assign(data);
}

void ConfigString :: Assign(const std::string &data)
{
	if(data.size() == 0)
		return;
	mData.clear();

	STRINGLIST pairs;
	STRINGLIST values;
	Util::Split(data, "&", pairs);
	for(size_t i = 0; i < pairs.size(); i++)
	{
		Util::Split(pairs[i], "=", values);
		while(values.size() < 2)
			values.push_back("");
		mData.push_back(values);
		values.clear();
	}
}

int ConfigString :: GetValueInt(const char *key)
{
	for(size_t i = 0; i < mData.size(); i++)
	{
		if(mData[i].size() == 0)
			continue;
		if(mData[i][0].compare(key) == 0)
		{
			if(mData[i].size() >= 2)
				return atoi(mData[i][1].c_str());
			return 0;
		}
	}
	return 0;
}

float ConfigString :: GetValueFloat(const char *key)
{
	for(size_t i = 0; i < mData.size(); i++)
	{
		if(mData[i].size() == 0)
			continue;
		if(mData[i][0].compare(key) == 0)
		{
			if(mData[i].size() >= 2)
				return static_cast<float>(atof(mData[i][1].c_str()));
			return 0.0F;
		}
	}
	return 0.0F;
}

void ConfigString :: GetValueString(const char *key, std::string &output)
{
	output.clear();
	for(size_t i = 0; i < mData.size(); i++)
	{
		if(mData[i].size() == 0)
			continue;
		if(mData[i][0].compare(key) == 0)
		{
			if(mData[i].size() >= 2)
				output = mData[i][1];
		}
	}
}

bool ConfigString :: HasKey(const char *key)
{
	for(size_t i = 0; i < mData.size(); i++)
	{
		if(mData[i][0].compare(key) == 0)
			return true;
	}
	return false;
}

/*  Not tested, functions weren't needed.

void ConfigString :: RemoveKey(const char *key)
{
	for(size_t i = 0; i < mData.size(); i++)
	{
		if(mData[i][0].compare(key) == 0)
		{
			mData.erase(mData.begin() + i);
			return;
		}
	}
}
*/

void ConfigString :: SetKeyValue(const char *key, const char *value)
{
	for(size_t i = 0; i < mData.size(); i++)
	{
		if(mData[i][0].compare(key) == 0)
		{
			mData[i][1] = value;
			return;
		}
	}
	STRINGLIST temp;
	temp.push_back(key);
	temp.push_back(value);
	mData.push_back(temp);
}

void ConfigString :: GenerateString(std::string &output)
{
	output.clear();
	for(size_t i = 0; i < mData.size(); i++)
	{
		if(i > 0)
			output.append("&");

		output.append(mData[i][0]);
		if(mData[i].size() > 0)
		{
			output.append("=");
			output.append(mData[i][1]);
		}
	}
}
