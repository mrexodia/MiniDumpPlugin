#include "plugin.h"
#include <DbgHelp.h>

#pragma comment(lib, "dbghelp.lib")

static bool g_hasException = false;
static EXCEPTION_DEBUG_INFO g_exception;

PLUG_EXPORT void CBEXCEPTION(CBTYPE, PLUG_CB_EXCEPTION* exception)
{
	if (exception->Exception)
	{
		g_hasException = true;
		memcpy(&g_exception, exception->Exception, sizeof(g_exception));
	}
}

PLUG_EXPORT void CBSTOPDEBUG(CBTYPE, PLUG_CB_STOPDEBUG*)
{
	g_hasException = false;
}

static bool cbMiniDump(int argc, char* argv[])
{
	if (DbgIsRunning())
	{
		dputs("Cannot dump while running...");
		return false;
	}

	if (argc < 2)
	{
		dputs("Usage: MiniDump my.dmp");
		return false;
	}

	HANDLE hFile = CreateFileA(argv[1], GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		dprintf("Failed to create '%s'\n", argv[1]);
		return false;
	}

	// Disable all software breakpoints
	std::vector<duint> disabled_breakpoints;
	{
		BPMAP bplist = {};
		DbgGetBpList(bp_normal, &bplist);
		for (int i = 0; i < bplist.count; i++)
		{
			const auto& bp = bplist.bp[i];
			if (bp.active && bp.enabled)
			{
				char cmd[256] = "";
				sprintf_s(cmd, "bd 0x%p", (void*)bp.addr);
				if (DbgCmdExecDirect(cmd))
					disabled_breakpoints.push_back(bp.addr);
			}
		}
		BridgeFree(bplist.bp);
	}

	CONTEXT context = {};
	context.ContextFlags = CONTEXT_ALL;
	GetThreadContext(DbgGetThreadHandle(), &context);
	context.EFlags &= ~0x100; // remove trap flag

	EXCEPTION_POINTERS exceptionPointers = {};
	exceptionPointers.ContextRecord = &context;
	exceptionPointers.ExceptionRecord = &g_exception.ExceptionRecord;
	if (exceptionPointers.ExceptionRecord->ExceptionCode == 0)
	{
		auto& exceptionRecord = *exceptionPointers.ExceptionRecord;
		exceptionRecord.ExceptionCode = 0xFFFFFFFF;
#ifdef _WIN64
		exceptionRecord.ExceptionAddress = PVOID(context.Rip);
#else
		exceptionRecord.ExceptionAddress = PVOID(context.Eip);
#endif // _WIN64
	}

	MINIDUMP_EXCEPTION_INFORMATION exceptionInfo = {};
	exceptionInfo.ThreadId = DbgGetThreadId();
	exceptionInfo.ExceptionPointers = &exceptionPointers;
	exceptionInfo.ClientPointers = FALSE;
	auto dumpType = MINIDUMP_TYPE(MiniDumpWithFullMemory | MiniDumpWithFullMemoryInfo | MiniDumpIgnoreInaccessibleMemory | MiniDumpWithHandleData);
	auto dumpSaved = !!MiniDumpWriteDump(DbgGetProcessHandle(), DbgGetProcessId(), hFile, dumpType, &exceptionInfo, nullptr, nullptr);

	// Re-enable all breakpoints that were previously disabled
	for (auto addr : disabled_breakpoints)
	{
		char cmd[256] = "";
		sprintf_s(cmd, "be 0x%p", (void*)addr);
		DbgCmdExecDirect(cmd);
	}

	if (dumpSaved)
	{
		dputs("Dump saved!");
	}
	else
	{
		dprintf("MiniDumpWriteDump failed :( LastError = %d\n", GetLastError());
	}

	CloseHandle(hFile);
	return true;
}

//Initialize your plugin data here.
bool pluginInit(PLUG_INITSTRUCT* initStruct)
{
	_plugin_registercommand(pluginHandle, "MiniDump", cbMiniDump, true);
	return true; //Return false to cancel loading the plugin.
}

//Deinitialize your plugin data here.
void pluginStop()
{
}

//Do GUI/Menu related things here.
void pluginSetup()
{
}
