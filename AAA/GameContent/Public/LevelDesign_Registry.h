#pragma once
#include "LevelDesign_LoadTypes.h"

NS_BEGIN(Client)

using LD_OBJECT_PROTOTYPE_FACTORY =
CGameObject * (*)(ID3D11Device*, ID3D11DeviceContext*);

struct LD_MODEL_REQUIREMENT
{
	_wstring strPrototypeTag;
	_string strFilePath;
	_uint iPrototypeLevel = ETOUI(LEVEL::GAMEPLAY);
};

struct LD_SPAWN_SPEC
{
	_wstring strObjectName;
	_wstring strPrototypeTag;
	_wstring strLayerTag;
	LD_CATEGORY eCategory = LD_CATEGORY::UNSUPPORTED;

	LD_BREAKABLE_TYPE eBreakableType = LD_BREAKABLE_TYPE::UNKNOWN;
	_wstring wstrModelProtoTag;

	LD_OBJECT_PROTOTYPE_FACTORY pPrototypeFactory = nullptr;
	vector<LD_MODEL_REQUIREMENT> ModelRequirements;
};

struct LD_RESOLVED_SPAWN
{
	LD_SPAWN_SPEC Spec;

	LD_PARSED_OBJECT ParsedDesc;
	LD_BREAKABLE_OBJECT_DESC BreakableDesc;

	_bool bUseBreakableDesc = false;
	_bool bFallback = false;

	void* Get_SpawnArgument()
	{
		if (bUseBreakableDesc)
			return &BreakableDesc;

		return &ParsedDesc;
	}
};

class CLIENT_DLL CLevelDesign_Registry final
{
private:
	CLevelDesign_Registry() = delete;
	~CLevelDesign_Registry() = delete;

public:
	static void Initialize();
	static const LD_SPAWN_SPEC* Find(const _wstring& strObjectName);
	static const LD_SPAWN_SPEC& Get_FallbackSpec();
	static _bool Is_LevelDesignLayer(const _wstring& strLayerTag);
	static _bool Register(const _wstring& strObjectName, const LD_SPAWN_SPEC& Spec);
	static _bool Resolve(const LD_PARSED_OBJECT& Desc, LD_RESOLVED_SPAWN* pOutResolved);

private:
	static void Register_Core();
	static void Register_Volumes();
	static void Register_GuideAudio();
	static void Register_ItemsAndBreakables();
	static void Register_EnemiesAndGimmicks();
};

NS_END