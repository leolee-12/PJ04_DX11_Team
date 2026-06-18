#pragma once
#include "LevelDesign_LoadTypes.h"

NS_BEGIN(Client)

struct LD_SPAWN_SPEC
{
	_wstring strObjectName;
	_wstring strPrototypeTag;
	_wstring strLayerTag;
	LD_CATEGORY eCategory = LD_CATEGORY::UNSUPPORTED;
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

private:
	static void Register_Core();
	static void Register_Volumes();
	static void Register_GuideAudio();
	static void Register_ItemsAndBreakables();
	static void Register_EnemiesAndGimmicks();
};

NS_END