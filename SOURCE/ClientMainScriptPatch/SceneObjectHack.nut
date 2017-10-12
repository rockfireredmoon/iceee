require("SceneObject");

/* Reduce loading screens by potentially eliminating a heuristic scenery count that
   forces load screens even for empty tiles.

   This original function scans 9 tiles (3x3 grid) around the avatar's standing tile,
   tallying scenery that needs to be assembled.  The default is to add a heuristic of
   150 objects, even for empty tiles.

   This isn't entirely accurate as it's technically possible that other
   scenery counts may legitimately total up to multiples of 150 exactly, but those
   situations might(?) be rare.

   Alternatively the compiled code could be hex edited to replace 150 with 0, but this
   option should be portable to other client versions which may have modified code and
   different offsets.

   The original function returns either null or a number.
*/

function SceneObjectManager::getPendingRequiredAssemblyCount_Hack()
{
	local result = getPendingRequiredAssemblyCount_Original();
	if(result)
	{
		if(result % 150 == 0)
			result = 0;
	}
	return result;
}

SceneObjectManager.getPendingRequiredAssemblyCount_Original <- SceneObjectManager.getPendingRequiredAssemblyCount;
SceneObjectManager.getPendingRequiredAssemblyCount <- SceneObjectManager.getPendingRequiredAssemblyCount_Hack;
