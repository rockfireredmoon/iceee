// IN-GAME FORUM

#ifndef IGFORUM_H
#define IGFORUM_H

#include <vector>
#include <map>
#include <string>

#include "Util.h"
#include "Entities.h"
#include "Account.h"

static std::string KEYPREFIX_IGF_CATEGORY = "IGFCategory";
static std::string KEYPREFIX_IGF_THREAD = "IGFThread";
static std::string KEYPREFIX_IGF_POST = "IGFPost";
static std::string LISTPREFIX_IGF_CATEGORIES = "IGFCategories";
static std::string ID_IGF_CATEGORY_ID = "NextIGFCategoryID";
static std::string ID_IGF_THREAD_ID = "NextIGFThreadID";
static std::string ID_IGF_POST_ID = "NextIGFPostID";

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

class IGFCategory: public AbstractEntity {
public:
	int mID;                       //ID of this category.
	std::string mTitle;            //The title of this category.
	int mParentCategory;           //The ID of the parent category, so it knows what to track back to.
	std::vector<int> mThreadList;  //A list of thread IDs.
	std::vector<int> mMRUList;     //Most Recently Updated
	bool mLocked;                  //Category is locked.
	unsigned long mLastUpdateTime; //Time index of the last post reply.  Not the actual time.  For relational purposes.
	IGFFlags mFlags;
	IGFCategory();

	bool WriteEntity(AbstractEntityWriter *writer);
	bool ReadEntity(AbstractEntityReader *reader);
	bool EntityKeys(AbstractEntityReader *reader);


	void Clear(void);
	void UnattachThread(int threadID);
	void AttachThread(int threadID);
	void SetLastUpdateTime(void);
};

// Threads contain a list of posts.
class IGFThread: public AbstractEntity {
public:
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

	bool WriteEntity(AbstractEntityWriter *writer);
	bool ReadEntity(AbstractEntityReader *reader);
	bool EntityKeys(AbstractEntityReader *reader);

	void Clear(void);
	void DeletePost(int ID);
	int GetLastPostID(void);
	void SetLastUpdateTime(void);
};

class IGFPost: public AbstractEntity {
public:
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

	bool WriteEntity(AbstractEntityWriter *writer);
	bool ReadEntity(AbstractEntityReader *reader);
	bool EntityKeys(AbstractEntityReader *reader);

	void Clear(void);
	void SetLastUpdateTime(void);
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
		ERROR_TARGETSAME          = -17,   //The target location is the same as the source.
		ERROR_CLUSTER	          = -18   //Cluster error saving or loading.
	};

	static const int ROOT_CATEGORY = 0;  //An invalid category. This allows a root category while allowing searches to exclude it.
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

	bool mForumLocked;

	void EnumCategoryList(int parentID, MULTISTRING &output);
	void EnumThreadList(int parentID, MULTISTRING &output);
	static bool ThreadSortAlphabetical(const IGFThread &lhs, const IGFThread &rhs);

	void GetCategory(int id, MULTISTRING &output);
	void OpenCategory(int type, int id, MULTISTRING &output);
	void OpenThread(int threadID, int startPost, int requestedCount, MULTISTRING &output);
	int SendPost(AccountData *callerAccount, int type, int placementID, int postID, const char *threadTitle, const char *postBody, const char *displayName);
	bool HasInvalidCharacters(const char *text, bool allowMarkup);
	void ProcessPostBody(std::string &postBody);

	bool GetEditPermission(AccountData *callerAccount, int objectOwnerID);
	int DeletePost(AccountData *callerAccount, int threadID, int postID);
	int DeleteObject(AccountData *callerAccount, int objectType, int objectID);
	int SetLockStatus(AccountData *callerAccount, int categoryType, int objectID, bool status);
	int SetStickyStatus(AccountData *callerAccount, int categoryType, int objectID, bool status);
	int CreateCategory(AccountData *callerAccount, int parentCategoryID, const char *name);
	int EditObject(AccountData *callerAccount, int categoryType, int parentID, int renameID, const char *name);
	int RunAction(AccountData *callerAccount, int actionType, int param1, int param2); 
	int RunMove(AccountData *callerAccount, int srcType, int srcID, int dstType, int dstID);

	void SaveCategory();
	void LoadCategory();

	void Init(void);

	const char* GetErrorString(int errCode);
	int GetNewCategoryID(void);
	int GetNewThreadID(void);
	int GetNewPostID(void);

private:

	IGFCategory GetPagedCategoryPtr(int elementID);
	IGFThread GetPagedThreadPtr(int elementID);
	IGFPost GetPagedPostPtr(int elementID);

	void DeleteObjectData(int objectType, int objectID);
	void SortCategoryThreads(IGFCategory *category);

	const char* ConvertInteger(int value);
};

extern IGFManager g_IGFManager;

#endif  //#define IGFORUM_H
