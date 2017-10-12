this.require("ErrorChecking");
this.PropertyType <- {};
class this.AbstractPropertyType 
{
	function toString( value )
	{
		return value == null ? "" : "" + value;
	}

	function toValue( value )
	{
		throw this.Exception("PropertyEditor.toValue() not implemented");
	}

	function createEditorComponent()
	{
		return null;
	}

	function hasCustomEditor()
	{
		return false;
	}

	function getEnumValues()
	{
		return null;
	}

	function setEditorValue( editor, value )
	{
		editor.setValue(this.toValue(value));
	}

}

class this.PropertyType.Vector3 extends this.AbstractPropertyType
{
	function toValue( value )
	{
		return this.Vector3(value);
	}

}

class this.PropertyType.Quaternion extends this.AbstractPropertyType
{
	function toValue( value )
	{
		return this.Quaternion(value);
	}

}

class this.PropertyType.Boolean extends this.AbstractPropertyType
{
	constructor( value )
	{
	}

	function toString( value )
	{
		return value ? "True" : "False";
	}

	function toValue( value )
	{
		return this.Util.atob(value);
	}

	function setEditorValue( editor, value )
	{
		editor.setValue(this.Util.atob(value) ? "True" : "False");
	}

	function getEnumValues()
	{
		return [
			"False",
			"True"
		];
	}

}

class this.PropertyType.BitFieldBoolean extends this.AbstractPropertyType
{
	mMask = 0;
	constructor( mask )
	{
		this.mMask = mask;
	}

	function toString( value )
	{
		return value ? "True" : "False";
	}

	function toValue( value )
	{
		value = value.tolower();

		if (value == "true" || value == "yes" || value == "on")
		{
			return this.mMask;
		}

		return 0;
	}

	function getEnumValues()
	{
		return [
			"False",
			"True"
		];
	}

}

class this.PropertyType.Integer extends this.AbstractPropertyType
{
	function toValue( value )
	{
		return value.tointeger();
	}

}

class this.PropertyType.Float extends this.AbstractPropertyType
{
	function toValue( value )
	{
		return value.tofloat();
	}

}

class this.PropertyType.String extends this.AbstractPropertyType
{
	function toValue( value )
	{
		return value;
	}

}

class this.PropertyType.AssetReference extends this.AbstractPropertyType
{
	function toValue( value )
	{
		return this.AssetReference(value);
	}

	function hasCustomEditor()
	{
		return true;
	}

	function createEditorComponent()
	{
		return this.GUI.AssetRefInputBox();
	}

}

class this.PropertyType.StringTable extends this.AbstractPropertyType
{
	function toString( value )
	{
		return value == null ? "" : this.System.encodeVars(value);
	}

	function toValue( value )
	{
		return this.System.decodeVars(value);
	}

}

class this.PropertyType.AxisRotation extends this.AbstractPropertyType
{
	static Y_AXIS = this.Vector3().UNIT_Y;
	function toString( value )
	{
		local axis = value.getAxis();
		local angle = value.getAngle() * 180.0 / this.Math.PI;
		local dot = axis.dot(this.Vector3().UNIT_Y);

		if (dot > 0.99900001)
		{
			return "" + angle;
		}
		else if (dot < -0.99900001)
		{
			return "" + (360 - angle);
		}
		else if (dot < 0)
		{
			axis.negate();
			angle = 360 - angle;
		}

		return "" + angle + " @ " + axis;
	}

	function toValue( value )
	{
		local angle;
		local axis;
		local p = value.find("@");

		if (p)
		{
			angle = value.slice(0, p);
			axis = value.slice(p + 1);
			angle = angle.tofloat() * this.Math.PI / 180.0;
			axis = this.Vector3(axis);
		}
		else
		{
			local parts = ::Util.split(value, " ");

			if (parts.len() == 4)
			{
				return this.Quaternion(parts[0].tofloat(), parts[1].tofloat(), parts[2].tofloat(), parts[3].tofloat());
			}
			else
			{
				angle = value.tofloat() * this.Math.PI / 180.0;
				axis = this.Vector3().UNIT_Y;
			}
		}

		return this.Quaternion(angle, axis);
	}

}

class this.PropertyType.UniformScale extends this.AbstractPropertyType
{
	function toString( value )
	{
		return "" + value.x;
	}

	function toValue( value )
	{
		local x = value.tofloat();
		return this.Vector3(x, x, x);
	}

}

class this.PropertyType.ObjectID extends this.AbstractPropertyType
{
	function toValue( value )
	{
		return value.tointeger();
	}

	function createEditorComponent()
	{
		return this.GUI.Label();
	}

}

class this.PropertyType.ErrorDetails extends this.AbstractPropertyType
{
	function toValue( value )
	{
		return value;
	}

	function createEditorComponent()
	{
		return this.GUI.Button("Error Details");
	}

}

class this.PropertyType.CreatureAppearance extends this.AbstractPropertyType
{
	function toValue( value )
	{
		return value;
	}

	function createEditorComponent()
	{
		return this.GUI.CreatureTweakButton();
	}

}

this._ExamplePropertyDescriptor <- {
	displayName = "World Position",
	serverName = "worldpos",
	name = "position",
	type = this.PropertyType.Vector3,
	setMethod = null,
	getMethod = null,
	shortDescription = null
};
class this.Property 
{
	static _rxPropertyName = this.regexp("^[a-zA-Z0-9_]+$");
	constructor( name, type, ... )
	{
		if (typeof name != "string" || !this._rxPropertyName.match(name))
		{
			throw this.Exception("Invalid property name: " + name);
		}

		if (!("toValue" in type) || !("toString" in type) || !("getEnumValues" in type))
		{
			throw this.Exception("Invalid property type implementation: " + type);
		}

		this.mName = name;
		this.mType = type;

		if (vargc > 0)
		{
			local opts = vargv[0];

			if ("displayName" in opts)
			{
				this.mDisplayName = opts.displayName;
			}

			if ("serverName" in opts)
			{
				this.mServerName = opts.serverName;
			}

			if ("serverGetMethod" in opts)
			{
				this.mServerGetMethod = opts.serverGetMethod;
			}

			if ("getMethod" in opts)
			{
				this.mGetMethod = opts.getMethod;
			}

			if ("serverGetQuery" in opts)
			{
				this.mServerGetQuery = opts.serverGetQuery;
			}

			if ("defaultValue" in opts)
			{
				this.mDefaultValue = opts.defaultValue;
			}

			if ("setMethod" in opts)
			{
				this.mSetMethod = opts.setMethod;
			}

			if ("shortDescription" in opts)
			{
				this.mShortDescription = opts.shortDescription;
			}
		}
	}

	static function _ucfirst( str )
	{
		if (str.len() == 0)
		{
			return str;
		}

		if (str.len() == 1)
		{
			return str.toupper();
		}

		return str.slice(0, 1).toupper() + str.slice(1);
	}

	function getDisplayName()
	{
		return this.mDisplayName != null ? this.mDisplayName : this._ucfirst(this.mName);
	}

	function isReadOnly()
	{
		return (this.mSetMethod == null || this.mSetMethod == "") && this.mServerName == null;
	}

	function hasCustomEditor()
	{
		return "createEditorComponent" in this.mType;
	}

	function getType()
	{
		return this.mType;
	}

	function getName()
	{
		return this.mName;
	}

	function getServerGetQuery()
	{
		return this.mServerGetQuery;
	}

	function getDefaultValue()
	{
		return this.mDefaultValue;
	}

	function getValue( bean )
	{
		if (this.mServerGetQuery)
		{
			return this.mDefaultValue;
		}

		local methodName = this.mGetMethod;

		if (typeof methodName == "closure")
		{
			return methodName(bean);
		}

		if (typeof methodName == "function")
		{
			return methodName(bean);
		}

		if (methodName == null)
		{
			local suffix = this._ucfirst(this.mName);

			if ("get" + suffix in bean)
			{
				methodName = "get" + suffix;
			}
			else if ("is" + suffix in bean)
			{
				methodName = "is" + suffix;
			}
		}

		if (methodName != null)
		{
			return bean[methodName]();
		}
		else
		{
			local name = this.mName;

			if (name in bean)
			{
				return bean[this.mName];
			}

			name = "m" + this._ucfirst(this.mName);

			if (name in bean)
			{
				return bean[name];
			}

			throw this.Exception("Cannot find getter or field for property: " + this.mName);
		}
	}

	function getAsText( bean )
	{
		return this.mType.toString(this.getValue(bean));
	}

	function onQueryComplete( qa, results )
	{
		local bean = ::_sceneObjectManager.getSceneryByID(qa.args[0]);

		if (bean == null)
		{
			bean = ::_sceneObjectManager.getCreatureByID(qa.args[0]);
		}

		if (bean == null)
		{
			return;
		}

		local valueStr = qa.args[2];
		local value = this.mType.toValue(valueStr);
		local methodName = this.mSetMethod;

		if (this.mServerGetMethod != null)
		{
			if (this.mServerGetMethod == "getFlags")
			{
				if (methodName == "setLocked")
				{
					value = valueStr.tointeger();
					value = value & bean.LOCKED;
				}
				else if (methodName == "setPrimary")
				{
					value = valueStr.tointeger();
					value = value & bean.PRIMARY;
				}
			}
		}

		this._setRealValue(bean, value);

		if ("fireUpdate" in bean)
		{
			bean.fireUpdate();
		}
	}

	function onQueryError( qa, error )
	{
		if (qa.query == "scenery.edit" || qa.query == "scenery.delete")
		{
			::_ChatWindow.addMessage("err/", error, "General");
		}
	}

	function _setRealValue( bean, value )
	{
		if (this.isReadOnly())
		{
			throw this.Exception("Property \"" + this.mName + "\" is read-only");
		}

		local methodName = this.mSetMethod;

		if (typeof methodName == "closure")
		{
			methodName(bean, value);
			return;
		}

		if (typeof methodName == "function")
		{
			methodName(bean, value);
			return;
		}

		if (methodName == null)
		{
			local suffix = this._ucfirst(this.mName);

			if ("set" + suffix in bean)
			{
				methodName = "set" + suffix;
			}
		}

		if (methodName != null)
		{
			bean[methodName](value);
			return;
		}
		else
		{
			local name = this.mName;

			if (name in bean)
			{
				bean[name] = value;
				return;
			}

			name = "m" + this._ucfirst(this.mName);

			if (name in bean)
			{
				bean[name] = value;
				return;
			}

			if (this.mServerName == null || this.mServerName == "")
			{
				throw this.Exception("Cannot find setter or field for " + this.mName);
			}
		}
	}

	function _postUpdate( bean, ... )
	{
		local def = this.Bean.getBeanDescriptor(bean);

		if (("serverSetQuery" in def) && this.mServerName)
		{
			if (!("getID" in bean))
			{
				throw this.Exception("cannot determine server ID for bean");
			}

			local value;

			if (vargc == 0)
			{
				if (this.mServerGetMethod == null)
				{
					value = this.mType.toString(this.getValue(bean));
				}
				else
				{
					value = "" + bean[this.mServerGetMethod]().tostring();
				}
			}
			else
			{
				value = vargv[0];
			}

			this._Connection.sendQuery(def.serverSetQuery, this, [
				bean.getID(),
				this.mServerName,
				value
			]);
		}
	}

	function setValue( bean, value )
	{
		if (this.isReadOnly())
		{
			throw this.Exception("Property \"" + this.mName + "\" is read-only");
		}

		local methodName = this.mSetMethod;
		local oldValue = value;

		if (this.mServerGetMethod == null)
		{
			value = this.mType.toString(value);
		}
		else
		{
			local previousData;

			if (methodName != null && this.mServerGetMethod == "getFlags")
			{
				if (methodName == "setLocked")
				{
					previousData = bean.isLocked;
				}
				else if (methodName == "setPrimary")
				{
					previousData = bean.isPrimary;
				}

				bean[methodName](value);
			}

			value = "" + bean[this.mServerGetMethod]().tostring();

			if (this.mServerGetMethod == "getFlags" && previousData)
			{
				bean[methodName](previousData);
			}
		}

		this.print("ServerGetMethod: " + this.mServerGetMethod);
		local def = this.Bean.getBeanDescriptor(bean);

		if (typeof methodName == "closure")
		{
			if (("serverSetQuery" in def) && this.mServerName)
			{
				this._postUpdate(bean, value);
			}
			else
			{
				methodName(bean, oldValue);
			}

			return;
		}

		if (typeof methodName == "function")
		{
			if (("serverSetQuery" in def) && this.mServerName)
			{
				this._postUpdate(bean, value);
			}
			else
			{
				methodName(bean, oldValue);
			}

			return;
		}

		if (methodName == null)
		{
			local suffix = this._ucfirst(this.mName);

			if ("set" + suffix in bean)
			{
				methodName = "set" + suffix;
			}
		}

		if (methodName != null)
		{
			this._postUpdate(bean, value);
			return;
		}
		else
		{
			local name = this.mName;

			if (name in bean)
			{
				this._postUpdate(bean, value);
				return;
			}

			name = "m" + this._ucfirst(this.mName);

			if (name in bean)
			{
				this._postUpdate(bean, value);
				return;
			}

			if (this.mServerName != null && this.mServerName != "")
			{
				this._postUpdate(bean, value);
				return;
			}

			throw this.Exception("Cannot find setter or field for " + this.mName);
		}
	}

	function setAsText( bean, value )
	{
		if (typeof value != "string")
		{
			throw this.Exception("Invalid value argument (must be string)");
		}

		this.setValue(bean, this.mType.toValue(value));
	}

	function _tostring()
	{
		return this.getDisplayName();
	}

	mName = null;
	mType = null;
	mDisplayName = null;
	mServerName = null;
	mServerGetMethod = null;
	mServerGetQuery = null;
	mDefaultValue = null;
	mSetMethod = null;
	mGetMethod = null;
	mShortDescription = null;
}

this.Bean <- {};
this.Bean.getBeanDescriptor <- function ( bean )
{
	if (typeof bean != "instance" && typeof bean != "table")
	{
		throw this.Exception("Invalid bean (must be table or instance)");
	}

	foreach( bd in this.BeanDef )
	{
		if ("matches" in bd)
		{
			if (bd.matches(bean))
			{
				return bd;
			}
		}
	}

	if (!("getObjectClass" in bean))
	{
		throw this.Exception("Bean (" + bean + ") does not have a getObjectClass method");
	}

	local type = bean.getObjectClass();

	if (!(type in ::BeanDef))
	{
		throw this.Exception("Bean type \"" + type + "\" is not defined");
	}

	return ::BeanDef[type];
};
this.Bean.getPropertyDescriptors <- function ( bean )
{
	local beandesc = this.Bean.getBeanDescriptor(bean);
	return beandesc.properties;
};
this.Bean.getPropertyDescriptor <- function ( bean, propertyName )
{
	local props = this.Bean.getPropertyDescriptors(bean);
	local i;
	local prop;
	propertyName = propertyName.tolower();

	foreach( i, prop in props )
	{
		if (prop.mName == propertyName)
		{
			return prop;
		}
	}

	throw this.Exception("Bean property not found for " + bean.getObjectClass() + ": " + propertyName);
};
this.require("SceneObject");
this.BeanDef <- {};
this.BeanDef.Scenery <- {
	serverSetQuery = "scenery.edit",
	serverDelQuery = "scenery.delete",
	properties = [
		this.Property("ID", this.PropertyType.Integer, {
			displayName = "ID",
			serverName = "ID",
			getMethod = "getID",
			setMethod = ""
		}),
		this.Property("name", this.PropertyType.String, {
			displayName = "Name",
			serverName = "name",
			getMethod = "getSceneryName",
			setMethod = "setSceneryName"
		}),
		this.Property("asset", this.PropertyType.AssetReference, {
			serverName = "asset",
			getMethod = "getVarsTypeAsAsset",
			setMethod = "setVarsTypeFromAsset"
		}),
		this.Property("position", this.PropertyType.Vector3, {
			serverName = "p"
		}),
		this.Property("scale", this.PropertyType.UniformScale, {
			serverName = "s"
		}),
		this.Property("rotation", this.PropertyType.AxisRotation, {
			displayName = "Rotation",
			serverName = "q",
			getMethod = "getOrientation",
			setMethod = "setOrientation"
		}),
		this.Property("locked", this.PropertyType.BitFieldBoolean(this.SceneObject.LOCKED), {
			displayName = "Locked",
			setMethod = "setLocked",
			serverName = "flags",
			serverGetMethod = "getFlags"
		}),
		this.Property("primary", this.PropertyType.BitFieldBoolean(this.SceneObject.PRIMARY), {
			displayName = "Primary",
			serverName = "flags",
			setMethod = "setPrimary",
			serverGetMethod = "getFlags"
		}),
		this.Property("layer", this.PropertyType.String, {
			displayName = "Layer",
			serverName = "layer",
			getMethod = "getSceneryLayer",
			setMethod = "setSceneryLayer"
		})
	]
};
this.BeanDef.Creature <- {
	serverSetQuery = "creature.edit",
	properties = [
		this.Property("ID", this.PropertyType.ObjectID, {
			setMethod = ""
		}),
		this.Property("type", this.PropertyType.Integer, {
			setMethod = ""
		}),
		this.Property("name", this.PropertyType.String, {
			setMethod = ""
		}),
		this.Property("appearance", this.PropertyType.CreatureAppearance, {
			getMethod = "getType",
			setMethod = ""
		}),
		this.Property("position", this.PropertyType.Vector3, {
			serverName = "position"
		}),
		this.Property("creatureSpawnLayer", this.PropertyType.String, {
			displayName = "Layer",
			serverName = "creatureSpawnLayer",
			serverGetQuery = "spawn.property"
		})
	]
};
this.BeanDef.SpawnPoint <- {
	function matches( bean )
	{
		if (("getType" in bean) && bean.getType() == "Manipulator-SpawnPoint")
		{
			return true;
		}

		return false;
	}

	serverSetQuery = "scenery.edit",
	serverDelQuery = "scenery.delete",
	properties = [
		this.Property("ID", this.PropertyType.Integer, {
			displayName = "ID",
			serverName = "ID",
			getMethod = "getID",
			setMethod = ""
		}),
		this.Property("position", this.PropertyType.Vector3, {
			serverName = "p"
		}),
		this.Property("rotation", this.PropertyType.AxisRotation, {
			displayName = "Rotation",
			serverName = "q",
			getMethod = "getOrientation",
			setMethod = "setOrientation",
			serverGetMethod = "getOrientation"
		}),
		this.Property("locked", this.PropertyType.BitFieldBoolean(this.SceneObject.LOCKED), {
			displayName = "Locked",
			serverName = "flags",
			setMethod = "setLocked",
			serverGetMethod = "getFlags"
		}),
		this.Property("InnerRadius", this.PropertyType.Float, {
			function setMethod( bean, value )
			{
				bean.addProperty("innerRadius", value);
				this._buildTool.updateSpawnerFeedback(bean);
			}

			function getMethod( bean )
			{
				local props = bean.getProperties();
				return this.Util.tableSafeGet(props, "innerRadius", 0);
			}

		}),
		this.Property("OuterRadius", this.PropertyType.Float, {
			function setMethod( bean, value )
			{
				bean.addProperty("outerRadius", value);
				this._buildTool.updateSpawnerFeedback(bean);
			}

			function getMethod( bean )
			{
				local props = bean.getProperties();
				return this.Util.tableSafeGet(props, "outerRadius", 0);
			}

		}),
		this.Property("spawnName", this.PropertyType.String, {
			displayName = "Name",
			serverName = "spawnName",
			serverGetQuery = "spawn.property"
		}),
		this.Property("leaseTime", this.PropertyType.Integer, {
			displayName = "Lease Time",
			serverName = "leaseTime",
			serverGetQuery = "spawn.property"
		}),
		this.Property("spawnPackage", this.PropertyType.String, {
			displayName = "Package",
			serverName = "spawnPackage",
			serverGetQuery = "spawn.property"
		}),
		this.Property("mobTotal", this.PropertyType.Integer, {
			displayName = "Mob Total",
			serverName = "mobTotal",
			serverGetQuery = "spawn.property"
		}),
		this.Property("maxActive", this.PropertyType.Integer, {
			displayName = "Max Active",
			serverName = "maxActive",
			serverGetQuery = "spawn.property"
		}),
		this.Property("aiModule", this.PropertyType.String, {
			displayName = "AI Module",
			serverName = "aiModule",
			serverGetQuery = "spawn.property"
		}),
		this.Property("maxLeash", this.PropertyType.Float, {
			displayName = "Leash Length",
			serverName = "maxLeash",
			serverGetQuery = "spawn.property"
		}),
		this.Property("loyaltyRadius", this.PropertyType.Integer, {
			displayName = "Loyalty Radius",
			serverName = "loyaltyRadius",
			serverGetQuery = "spawn.property"
		}),
		this.Property("wanderRadius", this.PropertyType.Integer, {
			displayName = "Wander Radius",
			serverName = "wanderRadius",
			serverGetQuery = "spawn.property"
		}),
		this.Property("despawnTime", this.PropertyType.Integer, {
			displayName = "Despawn Time",
			serverName = "despawnTime",
			serverGetQuery = "spawn.property"
		}),
		this.Property("sequential", this.PropertyType.Boolean, {
			displayName = "Sequential",
			serverName = "sequential",
			serverGetQuery = "spawn.property"
		}),
		this.Property("spawnLayer", this.PropertyType.String, {
			displayName = "Layer",
			serverName = "spawnLayer",
			serverGetQuery = "spawn.property"
		}),
		this.Property("dialog", this.PropertyType.String, {
			displayName = "Dialog",
			serverName = "dialog",
			serverGetQuery = "spawn.property"
		})
	]
};
