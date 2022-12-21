#include "Books.h"
#include "FileReader.h"
#include "Util.h"

#include "Character.h"
#include "Config.h"
#include "util/Log.h"

BookManager g_BookManager;

void BookDefinition::Clear(void) {
	title = "";
	pages.clear();
}

void BookDefinition::WriteToJSON(Json::Value &value) {
	value["id"] = bookID;
	value["title"] = title;
	Json::Value a;
	for(auto it = pages.begin(); it != pages.end() ; ++it) {
		a.append(*it);
	}
	value["pages"] = a;
}


BookManager::BookManager() {
}

BookManager::~BookManager() {
	books.clear();
}


void BookManager::Init() {
	auto path = g_Config.ResolveStaticDataPath() / "Books";
	for(const fs::directory_entry& entry : fs::directory_iterator(path)) {
		auto path = entry.path();
		if (path.extension() == ".txt") {
			LoadFile(path);
		}
	}
}

void BookManager::LoadFile(const fs::path &filename) {
	FileReader lfr;
	if (lfr.OpenText(filename) != Err_OK) {
		g_Logs.data->error("Could not open file [%v]", filename);
		return;
	}
	int bookID = stoi(filename.stem());
	BookDefinition newItem;
	newItem.bookID = bookID;
	int r = 0;
	string page;
	bool inPageText = false;
	while (lfr.FileOpen() == true) {
		r = lfr.ReadLine();
		string wholeLine = string(lfr.DataBuffer);
		bool escapedLine = Util::HasBeginning(wholeLine, "\\");
		if (r > 0 && !escapedLine) {
			lfr.SingleBreak("=");
			lfr.BlockToStringC(0, Case_Upper);
			if (strcmp(lfr.SecBuffer, "[PAGE]") == 0) {
				if(page.length() > 0) {
					newItem.pages.push_back(page);
					page = "";
				}
				inPageText = false;
			} else if (strcmp(lfr.SecBuffer, "TITLE") == 0)
				newItem.title = lfr.BlockToStringC(1, 0);
			else if (strcmp(lfr.SecBuffer, "TEXT") == 0) {
				page.clear();
				page.append(lfr.BlockToStringC(1, 0));
				inPageText = true;
			}
			else {
				if(inPageText) {
					if(page.length() > 0) {
						if(Util::HasEnding(page, "\\")) {
							page = page.substr(0, page.length() - 1);
						}
						else
							page.append("<br>");
					}
					page.append(wholeLine);
				}
			}
		}
		else if(inPageText) {
			if(escapedLine) {
				wholeLine = wholeLine.substr(1);
			}
			if(page.length() > 0) {
				if(Util::HasEnding(page, "\\")) {
					page = page.substr(0, page.length() - 1);
				}
				else
					page.append("<br>");
			}
			if(r != 0) {
				page.append(wholeLine);
			}
		}
	}
	if(page.length() > 0) {
		newItem.pages.push_back(page);
	}
	books[newItem.bookID] = newItem;
	lfr.CloseCurrent();
}

BookDefinition BookManager::GetBookDefinition(int bookDefID) {
	return books[bookDefID];
}
