#include "URL.h"
#include "StringList.h"
#include "FileReader3.h"
#include "DirectoryAccess.h"

URLManager g_URLManager;

URLManager::URLManager()
{
	mLoaded = false;
}

void URLManager::LoadFile(void)
{
	std::string filename;
	Platform::GenerateFilePath(filename, "Data", "URL.txt");

	FileReader3 fr;
	if(fr.OpenFile(filename.c_str()) != FileReader3::SUCCESS)
	{
		g_Log.AddMessageFormat("[ERROR] Could not open URL file [%s]", filename.c_str());
		return;
	}
	fr.SetCommentChar(';');
	while(fr.Readable() == true)
	{
		fr.ReadLine();
		int r = fr.SingleBreak("=");
		if(r >= 2)
		{
			STRINGLIST row;
			row.push_back(fr.BlockToStringC(0));
			row.push_back(fr.BlockToStringC(1));
			mLoadedURLs.push_back(row);
		}
	}
	fr.CloseFile();
	mLoaded = true;
}

const MULTISTRING& URLManager::GetURLs(void)
{
	if(mLoaded == false)
		LoadFile();

	return mLoadedURLs;
}