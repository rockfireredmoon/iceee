#include <ctime>
#include <algorithm>
#include "IGForum.h"
#include "FileReader.h"
#include "DirectoryAccess.h"

#include "util/Log.h"

IGFManager g_IGFManager;

IGFFlags :: IGFFlags()
{
	mFlag = 0;
}
void IGFFlags :: setFlag(unsigned short flagBit, bool status)
{
	if(status == true)
		mFlag |= flagBit;
	else
		mFlag &= (~(mFlag));
}
int IGFFlags :: getraw(void)
{
	return mFlag;
}
void IGFFlags :: setraw(int value)
{
	mFlag = value;
}
void IGFFlags :: reset(void)
{
	mFlag = 0;
}

bool IGFFlags :: hasFlag(const int flag)
{
	return ((mFlag & flag) ? true : false);
}


IGFCategoryPage :: IGFCategoryPage()
{
	mPendingChanges = 0;
	mPage = 0;
	mLastAccessTime = 0;
}

void IGFCategoryPage :: SaveFile(const char *filename)
{
	FILE *output = fopen(filename, "wb");
	if(output == NULL)
	{
		g_Logs.data->error("IGFCategoryPage::SaveFile failed to open: %v", filename);
		return;
	}
	CATEGORYENTRY::iterator it;
	for(it = mEntries.begin(); it != mEntries.end(); it++)
	{
		fprintf(output, "[ENTRY]\r\n");
		fprintf(output, "ID=%d\r\n", it->second.mID);
		fprintf(output, "Title=%s\r\n", it->second.mTitle.c_str());
		fprintf(output, "ParentCategory=%d\r\n", it->second.mParentCategory);
		fprintf(output, "Locked=%d\r\n", it->second.mLocked);
		fprintf(output, "Flags=%d\r\n", it->second.mFlags.getraw());
		fprintf(output, "LastUpdateTime=%lu\r\n", it->second.mLastUpdateTime);
		Util::WriteIntegerList(output, "ThreadList", it->second.mThreadList);
		fprintf(output, "\r\n");
	}
	fclose(output);
}

void IGFCategoryPage :: LoadFile(const char *filename)
{
	FileReader lfr;
	if(lfr.OpenText(filename) != Err_OK)
	{
		g_Logs.data->error("IGFCategoryPage::LoadFile failed to open file.");
		return;
	}
	lfr.CommentStyle = Comment_Semi;
	IGFCategory entry;

	while(lfr.FileOpen() == true)
	{
		lfr.ReadLine();
		int r = lfr.BreakUntil("=", '=');
		if(r > 0)
		{
			lfr.BlockToStringC(0, 0);
			if(strcmp(lfr.SecBuffer, "[ENTRY]") == 0)
			{
				if(entry.mID != 0)
					InsertEntry(entry, false);
				entry.Clear();
			}
			else if(strcmp(lfr.SecBuffer, "ID") == 0)
				entry.mID = lfr.BlockToIntC(1);
			else if(strcmp(lfr.SecBuffer, "Title") == 0)
				entry.mTitle = lfr.BlockToStringC(1, 0);
			else if(strcmp(lfr.SecBuffer, "ParentCategory") == 0)
				entry.mParentCategory = lfr.BlockToIntC(1);
			else if(strcmp(lfr.SecBuffer, "Locked") == 0)
				entry.mLocked = lfr.BlockToBoolC(1);
			else if(strcmp(lfr.SecBuffer, "Flags") == 0)
				entry.mFlags.setraw(lfr.BlockToIntC(1));
			else if(strcmp(lfr.SecBuffer, "LastUpdateTime") == 0)
				entry.mLastUpdateTime = lfr.BlockToULongC(1);
			else if(strcmp(lfr.SecBuffer, "ThreadList") == 0)
			{
				r = lfr.MultiBreak("=,");
				for(int i = 1; i < r; i++)
					entry.mThreadList.push_back(lfr.BlockToIntC(i));
			}
			else
				g_Logs.data->warn("IGFCategoryPage::LoadFile unknown identifier [%v] in file [%v] on line [%v]", lfr.SecBuffer, filename, lfr.LineNumber);
		}
	}
	if(entry.mID != 0)
		InsertEntry(entry, false);
	lfr.CloseCurrent();
}

void IGFCategoryPage :: InsertEntry(IGFCategory &entry, bool changePending)
{
	mEntries.insert(mEntries.end(), CATEGORYPAGE(entry.mID, entry));
	if(changePending == true)
		mPendingChanges++;
}

IGFCategory* IGFCategoryPage :: GetPointerByID(int ID)
{
	CATEGORYENTRY::iterator it;
	it = mEntries.find(ID);
	if(it == mEntries.end())
		return NULL;
	return &it->second;
}

void IGFCategoryPage :: DeleteObject(int objectID)
{
	CATEGORYENTRY::iterator it;
	it = mEntries.find(objectID);
	if(it != mEntries.end())
		mEntries.erase(it);
}

bool IGFCategoryPage :: QualifyGarbage(void)
{
	if(mPendingChanges != 0)
		return false;
	return (g_ServerTime >= (mLastAccessTime + IGFManager::GARBAGE_CHECK_EXPIRE));
}

IGFThreadPage :: IGFThreadPage()
{
	mPendingChanges = 0;
	mPage = 0;
	mLastAccessTime = 0;
}
void IGFThreadPage :: SaveFile(const char *filename)
{
	FILE *output = fopen(filename, "wb");
	if(output == NULL)
	{
		g_Logs.data->error("IGFThreadPage::SaveFile failed to open file.");
		return;
	}
	THREADENTRY::iterator it;
	for(it = mEntries.begin(); it != mEntries.end(); it++)
	{
		fprintf(output, "[ENTRY]\r\n");
		fprintf(output, "ID=%d\r\n", it->second.mID);
		fprintf(output, "Title=%s\r\n", it->second.mTitle.c_str());
		fprintf(output, "CreationAccount=%d\r\n", it->second.mCreationAccount);
		fprintf(output, "CreationTime=%s\r\n", it->second.mCreationTime.c_str());
		fprintf(output, "CreatorName=%s\r\n", it->second.mCreatorName.c_str());
		fprintf(output, "ParentCategory=%d\r\n", it->second.mParentCategory);
		fprintf(output, "Locked=%d\r\n", it->second.mLocked);
		fprintf(output, "Stickied=%d\r\n", it->second.mStickied);
		fprintf(output, "Flags=%d\r\n", it->second.mFlags.getraw());
		fprintf(output, "LastUpdateTime=%lu\r\n", it->second.mLastUpdateTime);

		Util::WriteIntegerList(output, "PostList", it->second.mPostList);
		fprintf(output, "\r\n");
	}
	fclose(output);
}

void IGFThreadPage :: LoadFile(const char *filename)
{
	FileReader lfr;
	if(lfr.OpenText(filename) != Err_OK)
	{
		g_Logs.data->error("IGFThreadPage::LoadFile failed to open file.");
		return;
	}
	lfr.CommentStyle = Comment_Semi;
	IGFThread entry;

	while(lfr.FileOpen() == true)
	{
		lfr.ReadLine();
		int r = lfr.BreakUntil("=", '=');
		if(r > 0)
		{
			lfr.BlockToStringC(0, 0);
			if(strcmp(lfr.SecBuffer, "[ENTRY]") == 0)
			{
				if(entry.mID != 0)
					InsertEntry(entry, false);
				entry.Clear();
			}
			else if(strcmp(lfr.SecBuffer, "ID") == 0)
				entry.mID = lfr.BlockToIntC(1);
			else if(strcmp(lfr.SecBuffer, "Title") == 0)
				entry.mTitle = lfr.BlockToStringC(1, 0);
			else if(strcmp(lfr.SecBuffer, "CreationAccount") == 0)
				entry.mCreationAccount = lfr.BlockToIntC(1);
			else if(strcmp(lfr.SecBuffer, "CreationTime") == 0)
				entry.mCreationTime = lfr.BlockToStringC(1, 0);
			else if(strcmp(lfr.SecBuffer, "CreatorName") == 0)
				entry.mCreatorName = lfr.BlockToStringC(1, 0);
			else if(strcmp(lfr.SecBuffer, "ParentCategory") == 0)
				entry.mParentCategory = lfr.BlockToIntC(1);
			else if(strcmp(lfr.SecBuffer, "Locked") == 0)
				entry.mLocked = lfr.BlockToBoolC(1);
			else if(strcmp(lfr.SecBuffer, "Stickied") == 0)
				entry.mStickied = lfr.BlockToBoolC(1);
			else if(strcmp(lfr.SecBuffer, "Flags") == 0)
				entry.mFlags.setraw(lfr.BlockToIntC(1));
			else if(strcmp(lfr.SecBuffer, "LastUpdateTime") == 0)
				entry.mLastUpdateTime = lfr.BlockToULongC(1);
			else if(strcmp(lfr.SecBuffer, "PostList") == 0)
			{
				r = lfr.MultiBreak("=,");
				for(int i = 1; i < r; i++)
					entry.mPostList.push_back(lfr.BlockToIntC(i));
			}
			else
				g_Logs.data->warn("IGFThreadPage::LoadFile unknown identifier [%v] in file [%v] on line [%v]", lfr.SecBuffer, filename, lfr.LineNumber);
		}
	}
	if(entry.mID != 0)
		InsertEntry(entry, false);
	lfr.CloseCurrent();
}

void IGFThreadPage :: InsertEntry(IGFThread &entry, bool changePending)
{
	mEntries.insert(mEntries.end(), THREADPAIR(entry.mID, entry));
	if(changePending == true)
		mPendingChanges++;
}

IGFThread * IGFThreadPage :: GetPointerByID(int ID)
{
	THREADENTRY::iterator it;
	it = mEntries.find(ID);
	if(it == mEntries.end())
		return NULL;
	return &it->second;
}

bool IGFThreadPage :: QualifyGarbage(void)
{
	if(mPendingChanges != 0)
		return false;
	return (g_ServerTime >= (mLastAccessTime + IGFManager::GARBAGE_CHECK_EXPIRE));
}

IGFPostPage :: IGFPostPage()
{
	mPendingChanges = 0;
	mPage = 0;
	mLastAccessTime = 0;
}

void IGFPostPage :: SaveFile(const char *filename)
{
	FILE *output = fopen(filename, "wb");
	if(output == NULL)
	{
		g_Logs.data->error("IGFPostPage::SaveFile failed to open file.");
		return;
	}
	POSTENTRY::iterator it;
	for(it = mEntries.begin(); it != mEntries.end(); it++)
	{
		fprintf(output, "[ENTRY]\r\n");
		fprintf(output, "ID=%d\r\n", it->second.mID);
		fprintf(output, "CreationAccount=%d\r\n", it->second.mCreationAccount);
		fprintf(output, "CreationTime=%s\r\n", it->second.mCreationTime.c_str());
		fprintf(output, "CreatorName=%s\r\n", it->second.mCreatorName.c_str());
		fprintf(output, "ParentThread=%d\r\n", it->second.mParentThread);
		fprintf(output, "PostedTime=%lu\r\n", it->second.mPostedTime);
		fprintf(output, "LastUpdateTime=%lu\r\n", it->second.mLastUpdateTime);
		fprintf(output, "EditCount=%d\r\n", it->second.mEditCount);
		fprintf(output, "BodyText=%s\r\n", it->second.mBodyText.c_str());
		fprintf(output, "\r\n");
	}
	fclose(output);
}

void IGFPostPage :: LoadFile(const char *filename)
{
	FileReader lfr;
	if(lfr.OpenText(filename) != Err_OK)
	{
		g_Logs.data->error("IGFPostPage::LoadFile failed to open: %v", filename);
		return;
	}
	//lfr.CommentStyle = Comment_Semi;  //No comments since they're valid characters for posts.
	IGFPost entry;

	while(lfr.FileOpen() == true)
	{
		lfr.ReadLine();
		int r = lfr.BreakUntil("=", '=');
		if(r > 0)
		{
			lfr.BlockToStringC(0, 0);
			if(strcmp(lfr.SecBuffer, "[ENTRY]") == 0)
			{
				if(entry.mID != 0)
					InsertEntry(entry, false);
				entry.Clear();
			}
			else if(strcmp(lfr.SecBuffer, "ID") == 0)
				entry.mID = lfr.BlockToIntC(1);
			else if(strcmp(lfr.SecBuffer, "CreationAccount") == 0)
				entry.mCreationAccount = lfr.BlockToIntC(1);
			else if(strcmp(lfr.SecBuffer, "CreationTime") == 0)
				entry.mCreationTime = lfr.BlockToStringC(1, 0);
			else if(strcmp(lfr.SecBuffer, "CreatorName") == 0)
				entry.mCreatorName = lfr.BlockToStringC(1, 0);
			else if(strcmp(lfr.SecBuffer, "ParentThread") == 0)
				entry.mParentThread = lfr.BlockToIntC(1);
			else if(strcmp(lfr.SecBuffer, "PostedTime") == 0)
				entry.mPostedTime = lfr.BlockToULongC(1);
			else if(strcmp(lfr.SecBuffer, "LastUpdateTime") == 0)
				entry.mLastUpdateTime = lfr.BlockToULongC(1);
			else if(strcmp(lfr.SecBuffer, "EditCount") == 0)
				entry.mEditCount = lfr.BlockToULongC(1);
			else if(strcmp(lfr.SecBuffer, "BodyText") == 0)
				entry.mBodyText = lfr.BlockToStringC(1, 0);
			else
				g_Logs.data->warn("IGFPostPage::LoadFile unknown identifier [%v] in file [%v] on line [%v]", lfr.SecBuffer, filename, lfr.LineNumber);
		}
	}
	if(entry.mID != 0)
		InsertEntry(entry, false);
	lfr.CloseCurrent();
}

void IGFPostPage :: InsertEntry(IGFPost &entry, bool changePending)
{
	mEntries.insert(mEntries.end(), POSTPAIR(entry.mID, entry));
	if(changePending == true)
		mPendingChanges++;
}

IGFPost * IGFPostPage :: GetPointerByID(int ID)
{
	POSTENTRY::iterator it;
	it = mEntries.find(ID);
	if(it == mEntries.end())
		return NULL;
	return &it->second;
}

bool IGFPostPage :: QualifyGarbage(void)
{
	if(mPendingChanges != 0)
		return false;
	return (g_ServerTime >= (mLastAccessTime + IGFManager::GARBAGE_CHECK_EXPIRE));
}

IGFCategory :: IGFCategory()
{
	Clear();
}

void IGFCategory :: Clear(void)
{
	mID = 0;
	mTitle.clear();
	mParentCategory = 0;
	mThreadList.clear();
	mLocked = false;
	mFlags.reset();
	mLastUpdateTime = 0;
}

void IGFCategory :: UnattachThread(int threadID)
{
	for(size_t i = 0; i < mThreadList.size(); i++)
	{
		if(mThreadList[i] == threadID)
		{
			mThreadList.erase(mThreadList.begin() + i);
			return;
		}
	}
}

void IGFCategory :: AttachThread(int threadID)
{
	for(size_t i = 0; i < mThreadList.size(); i++)
	{
		if(mThreadList[i] == threadID)
			return;
	}
	mThreadList.push_back(threadID);
}

void IGFCategory :: SetLastUpdateTime(void)
{
	mLastUpdateTime = g_PlatformTime.getAbsoluteMinutes();
}

IGFThread :: IGFThread()
{
	Clear();
}

void IGFThread :: Clear(void)
{
	mID = 0;
	mTitle.clear();
	mCreationAccount = 0;
	mCreationTime.clear();
	mCreatorName.clear();
	mParentCategory = 0;
	mPostList.clear();
	mLocked = false;
	mStickied = false;
	mLastUpdateTime = 0;
	mFlags.reset();
}

void IGFThread :: DeletePost(int ID)
{
	for(size_t i = 0; i < mPostList.size(); i++)
	{
		if(mPostList[i] == ID)
		{
			mPostList.erase(mPostList.begin() + i);
			return;
		}
	}
}

//Return the ID of the last post in the thread.  Some standard member editing permissions
//only work on the last post (assuming it's theirs).
int IGFThread :: GetLastPostID(void)
{
	if(mPostList.size() == 0)
		return -1;
	return mPostList.back();
}

void IGFThread :: SetLastUpdateTime(void)
{
	mLastUpdateTime = g_PlatformTime.getAbsoluteMinutes();
}

IGFPost :: IGFPost()
{
	Clear();
}

void IGFPost :: Clear(void)
{
	mID = 0;
	mCreationAccount = 0;
	mCreationTime.clear();
	mCreatorName.clear();
	mParentThread = 0;
	mBodyText.clear();
	mPostedTime = 0;
	mLastUpdateTime = 0;
	mEditCount = 0;
}

void IGFPost :: SetLastUpdateTime(void)
{
	mLastUpdateTime = g_PlatformTime.getAbsoluteMinutes();
}


IGFManager :: IGFManager()
{
	mNextCategoryID = 2;  //The root takes ID:1, so the next category needs to be a step higher
	mNextThreadID = 1;
	mNextPostID = 1;
	mNextGarbageCheck = 0;
	mNextAutosaveCheck = 0;
	mPlatformLaunchMinute = 0;
	mForumLocked = false;
	Init();
}

void IGFManager :: Init(void)
{
	LoadConfig();
	if(mPlatformLaunchMinute == 0)   //Config failed to load, so init the time value.
		mPlatformLaunchMinute = g_PlatformTime.getAbsoluteMinutes();

	//InitPaths(); OLD

	//There should always be a default root category.

	//Force all category pages to load.
	/*
	int maxPage = GetCategoryPage(mNextCategoryID - 1);
	for(int i = 0; i < maxPage; i++)
		GetPagedCategoryPtr(i * CATEGORY_PER_PAGE);
	*/

	IGFCategory *category = GetPagedCategoryPtr(0);
	if(category == NULL)
	{
		IGFCategory root;
		root.mID = 1;  //Zero ID indicates no data so the page file won't load the entry.
		root.mTitle = "root";
		root.mParentCategory = ROOT_CATEGORY;
		InsertPagedCategory(root);
	}
	char buffer[256];
	Util::SafeFormat(buffer, sizeof(buffer), "IGForum");
	Platform::MakeDirectory(buffer);

	Util::SafeFormat(buffer, sizeof(buffer), "IGForum\\Category");
	Platform::FixPaths(buffer);
	Platform::MakeDirectory(buffer);

	Util::SafeFormat(buffer, sizeof(buffer), "IGForum\\Thread");
	Platform::FixPaths(buffer);
	Platform::MakeDirectory(buffer);

	Util::SafeFormat(buffer, sizeof(buffer), "IGForum\\Post");
	Platform::FixPaths(buffer);
	Platform::MakeDirectory(buffer);
}

IGFManager :: ~IGFManager()
{

}

int IGFManager :: GetCategoryPage(int ID)
{
	return ID / CATEGORY_PER_PAGE;
}

int IGFManager :: GetThreadPage(int ID)
{
	return ID / THREAD_PER_PAGE;
}

int IGFManager :: GetPostPage(int ID)
{
	return ID / POST_PER_PAGE;
}

void IGFManager :: EnumCategoryList(int parentID, MULTISTRING &output)
{
	STRINGLIST entry;
	CATEGORYPAGE::iterator it;
	IGFCategoryPage::CATEGORYENTRY::iterator eit;
	for(it = mCategoryPages.begin(); it != mCategoryPages.end(); ++it)
	{
		for(eit = it->second.mEntries.begin(); eit != it->second.mEntries.end(); ++eit)
		{
			if(eit->second.mParentCategory == parentID)
			{
				entry.push_back(ConvertInteger(TYPE_CATEGORY));
				entry.push_back(ConvertInteger(eit->second.mID));
				entry.push_back(ConvertInteger(eit->second.mLocked));
				entry.push_back(ConvertInteger(0)); //Stickied.  Categories don't have this.
				entry.push_back(eit->second.mTitle);
				entry.push_back(ConvertInteger(eit->second.mThreadList.size()));
				entry.push_back(ConvertInteger(GetTimeOffset(eit->second.mLastUpdateTime)));

				output.push_back(entry);
				entry.clear();
			}
		}
	}
}

void IGFManager :: EnumThreadList(int parentID, MULTISTRING &output)
{
	IGFCategory* category = GetPagedCategoryPtr(parentID);
	if(category == NULL)
		return;

	//Pregenerate a list of threads
	std::vector<IGFThread*> results;
	for(size_t i = 0; i < category->mThreadList.size(); i++)
	{
		IGFThread* thread = GetPagedThreadPtr(category->mThreadList[i]);
		if(thread != NULL)
			results.push_back(thread);
	}

	if(category->mFlags.hasFlag(IGFFlags::FLAG_SORTALPHABETICAL))
		std::sort(results.begin(), results.end(), ThreadSortAlphabetical);

	STRINGLIST entry;
	for(size_t i = 0; i < results.size(); i++)
	{
		IGFThread* thread = results[i];
		entry.push_back(ConvertInteger(TYPE_THREAD));
		entry.push_back(ConvertInteger(thread->mID));
		entry.push_back(ConvertInteger(thread->mLocked));
		entry.push_back(ConvertInteger(thread->mStickied));
		entry.push_back(thread->mTitle);
		entry.push_back(ConvertInteger(thread->mPostList.size()));
		entry.push_back(ConvertInteger(GetTimeOffset(thread->mLastUpdateTime)));

		output.push_back(entry);
		entry.clear();
	}
}

bool IGFManager :: ThreadSortAlphabetical(const IGFThread* lhs, const IGFThread* rhs)
{
	if(lhs->mStickied == rhs->mStickied)
	{
		if(lhs->mTitle.compare(rhs->mTitle) <= 0)
			return true;
	}
	else
	{
		if(lhs->mStickied == true)
			return true;
	}
	
	return false;
}

void IGFManager :: GetCategory(int id, MULTISTRING &output)
{
	IGFCategory *category = GetPagedCategoryPtr(id);
	if(category == NULL)
		return;

	STRINGLIST header;

	header.push_back(ConvertInteger(id));
	header.push_back(category->mTitle);
	output.push_back(header);

	EnumCategoryList(id, output);
	EnumThreadList(id, output);
}

void IGFManager :: OpenCategory(int type, int id, MULTISTRING &output)
{
	//Expand an object.  If it's a category, enumerate a list of subcategories.
	if(type == TYPE_CATEGORY)
	{
		IGFCategory *category = GetPagedCategoryPtr(id);
		if(category != NULL)
		{
			STRINGLIST header;
			header.push_back(ConvertInteger(id));
			header.push_back(category->mTitle);

			output.push_back(header);

			int searchID = category->mID;
			EnumCategoryList(searchID, output);
			EnumThreadList(searchID, output);
		}
	}
}

unsigned long IGFManager :: GetTimeOffset(unsigned long LastUpdateTime)
{
	if(LastUpdateTime == 0)
		LastUpdateTime = mPlatformLaunchMinute;
	return g_PlatformTime.getAbsoluteMinutes() - LastUpdateTime;
}

void IGFManager :: OpenThread(int threadID, int startPost, int requestedCount, MULTISTRING &output)
{
	IGFThread *thread = GetPagedThreadPtr(threadID);
	if(thread == NULL)
		return;

	startPost = Util::ClipInt(startPost, 0, thread->mPostList.size() - 1);

	STRINGLIST row;

	//We retrieve the time offset since the first session since the client uses
	//4 byte integers which theoretically may not be large enough to hold the time data.
	unsigned long timeOffset = g_PlatformTime.getAbsoluteMinutes();

	//Prepare the header
	row.push_back(ConvertInteger(threadID));  //[0]
	row.push_back(thread->mTitle);   //[1]
	row.push_back(ConvertInteger(startPost));  //[2]
	row.push_back(ConvertInteger(thread->mPostList.size()));  //[3]
	row.push_back(ConvertInteger(timeOffset - thread->mLastUpdateTime));  //[4]
	output.push_back(row);
	row.clear();

	//Append the post data.
	int count = 0;
	for(size_t i = startPost; i < thread->mPostList.size(); i++)
	{
		IGFPost *post = GetPagedPostPtr(thread->mPostList[i]);
		if(post == NULL)
		{
			g_Logs.data->error("OpenThread: unable to find post: %v", thread->mPostList[i]);
			continue;
		}

		row.push_back(ConvertInteger(post->mID));  //[0]
		row.push_back(post->mCreatorName.c_str());  //[1]
		row.push_back(post->mCreationTime.c_str());  //[2]
		row.push_back(ConvertInteger(timeOffset - post->mPostedTime)); //[3]
		row.push_back(post->mBodyText.c_str());  //[4]
		row.push_back(ConvertInteger(post->mEditCount)); //[5]
		row.push_back(ConvertInteger(timeOffset - post->mLastUpdateTime)); //[6]

		output.push_back(row);
		row.clear();

		if(++count >= requestedCount)
			break;
	}
}

int IGFManager :: SendPost(AccountData *callerAccount, int type, int placementID, int postID, const char *threadTitle, const char *postBody, const char *displayName)
{
	if(mForumLocked == true && callerAccount->HasPermission(Perm_Account, Permission_Admin) == false)
		return ERROR_FORUMLOCKED;

	if(callerAccount->HasPermission(Perm_Account, Permission_ForumPost) == false)
		return ERROR_POSTBLOCK;

	if(HasInvalidCharacters(postBody, true))
		return ERROR_INVALIDPOSTTEXT;
	if(HasInvalidCharacters(displayName, false))
		return ERROR_INVALIDNAMETEXT;
	
	int threadID = 0;
	if(type == POST_THREAD)  //New thread
	{
		if(HasInvalidCharacters(threadTitle, false))
			return ERROR_INVALIDTITLETEXT;
		if(strlen(threadTitle) > MAX_TITLE_LENGTH)
			return ERROR_TITLELENGTH;

		IGFCategory *category = GetPagedCategoryPtr(placementID);
		if(category == NULL)
			return ERROR_INVALIDCATEGORY;
		if(category->mLocked == true)
			return ERROR_CATEGORYLOCKED;

		IGFThread newThread;
		newThread.mID = GetNewThreadID();
		newThread.mParentCategory = category->mID;
		newThread.mTitle = threadTitle;
		newThread.mCreationAccount = callerAccount->ID;
		newThread.mCreatorName = displayName;
		newThread.SetLastUpdateTime();
		InsertPagedThread(newThread);

		category->mThreadList.push_back(newThread.mID);

		threadID = newThread.mID; //Update with thread ID so the rest of the function can proceed
	}
	else if(type == POST_REPLY)
		threadID = placementID;

	//If creating a thread or replying, take the thread ID and append a new post.
	if(type == POST_THREAD || type == POST_REPLY)
	{
		IGFThread *thread = GetPagedThreadPtr(threadID);
		if(thread == NULL)
			return ERROR_INVALIDTHREAD;
		if(thread->mLocked == true)
			return ERROR_THREADLOCKED;

		if(strlen(postBody) > MAX_POST_LENGTH)
			return ERROR_POSTLENGTH;

		IGFCategory *category = GetPagedCategoryPtr(thread->mParentCategory);
		if(category == NULL)
			return ERROR_INVALIDCATEGORY;

		category->SetLastUpdateTime();
		thread->SetLastUpdateTime();

		IGFPost newPost;
		newPost.mID = GetNewPostID();
		newPost.mParentThread = thread->mID;
		newPost.mBodyText = postBody;
		ProcessPostBody(newPost.mBodyText);
		newPost.mCreationAccount = callerAccount->ID;
		newPost.mCreatorName = displayName;
		newPost.SetLastUpdateTime();
		newPost.mPostedTime = g_PlatformTime.getAbsoluteMinutes();

		char timeBuf[256];
		time_t curtime;
		time(&curtime);
		strftime(timeBuf, sizeof(timeBuf), "%x %X", localtime(&curtime));
		newPost.mCreationTime = timeBuf;
		
		InsertPagedPost(newPost);
		thread->mPostList.push_back(newPost.mID);
		SortCategoryThreads(category);
		MarkChangedThread(threadID);
		MarkChangedCategory(category->mID);
	}
	else if(type == POST_EDIT)
	{
		IGFThread *thread = GetPagedThreadPtr(placementID);
		if(thread == NULL)
			return ERROR_INVALIDTHREAD;
		if(thread->mLocked == true && GetEditPermission(callerAccount, -1) == false)
			return ERROR_THREADLOCKED;

		if(strlen(postBody) > MAX_POST_LENGTH)
			return ERROR_POSTLENGTH;

		IGFPost *post = GetPagedPostPtr(postID);
		if(post == NULL)
			return ERROR_INVALIDPOST;

		IGFCategory *category = GetPagedCategoryPtr(thread->mParentCategory);
		if(category == NULL)
			return ERROR_INVALIDCATEGORY;

		if(GetEditPermission(callerAccount, post->mCreationAccount) == false)
			return ERROR_PERMISSIONDENIED;

		post->mBodyText = postBody;
		post->SetLastUpdateTime();
		post->mEditCount++;
		ProcessPostBody(post->mBodyText);
		MarkChangedPost(postID);

		thread->SetLastUpdateTime();
		category->SetLastUpdateTime();
		SortCategoryThreads(category);
		MarkChangedThread(thread->mID);
		MarkChangedCategory(category->mID);
	}

	return ERROR_NONE;
}


/* OLD
void IGFManager :: SaveCategory(void)
{
	const char *fileName = mPathCategory.c_str();
	FILE *output = fopen(fileName, "wb");
	if(output == NULL)
	{
		g_Log.AddMessageFormat("[ERROR] Failed to open file [%s] for saving", fileName);
		return;
	}
	CATEGORY::iterator it;
	for(it = mCategory.begin(); it != mCategory.end(); ++it)
	{
		fprintf(output, "[ENTRY]\r\n");
		fprintf(output, "ID=%d\r\n", it->second.mID);
		fprintf(output, "Title=%s\r\n", it->second.mTitle.c_str());
		fprintf(output, "ParentCategory=%d\r\n", it->second.mParentCategory);
		Util::WriteIntegerList(output, "ThreadList", it->second.mThreadList);
		fprintf(output, "\r\n");
	}
	fclose(output);
}
*/

/* OLD
void IGFManager :: LoadCategory()
{
	const char *fileName = mPathCategory.c_str();
	FileReader lfr;
	if(lfr.OpenText(fileName) != Err_OK)
	{
		g_Log.AddMessageFormat("[ERROR] Failed to open file [%s] for reading", fileName);
		return;
	}
	lfr.CommentStyle = Comment_Semi;
	IGFCategory entry;
	while(lfr.FileOpen() == true)
	{
		int r = lfr.ReadLine();
		r = lfr.BreakUntil("=", '=');
		if(r > 0)
		{
			lfr.BlockToStringC(0, 0);
			if(strcmp(lfr.SecBuffer, "[ENTRY]") == 0)
			{
				if(entry.mID != 0)
					InsertCategory(entry);
				entry.Clear();
			}
			else if(strcmp(lfr.SecBuffer, "ID") == 0)
				entry.mID = lfr.BlockToIntC(1);
			else if(strcmp(lfr.SecBuffer, "Title") == 0)
				entry.mTitle = lfr.BlockToStringC(1, 0);
			else if(strcmp(lfr.SecBuffer, "ParentCategory") == 0)
				entry.mParentCategory = lfr.BlockToIntC(1);
			else if(strcmp(lfr.SecBuffer, "ThreadList") == 0)
			{
				int r = lfr.MultiBreak("=,");
				for(int i = 1; i < r; i++)
					entry.mThreadList.push_back(lfr.BlockToIntC(i));
			}
		}
	}
	if(entry.mID != 0)
		InsertCategory(entry);
	lfr.CloseCurrent();
}
*/

/* OLD
void IGFManager :: InitPaths(void)
{
	char buffer[256];

	Util::SafeFormat(buffer, sizeof(buffer), "IGForum\\CategoryDB.txt");
	Platform::FixPaths(buffer);
	mPathCategory = buffer;

	Util::SafeFormat(buffer, sizeof(buffer), "IGForum\\ThreadDB.txt");
	Platform::FixPaths(buffer);
	mPathThread = buffer;

	Util::SafeFormat(buffer, sizeof(buffer), "IGForum\\PostDB.txt");
	Platform::FixPaths(buffer);
	mPathPost = buffer;
}
*/

/* OLD
int IGFManager :: CreateCategory(int parentCategoryID, const char *name)
{
	if(name == NULL)
		return ERROR_INVALIDTITLE;
	if(strlen(name) == 0)
		return ERROR_INVALIDTITLE;

	CATEGORY::iterator it;
	it = mCategory.find(parentCategoryID);
	if(it == mCategory.end())
		return ERROR_INVALIDCATEGORY;

	IGFCategory entry;
	entry.mID = GetNewCategoryID();
	entry.mTitle = name;
	entry.mParentCategory = parentCategoryID;
	InsertCategory(entry);
	cdCategory.AddChange();
	return ERROR_NONE;
}
*/
int IGFManager :: CreateCategory(AccountData *callerAccount, int parentCategoryID, const char *name)
{
	if(GetEditPermission(callerAccount, -1) == false)
		return ERROR_PERMISSIONDENIED;

	if(name == NULL)
		return ERROR_INVALIDTITLETEXT;
	if(strlen(name) == 0)
		return ERROR_INVALIDTITLETEXT;

	IGFCategory *category = GetPagedCategoryPtr(parentCategoryID);
	if(category == NULL)
	{
		g_Logs.data->error("Invalid category: %v", parentCategoryID);
		return ERROR_INVALIDCATEGORY;
	}

	IGFCategory entry;
	entry.mID = GetNewCategoryID();
	entry.mTitle = name;
	entry.mParentCategory = parentCategoryID;
	InsertPagedCategory(entry);
	return ERROR_NONE;
}

const char* IGFManager :: GetErrorString(int errCode)
{
	switch(errCode)
	{
	case ERROR_NONE: return "No error.";
	case ERROR_INVALIDCATEGORY: return "Category is invalid.";
	case ERROR_INVALIDTHREAD: return "Thread does not exist.";
	case ERROR_INVALIDPOST: return "Post does not exist.";
	case ERROR_INVALIDTITLETEXT: return "Title name is invalid.";
	case ERROR_INVALIDPOSTTEXT: return "Post text is invalid.";
	case ERROR_INVALIDNAMETEXT: return "Character name is invalid.";
	case ERROR_PERMISSIONDENIED: return "Permission denied.";
	case ERROR_CATEGORYLOCKED: return "Category is locked.";
	case ERROR_THREADLOCKED: return "Thread is locked.";
	case ERROR_UNHANDLED: return "Unknown or unhandled operation.";
	case ERROR_NOTATHREAD: return "That operation only works on threads.";
	case ERROR_POSTBLOCK: return "You do not have permission to create or edit posts.";
	case ERROR_FORUMLOCKED: return "The forum is locked.  Only administrative edits are allowed.";
	case ERROR_TITLELENGTH: return "The thread or category name is too long.";
	case ERROR_POSTLENGTH: return "Post body text is too long.";
	case ERROR_TARGETNOTCATEGORY: return "The target location must be a category.";
	case ERROR_TARGETSAME: return "The target location must be different from the source.";
	}
	return "Unknown error.";
}

int IGFManager :: GetNewCategoryID(void)
{
	return mNextCategoryID++;
}

int IGFManager :: GetNewThreadID(void)
{
	return mNextThreadID++;
}

int IGFManager :: GetNewPostID(void)
{
	return mNextPostID++;
}

bool IGFManager :: HasInvalidCharacters(const char *text, bool allowMarkup)
{
	if(text == NULL)
		return true;
	if(strlen(text) == 0)
		return true;

	size_t len = strlen(text);
	for(size_t i = 0; i < len; i++)
	{
		if(text[i] < 32)
		{
			switch(text[i])
			{
			case '\t':
			case '\r':
			case '\n':
				if(allowMarkup == false)
					return true;
				break;
			default:
				return true;
			}
		}
		else if(text[i] >= 127)
			return true;
	}
	return false;
}

void IGFManager :: ProcessPostBody(std::string &postBody)
{
	size_t pos = 0;
	while(pos < postBody.size())
	{
		bool del = false;
		/*
		if(postBody[pos] == '<')
			del = true;
		else if(postBody[pos] == '>')
			del = true;*/
		/*
		if(postBody[pos] == '<')
			postBody.replace(pos, 1, "&lt;", 4);
		else if(postBody[pos] == '>')
			postBody.replace(pos, 1, "&gt;", 4);
		else if(postBody[pos] == '&')
			postBody.replace(pos, 1, "&amp;", 5);
			*/
		if(postBody[pos] == '\n')
			postBody.replace(pos, 1, "^N", 2);
		else if(postBody[pos] == '\r')
			del = true;

		if(del == true)
			postBody.erase(postBody.begin() + pos);
		else
			pos++;
	}
}


void IGFManager :: InsertPagedCategory(IGFCategory& object)
{
	int page = GetCategoryPage(object.mID);
	if(mCategoryPages.find(page) == mCategoryPages.end())
		LoadCategoryPage(page);

	mCategoryPages[page].InsertEntry(object, true);
	cdCategory.AddChange();
}

void IGFManager :: InsertPagedThread(IGFThread& object)
{
	int page = GetThreadPage(object.mID);
	if(mThreadPages.find(page) == mThreadPages.end())
		LoadThreadPage(page);

	mThreadPages[page].InsertEntry(object, true);
	cdThread.AddChange();
}

void IGFManager:: InsertPagedPost(IGFPost& object)
{
	int page = GetPostPage(object.mID);
	if(mPostPages.find(page) == mPostPages.end())
		LoadPostPage(page);

	mPostPages[page].InsertEntry(object, true);
	cdPost.AddChange();
}

IGFCategory* IGFManager :: GetPagedCategoryPtr(int elementID)
{
	int page = GetCategoryPage(elementID);
	CATEGORYPAGE::iterator it = mCategoryPages.find(page);
	if(it == mCategoryPages.end())
		LoadCategoryPage(page);

	mCategoryPages[page].mLastAccessTime = g_ServerTime;
	return mCategoryPages[page].GetPointerByID(elementID);
}

IGFThread* IGFManager:: GetPagedThreadPtr(int elementID)
{
	int page = GetThreadPage(elementID);
	THREADPAGE::iterator it = mThreadPages.find(page);
	if(it == mThreadPages.end())
		LoadThreadPage(page);
	mThreadPages[page].mLastAccessTime = g_ServerTime;
	return mThreadPages[page].GetPointerByID(elementID);
}

IGFPost* IGFManager:: GetPagedPostPtr(int elementID)
{
	int page = GetPostPage(elementID);
	POSTPAGE::iterator it = mPostPages.find(page);
	if(it == mPostPages.end())
		LoadPostPage(page);
	mPostPages[page].mLastAccessTime = g_ServerTime;
	return mPostPages[page].GetPointerByID(elementID);
}


void IGFManager :: LoadCategoryPage(int page)
{
	char buffer[256];
	Util::SafeFormat(buffer, sizeof(buffer), "IGForum\\Category\\%08d.txt", page);
	Platform::FixPaths(buffer);

	mCategoryPages[page].mPage = page;  //Set the page so autosave always write to page zero and overwrite old entries.
	mCategoryPages[page].LoadFile(buffer);
}

void IGFManager :: LoadThreadPage(int page)
{
	char buffer[256];
	Util::SafeFormat(buffer, sizeof(buffer), "IGForum\\Thread\\%08d.txt", page);
	Platform::FixPaths(buffer);

	mThreadPages[page].mPage = page;  //Set the page so autosave always write to page zero and overwrite old entries.
	mThreadPages[page].LoadFile(buffer);
}

void IGFManager :: LoadPostPage(int page)
{
	char buffer[256];
	Util::SafeFormat(buffer, sizeof(buffer), "IGForum\\Post\\%08d.txt", page);
	Platform::FixPaths(buffer);

	mPostPages[page].mPage = page;  //Set the page so autosave always write to page zero and overwrite old entries.
	mPostPages[page].LoadFile(buffer);
}

void IGFManager :: CheckAutoSave(bool force)
{
	if(g_ServerTime < mNextAutosaveCheck && force == false)
		return;

	mNextAutosaveCheck = g_ServerTime + AUTOSAVE_FREQUENCY;

	bool updateID = false;
	if((cdCategory.CheckUpdateAndClear(AUTOSAVE_TIME) == true) || (force == true))
	{
		AutosaveCategory();
		cdCategory.ClearPending();
		updateID = true;
	}
	if((cdThread.CheckUpdateAndClear(AUTOSAVE_TIME) == true) || (force == true))
	{
		AutosaveThread();
		cdThread.ClearPending();
		updateID = true;
	}
	if((cdPost.CheckUpdateAndClear(AUTOSAVE_TIME) == true) || (force == true))
	{
		AutosavePost();
		cdPost.ClearPending();
		updateID = true;
	}
	if(updateID == true)
		SaveConfig();
}

void IGFManager :: AutosaveCategory(void)
{
	char buffer[256];
	CATEGORYPAGE::iterator it;
	for(it = mCategoryPages.begin(); it != mCategoryPages.end(); ++it)
	{
		if(it->second.mPendingChanges == 0)
			continue;
		Util::SafeFormat(buffer, sizeof(buffer), "IGForum\\Category\\%08d.txt", it->second.mPage);
		Platform::FixPaths(buffer);
		it->second.SaveFile(buffer);
		it->second.mPendingChanges = 0;
	}
}

void IGFManager :: AutosaveThread(void)
{
	char buffer[256];
	THREADPAGE::iterator it;
	for(it = mThreadPages.begin(); it != mThreadPages.end(); ++it)
	{
		if(it->second.mPendingChanges == 0)
			continue;
		Util::SafeFormat(buffer, sizeof(buffer), "IGForum\\Thread\\%08d.txt", it->second.mPage);
		Platform::FixPaths(buffer);
		it->second.SaveFile(buffer);
		it->second.mPendingChanges = 0;
	}
}

void IGFManager :: AutosavePost(void)
{
	char buffer[256];
	POSTPAGE::iterator it;
	for(it = mPostPages.begin(); it != mPostPages.end(); ++it)
	{
		if(it->second.mPendingChanges == 0)
			continue;
		Util::SafeFormat(buffer, sizeof(buffer), "IGForum\\Post\\%08d.txt", it->second.mPage);
		Platform::FixPaths(buffer);
		it->second.SaveFile(buffer);
		it->second.mPendingChanges = 0;
	}
}

void IGFManager :: SaveConfig(void)
{
	char buffer[256];
	Platform::GenerateFilePath(buffer, "IGForum", "IGFSession.txt");
	FILE *output = fopen(buffer, "wb");
	if(output == NULL)
	{
		g_Logs.data->error("IGFManager::SaveConfig failed to open file.");
		return;
	}
	fprintf(output, "NextCategoryID=%d\r\n", mNextCategoryID);
	fprintf(output, "NextThreadID=%d\r\n", mNextThreadID);
	fprintf(output, "NextPostID=%d\r\n", mNextPostID);
	fprintf(output, "PlatformLaunchMinute=%lu\r\n", mPlatformLaunchMinute);
	fclose(output);
}


void IGFManager :: LoadConfig(void)
{
	char buffer[256];
	Platform::GenerateFilePath(buffer, "IGForum", "IGFSession.txt");
	FileReader lfr;
	if(lfr.OpenText(buffer) != Err_OK)
	{
		g_Logs.data->error("IGFManager::LoadConfig failed to open file.");
		return;
	}
	lfr.CommentStyle = Comment_Semi;
	while(lfr.FileOpen() == true)
	{
		lfr.ReadLine();
		int r = lfr.BreakUntil("=", '=');
		if(r > 0)
		{
			lfr.BlockToStringC(0, 0);
			if(strcmp(lfr.SecBuffer, "NextCategoryID") == 0)
				mNextCategoryID = lfr.BlockToIntC(1);
			else if(strcmp(lfr.SecBuffer, "NextThreadID") == 0)
				mNextThreadID = lfr.BlockToIntC(1);
			else if(strcmp(lfr.SecBuffer, "NextPostID") == 0)
				mNextPostID = lfr.BlockToIntC(1);
			else if(strcmp(lfr.SecBuffer, "PlatformLaunchMinute") == 0)
				mPlatformLaunchMinute = lfr.BlockToULongC(1);
			else
				g_Logs.data->warn("IGFManager::LoadConfig unknown identifier [%v] in file [%v] on line [%v]", lfr.SecBuffer, buffer, lfr.LineNumber);
		}
	}
	lfr.CloseCurrent();
}

const char* IGFManager :: ConvertInteger(int value)
{
	static char buffer[64];
	sprintf(buffer, "%d", value);
	return buffer;
}

bool IGFManager :: GetEditPermission(AccountData *callerAccount, int objectOwnerID)
{
	if(callerAccount == NULL)
		return false;

	if(mForumLocked == true && callerAccount->HasPermission(Perm_Account, Permission_Admin) == false)
		return false;

	if(callerAccount->ID == objectOwnerID)
		return true;
	if(callerAccount->HasPermission(Perm_Account, Permission_ForumAdmin))
		return true;
	if(callerAccount->HasPermission(Perm_Account, Permission_Admin))
		return true;

	return false;
}

int IGFManager :: DeletePost(AccountData *callerAccount, int threadID, int postID)
{
	IGFThread *thread = GetPagedThreadPtr(threadID);
	if(thread == NULL)
		return ERROR_INVALIDTHREAD;
	if(thread->mLocked == true && GetEditPermission(callerAccount, -1) == false)
		return ERROR_THREADLOCKED;

	IGFPost *post = GetPagedPostPtr(postID);
	if(post == NULL)
		return ERROR_INVALIDPOST;

	bool perm = false;
	if(post->mID == thread->GetLastPostID() && post->mCreationAccount == callerAccount->ID)
		perm = true;
	else
		perm = GetEditPermission(callerAccount, -1);
	if(perm == false)
		return ERROR_PERMISSIONDENIED;

	MarkChangedThread(threadID);
	thread->DeletePost(postID);
	return ERROR_NONE;
}

int IGFManager :: DeleteObject(AccountData *callerAccount, int objectType, int objectID)
{
	if(GetEditPermission(callerAccount, -1) == false)
		return ERROR_PERMISSIONDENIED;

	if(objectType == TYPE_CATEGORY)
	{
		IGFCategory *category = GetPagedCategoryPtr(objectID);
		if(category == NULL)
			return ERROR_INVALIDCATEGORY;

		DeleteObjectData(TYPE_CATEGORY, objectID);
	}
	else if(objectType == TYPE_THREAD)
	{
		IGFThread *thread = GetPagedThreadPtr(objectID);
		if(thread == NULL)
			return ERROR_INVALIDTHREAD;

		IGFCategory *parent = GetPagedCategoryPtr(thread->mParentCategory);
		if(parent == NULL)
			return ERROR_INVALIDCATEGORY;

		MarkChangedCategory(parent->mID);
		parent->UnattachThread(objectID);
	}
	else 
		return ERROR_UNHANDLED;

	return ERROR_NONE;
}

void IGFManager :: MarkChangedCategory(int objectID)
{
	int page = GetCategoryPage(objectID);
	if(mCategoryPages.find(page) == mCategoryPages.end())
		return;

	mCategoryPages[page].mPendingChanges++;
	cdCategory.AddChange();
}

void IGFManager :: MarkChangedThread(int objectID)
{
	int page = GetThreadPage(objectID);
	if(mThreadPages.find(page) == mThreadPages.end())
		return;

	mThreadPages[page].mPendingChanges++;
	cdThread.AddChange();
}

void IGFManager :: MarkChangedPost(int objectID)
{
	int page = GetPostPage(objectID);
	if(mPostPages.find(page) == mPostPages.end())
		return;

	mPostPages[page].mPendingChanges++;
	cdPost.AddChange();
}

int IGFManager :: SetLockStatus(AccountData *callerAccount, int categoryType, int objectID, bool status)
{
	if(categoryType == TYPE_CATEGORY)
	{
		IGFCategory *category = GetPagedCategoryPtr(objectID);
		if(category == NULL)
			return ERROR_INVALIDCATEGORY;
		if(GetEditPermission(callerAccount, -1) == false)
			return ERROR_PERMISSIONDENIED;

		category->mLocked = status;
		MarkChangedCategory(objectID);
		return ERROR_NONE;
	}
	if(categoryType == TYPE_THREAD)
	{
		IGFThread *thread = GetPagedThreadPtr(objectID);
		if(thread == NULL)
			return ERROR_INVALIDTHREAD;
		if(GetEditPermission(callerAccount, thread->mCreationAccount) == false)
			return ERROR_PERMISSIONDENIED;

		thread->mLocked = status;
		MarkChangedThread(objectID);
		return ERROR_NONE;
	}
	return ERROR_UNHANDLED;
}

int IGFManager :: SetStickyStatus(AccountData *callerAccount, int categoryType, int objectID, bool status)
{
	if(categoryType != TYPE_THREAD)
		return ERROR_NOTATHREAD;

	IGFThread *thread = GetPagedThreadPtr(objectID);
	if(thread == NULL)
		return ERROR_INVALIDTHREAD;
	if(GetEditPermission(callerAccount, -1) == false)
		return ERROR_PERMISSIONDENIED;

	IGFCategory *category = GetPagedCategoryPtr(thread->mParentCategory);
	if(category == NULL)
		return ERROR_INVALIDCATEGORY;

	thread->mStickied = status;
	SortCategoryThreads(category);
	MarkChangedThread(thread->mID);
	MarkChangedCategory(category->mID);
	return ERROR_NONE;
}

int IGFManager :: EditObject(AccountData *callerAccount, int categoryType, int parentID, int renameID, const char *name)
{
	//categoryType is either a Category or Thread
	//parentID is only used if creating a category.  This is the parent category ID.
	//renameID is only used if renaming an object.  This is ID being renamed.
	if(GetEditPermission(callerAccount, -1) == false)
		return ERROR_PERMISSIONDENIED;

	if(name == NULL)
		return ERROR_INVALIDTITLETEXT;
	if(strlen(name) == 0)
		return ERROR_INVALIDTITLETEXT;
	if(HasInvalidCharacters(name, false)) return ERROR_INVALIDTITLETEXT;

	if(categoryType == TYPE_CATEGORY)
	{
		if(strlen(name) > MAX_TITLE_LENGTH)
			return ERROR_TITLELENGTH;

		if(renameID == 0)
		{
			return CreateCategory(callerAccount, parentID, name);
		}
		else
		{
			IGFCategory *category = GetPagedCategoryPtr(renameID);
			if(category == NULL)
				return ERROR_INVALIDCATEGORY;
			category->mTitle = name;
			MarkChangedCategory(renameID);
		}
	}
	else if(categoryType == TYPE_THREAD)
	{
		//Only supports rename for threads.
		if(strlen(name) > MAX_TITLE_LENGTH)
			return ERROR_TITLELENGTH;

		if(renameID != 0)
		{
			IGFThread *thread = GetPagedThreadPtr(renameID);
			if(thread == NULL)
				return ERROR_INVALIDTHREAD;
			thread->mTitle = name;
			MarkChangedThread(renameID);
		}
	}
	else
		return ERROR_UNHANDLED;

	return ERROR_NONE;
}

int IGFManager :: RunAction(AccountData *callerAccount, int actionType, int param1, int param2)
{
	return ERROR_NONE;
}


int IGFManager :: RunMove(AccountData *callerAccount, int srcType, int srcID, int dstType, int dstID)
{
	if(GetEditPermission(callerAccount, -1) == false)
		return ERROR_PERMISSIONDENIED;
	if(srcType == TYPE_CATEGORY)
		return ERROR_NOTATHREAD;
	if(dstType != TYPE_CATEGORY)
		return ERROR_TARGETNOTCATEGORY;

	IGFThread *thread = GetPagedThreadPtr(srcID);
	if(thread == NULL)
		return ERROR_INVALIDTHREAD;

	IGFCategory *categorySrc = GetPagedCategoryPtr(thread->mParentCategory);
	if(categorySrc == NULL)
		return ERROR_INVALIDCATEGORY;

	IGFCategory *categoryDst = GetPagedCategoryPtr(dstID);
	if(categoryDst == NULL)
		return ERROR_INVALIDCATEGORY;

	if(categorySrc == categoryDst)
		return ERROR_TARGETSAME;

	//Change the thread's owning category, swap the threads between containers, and mark them both as changed. 
	thread->mParentCategory = categoryDst->mID;
	categorySrc->UnattachThread(thread->mID);
	categoryDst->AttachThread(thread->mID);
	SortCategoryThreads(categoryDst);
	MarkChangedCategory(categorySrc->mID);
	MarkChangedCategory(categoryDst->mID);

	return ERROR_NONE;
}

void IGFManager :: DeleteObjectData(int objectType, int objectID)
{
	if(objectType == TYPE_CATEGORY)
	{
		IGFCategory *category = GetPagedCategoryPtr(objectID);
		if(category == NULL)
			return;

		int page = GetCategoryPage(objectID);
		CATEGORYPAGE::iterator it = mCategoryPages.find(page);
		if(it != mCategoryPages.end())
		{
			it->second.DeleteObject(objectID);
			cdCategory.AddChange();
		}
	}
}

void IGFManager :: RunGarbageCheck(void)
{
	if(g_ServerTime < mNextGarbageCheck)
		return;

	mNextGarbageCheck = g_ServerTime + GARBAGE_CHECK_FREQUENCY;

	CATEGORYPAGE::iterator cit;
	cit = mCategoryPages.begin();
	while(cit != mCategoryPages.end())
	{
		if(cit->second.QualifyGarbage() == true)
			mCategoryPages.erase(cit++);
		else
			++cit;
	}

	THREADPAGE::iterator tit;
	tit = mThreadPages.begin();
	while(tit != mThreadPages.end())
	{
		if(tit->second.QualifyGarbage() == true)
			mThreadPages.erase(tit++);
		else
			++tit;
	}

	POSTPAGE::iterator pit;
	pit = mPostPages.begin();
	while(pit != mPostPages.end())
	{
		if(pit->second.QualifyGarbage() == true)
			mPostPages.erase(pit++);
		else
			++pit;
	}
}

struct IGFThreadSort
{
	int mID;
	unsigned long mLastUpdate;
	int mSticky;
	bool operator < (const IGFThreadSort &other) const
	{
		if(mSticky > other.mSticky)
			return true;
		else if(mSticky == other.mSticky)
			return mLastUpdate > other.mLastUpdate;

		return false;
	}
};

void IGFManager :: SortCategoryThreads(IGFCategory *category)
{
	std::vector<IGFThreadSort> threadList;
	IGFThreadSort sortObj;
	for(size_t i = 0; i < category->mThreadList.size(); i++)
	{
		int threadID = category->mThreadList[i];
		IGFThread *thread = GetPagedThreadPtr(threadID);
		if(thread == NULL)
		{
			g_Logs.data->error("IGFManager::SortCategoryThreads thread [%v] does not exist", threadID);
			continue;
		}
		sortObj.mID = threadID;
		sortObj.mLastUpdate = thread->mLastUpdateTime;
		sortObj.mSticky = thread->mStickied;
		threadList.push_back(sortObj);
	}
	std::sort(threadList.begin(), threadList.end());
	category->mThreadList.clear();
	for(size_t i = 0; i < threadList.size(); i++)
		category->mThreadList.push_back(threadList[i].mID);
}
