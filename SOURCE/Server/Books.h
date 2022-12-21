#pragma once
#ifndef BOOKS_H
#define BOOKS_H


#include <vector>
#include <string>
#include "json/json.h"
#include <filesystem>

using namespace std;
namespace fs = filesystem;


class BookDefinition
{
public:
	string title;
	vector<string> pages;
	int bookID;

	BookDefinition() { Clear(); }
	~BookDefinition() { }
	void Clear(void);
	void WriteToJSON(Json::Value &value);
};

class BookManager
{
public:
	BookManager();
	~BookManager();

	map<int, BookDefinition> books;

	void Init();
	void LoadFile(const fs::path &filename);
	BookDefinition GetBookDefinition(int bookID);
};

extern BookManager g_BookManager;


#endif /* BOOKS_H */
