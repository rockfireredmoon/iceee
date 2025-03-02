#include "ScriptCore.h"

#include <squirrel.h>
#include <sqstdaux.h>
#include "../squirrel/sqvm.h" // I have no idea ..... won't compile without, something about forward declarations
#include "sqrat.h"
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
#include "CommonTypes.h"
#include "Components.h"
#include "DirectoryAccess.h"
#include "FileReader.h"
#include "Simulator.h"

#include "Random.h"
#include "Cluster.h"
#include "Util.h"
#include "Config.h"
#include "util/Log.h"

extern unsigned long g_ServerTime;

static string KEYPREFIX_SCRIPT = "Script";

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
	int randint_32bit(int min, int max)
	{
	//	// Generate a 32 bit random number.
	//
	//	/*
	//		Explanation:
	//		rand() doesn't work well for larger numbers.
	//		RAND_MAX is limited to 32767.
	//		There are other quirks, where powers of two seem to generate more even
	//		distributions of numbers.
	//
	//		Since smaller numbers have better distribution, use a sequence
	//		of random numbers and use those to fill the bits of a larger number.
	//	*/
	//
	//	// RAND_MAX (as defined with a value of 0x7fff) is only 15 bits wide.
	//	if(min == max)
	//		return min;
	//	unsigned long rand_build = (rand() << 15) | rand();
	//	//unsigned long rand_build = ((rand() & 0xFF) << 24) | ((rand() & 0xFF) << 16) | ((rand() & 0xFF) << 8) | ((rand() & 0xFF));
	//	return min + (rand_build % (max - min + 1));
		return g_RandomManager.RandInt_32bit(min, max);
	}

	int randmod(int max) {
	//	if(max == 0)
	//		return 0;
	//	// Max is exclusive, e.g, max of 10 would give numbers between 0 and 9
	//	return rand()%max;
		return g_RandomManager.RandMod(max);
	}

	int randmodrng(int min, int max) {
	//	if(min == max)
	//		return min;
	//	// Min is inclusive, max is exclusive, e.g, min of 3, max of 10 would give numbers between 3 and 9
	//	return(rand()%(max-min)+min);
		return g_RandomManager.RandModRng(min, max);
	}

	int randi(int max) {
		// return randint(1, max);
		// TODO remove the above
		return g_RandomManager.RandI(max);
	}

	int randint(int min, int max)
	{
		//Returning <max> is possible, but highly unlikely compared to the individual
		//chances of any other value
		//return ((double) rand() / ((double) RAND_MAX) * (max - min)) + min;

		//This should be fixed to generate <max> as much as others
		//return (int) (((double) rand() / ((double)RAND_MAX + 1) * ((max + 1) - min)) + min);

		// TODO remove the above
		return g_RandomManager.RandInt(min, max);
	}

	double randdbl(double min, double max)
	{
		// return ((double)rand() / ((double)RAND_MAX) * (max - min)) + min;

		// TODO remove the above
		return g_RandomManager.RandDbl(min, max);
	}

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
	ScriptParam::ScriptParam(string v) {
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

	ScriptParam::ScriptParam(Sqrat::Table v) {
		type = OPT_TABLE;
		iValue = 0;
		fValue = 0;
		strValue = "";
		bValue = false;
		tValue = v;
	}


	//
	// Parses script names from things such as AIPackages to allow parameters to
	// be passed to those scripts. This allows much better script reuse, and the
	// possibility of groves customising scripts for the creatures they use
	//

	NutScriptCallStringParser::NutScriptCallStringParser(string callString) {
		int idx = callString.find('(');
		if(idx != -1) {
			mScriptName = callString.substr(0, idx);
			Util::Split(callString.substr(idx + 1, callString.find_last_not_of(')')), ",", mArgs);
			callString.find_last_not_of(')');
		}
		else {
			mScriptName = callString;
		}
	}

	NutDef::NutDef() {
		mQueueEvents = true;
		mFlags = 0;
		mVMSize = 0;
		queueCallStyle = 0;
		queueExternalJumps = false;
		mScriptIdleSpeed = 1;
		mScriptSpeed = 10;
		mLastModified = 0;
		fromCluster = false;
		mScriptContent = "";
	}

	NutDef::~NutDef() {

	}

	bool NutDef::EntityKeys(AbstractEntityReader *reader) {
		reader->Key(KEYPREFIX_SCRIPT, mSourceFile);
		return true;
	}

	bool NutDef::IsFromCluster() {
		return fromCluster;
	}

	bool NutDef::ReadEntity(AbstractEntityReader *reader) {
		fromCluster = true;
		if (!reader->Exists())
			return false;

		scriptName = reader->Value("Name");
		mLastModified = reader->ValueULong("LastModified");
		mScriptContent = reader->Value("Content");

		return true;
	}

	bool NutDef::WriteEntity(AbstractEntityWriter *writer) {
		writer->Key(KEYPREFIX_SCRIPT, mSourceFile);
		writer->Value("Name", scriptName);
		writer->Value("LastModified", mLastModified);
		writer->Value("Content", mScriptContent);
		return true;
	}

	void NutDef::CheckReload() {
		if(!fromCluster) {
			if(GetLastModified() != Platform::GetLastModified(mSourceFile)) {
				Reload();
			}
		}
	}

	void NutDef::Reload() {
		if(!fromCluster) {
			LoadFromLocalFile(mSourceFile);
		}
	}

	unsigned long NutDef::GetLastModified() {
		return mLastModified;
	}

	void NutDef::SetLastModified(unsigned long lastModified) {
		mLastModified = lastModified;
		if(fromCluster) {
			g_Logs.script->info("Saving script %v to cluster", mSourceFile);
			g_ClusterManager.WriteEntity(this, false);
		}
		else
			Platform::SetLastModified(mSourceFile, mLastModified);
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

	fs::path NutDef :: GetBytecodeLocation() {
		auto base = mSourceFile.stem();
		auto dir = mSourceFile.parent_path();
		if(fromCluster) {
			return g_Config.ResolveTmpDataPath() / dir / ( base.string() + ".cnut" );
		}
		else {
			return dir / ( base.string() + ".cnut" );
		}
	}

	void NutDef::LoadFromLocalFile(const fs::path &sourceFile) {
		g_Logs.script->info("Initializing Squirrel script from local file '%v'", sourceFile.string());
		mSourceFile = sourceFile;
		scriptName = mSourceFile.stem();
		mLastModified = Platform::GetLastModified(mSourceFile);
		fromCluster = false;

		FileReader lfr;
		if (fs::exists(mSourceFile)) {
			if (lfr.OpenText(mSourceFile) != Err_OK) {
				/* Error */
				mScriptContent = "#!/bin/sq\n#\n# Failed to load script content.\n";
				return;
			}

			mScriptContent = "";
			while (lfr.FileOpen() == true) {
				lfr.ReadLine();
				mScriptContent.append(lfr.DataBuffer);
				mScriptContent.append("\n");
			}
		}
		else {
			mScriptContent = "#!/bin/sq\n";
		}
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
		mResult = Result::WAITING;
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
	// 'Never' condition implementation. Prevents from ever running (until removed?)
	//

	bool NeverCondition::CheckCondition() {
		return false;
	}

	//
	// 'Pause' condition implementation. Is true when mPaused becomes fale
	//

	PauseCondition::PauseCondition()
	{
		mPaused = true;
	}
	PauseCondition::~PauseCondition() {}

	bool PauseCondition::CheckCondition() {
		return !mPaused;
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
		if(sq_getvmstate(mNut->vm) != SQ_VMSTATE_SUSPENDED) {
			g_Logs.script->debug("Resume event fired, but VM was already awake for script %v", mNut->def->scriptName);
			return true;
		}
		else {
			g_Logs.script->debug("Waking VM for script %v. Stack has %v", mNut->def->scriptName, sq_gettop(mNut->vm));
			sq_pushbool(mNut->vm, false);
			if(SQ_SUCCEEDED(sq_wakeupvm(mNut->vm,true,false,false, false))) {
				mNut->mSleeping = 0;
				sq_poptop(mNut->vm);
				if(sq_getvmstate(mNut->vm) == SQ_VMSTATE_IDLE) {
					sq_settop(mNut->vm, mNut->mSuspendTop); //pop roottable
				}
				return true;
			}
			else {
				const SQChar *err;
				sq_getlasterror(mNut->vm);
				if(SQ_SUCCEEDED(sq_getstring(mNut->vm,-1,&err))) {
					g_Logs.script->debug("Wakeup failed for script %v, state now %v. %v", mNut->def->scriptName, sq_getvmstate(mNut->vm), err);
				}
				else {
					g_Logs.script->debug("Wakeup failed for script %v, state now %v. No error code could be determined.", mNut->def->scriptName, sq_getvmstate(mNut->vm));

				}
			}
			return false;
		}
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
		mNut->HaltVM();
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

	bool SquirrelFunctionCallback::Execute() {
		bool wasRunning = mNut->mRunning;
		mNut->mRunning = true;
		bool v ;
		try {
			Sqrat::SharedPtr<bool> ptr = mFunction.Evaluate<bool>();
			v = ptr.Get() == NULL || ptr.Get();
		}
		catch(Sqrat::Exception &e) {
			g_Logs.script->error("Exception while execute script function.");
		}
		mNut->mRunning = wasRunning;
		return v;
	}

	//
	// Run a named function.
	//
	RunFunctionCallback::RunFunctionCallback(NutPlayer *nut, string functionName) {
		mNut = nut;
		mCaller = mNut->mCaller;
		mFunctionName = functionName;
	}
	RunFunctionCallback::RunFunctionCallback(NutPlayer *nut, string functionName, vector<ScriptParam> args) {
		mNut = nut;
		mCaller = mNut->mCaller;
		mFunctionName = functionName;
		mArgs = args;
	}


	RunFunctionCallback::~RunFunctionCallback() {
	}

	bool RunFunctionCallback::Execute()
	{
		int was = mNut->mCaller;
		mNut->mCaller = mCaller;
		mNut->RunFunction(mFunctionName, mArgs, false);
		mNut->mCaller = was;
		return true;
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
		mId = -1;
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
		mSleeping = 0;
		mClear = false;
		vm = NULL;
		def = NULL;
		mActive = false;
		mExecutingEvent = NULL;
		mProcessingTime = 0;
		mGCCounter = 0;
		mMaybeGC = 0;
		mForceGC = 0;
		mGCTime = 0;
		mCalls = 0;
		mRunning = false;
		mHalting = false;
		mHalted = false;
		mNextId = 1;
		mCaller = 0;
		mSuspendTop = 0;
		mPreventReentry = false;
		mPauseEvent = -1;
	}

	NutPlayer::~NutPlayer() {
		ClearQueue();
		if(mHalted) {
			if(g_Logs.script->Enabled(el::Level::Trace)) {
				if(def == NULL)
					g_Logs.script->trace("Closing virtual machine for uninitialized Squirrel Script");
				else
					g_Logs.script->trace("Closing virtual machine for %v", def->mSourceFile.c_str());
			}
			sq_close(vm);
		}
	}

	string NutPlayer::GetStatus() {
		if(mHalting) {
			return "Halting";
		}
		else if(mActive) {
			if(mExecutingEvent != NULL)
				return "Executing";
			else
				return "Active";
		}
		else {
			return "Inactive";
		}
	}

	int NutPlayer::GC() {
		return sq_collectgarbage(vm);
	}

	int NutPlayer::ClearQueue() {
		int total = 0;
		if(mHalting) {
			g_Logs.script->warn("Attempt to clear queue while halting in %v", def->scriptName.c_str());
		}
		else if(mExecutingEvent != NULL) {
			mClear = true;
		}
		else {
			WakeVM("NutPlayer::ClearQueue");
			vector<ScriptCore::NutScriptEvent*>::iterator it;
			total += mQueue.size() + mQueueAdd.size() + mQueueInsert.size();
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
		return total;
	}

	void NutPlayer::Initialize(NutDef *defPtr, string &errors) {
		unsigned long started = g_PlatformTime.getMilliseconds();

		def = defPtr;
		def->CheckReload();

		vm = sq_open(g_Config.SquirrelVMStackSize);

		// Register functions needed in scripts
		RegisterFunctions();
		sq_pushroottable(vm);
		sqstd_register_stringlib(vm);
		sq_pop(vm,1);

		sqstd_seterrorhandlers(vm); //registers the default error handlers
		sq_setprintfunc(vm, PrintFunc, Errorfunc); //sets the print function

		if (g_Logs.script->Enabled(el::Level::Debug)) {
			g_Logs.script->debug("Processing Squirrel script '%v'", def->mSourceFile);
		}


		/* Look for the compiled NUT file (.cnut). If it exists, test if the modification
		 * time is the same as the .nut file. If it isn't (or the .cnut doesn't exist at all),
		 * then compile AND write the bytecode
		 */
		auto cnut = def->GetBytecodeLocation();
		unsigned long cnutMod = Platform::GetLastModified(cnut);

		Sqrat::Script script(vm);
		bool compiled;

		if(cnutMod != def->GetLastModified()) {
			g_Logs.script->info("Recompiling Squirrel script '%v' (%v)", def->mSourceFile.string(), def->scriptName);
			if(g_Logs.script->Enabled(el::Level::Trace)) {
				g_Logs.script->trace("-------------------------------\n%v-------------------------------", def->mScriptContent);
			}
			compiled = script.CompileString(def->mScriptContent, errors, def->scriptName);
		}
		else {
			if (g_Logs.script->Enabled(el::Level::Debug)) {
				g_Logs.script->debug("Loading existing Squirrel script bytecode for '%v'", cnut.string());
			}
			compiled = script.CompileFile(_SC(cnut.c_str()), errors);
		}

		if (!compiled /*Sqrat::Error::Occurred(vm) */) {
			errors.append(Sqrat::Error::Message(vm).c_str());
			g_Logs.script->error("Squirrel script %v failed to compile. %v", def->mSourceFile.string(), Sqrat::Error::Message(vm).c_str());
		}
		else {
			if(cnutMod != def->GetLastModified()) {
				g_Logs.script->info("Writing Squirrel script bytecode for '%v' to '%v' (in %v)", def->mSourceFile.string(), cnut.string(), cnut.parent_path().string());
				fs::create_directories(cnut.parent_path());
				try {
					script.WriteCompiledFile(cnut);
				}
				catch(int e) {
					g_Logs.script->error("Failed to write Squirrel script bytecode for '%v' to '%v'. Err %v", def->mSourceFile, cnut, e);
				}
				Platform::SetLastModified(cnut,  def->GetLastModified());
			}
			mActive = true;
			mRunning = true;
			script.Run();
			mRunning = false;
			if (Sqrat::Error::Occurred(vm)) {
				mActive = false;
				errors.append(Sqrat::Error::Message(vm).c_str());
 				g_Logs.script->error("Squirrel script %v failed to run. %v", def->mSourceFile.string(), Sqrat::Error::Message(vm).c_str());
			}

			// The script might have provided an info table
			Sqrat::Object infoObject = Sqrat::RootTable(vm).GetSlot(_SC("info"));
			if(!infoObject.IsNull()) {
				Sqrat::Object author = infoObject.GetSlot("author");
				if(!author.IsNull()) {
					def->mAuthor = author.Cast<string>();
				}
				Sqrat::Object description = infoObject.GetSlot("description");
				if(!description.IsNull()) {
					def->mAuthor = description.Cast<string>();
				}
				Sqrat::Object queueEvents = infoObject.GetSlot("queue_events");
				if(!queueEvents.IsNull()) {
					def->mQueueEvents = description.Cast<bool>();
				}
				Sqrat::Object idleSpeed = infoObject.GetSlot("idle_speed");
				if(!idleSpeed.IsNull()) {
					def->mScriptIdleSpeed = idleSpeed.Cast<int>();
				}
				Sqrat::Object vmSize = infoObject.GetSlot("vm_size");
				if(!vmSize.IsNull()) {
					def->mVMSize = vmSize.Cast<int>();
				}
				else
					def->mVMSize = 0;
				Sqrat::Object speed = infoObject.GetSlot("speed");
				if(!speed.IsNull()) {
					def->mScriptSpeed = Util::ClipInt(speed.Cast<int>(), 1, 100);
				}

				if(def->mVMSize != 0 && def->mVMSize != g_Config.SquirrelVMStackSize) {
					vm->_stack.resize(def->mVMSize);
					g_Logs.script->info("Squirrel script %v has requested a different VM size (%v) to the default (%v), reinitializing. ", def->mSourceFile, def->mVMSize, g_Config.SquirrelVMStackSize);
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
		return JumpToLabel(name, vector<ScriptParam>());
	}

	bool NutPlayer::JumpToLabel(const char *name, vector<ScriptParam> parms)
	{
		return JumpToLabel(name, parms, def->mQueueEvents);
	}

	bool NutPlayer::JumpToLabel(const char *name, vector<ScriptParam> parms, bool queue)
	{
		if(queue) {
			g_Logs.script->debug("Queue Jump to label %v in %v", name, def->scriptName.c_str());
			QueueAdd(new NutScriptEvent(new TimeCondition(0), new RunFunctionCallback(this, name, parms)));
			return true;
		}
		else {
			g_Logs.script->debug("Run Jump to label %v in %v", name, def->scriptName.c_str());
			return RunFunction(name, parms, true);
		}
	}

	long NutPlayer::Exec(Sqrat::Function function) {
		if(def == NULL) {
			g_Logs.script->error("Exec when there is no script def!");
			return -1;
		}
		else {
			unsigned long spd = g_Config.SquirrelQueueSpeed / def->mScriptSpeed;
			if(g_Logs.script->Enabled(el::Level::Trace)) {
				g_Logs.script->trace("Queueing call (exec) in %v in %v", def->scriptName.c_str(), spd);
			}
			return QueueAdd(new ScriptCore::NutScriptEvent(
						new ScriptCore::TimeCondition(spd),
						new ScriptCore::SquirrelFunctionCallback(this, function)));
		}
	}

	bool NutPlayer::Cancel(long id) {
		NutScriptEvent* nse = GetEvent(id);
		if(nse == NULL)
			return false;
		QueueRemove(nse);
		return true;
	}

	long NutPlayer::Queue(Sqrat::Function function, int fireDelay) {
		if(def == NULL) {
			g_Logs.script->error("Exec when there is no script def!");
			return -1;
		}
		else {
			if(g_Logs.script->Enabled(el::Level::Trace)) {
				g_Logs.script->trace("Queueing call in %v in %v", def->scriptName.c_str(), fireDelay);
			}
			return QueueAdd(new ScriptCore::NutScriptEvent(
						new ScriptCore::TimeCondition(fireDelay),
						new ScriptCore::SquirrelFunctionCallback(this, function)));
		}
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
		vector3Class.Func("dist", &Squirrel::Vector3I::Distance);
		vector3Class.Func("dist_plane", &Squirrel::Vector3I::DistanceOnPlane);

		// Vector3F Object, floating point X/Y/Z location
		Sqrat::Class<Squirrel::Vector3> vector3FClass(vm, "Vector3", true);
		vector3FClass.Ctor<float,float, float>();
		vector3FClass.Ctor();
		Sqrat::RootTable(vm).Bind(_SC("Vector3"), vector3FClass);
		vector3FClass.Var("x", &Squirrel::Vector3::mX);
		vector3FClass.Var("y", &Squirrel::Vector3::mY);
		vector3FClass.Var("z", &Squirrel::Vector3::mZ);
		vector3FClass.Func("dist", &Squirrel::Vector3::Distance);
		vector3FClass.Func("dist_plane", &Squirrel::Vector3::DistanceOnPlane);

		clazz->Func(_SC("exec"), &NutPlayer::Exec);
		clazz->Func(_SC("cancel"), &NutPlayer::Cancel);
		clazz->Func(_SC("queue"), &NutPlayer::Queue);
		clazz->Func(_SC("clear_queue"), &NutPlayer::QueueClear);
		clazz->Func(_SC("broadcast"), &NutPlayer::Broadcast);
		clazz->Func(_SC("halt"), &NutPlayer::Halt);
		clazz->Func(_SC("get_server_time"), &NutPlayer::GetServerTime);
		clazz->Func(_SC("get_caller"), &NutPlayer::GetCaller);

		clazz->SquirrelFunc(_SC("sleep"), &Sleep);

		Sqrat::RootTable(vm).Func("randmodrng", &randmodrng);
		Sqrat::RootTable(vm).Func("randmod", &randmod);
		Sqrat::RootTable(vm).Func("randint", &randint);
		Sqrat::RootTable(vm).Func("randdbl", &randdbl);
		Sqrat::RootTable(vm).Func("rand", &randi);
		Sqrat::RootTable(vm).Func("array_contains", &ArrayContains);
		Sqrat::RootTable(vm).Func("elapsed_ms", &ElapsedMilliseconds);
		Sqrat::RootTable(vm).Func("abs_sec", &AbsoluteSeconds);
		Sqrat::RootTable(vm).Func("ms", &Milliseconds);

		// Add in the script arguments
		Sqrat::RootTable(vm).SetValue(_SC("__argc"), SQInteger(mArgs.size()));
		Sqrat::Array arr(vm, mArgs.size());
		int idx = 0;
		for(vector<string>::iterator it = mArgs.begin(); it != mArgs.end(); ++it)
			arr.SetValue(idx++, _SC(*it));
		Sqrat::RootTable(vm).SetValue(_SC("__argv"), arr);
	}

	int NutPlayer::GetCaller() {
		return mCaller;
	}

	void NutPlayer::SetCaller(int caller) {
		mCaller = caller;
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

		// TODO not sure about this ...
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

	long NutPlayer::ElapsedMilliseconds() {
		return g_PlatformTime.getElapsedMilliseconds();
	}

	long NutPlayer::AbsoluteSeconds() {
		return g_PlatformTime.getAbsoluteSeconds();
	}

	long NutPlayer::Milliseconds() {
		return g_PlatformTime.getMilliseconds();
	}

	int NutPlayer::Rand(int max) {
		return g_RandomManager.RandInt(1, max);
	}

	int NutPlayer::RandInt(int min, int max) {
		return g_RandomManager.RandInt(min, max);
	}

	int NutPlayer::RandMod(int max) {
		return g_RandomManager.RandMod(max);
	}

	int NutPlayer::RandModRng(int min, int max) {
		return g_RandomManager.RandModRng(min, max);
	}

	int NutPlayer::RandDbl(double min, double max) {
		return g_RandomManager.RandDbl(min, max);
	}

	void NutPlayer::Halt(void) {
		HaltEvent(false);
	}

	void NutPlayer::HaltEvent(bool immediate) {
    	HaltCallback *cb = new HaltCallback(this);
    	NutScriptEvent *nse = new NutScriptEvent(new TimeCondition (0), cb);
    	nse->mRunWhenSuspended = true;
    	if(immediate) {
    		QueueInsert(nse);
    	}
    	else {
    		QueueAdd(nse);
    	}
		mHalting = true;
	}

	void NutPlayer :: HaltedDerived(void) { }

	void NutPlayer :: HaltDerivedExecution(void) { }

	void NutPlayer :: HaltExecution(void)
	{
		if(mHalting) {
			g_Logs.script->warn("Attempt to halt halting script %v.", def->mSourceFile.c_str());
			return;
		}
		if(!mActive) {
			g_Logs.script->info("Attempt to halt inactive script %v.", def->mSourceFile.c_str());
			return;
		}

		if(mRunning) {
			g_Logs.script->debug("Queueing halt of VM [%v]", def->scriptName.c_str());

			/* If we reached here via a script function, we already executing and don't want to close the VM.
			 * In this case the halt is queued instance
			 */

			ScriptCore::NutScriptEvent* nse;
			for(vector<ScriptCore::NutScriptEvent*>::iterator it = mQueue.begin(); it != mQueue.end(); ++it) {
				nse = *it;
				if(nse->mCallback != NULL) {
					if(dynamic_cast<HaltCallback*>(nse->mCallback)) {
						nse->Cancel();
					}
				}
			}

			ClearQueue();
			HaltEvent(true);
			return;
		}
		mHalting = true;
		HaltVM();
	}

	void NutPlayer::HaltVM() {
		if(mActive) {
			g_Logs.script->debug("Halting VM [%v] for ", def->scriptName.c_str());
			vector<ScriptParam> v;
//			g_Log.AddMessageFormat("[REMOVEME] Halting VM");
			RunFunction("on_halt", v, true);
			HaltDerivedExecution();
			mActive = false;
			mExecutingEvent = NULL;
			mHalting = false;
			ClearQueue();
			mHalted = true;
//			g_Log.AddMessageFormat("[REMOVEME] Halted VM for %s", def->scriptName.c_str());
			if(def->HasFlag(NutDef::FLAG_REPORT_END))
				g_Logs.script->info("Script [%v] has ended", def->scriptName.c_str());
			else
				g_Logs.script->debug("Script [%v] has ended", def->scriptName.c_str());
//			else
//				g_Log.AddMessageFormat("[REMOVEME] VM Halted!");
			HaltedDerived();
		}
		else
			g_Logs.script->debug("Request to halt an inactive VM [%v]", def->scriptName.c_str());
	}

	void NutPlayer::FinaliseExecution(string name, int top) {
		const SQInteger state = sq_getvmstate(vm);
		if( state == SQ_VMSTATE_IDLE ) {
			sq_settop(vm,top);
		}
		else {
			g_Logs.script->debug("Script engine for %v is not idle, so not resetting stack. State is %v", name.c_str(), state);
		}
	}

	string NutPlayer::RunFunctionWithStringReturn(string name, vector<ScriptParam> parms, bool time, string defaultIfNoFunction) {
		if(!mActive) {
			if(g_Logs.script->Enabled(el::Level::Trace)) {
				g_Logs.script->trace("Attempt to run function on inactive script %v.", name.c_str());
			}
			return defaultIfNoFunction;
		}

		g_Logs.script->debug("Run function %v", name.c_str());

		unsigned long now = g_PlatformTime.getMilliseconds();
		mRunning = true;
		WakeVM(name);
		SQInteger top = sq_gettop(vm);
		string sval = "";
		try {
			if(DoRunFunction(name, parms, time, true)) {
				const SQChar* val;
				sq_getstring(vm,-1,&val);
				sval = val;
			}
			else
				sval = defaultIfNoFunction;
		}
		catch(int e) {
			g_Logs.script->error("Exception when running function %v, failed with %v", name.c_str(), e);
		}
		FinaliseExecution(name, top);

		if(time) {
			mCalls++;
			mGCCounter++;
			mProcessingTime += g_PlatformTime.getMilliseconds() - now;
		}
		mRunning = false;
		return sval;
	}

	bool NutPlayer::RunFunctionWithBoolReturn(string name, vector<ScriptParam> parms, bool time, bool defaultIfNoFunction) {
		if(!mActive) {
			if(g_Logs.server->Enabled(el::Level::Trace)) {
				g_Logs.script->trace("Attempt to run function on inactive script %s.", name.c_str());
			}
			return defaultIfNoFunction;
		}
		unsigned long now = g_PlatformTime.getMilliseconds();
		mRunning = true;
		WakeVM(name);
		SQInteger top = sq_gettop(vm);
		SQBool val = SQFalse;
		try {
			if(DoRunFunction(name, parms, time, true)) {
				sq_getbool(vm,-1,&val);
			}
			else {
				val = defaultIfNoFunction;
			}
		}
		catch(int e) {
			g_Logs.script->error("Exception when running function %s, failed with %d", name.c_str(), e);
		}
		FinaliseExecution(name, top);
		if(time) {
			mCalls++;
			mGCCounter++;
			mProcessingTime += g_PlatformTime.getMilliseconds() - now;
		}
		mRunning = false;
		return val;
	}

	bool NutPlayer::RunFunction(string name, vector<ScriptParam> parms, bool time) {

//		g_Log.AddMessageFormat("[REMOVEME] Run function %s in %s", name.c_str(), def->mSourceFile.c_str());

		Util::ReplaceAll(name, "-", "_MINUS_");

//		g_Log.AddMessageFormat("[REMOVEME] Running function %s in %s (active: %s).", name.c_str(), def->mSourceFile.c_str(), mActive ? "yes" : "no");

		if(!mActive) {
			if(g_Logs.script->Enabled(el::Level::Trace)) {
				g_Logs.script->trace("Attempt to run function on inactive script %v.", name.c_str());
			}
			return false;
		}
		unsigned long now = g_PlatformTime.getMilliseconds();

		mRunning = true;

		WakeVM(name);
		SQInteger top = sq_gettop(vm);
		bool ok;
		try {
			ok = DoRunFunction(name, parms, time, false);
		}
		catch(int e) {
			g_Logs.script->error("Exception when running function %s, failed with %d", name.c_str(), e);
		}
		FinaliseExecution(name, top);

		if(time) {
			mCalls++;
			mGCCounter++;
			mProcessingTime += g_PlatformTime.getMilliseconds() - now;
		}

		mRunning = false;
		return ok;
	}

	bool NutPlayer :: WakeVM(string name) {

		// Wake the VM up if it is suspend so the onFinish can be run
		if(sq_getvmstate(vm) == SQ_VMSTATE_SUSPENDED) {
			g_Logs.script->debug("Interrupt VM to run %v.", name.c_str());
			mSleeping = 0;
			sq_pushbool(vm, true); // return true. scripts test this and leave their loop if appropriate on interrupt
			if(SQ_SUCCEEDED(sq_wakeupvm(vm,true,false,false, false))) {
				sq_pop(vm,1); //pop retval
				if(sq_getvmstate(vm) == SQ_VMSTATE_IDLE) {
					sq_settop(vm, mSuspendTop); //pop roottable
				}
			}
			else {
				const SQChar *err;
				sq_getlasterror(vm);
				if(SQ_SUCCEEDED(sq_getstring(vm,-1,&err))) {
					g_Logs.script->debug("Interrupt failed for script %v, state now %v. %v", def->scriptName.c_str(), sq_getvmstate(vm), err);
				}
				else {
					g_Logs.script->debug("Interrupt failed for script %v, state now %v. No error code could be determined.", def->scriptName.c_str(), sq_getvmstate(vm));
				}
			}
			return true;
		}
		else {
			if(g_Logs.script->Enabled(el::Level::Trace)) {
				g_Logs.script->trace("Request to wake an already awake VM to run %v.", name.c_str());
			}
		}
		return false;
	}

	bool NutPlayer::DoRunFunction(string name, vector<ScriptParam> parms, bool time, bool retVal) {
		if(g_Logs.server->Enabled(el::Level::Trace)) {
			g_Logs.script->debug("Run function %v in %v", name.c_str(), def->mSourceFile.c_str());
		}
		sq_pushroottable(vm);
		sq_pushstring(vm,_SC(name.c_str()),-1);
		if(SQ_SUCCEEDED(sq_get(vm,-2))) {
			sq_pushroottable(vm);
			vector<ScriptCore::ScriptParam>::iterator it;
			for(it = parms.begin(); it != parms.end(); ++it)
			{
				switch(it->type) {
				case OPT_INT:
					sq_pushinteger(vm,it->iValue);
					break;
				case OPT_BOOL:
					sq_pushbool(vm, it->bValue);
					break;
				case OPT_FLOAT:
					sq_pushfloat(vm,it->fValue);
					break;
				case OPT_STR:
					sq_pushstring(vm,_SC(it->strValue.c_str()), it->strValue.size());
					break;
				case OPT_TABLE:
					vm->Push(it->tValue.GetObject());
					break;
				default:
					g_Logs.script->error("Unsupported parameter type for Squirrel script. %d", it->type);
					break;
				}
			}
			sq_call(vm,parms.size() + 1,retVal ? SQTrue : SQFalse, Sqrat::ErrorHandling::IsEnabled()); //calls the function
			return true;
		}
		return false;
	}

	bool NutPlayer :: ExecEvent(NutScriptEvent *nse, unsigned int index)
	{
		unsigned long now = g_PlatformTime.getMilliseconds();
		NutCallback *cb = nse->mCallback;

		bool res = true;
		if(!nse->mCancelled) {
			try {
				res = cb->Execute();
			}
			catch(int e) {
				g_Logs.script->error("Callback failed. %v", e);
			}
		}

		if(mActive && !mHalting && mQueue.size() > 0 && index < mQueue.size()) {
			/*
			 * If the VM wasn't suspended while handling this event, and the
			 * event returned false, then we requeue this event for retry
			 */
			if(!res && sq_getvmstate(vm) != SQ_VMSTATE_SUSPENDED) {
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
		if(mExecutingEvent != NULL) {
			g_Logs.script->warn("Already executing. Something tried to executing the queue while it was already executing.");
			return true;
		}
		bool ok = false;
		for(size_t i = 0; !mClear && mActive && i < mQueue.size(); i++)
		{
			NutScriptEvent *nse = mQueue[i];
			mExecutingEvent = nse;

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
			// All done
			mExecutingEvent = NULL;
		}

		mExecutingEvent = NULL;

		if(mClear) {
			ClearQueue();
			mClear = false;
		}
		else {

			// Apply any changes to the queue made while running the queued event
			for(size_t i = 0; i < mQueueRemove.size(); i++)	{
				NutScriptEvent *nse = mQueueRemove[i];
				mQueue.erase(remove(mQueue.begin(), mQueue.end(), nse), mQueue.end());
				mQueueAdd.erase(remove(mQueueAdd.begin(), mQueueAdd.end(), nse), mQueueAdd.end());
				mQueueInsert.erase(remove(mQueueInsert.begin(), mQueueInsert.end(), nse), mQueueInsert.end());
				delete nse;
			}
			mQueue.insert(mQueue.end(), mQueueAdd.begin(), mQueueAdd.end());
			mQueue.insert(mQueue.begin(), mQueueInsert.begin(), mQueueInsert.end());

			mQueueAdd.clear();
			mQueueInsert.clear();
			mQueueRemove.clear();
		}

		// If nothing was executed, and the GC counter has been reached
		if(mActive && !ok && mGCCounter > g_Config.SquirrelGCCallCount) {
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
				g_Logs.script->info("GC performed because callcount reached %v and returned %v objects taking %v ms", mGCCounter, took, objs);
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

	long NutPlayer::QueueInsert(NutScriptEvent *evt)
	{
		if(mHalting) {
			g_Logs.script->warn("Squirrel script event when halting");
			return -1;
		}
		if(!mActive) {
			g_Logs.script->warn("Squirrel script event when not active");
			return-1;
		}
		evt->mId = mNextId++;
		if(mExecutingEvent != NULL)
		{
			if(mQueueInsert.size() >= MAX_QUEUE_SIZE)
			{
				g_Logs.script->error("Squirrel Script error: Deferred QueueEvent() list is full %v of %v", mQueueInsert.size(), MAX_QUEUE_SIZE);
				return -1;
			}
			mQueueInsert.insert(mQueueInsert.begin(), evt);
		} else
		{

			if(mQueue.size() >= MAX_QUEUE_SIZE)
			{
				g_Logs.script->error("Squirrel Script error: QueueEvent() list is full [script: %v]", def->scriptName.c_str());
				return -1;
			}
			mQueue.insert(mQueue.begin(), evt);
		}
		return evt->mId;
	}

	void NutPlayer::QueueClear()
	{
		if(mExecutingEvent != NULL)
		{
			for(size_t i = 0; i < mQueue.size(); i++)	{
				NutScriptEvent *nse = mQueue[i];
				if(nse != mExecutingEvent)
					mQueueRemove.push_back(nse);
			}
		}
		else
		{
			ClearQueue();
		}
	}

	NutScriptEvent* NutPlayer::GetEvent(unsigned long id) {
		// Look for the event in all queues
		vector<ScriptCore::NutScriptEvent*>::iterator it;
		for(it = mQueue.begin(); it != mQueue.end(); ++it) {
			if((*it)->mId == id) {
				return *it;
			}
		}
		return NULL;
	}

	void NutPlayer::QueueRemove(NutScriptEvent *evt)
	{
		if(mExecutingEvent != NULL)
		{
			if(evt != mExecutingEvent)
				mQueueRemove.insert(mQueueRemove.begin(), evt);
		}
		else
		{
			if(find(mQueue.begin(), mQueue.end(), evt) != mQueue.end())
				mQueue.erase(find(mQueue.begin(), mQueue.end(), evt));

			if(find(mQueueAdd.begin(), mQueueAdd.end(), evt) != mQueueAdd.end())
				mQueueAdd.erase(find(mQueueAdd.begin(), mQueueAdd.end(), evt));

			if(find(mQueueInsert.begin(), mQueueInsert.end(), evt) != mQueueInsert.end())
				mQueueAdd.erase(find(mQueueAdd.begin(), mQueueAdd.end(), evt));

			if(find(mQueueRemove.begin(), mQueueRemove.end(), evt) != mQueueRemove.end())
				mQueueAdd.erase(find(mQueueAdd.begin(), mQueueAdd.end(), evt));
		}
	}

	long NutPlayer::QueueAdd(NutScriptEvent *evt)
	{
		if(mHalting) {
			g_Logs.script->warn("Squirrel script event when halting");
			return -1;
		}
		if(!mActive) {
			g_Logs.script->warn("Squirrel script event when not active");
			return -1;
		}
		evt->mId = mNextId++;

		if(mExecutingEvent != NULL)
		{
			if(mExecutingEvent != evt) {
				if(mQueueAdd.size() >= MAX_QUEUE_SIZE)
				{
					g_Logs.script->error("Squirrel script error: Deferred QueueEvent() list is full %v of %v", mQueueAdd.size(), MAX_QUEUE_SIZE);
					return -1;
				}
				mQueueAdd.push_back(evt);
			}
		} else
		{

			if(mQueue.size() >= MAX_QUEUE_SIZE)
			{
				g_Logs.script->error("Squirrel script error: QueueEvent() list is full [script: %v]", def->scriptName.c_str());
				return -1;
			}
			mQueue.push_back(evt);
		}
		return evt->mId;
	}

	void NutPlayer::Broadcast(const char *message)
	{
		g_SimulatorManager.BroadcastMessage(message);
	}

	bool NutPlayer::Resume()
	{
		if(!IsPaused()) {
			g_Logs.script->warn("Attempt to resume not paused script");
		}
		else {
			NutScriptEvent *nse = NULL;
			if(mPauseEvent != -1)
				nse = GetEvent(mPauseEvent);
			if(nse != NULL) {
				((PauseCondition*)nse->mCondition)->mPaused = false;
				mPauseEvent = -1;
				return true;
			}
		}
		return false;
	}

	bool NutPlayer::IsPaused()
	{
		NutScriptEvent *nse = NULL;
		if(mPauseEvent != -1)
			nse = GetEvent(mPauseEvent);
		return nse != NULL;
	}

	bool NutPlayer::Pause()
	{
		if(IsPaused()) {
			g_Logs.script->warn("Attempt to pause already paused script");
			return false;
		}
		if(mSleeping > 0) {
			g_Logs.script->warn("Attempt to pause already sleeping script");
			return false;
		}

		ResumeCallback *cb = new ResumeCallback(this);
		PauseCondition *mPause = new PauseCondition ();
		NutScriptEvent *nse = new NutScriptEvent(mPause, cb);
		nse->mRunWhenSuspended = true;
		mPauseEvent = QueueAdd(nse);
		SQInteger ret = sq_suspendvm(vm);
		mSuspendTop = sq_gettop(vm);
		return ret;
	}


	SQInteger NutPlayer::Sleep(HSQUIRRELVM v)
	{
		if (sq_gettop(v) == 2) {
			Sqrat::Var<NutPlayer&> left(v, 1);
			if (!Sqrat::Error::Occurred(v)) {
	        	if((&left.value)->mPauseEvent != -1)
					return sq_throwerror(v, _SC("already paused"));
				if((&left.value)->mSleeping > 0)
					return sq_throwerror(v, _SC("already sleeping"));
				Sqrat::Var<unsigned long> right(v, 2);
				vector<int> vv;
				(&left.value)->mSleeping = right.value;
				ResumeCallback *cb = new ResumeCallback(&left.value);
				NutScriptEvent *nse = new NutScriptEvent(new TimeCondition (right.value), cb);
				nse->mRunWhenSuspended = true;
				left.value.QueueAdd(nse);
				sq_poptop(v);
	        	sq_poptop(v);
	        	g_Logs.script->debug("Sleeping VM %v for %v. Stack at this point is %v", (&left.value)->def->mSourceFile.c_str(), right.value, sq_gettop(v));
	            SQInteger ret = sq_suspendvm(v);
				left.value.mSuspendTop = sq_gettop(v);
	            return ret;
			}
			return sq_throwerror(v, Sqrat::Error::Message(v).c_str());
		}
		return sq_throwerror(v, _SC("wrong number of parameters"));
	}

	bool NutPlayer::ArrayContains(Sqrat::Array arr, int value) {
		for(int i = 0 ; i < arr.GetSize(); i++) {
			Sqrat::SharedPtr<int> a = arr.GetValue<int>(i);
			if(*(arr.GetValue<int>(i).Get()) == value)
				return true;
		}
		return false;
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
	scriptName = "";
}

ScriptDef :: ~ScriptDef()
{
}

void ScriptDef :: ClearBase(void)
{
	scriptName = "";
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

void ScriptDef :: CompileFromSource(const fs::path &sourceFile)
{
	if(g_Logs.script->Enabled(el::Level::Trace)) {
		g_Logs.script->trace("Compiling TSL script %v", sourceFile);
	}
	FileReader lfr;
	if(lfr.OpenText(sourceFile) != Err_OK)
	{
		g_Logs.script->warn("InstanceScript::CompileFromSource() unable to open file: %v", sourceFile);
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
				g_Logs.script->error("Syntax error: not enough operands for IF statement in [%v line %v]", compileData.mSourceFile, compileData.mLineNumber);
			else
			{
				int vleft = 0;
				int vright = 0;
				int left = ResolveOperandType(tokens[1].c_str(), vleft);
				int cmp = ResolveComparisonType(tokens[2].c_str());
				int right = ResolveOperandType(tokens[3].c_str(), vright);
				if(cmp == CMP_INV)
					g_Logs.script->error("Invalid comparison operator: [%v] in [%v line %v]", compileData.mSourceFile, compileData.mLineNumber);
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
					default: g_Logs.script->error("Invalid operator [%v] for IF statement [%v line %v]", tokens[3].c_str(), compileData.mSourceFile, compileData.mLineNumber); valid = false; break;
					}
		
					switch(left)
					{
					case OPT_VAR: PushOpCode(OP_PUSHVAR, vleft, 0); break;
					case OPT_INT: PushOpCode(OP_PUSHINT, vleft, 0); break;
					case OPT_FLOAT: PushOpCode(OP_PUSHFLOAT, vleft, 0); break;
					case OPT_APPINT: PushOpCode(OP_PUSHAPPVAR, vleft, 0); break;
					default: g_Logs.script->error("Invalid operator [%v] for IF statement [%v line %v]", tokens[1].c_str(), compileData.mSourceFile, compileData.mLineNumber); valid = false; break;
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
				g_Logs.script->warn("ENDIF without a matching IF (%v line %v)", compileData.mSourceFile, compileData.mLineNumber);
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
				g_Logs.script->warn("ELSE without a matching IF (%v line %v)", compileData.mSourceFile, compileData.mLineNumber);
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
				g_Logs.script->warn("RECOMPARE without a matching IF (%v line %v)", compileData.mSourceFile, compileData.mLineNumber);
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
				g_Logs.script->error("Unknown instruction [%v] [%v line %v]", opname, compileData.mSourceFile, compileData.mLineNumber);
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
							g_Logs.script->error("Could not convert parameter token [%v] to expected type [%v line %v]", paramToken, compileData.mSourceFile, compileData.mLineNumber);
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
			g_Logs.script->error("Unresolved label: %v", label[i].name.c_str());
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
		g_Logs.script->error("Label name is null.");
		return -1;
	}
	if(name[0] == 0)
	{
		g_Logs.script->error("Label name cannot be empty.");
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
	map<string, int>::iterator it;
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
		g_Logs.script->error("String cannot be NULL.");
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
	string subStr;
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

	g_Logs.script->error("Unidentified opcode: %v", opcode);
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
		g_Logs.script->error("Token is null");
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
		if(tokens.size() >= 2) {
			scriptName = tokens[1];
		}
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
				g_Logs.script->error("Unknown flag [%v] [%v line %v]", flagName, compileData.mSourceFile, compileData.mLineNumber);
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
	g_Logs.script->error("Unhandled metadata token [%v] (%v line %v)", opname, compileData.mSourceFile, compileData.mLineNumber);
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
	g_Logs.script->info("Initialising TSL script %v", defPtr->scriptName.c_str());
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
		g_Logs.script->error("Instruction past end of script (%v of %v)", curInst, def->instr.size());
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
		g_Logs.script->info("%v", def->stringList[def->instr[curInst].param1].c_str());
		break;
	case OP_PRINTVAR:
		g_Logs.script->info("var[%v]=%v", def->varName[instr->param1].c_str(), GetVarValue(instr->param1));
		break;
	case OP_PRINTAPPVAR:
		g_Logs.script->info("appvar[%v]=%v", GetStringPtr(instr->param1), GetApplicationPropertyAsInteger(GetStringPtr(instr->param1)));
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
		g_Logs.script->error("Unidentified op type: %v", def->instr[curInst].opCode);
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
			g_Logs.script->error("Label [%v] not found in script [%v]", name, def->scriptName.c_str());

		//Just need to determine if the script should be halted on failure.  If it doesn't use
		//an event queue, it probably needs to be stopped.
		if(def->UseEventQueue() == false)
		{
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
		g_Logs.script->info("Script [%v] has ended", def->scriptName.c_str());
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
		g_Logs.script->error("Script error: PushVarStack() stack is full [script: %v]", def->scriptName.c_str());
	else
		varStack.push_back(value);
}

int ScriptPlayer :: PopVarStack(void)
{
	int retval = 0;
	if(varStack.size() == 0)
		g_Logs.script->error("TSL Script error: PopVarStack() stack is empty [script: %v]", def->scriptName.c_str());
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
		g_Logs.script->error("TSL Script error: PushCallStack() stack is full [script: %v]", def->scriptName.c_str());
	else
		callStack.push_back(value);
}

int ScriptPlayer :: PopCallStack(void)
{
	int retval = 0;
	if(callStack.size() == 0)
		g_Logs.script->error("TSL Script error: PopCallStack() stack is empty [script: %v]", def->scriptName.c_str());
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
		g_Logs.script->error("TSL Script error: QueueEvent() list is full [script: %v]", def->scriptName.c_str());
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

void ScriptPlayer :: ClearQueue(void)
{
	scriptEventQueue.clear();
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
		g_Logs.script->error("TSL Script error: string index out of range [%v] for script [%v]", index, def->scriptName.c_str());
		return NULL_RESPONSE;
	}
	return def->stringList[index].c_str();
}

int ScriptPlayer :: GetVarValue(int index)
{
	int retval = 0;
	if(index < 0 || index >= (int)vars.size())
		g_Logs.script->error("TSL Script error: variable index out of range [%v] for script [%v]", index, def->scriptName.c_str());
	else
		retval = vars[index];

	return retval;
}

const char * ScriptPlayer :: GetStringTableEntry(int index)
{
	const char *retval = "";
	if(index < 0 || index >= (int)def->stringList.size())
		g_Logs.script->error("TSL Script error: string index out of range [%v] for script [%v]", index, def->scriptName.c_str());
	else
		retval = def->stringList[index].c_str();

	return retval;
}

int ScriptPlayer :: VerifyIntArrayIndex(int index)
{
	if(index < 0 || index >= (int)def->mIntArray.size())
	{
		g_Logs.script->error("TSL Script error: IntArray index out of range [%v] for script [%v]", index, def->scriptName.c_str());
		return -1;
	}
	return index;
}

void ScriptPlayer :: SetVar(unsigned int index, int value)
{
	if(index >= vars.size())
	{
		g_Logs.script->error("TSL SetVar() index [%v] is outside range [%v]", index, vars.size());
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

void ScriptCompiler::AddSymbol(const string &key, const string &value)
{
	mSymbols[key] = value;
}

bool ScriptCompiler::HasSymbol(const string &token)
{
	map<string, string>::iterator it;
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
		g_Logs.script->error("[%v] expects parameters [%v] [%v line %v]", op, desc, mSourceFile, mLineNumber);
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
		g_Logs.script->error("IntArray [%v] cannot append more than %v elements", name.c_str(), MAX_ARRAY_DATA_SIZE);
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
		g_Logs.script->error("IntArray [%v] index [%v] out of range", name.c_str(), index);
		return false;
	}
	return true;
}

void IntArray::DebugPrintContents(void)
{
	char buffer[16];
	string str = "[";
	for(size_t i = 0; i < arrayData.size();  i++)
	{
		if(i > 0)
			str += ",";
		sprintf(buffer, "%d", arrayData[i]);
		str += buffer;
	}
	str += "]";
	g_Logs.script->info("IntArray[%v]=%v", name.c_str(), str.c_str());
}

ScriptEvent::ScriptEvent(const char *label, unsigned long fireTime)
{
	mLabel = label;
	mFireTime = fireTime;
}


//namespace ScriptCore
}
