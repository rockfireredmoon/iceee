this.File <- {
	rxExtension = this.regexp("\\.[^.]+$"),
	rxBasename = this.regexp("[^\\/]+$"),
	rxPkgParts = this.regexp("([^#]+)#(.*)$")
};
this.File.basename <- function ( file, ... )
{
	local res;

	if (vargc > 0)
	{
		res = this.rxExtension.search(file);

		if (res)
		{
			local ext = file.slice(res.begin, res.end);

			if (ext == vargv[0] || ext == "." + vargv[0])
			{
				file = file.slice(0, res.begin);
			}
		}
	}

	res = this.rxBasename.search(file);

	if (res)
	{
		file = file.slice(res.begin, res.end);
	}

	return file;
};
this.File.extension <- function ( filename )
{
	local res = this.rxExtension.search(filename);

	if (res)
	{
		return filename.slice(res.begin + 1, res.end);
	}

	return null;
};
this.File.splitAssetRef <- function ( name )
{
	this.log.warn("File::splitAssetRef(\"" + name + "\") is deprecated. Use AssetReference() class.");
	local res = this.rxPkgParts.capture(name);
	local parts = [];

	if (res != null)
	{
		parts.append(name.slice(res[1].begin, res[1].end));
		parts.append(name.slice(res[2].begin, res[2].end));

		if (res.len() > 3)
		{
			parts.append(this.System.decodeVars(name.slice(res[4].begin, res[4].end)));
		}
	}
	else
	{
		parts.append(name);
	}

	return parts;
};
