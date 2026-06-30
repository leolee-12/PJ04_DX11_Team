#pragma once
#include "LevelDesign_LoadTypes.h"

NS_BEGIN(Client)

struct LD_SPAWN_SPEC;

class CLevelDesign_MonsterCatalog final
{
private:
	CLevelDesign_MonsterCatalog() = delete;
	~CLevelDesign_MonsterCatalog() = delete;

public:
	static void Register_LevelDesignSpecs();
	static _bool Build_Desc(const LD_OBJECT_DESC& CommonDesc, const json& jEntry, const LD_SPAWN_SPEC& Spec, LD_OBJECT_ENTRY* pOutEntry);
};

NS_END