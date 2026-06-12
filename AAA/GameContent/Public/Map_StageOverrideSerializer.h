#pragma once
#include "GameContent_Defines.h"

NS_BEGIN(Client)
class CMapStage;

class CLIENT_DLL CMap_StageOverrideSerializer final
{
public:
	static json Serialize(const CMapStage* pStage);
	static HRESULT Apply(CMapStage* pStage, const json& jOverride);
};

NS_END