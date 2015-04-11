this.require("GUI/Component");
class this.GUI.Image extends this.GUI.Container
{
	static mClassName = "GUI.Image";
	mImageName = null;
	constructor( ... )
	{
		this.GUI.Container.constructor();
		this.setAppearance("Icon");
		this.setPreferredSize(32, 32);

		if (vargc > 0)
		{
			this.setImageName(vargv[0]);
		}
	}

	function _updateMaterial( imageName, componentToUpdate, suffix )
	{
		local extTest = imageName != null ? imageName.tolower() : "";

		if (this.Util.endsWith(extTest, ".png"))
		{
			local materialName;

			if (vargc > 0)
			{
				materialName = this.mSkin ? this.mSkin + "/" + vargv[0] : vargv[0];
			}
			else
			{
				materialName = this.mSkin + suffix;
			}

			local material = this._root.createMaterialUsingAliases(materialName, {
				Diffuse = imageName
			});
			componentToUpdate.setMaterial(material, false);
		}
		else if (imageName == null || imageName == "")
		{
			componentToUpdate.setMaterial("Icon/QuestionMark");
		}
		else
		{
			componentToUpdate.setMaterial(imageName);
		}
	}

	function setImageName( imageName )
	{
		this._updateMaterial(imageName, this, "/Icon");
		this.mImageName = imageName;
	}

	function getImageName()
	{
		return this.mImageName;
	}

}

