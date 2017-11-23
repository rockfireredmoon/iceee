// Contains definitions for all special objects that the player can interact with.
// Examples are dungeon entrance/exit or room warps.

#include <vector>
#include <string>

class InteractObject
{
public:
	InteractObject();
	~InteractObject();
	void Clear(void);
	void SetName(char *str);
	void SetType(char *str);
	void SetMessage(char *str);
	void SetScriptFunction(char *str);

	char internalName[64];
	char useMessage[64];  //Message to display in the client interact bar.
	int useTime;          //Time to display interact countdown bar.
	int CreatureDefID;  //Creature spawn to interact with.
	int opType;        //Corresponds to one of the type constants below.
	int questReq;    //Quest ID.  May only use this interact if the quest is active, or completed.
	bool questComp;  //Extention to above.  If this is true, the quest MUST be completed.  May not use if currently active.
	int zoneReq;     //The interact may only be used in this zone.  If zero, may be used in any standard gameplay (non grove) zone.
	short facing;    //Directional facing to set the player after they use the interact (0 to 255).
	int cost;        //Cost to use, if a hange.
	char scriptFunction[128]; // Function in instance script to call when interacted with

	static const int TYPE_NONE = 0;
	static const int TYPE_WARP = 1;
	static const int TYPE_HENGE = 2;
	static const int TYPE_LOCATIONRETURN = 3;
	static const int TYPE_SCRIPT = 4;

	static const int DEFAULT_USE_TIME = 2000;

	//Data for warps
	int WarpX;
	int WarpY;
	int WarpZ;
	int WarpID;
};

class InteractObjectContainer
{
public:
	InteractObjectContainer();
	~InteractObjectContainer();

	std::vector<InteractObject> objList;

	int FilterByName(char *className, std::vector<InteractObject*> &resultPtrList);
	int FilterByCDefID(int CDefID, std::vector<InteractObject*> &resultPtrList);
	InteractObject* GetObjectByID(int CDefID, int zoneID);
	InteractObject* GetHengeByDefID(int CDefID);
	InteractObject* GetHengeByTargetName(const char* name);

	void LoadFromFile(std::string filename);

private:
	void AddItem(InteractObject &newObject);
};

extern InteractObjectContainer g_InteractObjectContainer;
