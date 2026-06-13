#include "Map_StageOverrideSerializer.h"
#include "Map_EditFile.h"

NS_BEGIN(Client)

json CMap_StageOverrideSerializer::Serialize(const CMapStage* pStage)
{
	return CMap_EditFile::Save_Stage(pStage);
}

HRESULT CMap_StageOverrideSerializer::Apply(CMapStage* pStage, const json& jOverride)
{
	return CMap_EditFile::Apply_Stage(pStage, jOverride);
}

NS_END