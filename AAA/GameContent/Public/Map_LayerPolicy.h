#pragma once
#include "GameContent_Defines.h"

NS_BEGIN(Client)

class CLIENT_DLL CMap_LayerPolicy final
{
public:
	static constexpr const _tchar* LAYER_MAP_STAGE = L"Layer_MapStage";
	static constexpr const _tchar* LAYER_ENV_STATIC = L"Layer_EnvStatic";
	static constexpr const _tchar* LAYER_ENV_INTERACT = L"Layer_EnvInteract";
	static constexpr const _tchar* LAYER_ENV_EFFECT = L"Layer_EnvEffect";

public:
	static _bool Is_MapLayer(const _wstring& strLayerTag);
	static _bool Is_EnvLayer(const _wstring& strLayerTag);
};

NS_END