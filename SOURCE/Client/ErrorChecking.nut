function GetBacktraceString( ... )
{
	local level = 2;

	if (vargc > 0)
	{
		level += vargv[0];
	}

	local stack = this.getstackinfos(level);
	local str = "";

	while (stack != null)
	{
		str += "   " + stack.func + "()  " + stack.src + ":" + stack.line + "\n";
		level++;
		stack = this.getstackinfos(level);
	}

	return str;
}

function DumpStackTrace()
{
	this.print(this.GetBacktraceString(1) + "\n");
}

class this.Exception 
{
	constructor( msg, ... )
	{
		this.mBacktrace = this.GetBacktraceString(1);
		this.mMessage = msg;

		if (vargc > 0)
		{
			this.mCause = vargv[0];
		}
	}

	function _tostring()
	{
		local str = "Exception: " + this.mMessage + "\n" + this.mBacktrace;

		if (this.mCause != null)
		{
			str += "\nCaused by: " + this.mCause;
		}

		return str;
	}

	function getBacktrace()
	{
		return this.mBacktrace;
	}

	mMessage = null;
	mBacktrace = null;
	mCause = null;
}

class this.Logger 
{
	mName = "";
	constructor( ... )
	{
		if (vargc > 0)
		{
			this.mName = vargv[0];
		}
	}

	function trace( x, ... )
	{
		this.print("[TRACE] " + this.mName + " " + x);
	}

	function debug( x, ... )
	{
		this.print("[DEBUG] " + this.mName + " " + x);
	}

	function info( x, ... )
	{
		this.print("[INFO] " + this.mName + " " + x);
	}

	function warn( x, ... )
	{
		this.print("[WARNING] " + this.mName + " " + x);
	}

	function error( x, ... )
	{
		this.print("[ERROR] " + this.mName + " " + x);
	}

}

class this.DebugLogger 
{
	mName = "";
	constructor( ... )
	{
		if (vargc > 0)
		{
			this.mName = vargv[0];
		}
	}

	function trace( x, ... )
	{
		this.OutputDebug("[TRACE] " + this.mName + " " + x);
	}

	function debug( x, ... )
	{
		this.OutputDebug("[DEBUG] " + this.mName + " " + x);
	}

	function info( x, ... )
	{
		this.OutputDebug("[INFO] " + this.mName + " " + x);
	}

	function warn( x, ... )
	{
		this.OutputDebug("[WARNING] " + this.mName + " " + x);
	}

	function error( x, ... )
	{
		this.OutputDebug("[ERROR] " + this.mName + " " + x);
	}

	function stack( depth )
	{
		if (depth <= 0)
		{
			return;
		}

		local trace = "[STACK] ------------------------------------------------------------";

		for( local i = depth + 1; i > 1; --i )
		{
			local stack = this.getstackinfos(i);
			trace += this.format("\r[STACK] [%s.%s, line: %d]", stack.src, stack.func, stack.line);
		}

		trace += "\r[STACK] ------------------------------------------------------------";
		this.OutputDebug(trace);
	}

}

class this.NullLogger 
{
	mName = "";
	constructor( ... )
	{
		if (vargc > 0)
		{
			this.mName = vargv[0];
		}
	}

	function trace( x, ... )
	{
	}

	function debug( x, ... )
	{
	}

	function info( x, ... )
	{
	}

	function warn( x, ... )
	{
	}

	function error( x, ... )
	{
	}

}

this.log <- this.Logger();
this.fxLog <- this.NullLogger();
this.collisionLogger <- this.NullLogger();
this.Assert <- {};
this.Assert._throw <- function ( exception )
{
	this.log.error("Assertion failed: " + exception + ":\n" + this.GetBacktraceString(1));
	throw exception;
};
this.Assert.isIntegerAboveZero <- function ( pInteger, pCallerName, pVariableName )
{
	if (!this.gAssertTesting)
	{
		return;
	}

	::Assert.isValidType(pInteger, "integer", pCallerName, pVariableName);

	if (pInteger <= 0)
	{
		throw this.Exception(pCallerName + ": " + pVariableName + " must be above zero");
	}
};
this.Assert.isSlotInTable <- function ( pSlotName, pTable, pCallerName, pVariableName )
{
	if (!this.gAssertTesting)
	{
		return;
	}

	::Assert.isValidType(pTable, "table", pCallerName, pVariableName);

	if (!(pSlotName in pTable))
	{
		throw this.Exception(pCallerName + ": " + pSlotName + " is not in " + pVariableName);
	}
};
this.Assert.isEqual <- function ( value, expectedValue )
{
	if (value != expectedValue)
	{
		this.Assert._throw("" + value + " != " + expectedValue);
	}
};
this.Assert.isType <- function ( value, type )
{
	if (typeof value != type)
	{
		this.Assert._throw("" + value + " is not of type: " + type);
	}
};
this.Assert.isTable <- function ( value )
{
	if (typeof value != "table")
	{
		this.Assert._throw("" + value + " is not a valid table");
	}
};
this.Assert.isArray <- function ( value )
{
	if (typeof value != "array")
	{
		this.Assert._throw("" + value + " is not a valid array");
	}
};
this.Assert.isTableOrInstance <- function ( value )
{
	if (typeof value != "table" && typeof value != "instance")
	{
		this.Assert._throw("" + value + " is not a valid table or instance");
	}
};
this.Assert.isInstanceOf <- function ( value, clazz )
{
	if (typeof value != "instance" || !(value instanceof clazz))
	{
		this.Assert._throw("" + value + " is not an instance of " + clazz);
	}
};
this.Assert.isStringBlank <- function ( pString, pCallerName, pVariableName )
{
	if (!this.gAssertTesting)
	{
		return;
	}

	::Assert.isValidType(pString, "string", pCallerName, pVariableName);

	if (pString == null || pString == "")
	{
		throw this.Exception(pCallerName + ": " + pVariableName + " is null");
	}
};
this.Assert.isPercentage <- function ( pPercentage, pCallerName, pVariableName )
{
	if (!this.gAssertTesting)
	{
		return;
	}

	::Assert.isValidType(pPercentage, "float", pCallerName, pVariableName);

	if (!(pPercentage >= 0.0 && pPercentage <= 1.0))
	{
		throw this.Exception(pCallerName + ": " + pVariableName + " is in the range of 0.0 to 1.0");
	}
};
this.Assert.isValidType <- function ( pVariable, pTypeName, pCallerName, pVariableName )
{
	if (!this.gAssertTesting)
	{
		return;
	}

	if (this.type(pVariable) != pTypeName)
	{
		throw this.Exception(pCallerName + ": " + pVariableName + " not a " + pTypeName);
	}
};
this.Assert.isValidInstance <- function ( pInstance, pClass, pCallerName, pVariableName, pClassName, ... )
{
	if (!this.gAssertTesting)
	{
		return;
	}

	if (this.type(pInstance) != "instance")
	{
		throw this.Exception(pCallerName + ": " + pVariableName + " not an instance");
	}

	if (!pInstance.getclass() == pClass)
	{
		throw this.Exception(pCallerName + ": " + pVariableName + " not an instance of " + pClassName);
	}
};
this.ErrorChecking <- {};
this.ErrorChecking.isSlotInTable <- function ( pSlotName, pTable, pCallerName, pVariableName, ... )
{
	local logError = true;

	if (vargc > 0)
	{
		this.pLogError = vargv[0];
	}

	if (!::ErrorChecking.isValidType(pTable, "table", pCallerName, pVariableName))
	{
		return false;
	}

	if (!(pSlotName in pTable))
	{
		if (logError)
		{
			::print("***ERROR*** " + pCallerName + ": " + pSlotName + " is not in " + pVariableName);
		}

		return false;
	}

	return true;
};
this.ErrorChecking.isStringBlank <- function ( pString, pCallerName, pVariableName, ... )
{
	local logError = true;

	if (vargc > 0)
	{
		this.pLogError = vargv[0];
	}

	if (!::ErrorChecking.isValidType(pString, "string", pCallerName, pVariableName))
	{
		return false;
	}

	if (pString == null || pString == "")
	{
		if (logError)
		{
			::print("***ERROR*** " + pCallerName + ": " + pVariableName + " is null");
		}

		return false;
	}

	return true;
};
this.ErrorChecking.isValidType <- function ( pVariable, pTypeName, pCallerName, pVariableName, ... )
{
	this.AxisAlignedBox;
	local logError = true;

	if (vargc > 0)
	{
		this.pLogError = vargv[0];
	}

	if (this.type(pVariable) != pTypeName)
	{
		if (logError)
		{
			::print("***ERROR*** " + pCallerName + ": " + pVariableName + " not a " + pTypeName);
		}

		return false;
	}

	return true;
};
this.ErrorChecking.isValidInstance <- function ( pInstance, pClass, pCallerName, pVariableName, pClassName, ... )
{
	local logError = true;

	if (vargc > 0)
	{
		logError = vargv[0];
	}

	if (this.type(pInstance) != "instance")
	{
		if (logError)
		{
			::print("***ERROR*** " + pCallerName + ": " + pVariableName + " not an instance");
		}

		return false;
	}

	if (!pInstance.getclass() == pClass)
	{
		if (logError)
		{
			::print("***ERROR*** " + pCallerName + ": " + pVariableName + " not an instance of " + pClassName);
		}

		return false;
	}

	return true;
};
function IsInstanceOf( inst, classObj )
{
	return typeof inst == "instance" ? (inst instanceof classObj) : false;
}

