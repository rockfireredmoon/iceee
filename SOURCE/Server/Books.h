#pragma once
#ifndef BOOKS_H
#define BOOKS_H


#include <vector>
#include <string>
#include <map>

class BookDefinition
{
public:
	std::string title;
	std::vector<std::string> pages;
	int bookID;

	BookDefinition() { Clear(); }
	~BookDefinition() { }
	void Clear(void);
};

class BookManager
{
public:
	BookManager();
	~BookManager();

	std::map<int, BookDefinition> books;

	void Init();
	void LoadFile(const char *filename);
	BookDefinition GetBookDefinition(int bookID);
};

extern BookManager g_BookManager;


#endif /* BOOKS_H */
