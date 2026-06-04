#include "GameContent_Log.h"

#include <Windows.h>

NS_BEGIN(Client)

namespace
{
	GAMECONTENT_LOG_SINK g_pLogSink = nullptr;
}

void Set_GameContentLogSink(GAMECONTENT_LOG_SINK pSink)
{
	g_pLogSink = pSink;
}

void Emit_GameContentLog(GAMECONTENT_LOG_LEVEL eLevel, const char* pMessage)
{
	if (nullptr != g_pLogSink)
	{
		g_pLogSink(eLevel, pMessage);
		return;
	}

	if (nullptr == pMessage)
		return;

	OutputDebugStringA(pMessage);
	OutputDebugStringA("\n");
}

NS_END
