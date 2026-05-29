#pragma once
#include "Engine_Defines.h"
#include "GameObject.h"

NS_BEGIN(Client)

typedef struct tagObjectData
{
	wstring							strObjectTag;
	wstring							strPrototypeTag;
	wstring							strLayerTag;
	CGameObject::GAMEOBJECT_DESC	Desc;
}OBJECT_DATA;

class ObjectDataAdapter
{
public:
	static OBJECT_DATA Convert(const CGameObject::ENGINE_OBJECT_DATA& engineData)
	{
		OBJECT_DATA objData;
		objData.strPrototypeTag = engineData.strPrototypeTag;
		objData.strLayerTag = engineData.strLayerTag;
		objData.Desc = engineData.Desc;
		return objData;
	}
};

NS_END