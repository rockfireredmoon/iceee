/**
////////////////////////////////////////
//
// ErrorChecking.nut
//
// Version 0.4
//
// Primary Author: Martin Best
//
////////////////////////////////////////
*/


/**
****************************************
* Summary
****************************************

The function in this script are useful for both assertion and error checking.

****************************************
* Dependancies & Associated Files
****************************************

	Files
		config.nut

	Global Variables and Tables
		gAssertTesting - a bool that controls if assertions will be tested.
		gLogLevel - the log level

****************************************
* Function
****************************************
*/


/**
	Get a string representation of the current call stack. (Very useful
	for debugging.) You can optionally pass the number of "levels" back
	to begin checking the stack. The default value is 0, which will not
	include the call to this routine (usually what you want).
*/

function GetBacktraceString( ... ) {
	local level = 2;

	if (vargc > 0)
		level += vargv[0];

	local stack = getstackinfos(level);
	local str = "";

	while (stack != null) {
		str += "   " + stack.func + "()  " + stack.src + ":" + stack.line + "\n";
		level++;
		stack = getstackinfos(level);
	}

	return str;
}

function DumpStackTrace() {
	print(GetBacktraceString(1) + "\n");
}

/**
	This lets us do a bit better error checking. Unfortuntely, we
	cannot hook into the builtin squirrel errors, but oh well.
*/
class Exception 
{
	/**
		Create a new exception. If there was a "root cause", you should
		specify it as the second parameter. This allows us to do 
		exception chaining so we can see where things failed (complete
		with backtraces).
	*/
	constructor( msg, ... )	{
		mBacktrace = GetBacktraceString(1);
		mMessage = msg;

		if (vargc > 0)
			mCause = vargv[0];
	}

	function _tostring() {
		local str = "Exception: " + mMessage + "\n" + mBacktrace;

		if (mCause != null)
			str += "\nCaused by: " + mCause;

		return str;
	}

	function getBacktrace()	{
		return mBacktrace;
	}

	mMessage = null;
	mBacktrace = null;
	mCause = null;
}



/**
	A simple hierarchical logger implementation. (Well not even that
	currently.)
*/
class Logger {
	mName = "";
	constructor( ... ) {
		if (vargc > 0)
			mName = vargv[0];
	}

	function trace( x, ... ) {
		if(gLogLevel >= LogLevel.LEVEL_TRACE)
			print("[TRACE] " + mName + " " + x);
	}

	function debug( x, ... ) {
		if(gLogLevel >= LogLevel.LEVEL_DEBUG)
			print("[DEBUG] " + mName + " " + x);
	}

	function info( x, ... )	{
		if(gLogLevel >= LogLevel.LEVEL_INFO)
			print("[INFO] " + mName + " " + x);
	}

	function warn( x, ... )	{
		if(gLogLevel >= LogLevel.LEVEL_WARN)
			print("[WARNING] " + mName + " " + x);
	}

	function error( x, ... ) {
		if(gLogLevel >= LogLevel.LEVEL_ERROR)
			print("[ERROR] " + mName + " " + x);
	}

}

class DebugLogger 
{
	mName = "";
	constructor( ... ) {
		if (vargc > 0)
			mName = vargv[0];
	}

	function trace( x, ... ) {
		if(gLogLevel >= LogLevel.LEVEL_TRACE)
			OutputDebug("[TRACE] " + mName + " " + x);
	}

	function debug( x, ... ) {
		if(gLogLevel >= LogLevel.LEVEL_DEBUG)
			OutputDebug("[DEBUG] " + mName + " " + x);
	}

	function info( x, ... )	{
		if(gLogLevel >= LogLevel.LEVEL_INFO)
			OutputDebug("[INFO] " + mName + " " + x);
	}

	function warn( x, ... )	{
		if(gLogLevel >= LogLevel.LEVEL_WARN)
			OutputDebug("[WARNING] " + mName + " " + x);
	}

	function error( x, ... ) {
		if(gLogLevel >= LogLevel.LEVEL_ERROR)
			OutputDebug("[ERROR] " + mName + " " + x);
	}

	function stack( depth )	{
		if (depth <= 0)
			return;

		local trace = "[STACK] ------------------------------------------------------------";

		for( local i = depth + 1; i > 1; --i ) {
			local stack = getstackinfos(i);
			trace += format("\r[STACK] [%s.%s, line: %d]", stack.src, stack.func, stack.line);
		}

		trace += "\r[STACK] ------------------------------------------------------------";
		OutputDebug(trace);
	}

}

class NullLogger {
	mName = "";
	
	constructor( ... ) {
		if (vargc > 0)
			mName = vargv[0];
	}

	function trace( x, ... ) {}
	function debug( x, ... ) {}
	function info( x, ... ) {}
	function warn( x, ... )	{}
	function error( x, ... ) {}
}


// A single global logger that everyone can use. You can actually
// create individual (more customized) loggers by instantiating them.
// There is only one "root" logger, though, which has no name.
log <- Logger();

// Other loggers
fxLog <- NullLogger();
collisionLogger <- NullLogger();


Assert <- {};

/**
	A utility for the Assert group of functions to print an informative
	error message to the console before dumping with a squirrel exception.
*/
function Assert::_throw(exception) {
	log.error("Assertion failed: " + exception + ":\n" + GetBacktraceString(1));
	throw exception;
}


/**
	A passed table is tested to see if it contains a slot.  If the slot is not found an exception is thrown.
	
	@param pInteger
		The integer to be tested
		
	@param pCallerName
		The name of the calling function needs to be supplied so it can be added to the error message
		when it is passed to the log.
	
	@param pVariableName
		The name of the variable containing the string being checked, this is added to the error message placed
		in the log file.
*/
function Assert::isIntegerAboveZero(pInteger, pCallerName, pVariableName) {
	if (!gAssertTesting)
		return;

	::Assert.isValidType(pInteger, "integer", pCallerName, pVariableName);

	if (pInteger <= 0)
		throw Exception(pCallerName + ": " + pVariableName + " must be above zero");
}

/**
	A passed table is tested to see if it contains a slot.  If the slot is not found an exception is thrown.
	
	@param pSlotName
		The name of the slot to look for
	
	@param pSlotName
		The table for the slot in
	
	@param pCallerName
		The name of the calling function needs to be supplied so it can be added to the error message
		when it is passed to the log.
	
	@param pVariableName
		The name of the variable containing the string being checked, this is added to the error message placed
		in the log file.
*/
Assert.isSlotInTable <- function ( pSlotName, pTable, pCallerName, pVariableName )
{
	//is assert testing on?
	if (!gAssertTesting)
		return;

	//is pTable a table?
	::Assert.isValidType(pTable, "table", pCallerName, pVariableName);

	//is pSlotName in pTable?
	if (!(pSlotName in pTable))
		throw Exception(pCallerName + ": " + pSlotName + " is not in " + pVariableName);
}


Assert.isEqual <- function ( value, expectedValue ) {
	if (value != expectedValue)
	{
		Assert._throw("" + value + " != " + expectedValue);
	}
}

/**
	Check whether the value is of a specific type as returned by
	the builtin "typeof" operator. E.g. Assert.isType(foo, "table")
*/
Assert.isType <- function ( value, type ) {
	if (typeof value != type)
	{
		Assert._throw("" + value + " is not of type: " + type);
	}
}

/**
	Verify that the value is a valid table.
*/
Assert.isTable <- function ( value ) {
	if (typeof value != "table")
	{
		Assert._throw("" + value + " is not a valid table");
	}
}

/**
	Verify that the value is an array.
*/
Assert.isArray <- function ( value ) {
	if (typeof value != "array")
	{
		Assert._throw("" + value + " is not a valid array");
	}
}

/**
	Check whether the value is an instance of a specific class (or
	a subclass of the class). Uses the squirrel "instanceof" operator.
*/
Assert.isTableOrInstance <- function ( value ) {
	if (typeof value != "table" && typeof value != "instance")
	{
		Assert._throw("" + value + " is not a valid table or instance");
	}
}

Assert.isInstanceOf <- function ( value, clazz ) {
	if (typeof value != "instance" || !(value instanceof clazz))
	{
		Assert._throw("" + value + " is not an instance of " + clazz);
	}
}

/**
	A passed string is tested to see if it is blank and that it is a string. If the test fails and exception is thrown.
	
	@param pString
		The string to be tested.
	
	@param pCallerName
		The name of the calling function needs to be supplied so it can be added to the error message
		when it is passed to the log.
	
	@param pVariableName
		The name of the variable containing the string being checked, this is added to the error message placed
		in the log file.
*/
Assert.isStringBlank <- function ( pString, pCallerName, pVariableName ) {
	if (!gAssertTesting)
		return;

	::Assert.isValidType(pString, "string", pCallerName, pVariableName);

	if (pString == null || pString == "")
		throw Exception(pCallerName + ": " + pVariableName + " is null");
}

/**
	A passed float is tested to see if it is within the bounds of a percentage. If the test fails and exception is thrown.
	
	@param pPercentage
		The string to be tested.
	
	@param pCallerName
		The name of the calling function needs to be supplied so it can be added to the error message
		when it is passed to the log.
	
	@param pVariableName
		The name of the variable containing the string being checked, this is added to the error message placed
		in the log file.
*/
Assert.isPercentage <- function ( pPercentage, pCallerName, pVariableName ) {
	if (!gAssertTesting)
		return;

	::Assert.isValidType(pPercentage, "float", pCallerName, pVariableName);

	if (!(pPercentage >= 0.0 && pPercentage <= 1.0))
		throw Exception(pCallerName + ": " + pVariableName + " is in the range of 0.0 to 1.0");
}

/**
	A passed variable is tested to make sure it is of a certain type.
	
	@param pInstance
		A reference to the instance to be tested.

	@param pClass
		A reference to the class to be tested.
	
	@param pCallerName
		The name of the calling function needs to be supplied so it can be added to the error message
		when it is passed to the log.
	
	@param pVariableName
		The name of the variable containing the string being checked, this is added to the error message placed
		in the log file.

	@param pClassName
		The name of the Class being compared , this is added to the error message placed
		in the log file.
*/
Assert.isValidType <- function ( pVariable, pTypeName, pCallerName, pVariableName ) {
	if (!gAssertTesting)
		return;

	if (type(pVariable) != pTypeName)
		throw Exception(pCallerName + ": " + pVariableName + " not a " + pTypeName);
}

/**
	A passed object is tested to make sure it is an instance of a specific class.
	
	@param pInstance
		A reference to the instance to be tested.

	@param pClass
		A reference to the class to be tested.
	
	@param pCallerName
		The name of the calling function needs to be supplied so it can be added to the error message
		when it is passed to the log.
	
	@param pVariableName
		The name of the variable containing the string being checked, this is added to the error message placed
		in the log file.

	@param pClassName
		The name of the Class being compared , this is added to the error message placed
		in the log file.
*/
Assert.isValidInstance <- function ( pInstance, pClass, pCallerName, pVariableName, pClassName, ... ) {
	if (!gAssertTesting)
		return;

	if (type(pInstance) != "instance")
		throw Exception(pCallerName + ": " + pVariableName + " not an instance");

	if (!pInstance.getclass() == pClass)
		throw Exception(pCallerName + ": " + pVariableName + " not an instance of " + pClassName);
}

ErrorChecking <- {};

/**
	A passed table is tested to see if it contains a slot
	
	@param pSlotName
		The name of the slot to look for
	
	@param pSlotName
		The table for the slot in
	
	@param pCallerName
		The name of the calling function needs to be supplied so it can be added to the error message
		when it is passed to the log.
	
	@param pVariableName
		The name of the variable containing the string being checked, this is added to the error message placed
		in the log file.

	@param ...
		an optional bool can be passed that controls whether to log an error message.  It is set to true by default.

	@return bool
*/
ErrorChecking.isSlotInTable <- function ( pSlotName, pTable, pCallerName, pVariableName, ... )
{
	local logError = true;

	if (vargc > 0)
		pLogError = vargv[0];

	if (!::ErrorChecking.isValidType(pTable, "table", pCallerName, pVariableName))
		return false;

	if (!(pSlotName in pTable))
	{
		if (logError)
			::print("***ERROR*** " + pCallerName + ": " + pSlotName + " is not in " + pVariableName);

		return false;
	}

	return true;
}

/**
	A passed string is tested to see if it is blank and that it is a string.
	
	@param pString
		The string to be tested.
	
	@param pCallerName
		The name of the calling function needs to be supplied so it can be added to the error message
		when it is passed to the log.
	
	@param pVariableName
		The name of the variable containing the string being checked, this is added to the error message placed
		in the log file.

	@param ...
		an optional bool can be passed that controls whether to log an error message.  It is set to true by default.

	@return bool
*/
ErrorChecking.isStringBlank <- function ( pString, pCallerName, pVariableName, ... )
{
	local logError = true;

	if (vargc > 0)
	{
		pLogError = vargv[0];
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
}

/**
	A passed variable is tested to make sure it is of a certain type.
	
	@param pInstance
		A reference to the instance to be tested.

	@param pClass
		A reference to the class to be tested.
	
	@param pCallerName
		The name of the calling function needs to be supplied so it can be added to the error message
		when it is passed to the log.
	
	@param pVariableName
		The name of the variable containing the string being checked, this is added to the error message placed
		in the log file.

	@param pClassName
		The name of the Class being compared , this is added to the error message placed
		in the log file.

	@param ...
		an optional bool can be passed that controls whether to log an error message.  It is set to true by default.

	@return bool
*/
ErrorChecking.isValidType <- function ( pVariable, pTypeName, pCallerName, pVariableName, ... ) {
	AxisAlignedBox;
	local logError = true;

	if (vargc > 0)
	{
		pLogError = vargv[0];
	}

	if (type(pVariable) != pTypeName)
	{
		if (logError)
		{
			::print("***ERROR*** " + pCallerName + ": " + pVariableName + " not a " + pTypeName);
		}

		return false;
	}

	return true;
}

/**
	A passed object is tested to make sure it is an instance of a specific class.
	
	@param pInstance
		A reference to the instance to be tested.

	@param pClass
		A reference to the class to be tested.
	
	@param pCallerName
		The name of the calling function needs to be supplied so it can be added to the error message
		when it is passed to the log.
	
	@param pVariableName
		The name of the variable containing the string being checked, this is added to the error message placed
		in the log file.

	@param pClassName
		The name of the Class being compared , this is added to the error message placed
		in the log file.

	@param ...
		an optional bool can be passed that controls whether to log an error message.  It is set to true by default.

	@return bool
*/
ErrorChecking.isValidInstance <- function ( pInstance, pClass, pCallerName, pVariableName, pClassName, ... ) {
	local logError = true;

	if (vargc > 0)
	{
		logError = vargv[0];
	}

	if (type(pInstance) != "instance")
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
}

/**
	Safely check instanceof for any object. If it's not an instance,
	it simply returns false. Otherwise it checks instanceof for the
	given instance.
*/
function IsInstanceOf( inst, classObj ) {
	return typeof inst == "instance" ? (inst instanceof classObj) : false;
}

