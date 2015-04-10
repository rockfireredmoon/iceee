class ItemEntry
{
	enum
	{
		TYPE_ITEM = 0,
		TYPE_GOLD = 1,
	}
	union
	{
		int mItemID;
		int mGoldAmount;
	};
};

struct ItemTransaction
{
	int mRecipientDefID;
	int mGold;
	std::vector<int> mItemID;
}