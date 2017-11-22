#pragma once
#ifndef CONFIGSTRING_H
#define CONFIGSTRING_H

#include "CommonTypes.h"

// Configuration strings may hold single or multiple key=value pairs.
// Commonly something like:
//   foo=1&bar=whatever
// This class allows methods to break apart the key=value pairs and return values for
// a particular key.
class ConfigString
{
public:
	ConfigString();                            //Assign() must be called manually.
	ConfigString(const std::string &data);     //Initializes by calling Assign() on the given data.
	~ConfigString();
	void Assign(const std::string &data);      //Breaks apart a string into its key=value data.
	int GetValueInt(const char *key);          //Retrieve a key value as integer, if it exists, otherwise return 0.
	int GetValueIntOrDefault(const char *key, int def);          //Retrieve a key value as integer, if it exists, otherwise return 0.
	float GetValueFloat(const char *key);      //Retrieve a key value as float, if it exists, otherwise return 0.
	float GetValueFloatOrDefault(const char *key, float def);      //Retrieve a key value as float, if it exists, otherwise return 0.
	void GetValueString(const char *key, std::string &output);
	bool HasKey(const char *key);
	bool IsEmpty();
	void Clear();
	void SetKeyValue(const char *key, const char *value);
	void GenerateString(std::string &output);
	MULTISTRING mData;
};

#endif // #CONFIGSTRING_H
