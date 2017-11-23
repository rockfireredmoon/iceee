// IN-GAME FORUM

#ifndef IGFORUM_H
#define IGFORUM_H

#include <vector>
#include <map>
#include <string>

#include "Util.h"
#include "Account.h"

//Categories are a nested tree, similar in heirarchy to file folders.
//Each category contains its own list of threads.

struct IGFFlags
{
	unsigned int mFlag;

	static const int FLAG_LOCKED = 1;
	static const int FLAG_STICKIED = 2;
	static const int FLAG_SORTALPHABETICAL = 4;

	IGFFlags();
	void setFlag(unsigned short flagBit, bool status);
	int getraw(void);
	void setraw(int value);
	void reset(void);
	bool hasFlag(const int flag);
};

struct IGFCategory
{
	int mID;                       //ID of this category.
	std::string mTitle;            //The title of this category.
	int mParentCategory;           //The ID of the parent category, so it knows what to track back to.
	std::vector<int> mThreadList;  //A list of thread IDs.
	std::vector<int> mMRUList;     //Most Recently Updated
	bool mLocked;                  //Category is locked.
	unsigned long mLastUpdateTime; //Time index of the last post reply.  Not the actual time.  For relational purposes.
	IGFFlags mFlags;
	IGFCategory();
	void Clear(void);
	void UnattachThread(int threadID);
	void AttachThread(int threadID);
	void SetLastUpdateTime(void);
};

struct IGFCategoryPage
{
	typedef std::map<int, IGFCategory> CATEGORYENTRY;
	typedef std::pair<int, IGFCategory> CATEGORYPAGE;
	CATEGORYENTRY mEntries;
	int mPage;
	int mPendingChanges;
	unsigned long mLastAccessTime;
	IGFCategoryPage();
	void SaveFile(std::string filename);
	void LoadFile(std::string filename);
	void InsertEntry(IGFCategory &entry, bool changePending);
	IGFCategory* GetPointerByID(int ID);
	void DeleteObject(int objectID);
	bool QualifyGarbage(void);
};

// Threads contain a list of posts.
struct IGFThread
{
	int mID;                       //ID of this thread.
	std::string mTitle;            //The title of this thread.
	int mCreationAccount;          //Account that created this thread.
	std::string mCreationTime;     //Time index this post was created.
	std::string mCreatorName;      //Name of the person who created this thread.
	int mParentCategory;           //The ID of the category this thread belongs to.
	std::vector<int> mPostList;    //The list of posts in this thread.
	unsigned long mLastUpdateTime; //Time index of the last post reply.  Not the actual time.  For relational purposes.
	bool mLocked;                  //Thread is locked and is not accepting new posts.
	bool mStickied;                //Thread is stickied and should always remain at the top of the list.
	IGFFlags mFlags;
	IGFThread();
	void Clear(void);
	void DeletePost(int ID);
	int GetLastPostID(void);
	void SetLastUpdateTime(void);
};

struct IGFThreadPage
{
	typedef std::map<int, IGFThread> THREADENTRY;
	typedef std::pair<int, IGFThread> THREADPAIR;
	THREADENTRY mEntries;
	int mPage;
	int mPendingChanges;
	unsigned long mLastAccessTime;
	IGFThreadPage();
	void SaveFile(std::string filename);
	void LoadFile(std::string filename);
	void InsertEntry(IGFThread &entry, bool changePending);
	IGFThread* GetPointerByID(int ID);
	bool QualifyGarbage(void);
};

struct IGFPost
{
	int mID;                       //ID of this object.
	int mCreationAccount;          //Account that created this object.
	std::string mCreationTime;     //Time index this object was created.
	std::string mCreatorName;      //Name of the person who created this object.
	int mParentThread;             //The ID of the thread this post belongs to.
	std::string mBodyText;         //Text of this post.
	unsigned long mPostedTime;     //Time index when this post was created.  Not the actual time.  For relational purposes.
	unsigned long mLastUpdateTime; //Time index of the last edit.  Not the actual time.  For relational purposes.
	int mEditCount;                //Number of edits applied to this post.
	IGFPost();
	void Clear(void);
	void SetLastUpdateTime(void);
};

struct IGFPostPage
{
	typedef std::map<int, IGFPost> POSTENTRY;
	typedef std::pair<int, IGFPost> POSTPAIR;
	POSTENTRY mEntries;
	int mPage;
	int mPendingChanges;
	unsigned long mLastAccessTime;
	IGFPostPage();
	void SaveFile(std::string filename);
	void LoadFile(std::string filename);
	void InsertEntry(IGFPost &entry, bool changePending);
	IGFPost* GetPointerByID(int ID);
	bool QualifyGarbage(void);
};

//The core manager that maintains the archives of categories, threads, and posts.
class IGFManager
{
public:
	IGFManager();
	~IGFManager();

	enum
	{
		ERROR_NONE                =  0,  //No error.
		ERROR_INVALIDCATEGORY     = -1,  //Category does not exist.
		ERROR_INVALIDTHREAD       = -2,  //The thread does not exist.
		ERROR_INVALIDPOST         = -3,  //The post does not exist.
		ERROR_INVALIDTITLETEXT    = -4,  //Category or thread title text is invalid.
		ERROR_INVALIDPOSTTEXT     = -5,  //Post text is invalid.
		ERROR_INVALIDNAMETEXT     = -6,  //Character name is invalid.
		ERROR_PERMISSIONDENIED    = -7,  //Permission was denied
		ERROR_CATEGORYLOCKED      = -8,  //Category is not accepting changes.
		ERROR_THREADLOCKED        = -9,  //Thread is not accepting changes.
		ERROR_UNHANDLED           = -10,  //Unhandled or unknown operation.
		ERROR_NOTATHREAD          = -11,  //The operation can only be performed on threads.
		ERROR_POSTBLOCK           = -12,  //The player does not have posting permission.
		ERROR_FORUMLOCKED         = -13,  //The forum is locked, only accessible to admin.
		ERROR_TITLELENGTH         = -14,  //A thread or category title name is too long.
		ERROR_POSTLENGTH          = -15,  //A post's body text is too long.
		ERROR_TARGETNOTCATEGORY   = -16,  //The target location is not a category.
		ERROR_TARGETSAME          = -17   //The target location is the same as the source.
	};

	typedef std::map<int, IGFCategory> CATEGORY;
	typedef std::pair<int, IGFCategory> CATEGORYPAIR;
	typedef std::map<int, IGFThread> THREAD;
	typedef std::pair<int, IGFThread> THREADPAIR;
	typedef std::map<int, IGFPost> POST;
	typedef std::pair<int, IGFPost> POSTPAIR;

	static const int ROOT_CATEGORY = 0;  //An invalid category. This allows a root category while allowing searches to exclude it.
	static const int CATEGORY_PER_PAGE = 64;  //The number of categories in a single savedata page file.
	static const int THREAD_PER_PAGE = 64;    //The number of threads in a single savedata page file.
	static const int POST_PER_PAGE = 64;      //The number of posts in a single savedata page file.

	static const int AUTOSAVE_TIME = 300000; //5 minutes

	static const int TYPE_CATEGORY = 0;
	static const int TYPE_THREAD = 1;

	static const int POST_THREAD = 0;   //A post is starting a new thread: create Thread and Post.
	static const int POST_REPLY  = 1;   //A post is replying to a thread: create Post within Thread.
	static const int POST_EDIT   = 2;   //A post is edited: adjust Post within Thread.
	
	static const int MAX_TITLE_LENGTH = 32;   
	static const int MAX_POST_LENGTH  = 4000;  //Need a sane limits to prevent buffer overflows when compiling the post information into a packet.

	static const unsigned long GARBAGE_CHECK_EXPIRE = 60000;
	static const unsigned long GARBAGE_CHECK_FREQUENCY = 10000;
	static const unsigned long AUTOSAVE_FREQUENCY = 10000;
	unsigned long mNextGarbageCheck;
	unsigned long mNextAutosaveCheck;

	unsigned long mPlatformLaunchMinute;  //Time that the forum system was first instantiatiated (preserved across sessions).

	/* OLD
	CATEGORY mCategory;
	THREAD mThread;
	POST mPost;
	*/
	
	typedef std::map<int, IGFCategoryPage> CATEGORYPAGE;
	typedef std::pair<int, IGFCategoryPage> CATEGORYPAGEPAIR;
	typedef std::map<int, IGFThreadPage> THREADPAGE;
	typedef std::pair<int, IGFThreadPage> THREADPAGEPAIR;
	typedef std::map<int, IGFPostPage> POSTPAGE;
	typedef std::pair<int, IGFPostPage> POSTPAGEPAIR;

	/* NEW */
	CATEGORYPAGE mCategoryPages;
	THREADPAGE mThreadPages;
	POSTPAGE mPostPages;

	int mNextCategoryID;
	int mNextThreadID;
	int mNextPostID;

	bool mForumLocked;

	int GetCategoryPage(int ID);
	int GetThreadPage(int ID);
	int GetPostPage(int ID);

	void EnumCategoryList(int parentID, MULTISTRING &output);
	void EnumThreadList(int parentID, MULTISTRING &output);
	static bool ThreadSortAlphabetical(const IGFThread* lhs, const IGFThread* rhs);

	void GetCategory(int id, MULTISTRING &output);
	void OpenCategory(int type, int id, MULTISTRING &output);
	void OpenThread(int threadID, int startPost, int requestedCount, MULTISTRING &output);
	int SendPost(AccountData *callerAccount, int type, int placementID, int postID, const char *threadTitle, const char *postBody, const char *displayName);
	bool HasInvalidCharacters(const char *text, bool allowMarkup);
	void ProcessPostBody(std::string &postBody);

	unsigned long GetTimeOffset(unsigned long LastUpdateTime);

	bool GetEditPermission(AccountData *callerAccount, int objectOwnerID);
	int DeletePost(AccountData *callerAccount, int threadID, int postID);
	int DeleteObject(AccountData *callerAccount, int objectType, int objectID);
	int SetLockStatus(AccountData *callerAccount, int categoryType, int objectID, bool status);
	int SetStickyStatus(AccountData *callerAccount, int categoryType, int objectID, bool status);
	int CreateCategory(AccountData *callerAccount, int parentCategoryID, const char *name);
	int EditObject(AccountData *callerAccount, int categoryType, int parentID, int renameID, const char *name);
	int RunAction(AccountData *callerAccount, int actionType, int param1, int param2); 
	int RunMove(AccountData *callerAccount, int srcType, int srcID, int dstType, int dstID);

	ChangeData cdCategory;
	ChangeData cdThread;
	ChangeData cdPost;

	void SaveCategory();
	void LoadCategory();

	//std::string mPathCategory;
	//std::string mPathThread;
	//std::string mPathPost;

	void Init(void);

	const char* GetErrorString(int errCode);
	int GetNewCategoryID(void);
	int GetNewThreadID(void);
	int GetNewPostID(void);

	void CheckAutoSave(bool force);
	void SaveConfig(void);
	void LoadConfig(void);

	void RunGarbageCheck(void);

private:
	/* OLD
	void InitPaths(void);
	void InsertCategory(IGFCategory& object);
	void InsertThread(IGFThread& object);
	void InsertPost(IGFPost& object);
	*/

	/* NEW */
	void InsertPagedCategory(IGFCategory& object);
	void InsertPagedThread(IGFThread& object);
	void InsertPagedPost(IGFPost& object);
	IGFCategory* GetPagedCategoryPtr(int elementID);
	IGFThread* GetPagedThreadPtr(int elementID);
	IGFPost* GetPagedPostPtr(int elementID);
	void LoadCategoryPage(int page);
	void LoadThreadPage(int page);
	void LoadPostPage(int page);
	void AutosaveCategory(void);
	void AutosaveThread(void);
	void AutosavePost(void);

	void MarkChangedCategory(int objectID);
	void MarkChangedPost(int objectID);
	void MarkChangedThread(int objectID);

	void DeleteObjectData(int objectType, int objectID);
	void SortCategoryThreads(IGFCategory *category);

	const char* ConvertInteger(int value);
};

extern IGFManager g_IGFManager;

#endif  //#define IGFORUM_H
