#pragma once
#include "LevelDesign_LoadTypes.h"

NS_BEGIN(Client)
struct LD_SPAWN_SPEC;

using LD_OBJECT_PROTO_FACTORY = CGameObject* (*)(ID3D11Device*, ID3D11DeviceContext*);
using LD_DESC_BUILD_FUNC = _bool (*)(const LD_OBJECT_DESC& CommonDesc, const json& jEntry, const LD_SPAWN_SPEC& Spec, LD_OBJECT_ENTRY* pOutEntry);
using LD_MAKE_DEFAULT_DESC_FUNC = _bool(*)(const LD_OBJECT_DESC& CommonDesc, _uint iModelProtoLevel, const LD_SPAWN_SPEC& Spec, LD_OBJECT_ENTRY* pOutEntry);

struct LD_MODEL_REQUIREMENT
{
	_wstring strPrototypeTag;
	_string strFilePath;
	MODEL eModelType = { MODEL::NONANIM };
	_bool bCookCollisionMesh = { false };
	_float4x4 PreTransformMatrix = { 1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f };
};

struct LD_SPAWN_SPEC
{
	_wstring strObjectName;
	_wstring strPrototypeTag;
	_wstring strLayerTag;
	_wstring strVariationOwnerName;
	LD_CATEGORY eCategory = { LD_CATEGORY::UNSUPPORTED };
	_wstring wstrModelProtoTag;
	MODEL eModelType = { MODEL::NONANIM };

	LD_OBJECT_PROTO_FACTORY pPrototypeFactory = { nullptr };
	LD_DESC_BUILD_FUNC pBuildDesc = { nullptr };
	LD_MAKE_DEFAULT_DESC_FUNC pMakeDefaultDesc = { nullptr };
	_bool bUseFactoryResourceLoader = { false };
	vector<LD_MODEL_REQUIREMENT> ModelRequirements;
};

struct LD_RESOLVED_SPAWN
{
	LD_SPAWN_SPEC Spec;
	LD_OBJECT_ENTRY ObjectDesc;
	
	_bool bFallback = { false };

	void* Get_SpawnArgument()
	{
		const auto& ToSpawnArgument = [](auto& Desc) -> void* { return static_cast<void*>(&Desc); };

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
	static const LD_SPAWN_SPEC* Find_PlacementSpec(const _wstring& strPrototypeTag);
	static _bool Make_DefaultDesc(const _wstring& strPrototypeTag, _uint iModelProtoLevel,
		const _wstring& strObjectTag, const _float3& vPosition, LD_OBJECT_ENTRY* pOutEntry);
	static const LD_SPAWN_SPEC& Get_FallbackSpec();
	static _bool Is_LevelDesignLayer(const _wstring& strLayerTag);
	static _bool Register(const _wstring& strObjectName, const LD_SPAWN_SPEC& Spec);
	static _bool Build_Entry(const LD_OBJECT_DESC& CommonDesc, const json& jEntry, LD_OBJECT_ENTRY* pOutEntry);
	static _bool Resolve(const LD_OBJECT_ENTRY& Desc, LD_RESOLVED_SPAWN* pOutResolved);

private:
	static void Register_Core();
	static void Register_Volumes();
	static void Register_GuideAudio();
	static void Register_NPCs();
	static void Register_ItemsAndBreakables();
	static void Register_EnemiesAndGimmicks();
};

NS_END