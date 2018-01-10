#include <ctime>
#include <algorithm>
#include "IGForum.h"
#include "FileReader.h"
#include "Config.h"
#include "Cluster.h"
#include "StringUtil.h"
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

//
// IGFCategory
//


IGFCategory :: IGFCategory()
{
	Clear();
}


bool IGFCategory :: WriteEntity(AbstractEntityWriter *writer) {
	writer->Key(KEYPREFIX_IGF_CATEGORY, StringUtil::Format("%d", mID));
	writer->Value("Title", mTitle);
	writer->Value("ParentCategory", mParentCategory);
	writer->Value("Locked", mLocked);
	writer->Value("Flags", mFlags.getraw());
	writer->Value("LastUpdateTime", mLastUpdateTime);
	STRINGLIST l;
	for(auto it = mThreadList.begin(); it != mThreadList.end(); ++it)
		l.push_back(StringUtil::Format("%d", *it));
	writer->ListValue("ThreadList", l);
	return true;
}

bool IGFCategory :: EntityKeys(AbstractEntityReader *reader) {
	reader->Key(KEYPREFIX_IGF_CATEGORY, StringUtil::Format("%d", mID), true);
	return true;
}

bool IGFCategory :: ReadEntity(AbstractEntityReader *reader) {
	mTitle = reader->Value("Title");
	mParentCategory = reader->ValueInt("ParentCategory");
	mLocked = reader->ValueBool("Locked");
	mFlags.setraw(reader->ValueInt("Flags"));
	mLastUpdateTime = reader->ValueULong("LastUpdateTime");
	STRINGLIST threads = reader->ListValue("ThreadList", ",");
	for(auto it = threads.begin(); it != threads.end(); ++it)
		mThreadList.push_back(atoi((*it).c_str()));
	return true;
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

bool IGFThread :: WriteEntity(AbstractEntityWriter *writer) {
	writer->Key(KEYPREFIX_IGF_THREAD, StringUtil::Format("%d", mID));

	writer->Value("Title", mTitle);
	writer->Value("CreationAccount", mCreationAccount);
	writer->Value("CreationTime", mCreationTime);
	writer->Value("CreatorName", mCreatorName);
	writer->Value("ParentCategory", mParentCategory);
	writer->Value("Locked", mLocked);
	writer->Value("Stickied", mStickied);
	writer->Value("Flags", mFlags.getraw());
	writer->Value("LastUpdateTime", mLastUpdateTime);
	STRINGLIST l;
	for(auto it = mPostList.begin(); it != mPostList.end(); ++it)
		l.push_back(StringUtil::Format("%d", *it));
	writer->ListValue("PostList", l);
	return true;
}

bool IGFThread :: EntityKeys(AbstractEntityReader *reader) {
	reader->Key(KEYPREFIX_IGF_THREAD, StringUtil::Format("%d", mID), true);
	return true;
}

bool IGFThread :: ReadEntity(AbstractEntityReader *reader) {
	mTitle = reader->Value("Title");
	mCreationAccount = reader->ValueInt("CreationAccount");
	mCreationTime = reader->Value("CreationTime");
	mCreatorName = reader->Value("CreatorName");
	mParentCategory = reader->ValueInt("ParentCategory");
	mLocked = reader->ValueBool("Locked");
	mStickied = reader->ValueBool("Stickied");
	mFlags.setraw(reader->ValueInt("Flags"));
	mLastUpdateTime = reader->ValueULong("LastUpdateTime");
	STRINGLIST posts = reader->ListValue("PostList", ",");
	for(auto it = posts.begin(); it != posts.end(); ++it)
		mPostList.push_back(atoi((*it).c_str()));
	return true;
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

bool IGFPost :: WriteEntity(AbstractEntityWriter *writer) {
	writer->Key(KEYPREFIX_IGF_POST, StringUtil::Format("%d", mID));
	writer->Value("CreationAccount", mCreationAccount);
	writer->Value("CreationTime", mCreationTime);
	writer->Value("CreatorName", mCreatorName);
	writer->Value("ParentThread", mParentThread);
	writer->Value("PostedTime", mPostedTime);
	writer->Value("LastUpdateTime", mLastUpdateTime);
	writer->Value("EditCount", mEditCount);
	writer->Value("BodyText", mBodyText);
	return true;
}

bool IGFPost :: EntityKeys(AbstractEntityReader *reader) {
	reader->Key(KEYPREFIX_IGF_POST, StringUtil::Format("%d", mID), true);
	return true;
}

bool IGFPost :: ReadEntity(AbstractEntityReader *reader) {
	mCreationAccount = reader->ValueInt("CreationAccount");
	mCreationTime = reader->Value("CreationTime");
	mCreatorName = reader->Value("CreatorName");
	mParentThread = reader->ValueInt("ParentThread");
	mPostedTime = reader->ValueULong("PostedTime");
	mLastUpdateTime = reader->ValueULong("LastUpdateTime");
	mEditCount = reader->ValueInt("EditCount");
	mBodyText = reader->Value("BodyText");
	return true;
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
	mForumLocked = false;
}

void IGFManager :: Init(void)
{
	if(!g_ClusterManager.HasKey(ID_IGF_CATEGORY_ID)) {
		//The root takes ID:1, so the next category needs to be a step higher
		g_ClusterManager.SetKey(ID_IGF_CATEGORY_ID, "1");
	}

	//InitPaths(); OLD

	//There should always be a default root category.

	//Force all category pages to load.
	/*
	int maxPage = GetCategoryPage(mNextCategoryID - 1);
	for(int i = 0; i < maxPage; i++)
		GetPagedCategoryPtr(i * CATEGORY_PER_PAGE);
	*/

	IGFCategory category = GetPagedCategoryPtr(0);
	if(category.mID == 0)
	{
		IGFCategory root;
		root.mID = 1;  //Zero ID indicates no data so the page file won't load the entry.
		root.mTitle = "root";
		root.mParentCategory = ROOT_CATEGORY;
		if(!g_ClusterManager.WriteEntity(&root)) {
			g_Logs.data->error("Failed to write default IGF category.");
		}
	}
}

IGFManager :: ~IGFManager()
{

}

void IGFManager :: EnumCategoryList(int parentID, MULTISTRING &output)
{
	STRINGLIST l = g_ClusterManager.GetList(LISTPREFIX_IGF_CATEGORIES);
	STRINGLIST entry;
	for(auto it = l.begin(); it != l.end(); ++it) {
		IGFCategory c;
		c.mID = atoi((*it).c_str());
		if(g_ClusterManager.ReadEntity(&c)) {
			if(c.mParentCategory == parentID) {
				entry.push_back(ConvertInteger(TYPE_CATEGORY));
				entry.push_back(ConvertInteger(c.mID));
				entry.push_back(ConvertInteger(c.mLocked));
				entry.push_back(ConvertInteger(0)); //Stickied.  Categories don't have this.
				entry.push_back(c.mTitle);
				entry.push_back(ConvertInteger(c.mThreadList.size()));
				entry.push_back(ConvertInteger(c.mLastUpdateTime));
				output.push_back(entry);
				entry.clear();
			}
		}
		else {
			g_Logs.data->error("Failed to read category %v", c.mID);
		}
	}
}

void IGFManager :: EnumThreadList(int parentID, MULTISTRING &output)
{
	IGFCategory category = GetPagedCategoryPtr(parentID);
	if(category.mID == 0)
		return;

	//Pregenerate a list of threads
	std::vector<IGFThread> results;
	for(size_t i = 0; i < category.mThreadList.size(); i++)
	{
		IGFThread thread = GetPagedThreadPtr(category.mThreadList[i]);
		if(thread.mID != 0)
			results.push_back(thread);
	}

	if(category.mFlags.hasFlag(IGFFlags::FLAG_SORTALPHABETICAL))
		std::sort(results.begin(), results.end(), ThreadSortAlphabetical);

	STRINGLIST entry;
	for(auto it = results.begin(); it != results.end(); ++it)
	{
		entry.push_back(ConvertInteger(TYPE_THREAD));
		entry.push_back(ConvertInteger(it->mID));
		entry.push_back(ConvertInteger(it->mLocked));
		entry.push_back(ConvertInteger(it->mStickied));
		entry.push_back(it->mTitle);
		entry.push_back(ConvertInteger(it->mPostList.size()));
		entry.push_back(ConvertInteger(it->mLastUpdateTime));

		output.push_back(entry);
		entry.clear();
	}
}

bool IGFManager :: ThreadSortAlphabetical(const IGFThread &lhs, const IGFThread &rhs)
{
	if(lhs.mStickied == rhs.mStickied)
	{
		if(lhs.mTitle.compare(rhs.mTitle) <= 0)
			return true;
	}
	else
	{
		if(lhs.mStickied == true)
			return true;
	}
	
	return false;
}

void IGFManager :: GetCategory(int id, MULTISTRING &output)
{
	IGFCategory category = GetPagedCategoryPtr(id);
	if(category.mID == 0)
		return;

	STRINGLIST header;

	header.push_back(ConvertInteger(id));
	header.push_back(category.mTitle);
	output.push_back(header);

	EnumCategoryList(id, output);
	EnumThreadList(id, output);
}

void IGFManager :: OpenCategory(int type, int id, MULTISTRING &output)
{
	//Expand an object.  If it's a category, enumerate a list of subcategories.
	if(type == TYPE_CATEGORY)
	{
		IGFCategory category = GetPagedCategoryPtr(id);
		if(category.mID != 0)
		{
			STRINGLIST header;
			header.push_back(ConvertInteger(id));
			header.push_back(category.mTitle);

			output.push_back(header);

			int searchID = category.mID;
			EnumCategoryList(searchID, output);
			EnumThreadList(searchID, output);
		}
	}
}

void IGFManager :: OpenThread(int threadID, int startPost, int requestedCount, MULTISTRING &output)
{
	IGFThread thread = GetPagedThreadPtr(threadID);
	if(thread.mID == 0)
		return;

	startPost = Util::ClipInt(startPost, 0, thread.mPostList.size() - 1);

	STRINGLIST row;

	//We retrieve the time offset since the first session since the client uses
	//4 byte integers which theoretically may not be large enough to hold the time data.
	unsigned long timeOffset = g_PlatformTime.getAbsoluteMinutes();

	//Prepare the header
	row.push_back(ConvertInteger(threadID));  //[0]
	row.push_back(thread.mTitle);   //[1]
	row.push_back(ConvertInteger(startPost));  //[2]
	row.push_back(ConvertInteger(thread.mPostList.size()));  //[3]
	row.push_back(ConvertInteger(timeOffset - thread.mLastUpdateTime));  //[4]
	output.push_back(row);
	row.clear();

	//Append the post data.
	int count = 0;
	for(size_t i = startPost; i < thread.mPostList.size(); i++)
	{
		IGFPost post = GetPagedPostPtr(thread.mPostList[i]);
		if(post.mID == 0)
		{
			g_Logs.data->error("OpenThread: unable to find post: %v", thread.mPostList[i]);
			continue;
		}

		row.push_back(ConvertInteger(post.mID));  //[0]
		row.push_back(post.mCreatorName.c_str());  //[1]
		row.push_back(post.mCreationTime.c_str());  //[2]
		row.push_back(ConvertInteger(timeOffset - post.mPostedTime)); //[3]
		row.push_back(post.mBodyText.c_str());  //[4]
		row.push_back(ConvertInteger(post.mEditCount)); //[5]
		row.push_back(ConvertInteger(timeOffset - post.mLastUpdateTime)); //[6]

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
	
	IGFThread thread;
	if(type == POST_THREAD)  //New thread
	{
		if(HasInvalidCharacters(threadTitle, false))
			return ERROR_INVALIDTITLETEXT;
		if(strlen(threadTitle) > MAX_TITLE_LENGTH)
			return ERROR_TITLELENGTH;

		IGFCategory category = GetPagedCategoryPtr(placementID);
		if(category.mID == 0)
			return ERROR_INVALIDCATEGORY;
		if(category.mLocked == true)
			return ERROR_CATEGORYLOCKED;

		thread.mID = GetNewThreadID();
		thread.mParentCategory = category.mID;
		thread.mTitle = threadTitle;
		thread.mCreationAccount = callerAccount->ID;
		thread.mCreatorName = displayName;
		thread.SetLastUpdateTime();

		category.mThreadList.push_back(thread.mID);
	}
	else if(type == POST_REPLY)
		thread = GetPagedThreadPtr(placementID);

	//If creating a thread or replying, take the thread ID and append a new post.
	if(type == POST_THREAD || type == POST_REPLY)
	{
		if(thread.mID == 0)
			return ERROR_INVALIDTHREAD;
		if(thread.mLocked == true)
			return ERROR_THREADLOCKED;

		if(strlen(postBody) > MAX_POST_LENGTH)
			return ERROR_POSTLENGTH;

		IGFCategory category = GetPagedCategoryPtr(thread.mParentCategory);
		if(category.mID == 0)
			return ERROR_INVALIDCATEGORY;

		category.SetLastUpdateTime();
		thread.SetLastUpdateTime();

		IGFPost newPost;
		newPost.mID = GetNewPostID();
		newPost.mParentThread = thread.mID;
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
		thread.mPostList.push_back(newPost.mID);
		SortCategoryThreads(&category);
		g_ClusterManager.WriteEntity(&category, false);
		g_ClusterManager.WriteEntity(&thread, false);
		g_ClusterManager.WriteEntity(&newPost, false);
	}
	else if(type == POST_EDIT)
	{
		if(thread.mID == 0)
			return ERROR_INVALIDTHREAD;
		if(thread.mLocked == true && GetEditPermission(callerAccount, -1) == false)
			return ERROR_THREADLOCKED;

		if(strlen(postBody) > MAX_POST_LENGTH)
			return ERROR_POSTLENGTH;

		IGFPost post = GetPagedPostPtr(postID);
		if(post.mID == 0)
			return ERROR_INVALIDPOST;

		IGFCategory category = GetPagedCategoryPtr(thread.mParentCategory);
		if(category.mID == 0)
			return ERROR_INVALIDCATEGORY;

		if(GetEditPermission(callerAccount, post.mCreationAccount) == false)
			return ERROR_PERMISSIONDENIED;

		post.mBodyText = postBody;
		post.SetLastUpdateTime();
		post.mEditCount++;
		ProcessPostBody(post.mBodyText);

		thread.SetLastUpdateTime();
		category.SetLastUpdateTime();
		SortCategoryThreads(&category);
		g_ClusterManager.WriteEntity(&category, false);
		g_ClusterManager.WriteEntity(&thread, false);
		g_ClusterManager.WriteEntity(&post, false);
	}

	return ERROR_NONE;
}

int IGFManager :: CreateCategory(AccountData *callerAccount, int parentCategoryID, const char *name)
{
	if(GetEditPermission(callerAccount, -1) == false)
		return ERROR_PERMISSIONDENIED;

	if(name == NULL)
		return ERROR_INVALIDTITLETEXT;
	if(strlen(name) == 0)
		return ERROR_INVALIDTITLETEXT;

	IGFCategory category = GetPagedCategoryPtr(parentCategoryID);
	if(category.mID == 0)
	{
		g_Logs.data->error("Invalid category: %v", parentCategoryID);
		return ERROR_INVALIDCATEGORY;
	}

	IGFCategory entry;
	entry.mID = GetNewCategoryID();
	entry.mTitle = name;
	entry.mParentCategory = parentCategoryID;
	if(!g_ClusterManager.WriteEntity(&entry))
		return ERROR_INVALIDCATEGORY;

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
	case ERROR_CLUSTER: return "This entity failed to save to the cluster.";
	}
	return "Unknown error.";
}

int IGFManager :: GetNewCategoryID(void)
{
	return g_ClusterManager.NextValue(ID_IGF_CATEGORY_ID);
}

int IGFManager :: GetNewThreadID(void)
{
	return g_ClusterManager.NextValue(ID_IGF_THREAD_ID);
}

int IGFManager :: GetNewPostID(void)
{
	return g_ClusterManager.NextValue(ID_IGF_POST_ID);
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

IGFCategory IGFManager :: GetPagedCategoryPtr(int elementID)
{
	IGFCategory cat;
	cat.mID = elementID;
	g_ClusterManager.ReadEntity(&cat);
	return cat;
}

IGFThread IGFManager:: GetPagedThreadPtr(int elementID)
{
	IGFThread thread;
	thread.mID = elementID;
	g_ClusterManager.ReadEntity(&thread);
	return thread;
}

IGFPost IGFManager:: GetPagedPostPtr(int elementID)
{
	IGFPost post;
	post.mID = elementID;
	g_ClusterManager.ReadEntity(&post);
	return post;
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
	IGFThread thread = GetPagedThreadPtr(threadID);
	if(thread.mID == 0)
		return ERROR_INVALIDTHREAD;
	if(thread.mLocked == true && GetEditPermission(callerAccount, -1) == false)
		return ERROR_THREADLOCKED;

	IGFPost post = GetPagedPostPtr(postID);
	if(post.mID == 0)
		return ERROR_INVALIDPOST;

	bool perm = false;
	if(post.mID == thread.GetLastPostID() && post.mCreationAccount == callerAccount->ID)
		perm = true;
	else
		perm = GetEditPermission(callerAccount, -1);
	if(perm == false)
		return ERROR_PERMISSIONDENIED;

	thread.DeletePost(postID);
	g_ClusterManager.WriteEntity(&thread);
	return ERROR_NONE;
}

int IGFManager :: DeleteObject(AccountData *callerAccount, int objectType, int objectID)
{
	if(GetEditPermission(callerAccount, -1) == false)
		return ERROR_PERMISSIONDENIED;

	if(objectType == TYPE_CATEGORY)
	{
		IGFCategory category = GetPagedCategoryPtr(objectID);
		if(category.mID == 0)
			return ERROR_INVALIDCATEGORY;

		DeleteObjectData(TYPE_CATEGORY, objectID);
	}
	else if(objectType == TYPE_THREAD)
	{
		IGFThread thread = GetPagedThreadPtr(objectID);
		if(thread.mID == 0)
			return ERROR_INVALIDTHREAD;

		IGFCategory parent = GetPagedCategoryPtr(thread.mParentCategory);
		if(parent.mID == 0)
			return ERROR_INVALIDCATEGORY;

		parent.UnattachThread(objectID);
		g_ClusterManager.WriteEntity(&parent, false);
		g_ClusterManager.RemoveEntity(&thread);
	}
	else 
		return ERROR_UNHANDLED;

	return ERROR_NONE;
}

int IGFManager :: SetLockStatus(AccountData *callerAccount, int categoryType, int objectID, bool status)
{
	if(categoryType == TYPE_CATEGORY)
	{
		IGFCategory category = GetPagedCategoryPtr(objectID);
		if(category.mID == 0)
			return ERROR_INVALIDCATEGORY;
		if(GetEditPermission(callerAccount, -1) == false)
			return ERROR_PERMISSIONDENIED;

		category.mLocked = status;
		g_ClusterManager.WriteEntity(&category, false);
		return ERROR_NONE;
	}
	if(categoryType == TYPE_THREAD)
	{
		IGFThread thread = GetPagedThreadPtr(objectID);
		if(thread.mID == 0)
			return ERROR_INVALIDTHREAD;
		if(GetEditPermission(callerAccount, thread.mCreationAccount) == false)
			return ERROR_PERMISSIONDENIED;

		thread.mLocked = status;
		g_ClusterManager.WriteEntity(&thread, false);
		return ERROR_NONE;
	}
	return ERROR_UNHANDLED;
}

int IGFManager :: SetStickyStatus(AccountData *callerAccount, int categoryType, int objectID, bool status)
{
	if(categoryType != TYPE_THREAD)
		return ERROR_NOTATHREAD;

	IGFThread thread = GetPagedThreadPtr(objectID);
	if(thread.mID == 0)
		return ERROR_INVALIDTHREAD;
	if(GetEditPermission(callerAccount, -1) == false)
		return ERROR_PERMISSIONDENIED;

	IGFCategory category = GetPagedCategoryPtr(thread.mParentCategory);
	if(category.mID == 0)
		return ERROR_INVALIDCATEGORY;

	thread.mStickied = status;
	SortCategoryThreads(&category);
	g_ClusterManager.WriteEntity(&thread, false);
	g_ClusterManager.WriteEntity(&category, false);
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
			IGFCategory category = GetPagedCategoryPtr(renameID);
			if(category.mID == 0)
				return ERROR_INVALIDCATEGORY;
			category.mTitle = name;
			g_ClusterManager.WriteEntity(&category, false);
		}
	}
	else if(categoryType == TYPE_THREAD)
	{
		//Only supports rename for threads.
		if(strlen(name) > MAX_TITLE_LENGTH)
			return ERROR_TITLELENGTH;

		if(renameID != 0)
		{
			IGFThread thread = GetPagedThreadPtr(renameID);
			if(thread.mID == 0)
				return ERROR_INVALIDTHREAD;
			thread.mTitle = name;
			g_ClusterManager.WriteEntity(&thread, false);
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

	IGFThread thread = GetPagedThreadPtr(srcID);
	if(thread.mID == 0)
		return ERROR_INVALIDTHREAD;

	IGFCategory categorySrc = GetPagedCategoryPtr(thread.mParentCategory);
	if(categorySrc.mID == 0)
		return ERROR_INVALIDCATEGORY;

	IGFCategory categoryDst = GetPagedCategoryPtr(dstID);
	if(categoryDst.mID == 0)
		return ERROR_INVALIDCATEGORY;

	if(categorySrc.mID == categoryDst.mID)
		return ERROR_TARGETSAME;

	//Change the thread's owning category, swap the threads between containers, and mark them both as changed. 
	thread.mParentCategory = categoryDst.mID;
	categorySrc.UnattachThread(thread.mID);
	categoryDst.AttachThread(thread.mID);
	SortCategoryThreads(&categoryDst);
	g_ClusterManager.WriteEntity(&thread, false);
	g_ClusterManager.WriteEntity(&categorySrc, false);
	g_ClusterManager.WriteEntity(&categoryDst, false);

	return ERROR_NONE;
}

void IGFManager :: DeleteObjectData(int objectType, int objectID)
{
	if(objectType == TYPE_CATEGORY)
	{
		IGFCategory category = GetPagedCategoryPtr(objectID);
		if(category.mID == 0)
			return;

		g_ClusterManager.RemoveEntity(&category);
		g_ClusterManager.ListRemove(LISTPREFIX_IGF_CATEGORIES, StringUtil::Format("%d", objectID));
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
		IGFThread thread = GetPagedThreadPtr(threadID);
		if(thread.mID == 0)
		{
			g_Logs.data->error("IGFManager::SortCategoryThreads thread [%v] does not exist", threadID);
			continue;
		}
		sortObj.mID = threadID;
		sortObj.mLastUpdate = thread.mLastUpdateTime;
		sortObj.mSticky = thread.mStickied;
		threadList.push_back(sortObj);
	}
	std::sort(threadList.begin(), threadList.end());
	category->mThreadList.clear();
	for(size_t i = 0; i < threadList.size(); i++)
		category->mThreadList.push_back(threadList[i].mID);
}
