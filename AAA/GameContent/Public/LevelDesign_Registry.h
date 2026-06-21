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
	MODEL eModelType = { MODEL::NONANIM };
	_bool bCookCollisionMesh = { false };
};

struct LD_SPAWN_SPEC
{
	_wstring strObjectName;
	_wstring strPrototypeTag;
	_wstring strLayerTag;
	LD_CATEGORY eCategory = { LD_CATEGORY::UNSUPPORTED };
	_wstring wstrModelProtoTag;
	MODEL eModelType = { MODEL::NONANIM };

	LD_OBJECT_PROTOTYPE_FACTORY pPrototypeFactory = { nullptr };
	vector<LD_MODEL_REQUIREMENT> ModelRequirements;
};

struct LD_RESOLVED_SPAWN
{
	LD_SPAWN_SPEC Spec;
	LD_OBJECT_ENTRY ObjectDesc;
	
	_bool bFallback = { false };

	void* Get_SpawnArgument()
	{
		const auto& ToSpawnArgument = [](auto& Desc) -> void* { return static_cast<void*>(&Desc);; };

		return std::visit(ToSpawnArgument, ObjectDesc);
	}

	const LD_OBJECT_DESC& Get_BaseDesc() const
	{
		return Get_LDObjectDesc(ObjectDesc);
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
	static _bool Resolve(const LD_OBJECT_ENTRY& Desc, LD_RESOLVED_SPAWN* pOutResolved);

private:
	static void Register_Core();
	static void Register_Volumes();
	static void Register_GuideAudio();
	static void Register_ItemsAndBreakables();
	static void Register_EnemiesAndGimmicks();
};

NS_END