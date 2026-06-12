#include "Map_StageOverrideSerializer.h"
#include "MapStage.h"
#include "MapSection.h"

NS_BEGIN(Client)

json CMap_StageOverrideSerializer::Serialize(const CMapStage* pStage)
{
	if (nullptr == pStage)
		return json::object();

	json jOverride = json::object();
	jOverride["StageName"] = WstrToStr(pStage->Get_StageName());
	jOverride["Sections"] = json::array();

	const auto& Sections = pStage->Get_Sections();
	for (const CMapSection* pSection : Sections)
	{
		if (nullptr == pSection)
			continue;

		jOverride["Sections"].push_back(pSection->Serialize_SectionState());
	}

	return jOverride;
}

HRESULT CMap_StageOverrideSerializer::Apply(CMapStage* pStage, const json& jOverride)
{
	if (nullptr == pStage)
		return E_FAIL;

	if (jOverride.is_null())
		return S_OK;

	if (!jOverride.is_object())
		return E_FAIL;

	pStage->Deserialize(jOverride);
	return S_OK;
}

NS_END