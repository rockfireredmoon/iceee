function deepClone( pData )
{
	if (this.type(pData) == "table")
	{
		return this.cloneTable(pData);
	}
	else if (this.type(pData) == "array")
	{
		return this.cloneArray(pData);
	}
}

function cloneArray( pArray )
{
	local newArray = [];

	foreach( x, i in pArray )
	{
		if (this.type(i) == "table")
		{
			newArray.insert(x, this.cloneTable(i));
		}
		else if (this.type(i) == "array")
		{
			newArray.insert(x, this.cloneArray(i));
		}
		else
		{
			newArray.insert(x, i);
		}
	}

	return newArray;
}

function cloneTable( pTable )
{
	local newTable = {};

	foreach( x, i in pTable )
	{
		if (this.type(i) == "table")
		{
			newTable[x] <- this.cloneTable(i);
		}
		else if (this.type(i) == "array")
		{
			newTable[x] <- this.cloneArray(i);
		}
		else
		{
			newTable[x] <- i;
		}
	}

	return newTable;
}

