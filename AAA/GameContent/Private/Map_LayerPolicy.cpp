#include "Map_LayerPolicy.h"

NS_BEGIN(Client)

_bool CMap_LayerPolicy::Is_MapLayer(const _wstring& strLayerTag)
{
	return strLayerTag == LAYER_MAP_STAGE
		|| Is_EnvLayer(strLayerTag);
}

_bool CMap_LayerPolicy::Is_EnvLayer(const _wstring& strLayerTag)
{
	return strLayerTag == LAYER_ENV_STATIC
		|| strLayerTag == LAYER_ENV_INTERACT
		|| strLayerTag == LAYER_ENV_EFFECT;
}

NS_END