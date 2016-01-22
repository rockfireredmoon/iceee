#include "ScriptCore.h"

#include "../squirrel/sqrat/sqrat/sqratClass.h"
#include "../squirrel/sqrat/sqrat/sqratFunction.h"
#include "../squirrel/sqrat/sqrat/sqratScript.h"
#include "../squirrel/sqrat/sqrat/sqratTable.h"
#include "../squirrel/sqrat/sqrat/sqratUtil.h"
#include <stdarg.h>
#include <stddef.h>
#include <string.h>
#include <sys/types.h>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <iterator>
#include <utility>
#include <algorithm>

#include "../squirrel/squirrel/sqvm.h"
#include "CommonTypes.h"
#include "Components.h"
#include "DirectoryAccess.h"
#include "FileReader.h"
#include "Simulator.h"
#include "StringList.h"
#include "Util.h"
#include "Config.h"

extern unsigned long g_ServerTime;


void PrintFunc(HSQUIRRELVM v, const SQChar *s, ...) {
	va_list vl;
	va_start(vl, s);
	vfprintf(stdout, s, vl);
	va_end(vl);
}

void Errorfunc(HSQUIRRELVM v, const SQChar *s, ...) {
	va_list vl;
	va_start(vl, s);
	vfprintf(stderr, s, vl);
	va_end(vl);
}


namespace ScriptCore
{
	ScriptParam::ScriptParam() {
		type = OPT_INT;
		iValue = 0;
		fValue = 0;
		strValue = "";
		bValue = false;
	}
	ScriptParam::ScriptParam(int v) {
		type = OPT_INT;
		iValue = v;
		fValue = 0;
		strValue = "";
		bValue = false;
	}
	ScriptParam::ScriptParam(float v) {
		type = OPT_FLOAT;
		iValue = 0;
		fValue = v;
		strValue = "";
		bValue = false;
	}
	ScriptParam::ScriptParam(std::string v) {
		type = OPT_STR;
		iValue = 0;
		fValue = 0;
		strValue = v;
		bValue = false;
	}
	ScriptParam::ScriptParam(bool v) {
		type = OPT_BOOL;
		iValue = 0;
		fValue = 0;
		strValue = "";
		bValue = v;
	}


	//
	// Parses script names from things such as AIPackages to allow parameters to
	// be passed to those scripts. This allows much better script reuse, and the
	// possibility of groves customising scripts for the creatures they use
	//

	NutScriptCallStringParser::NutScriptCallStringParser(std::string callString) {
		int idx = callString.find('(');
		if(idx != -1) {
			mScriptName = callString.substr(0, idx);
			Util::Split(callString.substr(idx + 1, callString.find_last_not_of(')')), ",", mArgs);
			callString.find_last_not_of(')');
		}
		else {
			mScriptName = callString;
		}

		mEnabled = mScriptName.length() > 0 && mScriptName.compare("none") != 0 && mScriptName.compare("nothing") != 0;
	}

	NutDef::NutDef() {
		mQueueEvents = true;
		mFlags = 0;
		queueCallStyle = 0;
		queueExternalJumps = false;
		mScriptIdleSpeed = 1;
		mScriptSpeed = 10;
	}

	NutDef::~NutDef() {

	}

	void NutDef::ClearBase(void) {
		scriptName.clear();
		ClearDerived();
	}

	// Stub function, if additional members are defined in a derived class, then override this function
	// to clear the new members.
	void NutDef::ClearDerived(void) {
	}

	bool NutDef :: CanIdle() {
		return mScriptIdleSpeed > 0;
	}

	bool NutDef :: HasFlag(unsigned int flag)
	{
		return ((mFlags & flag) != 0);
	}

	void NutDef::Initialize(const char *sourceFile) {
		g_Log.AddMessageFormat("Compiling Squirrel script '%s'", sourceFile);
		mSourceFile = sourceFile;
		scriptName = Platform::Basename(mSourceFile.c_str());
	}

	//
	// Abstract condition implementation
	//

	NutCondition::NutCondition()
	{
	}

	NutCondition::~NutCondition()
	{
	}

	bool NutCondition::CheckCondition()
	{
		return false;
	}

	//
	// Abstract callback implementation
	//

	NutCallback::NutCallback()
	{
	}

	NutCallback::~NutCallback()
	{
	}

	//
	// 'Time' condition implementation. Is true when the current server time reaches
	// the fireTime
	//

	TimeCondition::TimeCondition(unsigned long delay)
	{
		mFireTime = g_PlatformTime.getElapsedMilliseconds() + delay;
	}
	TimeCondition::~TimeCondition() {}

	bool TimeCondition::CheckCondition() {
		return g_PlatformTime.getElapsedMilliseconds() >= mFireTime;
	}

	//
	// 'Resume' callback implementation. Resumes a suspended Squirrel VM, probably
	// suspended as the result of a sleep() call.
	//
	ResumeCallback::ResumeCallback(NutPlayer *nut)
	{
		mNut = nut;
	}

	ResumeCallback::~ResumeCallback()
	{
	}

	bool ResumeCallback::Execute()
	{
		sq_wakeupvm(mNut->vm, false, false, false, false);
		return true;
	}

	//
	// 'Halt' callback implementation. Halts a script.
	//
	HaltCallback::HaltCallback(NutPlayer *nut)
	{
		mNut = nut;
	}

	HaltCallback::~HaltCallback()
	{
	}

	bool HaltCallback::Execute()
	{
		mNut->HaltExecution();
		return true;
	}

	//
	// Squirrel function callback. Used by queue() script function to queue execution of
	// a scripted function
	//
	SquirrelFunctionCallback::SquirrelFunctionCallback(NutPlayer *nut, Sqrat::Function function) {
		mNut = nut;
		mFunction = function;
	}

	SquirrelFunctionCallback::~SquirrelFunctionCallback() {
	}

	bool SquirrelFunctionCallback::Execute()
	{
		Sqrat::SharedPtr<bool> ptr = mFunction.Evaluate<bool>();
		return ptr.Get() == NULL || ptr.Get();
	}

	//
	// Run a named function.
	//
	RunFunctionCallback::RunFunctionCallback(NutPlayer *nut, std::string functionName) {
		mNut = nut;
		mFunctionName = functionName;
	}
	RunFunctionCallback::RunFunctionCallback(NutPlayer *nut, std::string functionName, std::vector<ScriptParam> args) {
		mNut = nut;
		mFunctionName = functionName;
		mArgs = args;
	}


	RunFunctionCallback::~RunFunctionCallback() {
	}

	bool RunFunctionCallback::Execute()
	{
		return mNut->RunFunction(mFunctionName, mArgs, false);
	}

	//
	// Each event stores a callback and a condition. When the condition is met, the callback
	// is executed.
	//
	NutScriptEvent::NutScriptEvent(NutCondition *condition, NutCallback *callback)
	{
		mCondition = condition;
		mCallback = callback;
		mRunWhenSuspended = false;
		mCancelled = false;
	}

	NutScriptEvent::~NutScriptEvent() {
		delete mCallback;
		delete mCondition;
	}

	void NutScriptEvent::Cancel() {
		mCancelled = true;
	}

	//
	// Abstract squirrel script player.
	//

	NutPlayer::NutPlayer() {
		mInitTime = 0;
		vm = NULL;
		def = NULL;
		mActive = false;
		mExecuting = false;
		mProcessingTime = 0;
		mGCCounter = 0;
		mMaybeGC = 0;
		mForceGC = 0;
		mGCTime = 0;
		mCalls = 0;
		mRunning = false;
		mHalting = false;
	}

	NutPlayer::~NutPlayer() {
		ClearQueue();
	}

	int NutPlayer::GC() {
		return sq_collectgarbage(vm);
	}

	void NutPlayer::ClearQueue() {
		std::vector<ScriptCore::NutScriptEvent*>::iterator it;
		for(it = mQueue.begin(); it != mQueue.end(); ++it)
			delete *it;
		for(it = mQueueAdd.begin(); it != mQueueAdd.end(); ++it)
			delete *it;
		for(it = mQueueInsert.begin(); it != mQueueInsert.end(); ++it)
			delete *it;
		mQueue.clear();
		mQueueAdd.clear();
		mQueueInsert.clear();
		mQueueRemove.clear();
	}

	void NutPlayer::Initialize(NutDef *defPtr, std::string &errors) {
		unsigned long started = g_PlatformTime.getMilliseconds();

		def = defPtr;
		vm = sq_open(g_Config.SquirrelVMStackSize);

		// Register functions needed in scripts
		RegisterFunctions();
		sq_pushroottable(vm);
		sqstd_register_stringlib(vm);
		sq_pop(vm,1);

		sqstd_seterrorhandlers(vm); //registers the default error handlers
		sq_setprintfunc(vm, PrintFunc, Errorfunc); //sets the print function
		g_Log.AddMessageFormat("Processing Squirrel script '%s'", def->mSourceFile.c_str());


		/* Look for the compiled NUT file (.cnut). If it exists, test if the modification
		 * time is the same as the .nut file. If it isn't (or the .cnut doesn't exist at all),
		 * then compile AND write the bytecode
		 */
		std::string base = Platform::Basename(def->mSourceFile.c_str());
		std::string dir = Platform::Dirname(def->mSourceFile.c_str());
		STRINGLIST v;
		const std::string d(1, PLATFORM_FOLDERVALID);
		std::string cnut;
		v.push_back(dir);
		v.push_back(base);
		Util::Join(v, d.c_str(), cnut);
		cnut.append(".cnut");
		unsigned long cnutMod = Platform::GetLastModified(cnut.c_str());
		unsigned long nutMod = Platform::GetLastModified(def->mSourceFile.c_str());

		Sqrat::Script script(vm);

		if(cnutMod != nutMod) {
			g_Log.AddMessageFormat("Recompiling Squirrel script '%s'", def->mSourceFile.c_str());
			script.CompileFile(_SC(def->mSourceFile), errors);
		}
		else {
			g_Log.AddMessageFormat("Loading existing Squirrel script bytecode for '%s'", cnut.c_str());
			script.CompileFile(_SC(cnut.c_str()), errors);
		}

		if (Sqrat::Error::Occurred(vm)) {
			errors.append(Sqrat::Error::Message(vm).c_str());
			g_Log.AddMessageFormat("Squirrel script  %s failed to compile. %s", def->mSourceFile.c_str(), Sqrat::Error::Message(vm).c_str());
		}
		else {
			if(cnutMod != nutMod) {
				g_Log.AddMessageFormat("Writing Squirrel script bytecode for '%s' to '%s'", def->mSourceFile.c_str(), cnut.c_str());
				try {
					script.WriteCompiledFile(cnut);
				}
				catch(int e) {
					g_Log.AddMessageFormat("Failed to write Squirrel script bytecode for '%s' to '%s'. Err %d", def->mSourceFile.c_str(), cnut.c_str(), e);
				}
				Platform::SetLastModified(cnut.c_str(), nutMod);
			}
			mActive = true;
			mRunning = true;
			script.Run();
			mRunning = false;
			if (Sqrat::Error::Occurred(vm)) {
				mActive = false;
				errors.append(Sqrat::Error::Message(vm).c_str());
				g_Log.AddMessageFormat("Squirrel script  %s failed to run. %s", def->mSourceFile.c_str(), Sqrat::Error::Message(vm).c_str());
			}

			// The script might have provided an info table
			Sqrat::Object infoObject = Sqrat::RootTable(vm).GetSlot(_SC("info"));
			if(!infoObject.IsNull()) {
				Sqrat::Object author = infoObject.GetSlot("author");
				if(!author.IsNull()) {
					def->mAuthor = author.Cast<std::string>();
				}
				Sqrat::Object description = infoObject.GetSlot("description");
				if(!description.IsNull()) {
					def->mAuthor = description.Cast<std::string>();
				}
				Sqrat::Object queueEvents = infoObject.GetSlot("queue_events");
				if(!queueEvents.IsNull()) {
					def->mQueueEvents = description.Cast<bool>();
				}
				Sqrat::Object idleSpeed = infoObject.GetSlot("idle_speed");
				if(!idleSpeed.IsNull()) {
					def->mScriptIdleSpeed = idleSpeed.Cast<int>();
				}
				Sqrat::Object speed = infoObject.GetSlot("speed");
				if(!speed.IsNull()) {
					def->mScriptSpeed = Util::ClipInt(speed.Cast<int>(), 1, 100);
				}
			}
		}



//		if (SQ_SUCCEEDED(sqstd_dofile(vm, _SC(def->mSourceFile), SQFalse, SQTrue))) // also prints syntax errors if any
//				{
//
//			mHasScript = true;
//			g_Log.AddMessageFormat("Squirrel script  %s processed", def->mSourceFile);
//			active = true;
//		}
//		else
//		{
//			active = false;
//		}

//		sq_pop(vm, 1); //pops the root table
		//sq_close(vm);

		unsigned long time = g_PlatformTime.getMilliseconds() - started;
		mInitTime = time;
		mCalls++;
		mGCCounter++;
	}

	bool NutPlayer::JumpToLabel(const char *name)
	{
		return JumpToLabel(name, std::vector<ScriptParam>());
	}

	bool NutPlayer::JumpToLabel(const char *name, std::vector<ScriptParam> parms)
	{
		if(def->mQueueEvents) {
			QueueAdd(new NutScriptEvent(new TimeCondition(0), new RunFunctionCallback(this, name, parms)));
			return true;
		}
		else {
			return RunFunction(name, parms, true);
		}
	}

	void NutPlayer::Exec(Sqrat::Function function) {
		QueueAdd(new ScriptCore::NutScriptEvent(
					new ScriptCore::TimeCondition(g_Config.SquirrelQueueSpeed / def->mScriptSpeed),
					new ScriptCore::SquirrelFunctionCallback(this, function)));
	}

	void NutPlayer::Queue(Sqrat::Function function, int fireDelay) {
		QueueAdd(new ScriptCore::NutScriptEvent(
					new ScriptCore::TimeCondition(fireDelay),
					new ScriptCore::SquirrelFunctionCallback(this, function)));
	}

	void NutPlayer::RegisterFunctions() { }

	void NutPlayer::RegisterCoreFunctions(NutPlayer *instance, Sqrat::Class<NutPlayer> *clazz) {

		// Instance Location Object, X1/Z1,X2/Z2 location defining a rectangle
		Sqrat::Class<Squirrel::Area> areaClass(vm, "Area", true);
		areaClass.Ctor<int,int,int,int>();
		areaClass.Ctor<int,int,int>();
		areaClass.Ctor();
		Sqrat::RootTable(vm).Bind(_SC("Area"), areaClass);
		areaClass.Var("x1", &Squirrel::Area::mX1);
		areaClass.Var("radius", &Squirrel::Area::mRadius);
		areaClass.Var("x2", &Squirrel::Area::mX2);
		areaClass.Var("y1", &Squirrel::Area::mY1);
		areaClass.Var("y2", &Squirrel::Area::mY2);
		areaClass.Func("point", &Squirrel::Area::ToPoint);

		// Point Object, X/Z location
		Sqrat::Class<Squirrel::Point> pointClass(vm, "Point", true);
		pointClass.Ctor<int,int>();
		pointClass.Ctor();
		Sqrat::RootTable(vm).Bind(_SC("Point"), pointClass);
		pointClass.Var("x", &Squirrel::Point::mX);
		pointClass.Var("z", &Squirrel::Point::mZ);

		// Vector3 Object, X/Y/Z location
		Sqrat::Class<Squirrel::Vector3I> vector3Class(vm, "Vector3I", true);
		vector3Class.Ctor<int,int, int>();
		vector3Class.Ctor();
		Sqrat::RootTable(vm).Bind(_SC("Vector3I"), vector3Class);
		vector3Class.Var("x", &Squirrel::Vector3I::mX);
		vector3Class.Var("y", &Squirrel::Vector3I::mY);
		vector3Class.Var("z", &Squirrel::Vector3I::mZ);

		// Vector3F Object, floating point X/Y/Z location
		Sqrat::Class<Squirrel::Vector3> vector3FClass(vm, "Vector3", true);
		vector3FClass.Ctor<float,float, float>();
		vector3FClass.Ctor();
		Sqrat::RootTable(vm).Bind(_SC("Vector3"), vector3FClass);
		vector3FClass.Var("x", &Squirrel::Vector3::mX);
		vector3FClass.Var("y", &Squirrel::Vector3::mY);
		vector3FClass.Var("z", &Squirrel::Vector3::mZ);

		clazz->Func(_SC("exec"), &NutPlayer::Exec);
		clazz->Func(_SC("queue"), &NutPlayer::Queue);
		clazz->Func(_SC("clear_queue"), &NutPlayer::QueueClear);
		clazz->Func(_SC("broadcast"), &NutPlayer::Broadcast);
		clazz->Func(_SC("halt"), &NutPlayer::Halt);
		clazz->Func(_SC("get_server_time"), &NutPlayer::GetServerTime);
		clazz->SquirrelFunc(_SC("sleep"), &Sleep);

		Sqrat::RootTable(vm).Func("randmodrng", &randmodrng);
		Sqrat::RootTable(vm).Func("randmod", &randmod);
		Sqrat::RootTable(vm).Func("randint", &randint);
		Sqrat::RootTable(vm).Func("randdbl", &randdbl);
		Sqrat::RootTable(vm).Func("rand", &randi);

		// Add in the script arguments
		Sqrat::RootTable(vm).SetValue(_SC("__argc"), SQInteger(mArgs.size()));
		Sqrat::Array arr(vm, mArgs.size());
		int idx = 0;
		for(std::vector<std::string>::iterator it = mArgs.begin(); it != mArgs.end(); ++it)
			arr.SetValue(idx++, _SC(*it));
		Sqrat::RootTable(vm).SetValue(_SC("__argv"), arr);
	}

	unsigned long NutPlayer::GetServerTime() {
		return g_PlatformTime.getElapsedMilliseconds();
	}

	void NutPlayer :: FullReset(void)
	{
		if(def == NULL)
		{
			mActive = false;
			return;
		}

		mActive = true;

		// TODO somehow reset state of script
		ClearQueue();
	}

	void NutPlayer::RunScript(void) {
	}

	bool NutPlayer::Tick(void) {
		if(mActive) {
			ExecQueue();
		}
		return true;
	}

	int NutPlayer::Rand(int max) {
		return randint(1, max);
	}

	int NutPlayer::RandInt(int min, int max) {
		return randint(min, max);
	}

	int NutPlayer::RandMod(int max) {
		return randmod(max);
	}

	int NutPlayer::RandModRng(int min, int max) {
		return randmodrng(min, max);
	}

	int NutPlayer::RandDbl(double min, double max) {
		return randdbl(min, max);
	}

	void NutPlayer::Halt(void) {
    	HaltCallback *cb = new HaltCallback(this);
    	NutScriptEvent *nse = new NutScriptEvent(new TimeCondition (0), cb);
    	nse->mRunWhenSuspended = true;
    	QueueInsert(nse);
	}

	void NutPlayer :: HaltedDerived(void) { }

	void NutPlayer :: HaltDerivedExecution(void) { }

	void NutPlayer :: HaltExecution(void)
	{
		if(mRunning) {
			/* If we reached here via a script function, we already executing and don't want to close the VM.
			 * In this case the halt is queued instance
			 */
			Halt();
			return;
		}
		if(mActive && !mHalting) {
			mHalting = true;
			vector<ScriptParam> v;
			RunFunction("on_halt", v, true);
			HaltDerivedExecution();
			mActive = false;
			ClearQueue();
			sq_close(vm);
			if(def->HasFlag(NutDef::FLAG_REPORT_END))
				PrintMessage("Script [%s] has halted", def->scriptName.c_str());
			mHalting = false;
			HaltedDerived();
		}
	}

	bool NutPlayer::RunFunction(std::string name, std::vector<ScriptParam> parms, bool time) {
		if(!mActive) {
			g_Log.AddMessageFormat("[WARNING] Attempt to run function on inactive script %s.", name.c_str());
			return false;
		}
		g_Log.AddMessageFormat("RunFunction(%s)", name.c_str());
		unsigned long now = g_PlatformTime.getMilliseconds();
		mRunning = true;

		// Wake the VM up if it is suspend so the onFinish can be run
		if(sq_getvmstate(vm) == SQ_VMSTATE_SUSPENDED) {
			g_Log.AddMessageFormat("Waking up VM to run %s.", name.c_str());
			sq_wakeupvm(vm, false, false, false, true);
		}

		SQInteger top = sq_gettop(vm);
		sq_pushroottable(vm);
		sq_pushstring(vm,_SC(name.c_str()),-1);
		if(SQ_SUCCEEDED(sq_get(vm,-2))) {
			sq_pushroottable(vm);
			std::vector<ScriptCore::ScriptParam>::iterator it;
			for(it = parms.begin(); it != parms.end(); ++it)
			{
				switch(it->type) {
				case OPT_INT:
					sq_pushinteger(vm,it->iValue);
					break;
				case OPT_FLOAT:
					sq_pushfloat(vm,it->fValue);
					break;
				case OPT_STR:
					sq_pushstring(vm,_SC(it->strValue.c_str()), it->strValue.size());
					break;
				default:
					g_Log.AddMessageFormat("Unsupported parameter type for Squirrel script. %d", it->type);
					break;
				}
			}
			sq_call(vm,parms.size() + 1,SQFalse,SQTrue); //calls the function
		}
		sq_settop(vm,top);

		if(time) {
			mCalls++;
			mGCCounter++;
			mProcessingTime += g_PlatformTime.getMilliseconds() - now;
		}

		mRunning = false;

		return true;
	}

	bool NutPlayer :: ExecEvent(NutScriptEvent *nse, int index)
	{
		unsigned long now = g_PlatformTime.getMilliseconds();
		NutCallback *cb = nse->mCallback;

		bool res = true;
		if(!nse->mCancelled) {
			try {
				res = cb->Execute();
			}
			catch(int e) {
				g_Log.AddMessageFormat("Callback failed. %d", e);
			}
		}

		if(mQueue.size() > 0) {
			/*
			 * If the VM wasn't suspended while handling this event, and the
			 * event returned false, then we requeue this event for retry
			 */
			if(sq_getvmstate(vm) != SQ_VMSTATE_SUSPENDED && !res) {
				mQueueAdd.push_back(nse);
				mQueue.erase(mQueue.begin() + index);
			}
			else {
				mQueue.erase(mQueue.begin() + index);
				delete nse;
			}
		}

		mCalls++;
		mGCCounter++;
		mProcessingTime += g_PlatformTime.getMilliseconds() - now;
		return res;
	}

	bool NutPlayer :: ExecQueue(void)
	{
		if(mExecuting) {
			g_Log.AddMessageFormat("Already executing. Something tried to executing the queue while it was already executing.");
			return true;
		}
		bool ok = false;
		mExecuting = true;
		for(size_t i = 0; i < mQueue.size(); i++)
		{
			NutScriptEvent *nse = mQueue[i];

			// If the VM is suspended, ignore events that dont have mRunWhenSuspended = true. In
			// practice, this is currently only the ResumeCallback
			if(sq_getvmstate(vm) == SQ_VMSTATE_SUSPENDED && !nse->mRunWhenSuspended)
				continue;

			NutCondition *cnd = nse->mCondition;
			if(cnd->CheckCondition()) {
				ExecEvent(nse, i);
				ok = true;
				break;
			}
		}

		// Apply any changes to the queue made while running the queued event
		for(size_t i = 0; i < mQueueRemove.size(); i++)	{
			NutScriptEvent *nse = mQueueRemove[i];
			mQueue.erase(std::remove(mQueue.begin(), mQueue.end(), nse), mQueue.end());
			mQueueAdd.erase(std::remove(mQueueAdd.begin(), mQueueAdd.end(), nse), mQueueAdd.end());
			mQueueInsert.erase(std::remove(mQueueInsert.begin(), mQueueInsert.end(), nse), mQueueInsert.end());
		}
		mQueue.insert(mQueue.end(), mQueueAdd.begin(), mQueueAdd.end());
		mQueue.insert(mQueue.begin(), mQueueInsert.begin(), mQueueInsert.end());
		mQueueAdd.clear();
		mQueueInsert.clear();
		mQueueRemove.clear();

		// All done
		mExecuting = false;

		// If nothing was executed, and the GC counter has been reached
		if(!ok && mGCCounter > g_Config.SquirrelGCCallCount) {
			unsigned long now = g_PlatformTime.getElapsedMilliseconds();
			if(mForceGC == 0) {
				mForceGC = now + g_Config.SquirrelGCMaxDelay;
			}
			if(mMaybeGC == 0) {
				mMaybeGC = now;
			}
			if(now >= mMaybeGC + g_Config.SquirrelGCDelay || now >= mForceGC) {
				int objs = GC();
				unsigned long took = g_PlatformTime.getElapsedMilliseconds() - now;
				g_Log.AddMessageFormat("GC performed because callcount reached %d and returned %d objects taking %ul ms", mGCCounter, took, objs);
				mGCTime += took;
				mGCCounter = 0;
				mForceGC = 0;
				mMaybeGC = 0;
			}
		}
		else {
			mMaybeGC = 0;
		}

		return ok;
	}

	void NutPlayer::QueueInsert(NutScriptEvent *evt)
	{
		if(!mActive) {
			PrintMessage("[WARNING] Script event when not active");
			return;
		}
		if(mExecuting)
		{
			if(mQueueInsert.size() >= MAX_QUEUE_SIZE)
			{
				PrintMessage("[ERROR] Script error: Deferred QueueEvent() list is full %d of %d", mQueueInsert.size(), MAX_QUEUE_SIZE);
				return;
			}
			mQueueInsert.insert(mQueueInsert.begin(), evt);
		} else
		{

			if(mQueue.size() >= MAX_QUEUE_SIZE)
			{
				PrintMessage("[ERROR] Script error: QueueEvent() list is full [script: %s]", def->scriptName.c_str());
				return;
			}
			mQueue.insert(mQueue.begin(), evt);
		}
	}

	void NutPlayer::QueueClear()
	{
		if(mExecuting)
		{
			for(size_t i = 0; i < mQueue.size(); i++)	{
				NutScriptEvent *nse = mQueue[i];
				mQueueRemove.push_back(nse);
			}
		}
		else
		{
			ClearQueue();
		}
	}

	void NutPlayer::QueueRemove(NutScriptEvent *evt)
	{
		if(mExecuting)
		{
			mQueueRemove.insert(mQueueRemove.begin(), evt);
		}
		else
		{
			mQueue.erase(std::find(mQueue.begin(), mQueue.end(), evt));
		}
	}

	void NutPlayer::QueueAdd(NutScriptEvent *evt)
	{
		if(!mActive) {
			PrintMessage("[WARNING] Script event when not active");
			return;
		}

		if(mExecuting)
		{
			if(mQueueAdd.size() >= MAX_QUEUE_SIZE)
			{
				PrintMessage("[ERROR] Script error: Deferred QueueEvent() list is full %d of %d", mQueueAdd.size(), MAX_QUEUE_SIZE);
				return;
			}
			mQueueAdd.push_back(evt);
		} else
		{

			if(mQueue.size() >= MAX_QUEUE_SIZE)
			{
				PrintMessage("[ERROR] Script error: QueueEvent() list is full [script: %s]", def->scriptName.c_str());
				return;
			}
			mQueue.push_back(evt);
		}
	}

	void NutPlayer::Broadcast(const char *message)
	{
		g_SimulatorManager.BroadcastMessage(message);
	}

	SQInteger NutPlayer::Sleep(HSQUIRRELVM v)
	{
	    if (sq_gettop(v) == 2) {
	        Sqrat::Var<NutPlayer&> left(v, 1);
	        if (!Sqrat::Error::Occurred(v)) {
	            Sqrat::Var<unsigned long> right(v, 2);
	        	std::vector<int> vv;
	        	ResumeCallback *cb = new ResumeCallback(&left.value);
	        	NutScriptEvent *nse = new NutScriptEvent(new TimeCondition (right.value), cb);
	        	nse->mRunWhenSuspended = true;
	        	left.value.QueueAdd(nse);
	            return sq_suspendvm(v);
	        }
	        return sq_throwerror(v, Sqrat::Error::Message(v).c_str());
	    }
	    return sq_throwerror(v, _SC("wrong number of parameters"));
	}


	void PrintMessage(const char *format, ...) {
		char messageBuf[2048];

		va_list args;
		va_start(args, format);
		vsnprintf(messageBuf, sizeof(messageBuf) - 1, format, args);
		va_end(args);

		g_Log.AddMessageFormat("%s", messageBuf);
	}



OpCodeInfo defCoreOpCode[] = {
	{ "nop",          OP_NOP,          0, {OPT_NONE,   OPT_NONE,   OPT_NONE  }},
	{ "end",          OP_END,          0, {OPT_NONE,   OPT_NONE,   OPT_NONE  }},
	{ "goto",         OP_GOTO,         1, {OPT_LABEL,  OPT_NONE,   OPT_NONE  }},
	{ "reset_goto",   OP_RESETGOTO,    1, {OPT_LABEL,  OPT_NONE,   OPT_NONE  }},
	{ "print",        OP_PRINT,        1, {OPT_STR,    OPT_NONE,   OPT_NONE  }},
	{ "printvar",     OP_PRINTVAR,     1, {OPT_VAR,    OPT_NONE,   OPT_NONE  }},
	{ "printappvar",  OP_PRINTAPPVAR,  1, {OPT_STR,    OPT_NONE,   OPT_NONE  }},
	{ "printintarr",  OP_PRINTINTARR,  1, {OPT_INTARR, OPT_NONE,   OPT_NONE  }},
	{ "wait",         OP_WAIT,         1, {OPT_INT,    OPT_NONE,   OPT_NONE  }},
	{ "inc",          OP_INC,          1, {OPT_VAR,    OPT_NONE,   OPT_NONE  }},
	{ "dec",          OP_DEC,          1, {OPT_VAR,    OPT_NONE,   OPT_NONE  }},
	{ "set",          OP_SET,          2, {OPT_VAR,    OPT_INT,    OPT_NONE  }},
	{ "copyvar",      OP_COPYVAR,      2, {OPT_VAR,    OPT_VAR,    OPT_NONE  }},
	{ "getappvar",    OP_GETAPPVAR,    2, {OPT_STR,    OPT_VAR,    OPT_NONE  }},
	{ "call",         OP_CALL,         1, {OPT_LABEL,  OPT_NONE,   OPT_NONE  }},
	{ "return",       OP_RETURN,       0, {OPT_NONE,   OPT_NONE,   OPT_NONE  }},
	{ "iarrappend",   OP_IARRAPPEND,   2, {OPT_INTARR, OPT_INTSTK, OPT_NONE  }},
	{ "iarrdelete",   OP_IARRDELETE,   2, {OPT_INTARR, OPT_INTSTK, OPT_NONE  }},
	{ "iarrvalue",    OP_IARRVALUE,    3, {OPT_INTARR, OPT_INTSTK, OPT_VAR   }},
	{ "iarrclear",    OP_IARRCLEAR,    1, {OPT_INTARR, OPT_NONE,   OPT_NONE  }},
	{ "iarrsize",     OP_IARRSIZE,     2, {OPT_INTARR, OPT_VAR,    OPT_NONE  }},
	{ "queue_event",  OP_QUEUEEVENT,   2, {OPT_STR,    OPT_INT,    OPT_NONE  }},
	{ "exec_queue",   OP_EXECQUEUE,    0, {OPT_NONE,   OPT_NONE,   OPT_NONE  }},

	//These functions are used internally, but can be called directly in the script as well.
	{ "pushvar",      OP_PUSHVAR,      1, {OPT_VAR,    OPT_NONE,   OPT_NONE  }},
	{ "pushint",      OP_PUSHINT,      1, {OPT_INT,    OPT_NONE,   OPT_NONE  }},
	{ "pop",          OP_POP,          1, {OPT_VAR,    OPT_NONE,   OPT_NONE  }},

	//These functions are for internal purpose only, and should not be used by a script.
	{ "_pushappvar",  OP_PUSHAPPVAR,   0, {OPT_NONE,   OPT_NONE,   OPT_NONE  }},
	{ "_cmp",         OP_CMP,          0, {OPT_NONE,   OPT_NONE,   OPT_NONE  }},
	{ "_jmp",         OP_JMP,          0, {OPT_NONE,   OPT_NONE,   OPT_NONE  }},
};

OpCodeInfo extCoreOpCode[] = {
	{ "nop",   OP_NOP,   0, {OPT_NONE,   OPT_NONE }},
};

const int maxCoreOpCode = sizeof(defCoreOpCode) / sizeof(OpCodeInfo);
const int maxExtOpCode = sizeof(extCoreOpCode) / sizeof(OpCodeInfo);

OpData:: OpData()
{
	opCode = OP_NOP;
	param1 = 0;
	param2 = 0;
	param3 = 0;
}

OpData :: OpData(int op, int p1, int p2)
{ 
	opCode = op;
	param1 = p1;
	param2 = p2;
	param3 = 0;
}

OpData :: OpData(int op, int p1, int p2, int p3)
{
	opCode = op;
	param1 = p1;
	param2 = p2;
	param3 = p3;
}

LabelDef :: LabelDef(const char *labelName, int targInst)
{
	name = labelName;
	instrOffset = targInst;
}


ScriptDef :: ScriptDef()
{
	curInst = 0;
	scriptSpeed = DEFAULT_INSTRUCTIONS_PER_CYCLE;
	scriptIdleSpeed = DEFAULT_INSTRUCTIONS_PER_IDLE_CYCLE;
	queueExternalJumps = false;
	queueCallStyle = CALLSTYLE_CALL;
	mFlags = FLAG_DEFAULT;
}

ScriptDef :: ~ScriptDef()
{
}

void ScriptDef :: ClearBase(void)
{
	scriptName.clear();
	instr.clear();
	stringList.clear();

	label.clear();
	varName.clear();
	curInst = 0;
	extVarName.clear();
	mLabelMap.clear();
	mIntArray.clear();

	scriptSpeed = DEFAULT_INSTRUCTIONS_PER_CYCLE;
	scriptIdleSpeed = DEFAULT_INSTRUCTIONS_PER_IDLE_CYCLE;
	queueExternalJumps = false;
	queueCallStyle = CALLSTYLE_CALL;
	mFlags = FLAG_DEFAULT;

	ClearDerived();
}

// Stub function, if additional members are defined in a derived class, then override this function
// to clear the new members.
void ScriptDef :: ClearDerived(void)
{
}

void ScriptDef :: CompileFromSource(const char *sourceFile)
{
	FileReader lfr;
	if(lfr.OpenText(sourceFile) != Err_OK)
	{
		PrintMessage("[WARNING] InstanceScript::CompileFromSource() unable to open file: %s", sourceFile);
		return;
	}

	lfr.CommentStyle = Comment_Semi;

	ScriptCompiler compileData;
	compileData.mSourceFile = sourceFile;

	while(lfr.FileOpen() == true)
	{
		lfr.ReadLine();
		compileData.mLineNumber = lfr.LineNumber;
		CompileLine(lfr.DataBuffer, compileData);
	}
	FinalizeCompile(compileData);
}

void ScriptDef :: CompileLine(char *line, ScriptCompiler &compileData)
{
	STRINGLIST &tokens = compileData.mTokens;  //Alias so we don't have to change the rest of the code.

	Tokenize(line, tokens);
	compileData.CheckSymbolReplacements();
	const char *opname = GetSubstring(tokens, 0);
	if(opname != NULL)
	{
		if(opname[0] == ':')
		{
			CreateLabel(GetOffsetIntoString(opname, 1), curInst);
		}
		else if(strcmp(opname, "if") == 0)
		{
			if(tokens.size() < 4)
				PrintMessage("Syntax error: not enough operands for IF statement in [%s line %d]", compileData.mSourceFile, compileData.mLineNumber);
			else
			{
				int vleft = 0;
				int vright = 0;
				int left = ResolveOperandType(tokens[1].c_str(), vleft);
				int cmp = ResolveComparisonType(tokens[2].c_str());
				int right = ResolveOperandType(tokens[3].c_str(), vright);
				if(cmp == CMP_INV)
					PrintMessage("Invalid comparison operator: [%s] in [%s line %d]", compileData.mSourceFile, compileData.mLineNumber);
				else
				{
					bool valid = true;
					//Push the values onto the stack backwards, so that when
					//popped off they'll follow a left to right order.
					switch(right)
					{
					case OPT_VAR: PushOpCode(OP_PUSHVAR, vright, 0); break;
					case OPT_INT: PushOpCode(OP_PUSHINT, vright, 0); break;
					case OPT_FLOAT: PushOpCode(OP_PUSHFLOAT, vright, 0); break;
					case OPT_APPINT: PushOpCode(OP_PUSHAPPVAR, vright, 0); break;
					default: PrintMessage("Invalid operator [%s] for IF statement [%s line %d]", tokens[3].c_str(), compileData.mSourceFile, compileData.mLineNumber); valid = false; break;
					}
		
					switch(left)
					{
					case OPT_VAR: PushOpCode(OP_PUSHVAR, vleft, 0); break;
					case OPT_INT: PushOpCode(OP_PUSHINT, vleft, 0); break;
					case OPT_FLOAT: PushOpCode(OP_PUSHFLOAT, vleft, 0); break;
					case OPT_APPINT: PushOpCode(OP_PUSHAPPVAR, vleft, 0); break;
					default: PrintMessage("Invalid operator [%s] for IF statement [%s line %d]", tokens[1].c_str(), compileData.mSourceFile, compileData.mLineNumber); valid = false; break;
					}

					if(valid == true)
					{
						// Normally the IF command will continue to the next line
						// if it succeeds.  If it fails, jump over the next immediate
						// line by 2 spaces, unless otherwise programmed.
						// Line 1 = if a != b
						// Line 2 =   print "Not equal"
						// Line 3 = wait 1000
						int jumpCount = 2;
						if(tokens.size() >= 6)
							if(tokens[5].compare("else") == 0)
								jumpCount = atoi(tokens[5].c_str());

						compileData.OpenBlock(compileData.mLineNumber, curInst);
						PushOpCode(OP_CMP, cmp, jumpCount);
					}
				}
			}
		}
		else if(strcmp(opname, "endif") == 0)
		{
			BlockData *block = compileData.GetLastUnresolvedBlock();
			if(block == NULL)
				PrintMessage("[WARNING] ENDIF without a matching IF (%s line %d)", compileData.mSourceFile, compileData.mLineNumber);
			else
			{
				//In both cases, we want to alter the CMP instruction data with the jump offset
				//to skip over the 'true' statement block when the result is 'false'
				if(block->mUseElse == false)  //IF...ENDIF format
				{
					instr[block->mInstIndex].param2 = curInst - block->mInstIndex;
				}
				else  //IF...ELSE...ENDIF format
				{
					// +1 to accomodate the added JMP instruction.  mInstIndexElse points to the first
					// instruction of the 'false' block.
					instr[block->mInstIndex].param2 = block->mInstIndexElse - block->mInstIndex + 1;

					//For "else" blocks, we need to modify the JMP instruction (the last statement
					//in the 'true' block) to jump over the ELSE block.
					instr[block->mInstIndexElse].param1 = curInst;
				}
				compileData.CloseBlock();
			}
		}
		else if(strcmp(opname, "else") == 0)
		{
			BlockData *block = compileData.GetLastUnresolvedBlock();
			if(block == NULL)
				PrintMessage("[WARNING] ELSE without a matching IF (%s line %d)", compileData.mSourceFile, compileData.mLineNumber);
			else
			{
				block->mInstIndexElse = curInst;
				block->mUseElse = true;

				//The instructions executed in a 'true' condition will be followed by a jump
				//to bypass the instructions executed in a 'false' condition.
				//The instruction index will be adjusted when "endif" is encountered and processed,
				//for now it will default to fall through into the next instruction. 
				PushOpCode(OP_JMP, curInst + 1, 0);  
			}
		}
		else if(strcmp(opname, "recompare") == 0)
		{
			BlockData *block = compileData.GetLastUnresolvedBlock();
			if(block == NULL)
				PrintMessage("[WARNING] RECOMPARE without a matching IF (%s line %d)", compileData.mSourceFile, compileData.mLineNumber);
			else
			{
				//Jump to the CMP instruction of the "if" statement, plus the two PUSH operations
				//that come before. 
				PushOpCode(OP_JMP, block->mInstIndex - 2, 0);
			}
		}
		else if(opname[0] == '#')
		{
			SetMetaDataBase(opname, compileData);
		}
		else if(HandleAdvancedCommand(opname, compileData) == true)
		{
			//Do nothing, the function will take care of it.
		}
		else
		{
			OpCodeInfo *opinfo = GetInstructionDataByName(opname);
			if(opinfo->opCode == OP_NOP)
			{
				PrintMessage("Unknown instruction [%s] [%s line %d]", opname, compileData.mSourceFile, compileData.mLineNumber);
			}
			else
			{
				int param[3] = {0};
				bool resolveLabels = false;
				for(int i = 0; i < opinfo->numParams; i++)
				{
					const char *paramToken = GetSubstring(tokens, i + 1);
					int pushType = OPT_NONE;
					/*
					if(Expect(paramToken, opinfo->paramType[i]) == false)
						PrintMessage("Token [%s] does not match the expected type [%s] on [%s line %d]", paramToken, GetExpectedDetail(opinfo->paramType[i]), compileData.mSourceFile, compileData.mLineNumber);
					*/
					param[i] = ResolveOperand(opinfo->paramType[i], paramToken, pushType);

					if(opinfo->paramType[i] == OPT_LABEL)
						resolveLabels = true;

					// If this isn't an explicit argument type (like OPT_INT, OPT_VAR, etc), it means
					// that multiple sources may provide data via a stack PUSH operation, and won't be
					// embedded into the compiled instruction data.  This is similar to how the "if"
					// statement works, which allows and permutation of comparisions between 
					// integers and variables.
					// The associated opcode command must be responsible for popping the values off the
					// stack.
					if(pushType != OPT_NONE)
					{
						pushType = ConvertParamTypeToPushType(pushType);
						if(pushType == OPT_NONE)
						{
							PrintMessage("Could not convert parameter token [%s] to expected type [%s line %d]", paramToken, compileData.mSourceFile, compileData.mLineNumber);
						}
						else
						{
							PushOpCode(pushType, param[i], 0);
							param[i] = 0;  //Set to zero since the instruction will be using stack POP data instead.
						}
					}
				}

				//This appends a list of instruction indexes which need to have their label data resolved.
				//Add here before the current instruction index is incremented.
				if(resolveLabels == true)
					compileData.AddPendingLabelReference(curInst);

				PushOpCode(opinfo->opCode, param[0], param[1], param[2]);
			}
		}
	}
}

void ScriptDef :: BeginInlineBlock(ScriptCompiler &compileData)
{
	compileData.mSourceFile = "<inline>";
	compileData.mInlineBeginInstr = curInst;
}

void ScriptDef :: FinishInlineBlock(ScriptCompiler &compileData)
{
	FinalizeCompile(compileData);
}

void ScriptDef :: FinalizeCompile(ScriptCompiler &compileData)
{
	//Resolve labels and jumps
	for(size_t i = 0; i < label.size(); i++)
	{
		if(label[i].instrOffset == -1)
		{
			PrintMessage("Unresolved label: %s", label[i].name.c_str());
			label[i].instrOffset = 0;
		}
	}

	for(size_t i = 0; i < compileData.mPendingLabelReference.size(); i++)
	{
		int index = compileData.mPendingLabelReference[i];
		if(index >= 0)
		{
			OpCodeInfo *opInfo = GetInstructionData(instr[index].opCode);
			if(opInfo != NULL)
			{
				if(opInfo->paramType[0] == OPT_LABEL)
					instr[index].param1 = label[instr[index].param1].instrOffset;
				if(opInfo->paramType[1] == OPT_LABEL)
					instr[index].param2 = label[instr[index].param2].instrOffset;
				if(opInfo->paramType[2] == OPT_LABEL)
					instr[index].param3 = label[instr[index].param3].instrOffset;
			}
		}
	}
	compileData.mPendingLabelReference.clear();

	/*  OBSOLETE: needed a new system because extended opcode tables which used label resolution
	would not be resolved here and cause bugs.

	for(size_t i = startInstruction; i < instr.size(); i++)
	{
		//Replace the jump target from a label index to an instruction offet
		if(instr[i].opCode == OP_GOTO || instr[i].opCode == OP_CALL)
			instr[i].param1 = label[instr[i].param1].instrOffset;
	}
	*/
}

//Try to verify whether the token string matches the expected type.
bool ScriptDef :: Expect(const char *token, int paramType)
{
	switch(paramType)
	{
	case OPT_NONE: return false;
	case OPT_LABEL: return (GetLabelIndex(token) >= 0);
	case OPT_STR: return true;
	case OPT_INT: return true;
	case OPT_FLOAT: return true;
	case OPT_VAR: return (GetVariableIndex(token) >= 0);
	case OPT_APPINT: return true;
	case OPT_INTSTK: return true;
	case OPT_INTARR: return (GetIntArrayIndex(token) >= 0);
	default: return true;
	}
	return true;
}


const char* ScriptDef :: GetExpectedDetail(int paramType)
{
	switch(paramType)
	{
	case OPT_NONE: return "<none>";
	case OPT_LABEL: return "label";
	case OPT_STR: return "string";
	case OPT_INT: return "integer";
	case OPT_FLOAT: return "float";
	case OPT_VAR: return "variable";
	case OPT_APPINT: return "property name";
	case OPT_INTSTK: return "resolvable integer result";
	case OPT_INTARR: return "integer array name";
	}
	return "<unknown>";
}

int ScriptDef :: ResolveOperand(int paramType, const char *token, int &pushType)
{
	pushType = OPT_NONE;

	if(token == NULL)
		return 0;

	switch(paramType)
	{
	case OPT_LABEL:
		return CreateLabel(token, -1);
	case OPT_STR:
		return CreateString(token);
	case OPT_FLOAT:
		return atof(token);
	case OPT_INT:
		return atoi(token);
	case OPT_VAR:
		return CreateVariable(token);
	case OPT_INTARR:
		return CreateIntArray(token);
	case OPT_INTSTK:
		{
			int value = 0;
			pushType = ResolveOperandType(token, value);
			return value;
		}
		break;
	}
	return 0;
}

// Some command instructions accept parameters that may be resolved from multiple types, with the result
// placed on the stack.  This converts the parameter type to the associated push opcode that must
// be performed.
int ScriptDef :: ConvertParamTypeToPushType(int paramType)
{
	switch(paramType)
	{
	case OPT_INT:
		return OP_PUSHINT;
	case OPT_FLOAT:
		return OP_PUSHFLOAT;
	case OPT_VAR:
		return OP_PUSHVAR;
	case OPT_APPINT: return OP_PUSHAPPVAR;
	}
	return OPT_NONE;
}

int ScriptDef :: CreateLabel(const char *name, int targInst)
{
	if(name == NULL)
	{
		PrintMessage("Label name is null."); 
		return -1;
	}
	if(name[0] == 0)
	{
		PrintMessage("Label name cannot be empty."); 
		return -1;
	}

	int r = GetLabelIndex(name);
	if(r == -1)
	{
		label.push_back(LabelDef(name, targInst));
		r = label.size() - 1;
		mLabelMap[name] = r;
	}
	if(targInst >= 0)
		label[r].instrOffset = targInst;
	return r;
}

int ScriptDef :: GetLabelIndex(const char *name)
{
	std::map<std::string, int>::iterator it;
	it = mLabelMap.find(name);
	if(it == mLabelMap.end())
		return -1;
	
	return it->second;

	/*
	for(size_t i = 0; i < label.size(); i++)
		if(label[i].name.compare(name) == 0)
			return i;
	return -1;
	*/
}

int ScriptDef :: CreateString(const char *name)
{
	if(name == NULL)
	{
		PrintMessage("String cannot be NULL."); 
		return -1;
	}

	int r = GetStringIndex(name);
	if(r == -1)
	{
		stringList.push_back(name);
		r = stringList.size() - 1;
	}
	return r;
}

int ScriptDef :: GetStringIndex(const char *name)
{
	for(size_t i = 0; i < stringList.size(); i++)
		if(stringList[i].compare(name) == 0)
			return i;
	return -1;
}

int ScriptDef :: CreateVariable(const char *name)
{
	for(size_t i = 0; i < varName.size(); i++)
		if(varName[i].compare(name) == 0)
			return i;
	varName.push_back(name);
	return varName.size() - 1;
}

int ScriptDef :: GetVariableIndex(const char *name)
{
	for(size_t i = 0; i < varName.size(); i++)
		if(varName[i].compare(name) == 0)
			return i;
	return -1;
}

int ScriptDef :: CreateIntArray(const char *name)
{
	if(GetIntArrayIndex(name) == -1)
	{
		mIntArray.push_back(IntArray(name));
	}
	return (int)mIntArray.size() - 1;
}

int ScriptDef :: GetIntArrayIndex(const char *name)
{
	for(size_t i = 0; i < mIntArray.size(); i++)
		if(mIntArray[i].name.compare(name) == 0)
			return (int)i;
	return -1;
}

void ScriptDef :: OutputDisassemblyToFile(FILE *output)
{
	if(output == NULL)
		output = stdout;

	fprintf(output, "Script: %s\r\n", scriptName.c_str());
	fprintf(output, "Labels: %lu\r\n", label.size());
	for(size_t i = 0; i < label.size(); i++)
		fprintf(output, "[%lu] = %s : %d\r\n", i, label[i].name.c_str(), label[i].instrOffset);

	fprintf(output, "\r\nStrings:\r\n");
	for(size_t i = 0; i < stringList.size(); i++)
		fprintf(output, "[%lu]=[%s]\r\n", i, stringList[i].c_str());

	fprintf(output, "\r\nVariables:\r\n");
	for(size_t i = 0; i < varName.size(); i++)
		fprintf(output, "[%lu]=[%s]\r\n", i, varName[i].c_str());

	fprintf(output, "\r\nInstructions:\r\n");
	for(size_t i = 0; i < instr.size(); i++)
	{
		OpCodeInfo *opi = GetInstructionData(instr[i].opCode);
		fprintf(output, "[%lu] : %s %d %d\r\n", i, opi->name, instr[i].param1, instr[i].param2);
	}
	fprintf(output, "\r\n");
}

void ScriptDef :: Tokenize(const char *srcString, STRINGLIST &destList)
{
	std::string subStr;
	destList.clear();
	//printf(" 12345678901234567890\n");
	//printf("[%s]\n", srcBuf);
	int len = strlen(srcString);
	int start = -1;
	int end = -1;
	bool quote = false;
	for(int i = 0; i <= len; i++)
	{
		switch(srcString[i])
		{
		case '\t':
		case ' ': //Terminate a word if it has started, and is not within a quoted phrase.
			if(start != -1)
				if(quote == false)
					end = i - 1;
			break;
		case '"': //Begin or terminate a quote phrase.
			//printf("Pos: %d\n", i);
			if(quote == false)
			{
				quote = true;
				start = i;
			}
			else
				end = i;
			break;
		case '\0': //Terminate a word if it has started, and abort a quote if it hasn't completed.
			if(quote == false)
			{
				if(start != -1)
					end = i - 1;
			}
			else
			{
				//printf("aborted\n");
				start = -1;
				end = -1;
				quote = false;
			}
			break;
		default:
			if(start == -1)
				start = i;
			break;
		}
		if(end != -1)
		{
			if(quote == true)
			{
				//Drop the quotation marks from both ends.
				start++;
				end--;
			}
			if(end + 1 <= len)
			{
				subStr.assign(&srcString[start], end - start + 1);
				destList.push_back(subStr);
				if(quote == true)
				{
					stringList.push_back(subStr);
					quote = false;
				}
				/*
				char temp = srcString[end];
				srcString[end + 1] = 0;
				destList.push_back(&srcString[start]);
				if(quote == true)
				{
					stringList.push_back(&srcString[start]);
					quote = false;
				}

				srcString[end + 1] = temp;
				*/
			}
			start = -1;
			end = -1;
		}
	}

	/*
	printf("Tokenize: %d :", resList.size());
	for(size_t i = 0; i < resList.size(); i++)
		printf(" [%s]", resList[i].c_str());
	printf("\n\n\n");
	*/
}

const char * ScriptDef :: GetSubstring(STRINGLIST &strList, int index)
{
	if(index < 0 || index >= (int)strList.size())
		return NULL;
	return strList[index].c_str();
}

const char * ScriptDef :: GetOffsetIntoString(const char *value, int offset)
{
	if(offset < 0 || offset >= (int)strlen(value))
		return NULL;
	return &value[offset];
}

void ScriptDef :: PushOpCode(int opcode, int param1, int param2)
{
	instr.push_back(OpData(opcode, param1, param2));
	curInst++;
}

void ScriptDef :: PushOpCode(int opcode, int param1, int param2, int param3)
{
	instr.push_back(OpData(opcode, param1, param2, param3));
	curInst++;
}

OpCodeInfo* ScriptDef :: GetInstructionData(int opcode)
{
	for(int i = 0; i < maxCoreOpCode; i++)
		if(defCoreOpCode[i].opCode == opcode)
			return &defCoreOpCode[i];

	//Fetch the table, which may be different for derived classes.
	OpCodeInfo *arrayStart = NULL;
	size_t arraySize = 0;
	GetExtendedOpCodeTable(&arrayStart, arraySize);

	/*
	for(int i = 0; i < maxExtOpCode; i++)
		if(extCoreOpCode[i].opCode == opcode)
			return &extCoreOpCode[i];
	*/
	for(size_t i = 0; i < arraySize; i++)
	{
		if(arrayStart[i].opCode == opcode)
			return &arrayStart[i];
	}

	PrintMessage("Unidentified opcode: %d", opcode);
	return &defCoreOpCode[0];
}

OpCodeInfo* ScriptDef :: GetInstructionDataByName(const char *name)
{
	for(int i = 0; i < maxCoreOpCode; i++)
		if(strcmp(defCoreOpCode[i].name, name) == 0)
			return &defCoreOpCode[i];

	OpCodeInfo *arrayStart = NULL;
	size_t arraySize = 0;
	GetExtendedOpCodeTable(&arrayStart, arraySize);

	/*
	for(int i = 0; i < maxExtOpCode; i++)
		if(strcmp(extCoreOpCode[i].name, name) == 0)
			return &extCoreOpCode[i];
	*/
	for(size_t i = 0; i < arraySize; i++)
		if(strcmp(arrayStart[i].name, name) == 0)
			return &arrayStart[i];

	return &defCoreOpCode[0];
}

// Override this to return the pointer to the first element of the opcode definition array that the
// derived class should use, and its array size.
void ScriptDef :: GetExtendedOpCodeTable(OpCodeInfo **arrayStart, size_t &arraySize)
{
	*arrayStart = ScriptCore::extCoreOpCode;
	arraySize = ScriptCore::maxExtOpCode;
}

int ScriptDef :: ResolveOperandType(const char *token, int &value)
{
	if(token == NULL)
	{
		PrintMessage("Token is null");
		value = 0;
		return OPT_INT;
	}
	if(token[0] == '@')
	{
		value = CreateString(token);
		return OPT_APPINT;
	}
	int var = GetVariableIndex(token);
	if(var >= 0)
	{
		value = var;
		return OPT_VAR;
	}
	const char *pPosition = strchr(token, '.');
	if(pPosition != NULL)
	{
		value = atof(token);
		return OPT_FLOAT;
	}
	value = atoi(token);
	return OPT_INT;
}

int ScriptDef :: ResolveComparisonType(const char *token)
{
	static const char *name[6]  = {   "=",    "!=",    "<",    "<=",    ">",    ">=" };
	static const int value[6] = {CMP_EQ, CMP_NEQ, CMP_LT, CMP_LTE, CMP_GT, CMP_GTE };
	for(int i = 0; i < 6; i++)
		if(strcmp(token, name[i]) == 0)
			return value[i];

	return CMP_INV;
}

void ScriptDef :: SetScriptSpeed(const char *token)
{
	int amount = atoi(token);
	if(amount < 1)
		amount = 1;
	scriptSpeed = 1;
}

bool ScriptDef :: CanIdle(void)
{
	return (scriptIdleSpeed > 0);
}

bool ScriptDef :: UseEventQueue(void)
{
	return queueExternalJumps;
}

void ScriptDef :: SetMetaDataBase(const char *opname, ScriptCompiler &compileData)
{
	const STRINGLIST &tokens = compileData.mTokens;  //Alias so we don't have to change the rest of the code.
	if(strcmp(opname, "#name") == 0)
	{
		if(tokens.size() >= 2)
			scriptName = tokens[1];
	}
	else if(strcmp(opname, "#symbol") == 0)
	{
		if(tokens.size() >= 3)
			compileData.AddSymbol(tokens[1], tokens[2]);
	}
	else if(strcmp(opname, "#speed") == 0)
	{
		if(tokens.size() >= 2)
			scriptSpeed = atoi(tokens[1].c_str());
	}
	else if(strcmp(opname, "#idlespeed") == 0)
	{ 
		if(tokens.size() >= 2)
			scriptIdleSpeed = atoi(tokens[1].c_str());
	}
	else if(strcmp(opname, "#queue_jumps") == 0)
	{ 
		queueCallStyle = CALLSTYLE_CALL;
		queueExternalJumps = true;
		if(tokens.size() >= 2)
		{
			if(tokens[1].compare("goto") == 0)
				queueCallStyle = CALLSTYLE_GOTO;
		}
	}
	else if(strcmp(opname, "#flag") == 0)
	{
		if(tokens.size() >= 3)
		{
			const char *flagName = tokens[1].c_str();
			int value = atoi(tokens[2].c_str());
			unsigned int flag = 0;
			if(strcmp(flagName, "report_end") == 0)
				flag = FLAG_REPORT_END;
			else if(strcmp(flagName, "report_label") == 0)
				flag = FLAG_REPORT_LABEL;
			else if(strcmp(flagName, "report_all") == 0)
				flag = FLAG_REPORT_ALL;
			else if(strcmp(flagName, "bits") == 0)
				flag = FLAG_BITS;
			else
				PrintMessage("Unknown flag [%s] [%s line %d]", flagName, compileData.mSourceFile, compileData.mLineNumber);
			SetFlag(flag, value);
		}
	}
	else
	{
		SetMetaDataDerived(opname, compileData);
	}
}

void ScriptDef :: SetFlag(unsigned int flag, unsigned int value)
{
	if(flag == FLAG_BITS)
	{
		mFlags = value;
		return;
	}
	if(value)
		mFlags |= flag;
	else
		mFlags &= (~(flag));
}

bool ScriptDef :: HasFlag(unsigned int flag)
{
	return ((mFlags & flag) != 0);
}

void ScriptDef :: SetMetaDataDerived(const char *opname, ScriptCompiler &compileData)
{
	//const STRINGLIST &tokens = compileData.mTokens;  //Alias so we don't have to change the rest of the code.
	PrintMessage("Unhandled metadata token [%s] (%s line %d)", opname, compileData.mSourceFile, compileData.mLineNumber);
}



//Perform special handling for certain commands here, with parameter counts or behavior that cannot
//properly fit into the "command [param] ..." line format
//Return false if there was no handler for the operative token.
//virtual : Override in derived class if necessary.
bool ScriptDef :: HandleAdvancedCommand(const char *commandToken, ScriptCompiler &compileData)
{
	return false;
}

ScriptPlayer :: ScriptPlayer()
{
	def = NULL;
	curInst = 0;
	mExecuting = false;
	mActive = false;
	nextFire = 0;
	advance = 0;
	mProcessingTime = 0;
}

ScriptPlayer :: ~ScriptPlayer()
{
}

void ScriptPlayer :: Initialize(ScriptDef *defPtr)
{
	g_Log.AddMessageFormat("Initialising TSL script %s", defPtr->scriptName.c_str());
	def = defPtr;
	FullReset();
}

void ScriptPlayer :: RunScript(void)
{
	while(mExecuting && mActive)
		RunSingleInstruction();
}

bool ScriptPlayer :: RunSingleInstruction(void)
{
	//Return true if the script is interrupted or terminated.
	if(!mExecuting)
		return true;

	if(curInst >= (int)def->instr.size())
	{
		PrintMessage("[ERROR] Instruction past end of script (%d of %d)", curInst, def->instr.size());
		mExecuting = false;
		return true;
	}
	if(g_ServerTime < nextFire)
		return true;

	unsigned long now = g_PlatformTime.getElapsedMilliseconds();
	bool breakScript = false;
	advance = 1;

	OpData *instr = &def->instr[curInst];

	switch(instr->opCode)
	{
	case OP_END:
		g_Log.AddMessageFormat("[REMOVEME] Ending script %s because OP_END.", def->scriptName.c_str());
		EndExecution();
		breakScript = true;
		break;
	case OP_GOTO:
		//printf("Jumping:\n");c
		curInst = instr->param1;
		advance = 0;
		/*
		{
			int t = def->instr[curInst].param1;
			if(t < 0 || t >= (int)def->label.size())
				g_Log.AddMessageFormat("Index out of range: %d/%d", t, def->label.size());
			else
				g_Log.AddMessageFormat("%d = %s,%d", t, def->label[t].name.c_str(), def->label[t].instrOffset);
		}
		*/
		break;
	case OP_RESETGOTO:
		ResetGoto(def->instr[curInst].param1);
		advance = 0;
		break;
	case OP_PRINT:
		PrintMessage("%s", def->stringList[def->instr[curInst].param1].c_str());
		break;
	case OP_PRINTVAR:
		PrintMessage("var[%s]=%d", def->varName[instr->param1].c_str(), GetVarValue(instr->param1));
		break;
	case OP_PRINTAPPVAR:
		PrintMessage("appvar[%s]=%d", GetStringPtr(instr->param1), GetApplicationPropertyAsInteger(GetStringPtr(instr->param1)));
		break;
	case OP_PRINTINTARR:
		if(VerifyIntArrayIndex(instr->param1) >= 0)
			intArray[instr->param1].DebugPrintContents();
		break;
	case OP_WAIT:
		nextFire = g_ServerTime + instr->param1;
		breakScript = true;
		break;
	case OP_PUSHVAR:
		int value;
		value = GetVarValue(instr->param1);
		PushVarStack(value);
		break;
	case OP_PUSHAPPVAR:
		PushVarStack(GetApplicationPropertyAsInteger(GetStringPtr(instr->param1)));
		break;
	case OP_PUSHINT:
		PushVarStack(instr->param1);
		break;
	case OP_POP:
		SetVar(instr->param1, PopVarStack());
		break;
	case OP_CMP:
		int left, right, cmp;
		bool result;
		result = false;
		left = PopVarStack();
		right = PopVarStack();
		cmp = instr->param1;
		//PrintMessage("IF %d (%d) %d", left, cmp, right);
		switch(cmp)
		{
		case CMP_EQ: result = (left == right); break;
		case CMP_NEQ: result = (left != right); break;
		case CMP_LT: result = (left < right); break;
		case CMP_LTE: result = (left <= right); break;
		case CMP_GT: result = (left > right); break;
		case CMP_GTE: result = (left >= right); break;
		};
		if(result == false)
			advance = instr->param2;
		break;
	case OP_INC:
		vars[instr->param1]++;
		break;
	case OP_DEC:
		vars[instr->param1]--;
		break;
	case OP_SET:
		SetVar(instr->param1, instr->param2);
		break;
	case OP_COPYVAR:
		SetVar(instr->param2, GetVarValue(instr->param1));
		break;
	case OP_GETAPPVAR:
		SetVar(instr->param2, GetApplicationPropertyAsInteger(GetStringPtr(instr->param1)));
		break;
	case OP_CALL:
		Call(instr->param1);
		advance = 0;
		break;
	case OP_RETURN:
		curInst = PopCallStack();
		advance = 0;
		break;
	case OP_JMP:
		curInst = instr->param1;
		advance = 0;
		break;
	case OP_IARRAPPEND:
		{
		int value = PopVarStack();
		if(VerifyIntArrayIndex(instr->param1) >= 0)
			intArray[instr->param1].Append(value);
		}
		break;
	case OP_IARRDELETE:
		{
		int index = PopVarStack();
		if(VerifyIntArrayIndex(instr->param1) >= 0)
			intArray[instr->param1].RemoveByIndex(index);
		}
		break;
	case OP_IARRVALUE:
		{
		int index = PopVarStack();
		if(VerifyIntArrayIndex(instr->param1) >= 0)
			SetVar(instr->param3, intArray[instr->param1].GetValueByIndex(index));
		}
		break;
	case OP_IARRCLEAR:
		if(VerifyIntArrayIndex(instr->param1) >= 0)
			intArray[instr->param1].Clear();
		break;
	case OP_IARRSIZE:
		{
			int size = 0;
			if(VerifyIntArrayIndex(instr->param1) >= 0)
				size = intArray[instr->param1].Size();
			SetVar(instr->param2, size);
		}
		break;
	case OP_QUEUEEVENT:
		QueueEvent(GetStringPtr(instr->param1), (unsigned long)instr->param2);
		break;
	case OP_EXECQUEUE:
		//If we performed a jump, we don't want to advance because the instruction index was
		//already changed by the jump.
		if(ExecQueue() == true)
			advance = 0;
		break;
	default:
		RunImplementationCommands(instr->opCode);
		break;
	}

	curInst += advance;
	mProcessingTime += g_PlatformTime.getElapsedMilliseconds() - now;
	return breakScript;
}

void ScriptPlayer :: RunImplementationCommands(int opcode)
{
	switch(opcode)
	{
	case OP_NOP:
		break;
	default:
		PrintMessage("Unidentified op type: %d", def->instr[curInst].opCode);
		break;
	}
}

void ScriptPlayer :: RunUntilWait(void)
{
	while(mExecuting && mActive && (g_ServerTime >= nextFire))
	{
		if(RunSingleInstruction() == true)
			break;
	}
}

void ScriptPlayer :: RunAtSpeed(int maxCommands)
{
	int maxCount = maxCommands;
	if(def->scriptSpeed > maxCount)
		maxCount = def->scriptSpeed;

	int count = 0;
	while(mExecuting == true && mActive && (g_ServerTime >= nextFire))
	{
		if(RunSingleInstruction() == true)
			break;
		count++;
		if(count >= maxCount)
			break;
	}
}

bool ScriptPlayer :: CanRunIdle(void)
{
	return def->CanIdle();
}

bool ScriptPlayer :: JumpToLabel(const char *name)
{
	if(def->UseEventQueue() == true)
	{
		QueueEvent(name, 0);
		return true;
	}
	return PerformJumpRequest(name, ScriptDef::CALLSTYLE_GOTO);
}

bool ScriptPlayer::PerformJumpRequest(const char *name, int callStyle)
{
	int index = def->GetLabelIndex(name);
	if(index >= 0)
	{
		//Make sure we're still running.
		mExecuting = true;
		if(callStyle == ScriptDef::CALLSTYLE_GOTO)
		{
			ResetGoto(def->label[index].instrOffset);
		}
		else
		{
			Call(def->label[index].instrOffset);
		}
		return true;
	}
	else
	{
		if(def->HasFlag(ScriptDef::FLAG_REPORT_LABEL))
			PrintMessage("Label [%s] not found in script [%s]", name, def->scriptName.c_str());

		//Just need to determine if the script should be halted on failure.  If it doesn't use
		//an event queue, it probably needs to be stopped.
		if(def->UseEventQueue() == false)
		{
			g_Log.AddMessageFormat("[REMOVEME] Ending script %s on call to label %s because it doesn't exist.", def->scriptName.c_str(), name);
			EndExecution();
		}
	}
	return false;
}

void ScriptPlayer :: EndExecution(void)
{
	mExecuting = false;
	scriptEventQueue.clear();
	if(def->HasFlag(ScriptDef::FLAG_REPORT_END))
		PrintMessage("Script [%s] has ended", def->scriptName.c_str());
}

void ScriptPlayer :: Call(int targetInstructionIndex)
{
	//When we return, we want to be on the following instruction, not the call.
	PushCallStack(curInst + 1);
	curInst = targetInstructionIndex;
}

void ScriptPlayer :: ResetGoto(int targetInstructionIndex)
{
	callStack.clear();
	varStack.clear();
	curInst = targetInstructionIndex;
	nextFire = g_ServerTime;
}


//Override this function to substitute application-defined variables.
int ScriptPlayer :: GetApplicationPropertyAsInteger(const char *propertyName)
{
	if(propertyName == NULL)
		return 0;
	
	return 0;
}

void ScriptPlayer :: PushVarStack(int value)
{
	if(varStack.size() > MAX_STACK_SIZE)
		PrintMessage("[ERROR] Script error: PushVarStack() stack is full [script: %s]", def->scriptName.c_str());
	else
		varStack.push_back(value);
}

int ScriptPlayer :: PopVarStack(void)
{
	int retval = 0;
	if(varStack.size() == 0)
		PrintMessage("[ERROR] Script error: PopVarStack() stack is empty [script: %s]", def->scriptName.c_str());
	else
	{
		retval = varStack[varStack.size() - 1];
		varStack.pop_back();
	}
	return retval;
}

void ScriptPlayer :: PushCallStack(int value)
{
	if(callStack.size() > MAX_STACK_SIZE)
		PrintMessage("[ERROR] Script error: PushCallStack() stack is full [script: %s]", def->scriptName.c_str());
	else
		callStack.push_back(value);
}

int ScriptPlayer :: PopCallStack(void)
{
	int retval = 0;
	if(callStack.size() == 0)
		PrintMessage("[ERROR] Script error: PopCallStack() stack is empty [script: %s]", def->scriptName.c_str());
	else
	{
		retval = callStack[callStack.size() - 1];
		callStack.pop_back();
	}
	return retval;
}

void ScriptPlayer :: QueueEvent(const char *labelName, unsigned long fireDelay)
{
	if(scriptEventQueue.size() >= MAX_QUEUE_SIZE)
	{
		PrintMessage("[ERROR] Script error: QueueEvent() list is full [script: %s]", def->scriptName.c_str());
		return;
	}

	unsigned long fireTime = g_ServerTime + fireDelay;

	//If a event label is already registered, just update the fire time.
	for(size_t i = 0; i < scriptEventQueue.size(); i++)
	{
		if(scriptEventQueue[i].mLabel.compare(labelName) == 0)
		{
			scriptEventQueue[i].mFireTime = fireTime;
			return;
		}
	}

	//Not found, add a new event.
	scriptEventQueue.push_back(ScriptEvent(labelName, fireTime));
}

bool ScriptPlayer :: ExecQueue(void)
{
	for(size_t i = 0; i < scriptEventQueue.size(); i++)
	{
		if(g_ServerTime >= scriptEventQueue[i].mFireTime)
		{
			PerformJumpRequest(scriptEventQueue[i].mLabel.c_str(), def->queueCallStyle);
			scriptEventQueue.erase(scriptEventQueue.begin() + i);
			return true;
		}
	}
	return false;
}

const char * ScriptPlayer :: GetStringPtr(int index)
{
	static const char *NULL_RESPONSE = "<null>";

	if(index < 0 || index >= static_cast<int>(def->stringList.size()))
	{
		PrintMessage("Script error: string index out of range [%d] for script [%s]", index, def->scriptName.c_str());
		return NULL_RESPONSE;
	}
	return def->stringList[index].c_str();
}

int ScriptPlayer :: GetVarValue(int index)
{
	int retval = 0;
	if(index < 0 || index >= (int)vars.size())
		PrintMessage("Script error: variable index out of range [%d] for script [%s]", index, def->scriptName.c_str());
	else
		retval = vars[index];

	return retval;
}

const char * ScriptPlayer :: GetStringTableEntry(int index)
{
	const char *retval = "";
	if(index < 0 || index >= (int)def->stringList.size())
		PrintMessage("Script error: string index out of range [%d] for script [%s]", index, def->scriptName.c_str());
	else
		retval = def->stringList[index].c_str();

	return retval;
}

int ScriptPlayer :: VerifyIntArrayIndex(int index)
{
	if(index < 0 || index >= (int)def->mIntArray.size())
	{
		PrintMessage("Script error: IntArray index out of range [%d] for script [%s]", index, def->scriptName.c_str());
		return -1;
	}
	return index;
}

void ScriptPlayer :: SetVar(unsigned int index, int value)
{
	if(index >= vars.size())
	{
		PrintMessage("[ERROR] SetVar() index [%d] is outside range [%d]", index, vars.size());
		return;
	}
	vars[index] = value;
}

void ScriptPlayer :: FullReset(void)
{
	if(def == NULL)
	{
		mActive = false;
		mExecuting = false;
		return;
	}

	curInst = 0;
	mActive = true;
	mExecuting = true;
	nextFire = 0;

	varStack.clear();
	callStack.clear();

	vars.clear();
	vars.resize(def->varName.size(), 0);
	for(size_t i = 0; i < vars.size(); i++)
		vars[i] = 0;

	intArray.clear();
	intArray.assign(def->mIntArray.begin(), def->mIntArray.end());
	for(size_t i = 0; i < intArray.size(); i++)
		intArray[i].Clear();

	scriptEventQueue.clear();
}

bool ScriptPlayer :: IsWaiting(void)
{
	return (g_ServerTime > nextFire);
}

BlockData::BlockData()
{
	mLineNumber = 0;
	mInstIndex = 0;
	mInstIndexElse = 0;
	mNestLevel = 0;
	mResolved = false;
	mUseElse = false;
}

ScriptCompiler::ScriptCompiler()
{
	mCurrentNestLevel = 0;
	mLastNestLevel = 0;
	mSourceFile = "<no file>";
	mLineNumber = 0;
	mInlineBeginInstr = 0;
}

void ScriptCompiler::OpenBlock(int lineNumber, int instructionIndex)
{
	BlockData newBlock;
	newBlock.mLineNumber = lineNumber;
	newBlock.mInstIndex = instructionIndex;
	newBlock.mNestLevel = mCurrentNestLevel++;
	newBlock.mResolved = false;

	mBlockData.push_back(newBlock);
}

bool ScriptCompiler::CloseBlock(void)
{
	int size = (int)mBlockData.size();
	for(int i = size - 1; i >= 0; i--)
	{
		if(mBlockData[i].mResolved == true)
			continue;

		mBlockData[i].mResolved = true;
		mCurrentNestLevel--;
		return true;
	}
	return false;
}

BlockData* ScriptCompiler::GetLastUnresolvedBlock(void)
{
	int size = (int)mBlockData.size();
	for(int i = size - 1; i >= 0; i--)
	{
		if(mBlockData[i].mResolved == true)
			continue;

		return &mBlockData[i];
	}
	return NULL;
}

void ScriptCompiler::AddSymbol(const std::string &key, const std::string &value)
{
	mSymbols[key] = value;
}

bool ScriptCompiler::HasSymbol(const std::string &token)
{
	std::map<std::string, std::string>::iterator it;
	it = mSymbols.find(token);
	if(it == mSymbols.end())
		return false;
	return true;
}

void ScriptCompiler::CheckSymbolReplacements(void)
{
	if(mSymbols.size() == 0)
		return;

	for(size_t i = 0; i < mTokens.size(); i++)
	{
		if(HasSymbol(mTokens[i]) == true)
		{
			mTokens[i] = mSymbols[mTokens[i]];
		}
	}
}

bool ScriptCompiler :: ExpectTokens(size_t count, const char *op, const char *desc)
{
	if(mTokens.size() != count)
	{
		PrintMessage("[%s] expects parameters [%s] [%s line %d]", op, desc, mSourceFile, mLineNumber);
		return false;
	}
	return true;
}

void ScriptCompiler :: AddPendingLabelReference(int instructionIndex)
{
	mPendingLabelReference.push_back(instructionIndex);
}


IntArray::IntArray()
{
}

IntArray::IntArray(const char *intArrName)
{
	name = intArrName;
}

void IntArray::Clear(void)
{
	arrayData.clear();
}

int IntArray::Size(void)
{
	return static_cast<int>(arrayData.size());
}

void IntArray::Append(int value)
{
	if(arrayData.size() >= MAX_ARRAY_DATA_SIZE)
	{
		PrintMessage("IntArray [%s] cannot append more than %d elements", name.c_str(), MAX_ARRAY_DATA_SIZE);
		return;
	}
	arrayData.push_back(value);
}

void IntArray::RemoveByIndex(int index)
{
	if(VerifyIndex(index) == true)
	{
		arrayData.erase(arrayData.begin() + index);
	}
}

int IntArray::GetValueByIndex(int index)
{
	if(VerifyIndex(index) == true)
	{
		return arrayData[index];
	}
	return 0;
}

int IntArray::GetIndexByValue(int value)
{
	for(size_t i = 0; i < arrayData.size(); i++)
		if(arrayData[i] == value)
			return (int)i;
	return -1;
}

bool IntArray::VerifyIndex(int index)
{
	if(index < 0 || index >= (int)arrayData.size())
	{
		PrintMessage("IntArray [%s] index [%d] out of range", name.c_str(), index);
		return false;
	}
	return true;
}

void IntArray::DebugPrintContents(void)
{
	char buffer[16];
	std::string str = "[";
	for(size_t i = 0; i < arrayData.size();  i++)
	{
		if(i > 0)
			str += ",";
		sprintf(buffer, "%d", arrayData[i]);
		str += buffer;
	}
	str += "]";
	PrintMessage("IntArray[%s]=%s", name.c_str(), str.c_str());
}

ScriptEvent::ScriptEvent(const char *label, unsigned long fireTime)
{
	mLabel = label;
	mFireTime = fireTime;
}


//namespace ScriptCore
}
