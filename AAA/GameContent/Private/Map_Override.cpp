#include "Map_Override.h"
#include "Map_EditFile.h"

NS_BEGIN(Client)

_wstring CMap_Override::Build_EnvObjectStableKey(const ENV_OBJECT_DESC& Desc)
{
	return CMap_EditFile::Make_EnvKey(Desc);
}

json CMap_Override::Serialize(const MAP_EDIT_CHANGE& Desc)
{
	return CMap_EditFile::Save_Change(Desc);
}

HRESULT CMap_Override::Deserialize(const json& jOverride, MAP_EDIT_CHANGE* pOutDesc)
{
	return CMap_EditFile::Load_Change(jOverride, pOutDesc);
}

HRESULT CMap_Override::Apply(MAP_PACKAGE* pInOutPackage, const MAP_EDIT_CHANGE& OverrideDesc)
{
	return CMap_EditFile::Apply_Change(pInOutPackage, OverrideDesc);
}

NS_END