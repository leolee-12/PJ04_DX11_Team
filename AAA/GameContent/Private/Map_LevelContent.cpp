#include "Map_LevelContent.h"
#include "Map_EditFile.h"

NS_BEGIN(Client)

json CMap_LevelContent::Serialize(const MAP_EDIT_DATA& Desc)
{
    return CMap_EditFile::Save_Data(Desc);
}

HRESULT CMap_LevelContent::Deserialize(const json& jLevelRoot, MAP_EDIT_DATA* pOutDesc)
{
    return CMap_EditFile::Load_Data(jLevelRoot, pOutDesc);
}

NS_END