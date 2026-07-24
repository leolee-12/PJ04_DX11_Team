#pragma once
#include "Shader_PassMeta.h"

NS_BEGIN(Engine)
class CShader;
class CModel;
class CTransform;
class CCullingState; 
class CGameInstance_Proxy;
NS_END

NS_BEGIN(Client)

enum class MESH_LAYER_PROFILE : _uint
{
	MAP,

	WORLD_NONANIM,
	WORLD_ANIM,
	WORLD_INSTANCE,
};

enum class MESH_LAYER_RENDER_KIND : _uint
{
	MAIN,
	SHADOW,
	DECAL,
	MAIN_BLEND
};

struct MESH_LAYER_BIND_CONTEXT
{
	CShader* pShader = { nullptr };
	CModel* pModel = { nullptr };
	CCullingState* pCullingState = { nullptr };
	CGameInstance_Proxy* pGI_Proxy = { nullptr };

	void Set_Renderer(CShader* pShader_, CModel* pModel_, CGameInstance_Proxy* pProxy_, CCullingState* pCulling_ = nullptr)
	{
		pShader = pShader_; pModel = pModel_; pGI_Proxy = pProxy_; pCullingState = pCulling_;
	}

	_uint iMesh = { 0 };
	const MESH_LAYER_IDX* pLayer = { nullptr };

	MESH_LAYER_PROFILE eProfile = { MESH_LAYER_PROFILE::WORLD_NONANIM };
	MESH_LAYER_RENDER_KIND eKind = { MESH_LAYER_RENDER_KIND::MAIN };

	_uint iFallbackPass = { 0u };
	_uint iExtraFlags = { 0u };
	_float fDissolve = { 0.f };

	_bool bUseLayerEx = { true };
};

struct MESH_LAYER_BIND_RESULT
{
	_uint iPass = { 0u };
	_bool bSkipMesh = { false };
};

namespace MeshLayerBinder
{
	HRESULT Bind(const MESH_LAYER_BIND_CONTEXT& Ctx, MESH_LAYER_BIND_RESULT* pOutResult);

	// S_OK: *pOutPass 유효 / S_FALSE: 메시 스킵 / 실패: 바인딩 실패
	HRESULT Bind_OrSkip(const MESH_LAYER_BIND_CONTEXT& Ctx, _uint* pOutPass);

	HRESULT Bind_WorldViewProj(CShader* pShader, CTransform* pTransform, CGameInstance_Proxy* pGI_Proxy, PROJ_TYPE eProjType);

	HRESULT Bind_TextureSafe(CShader* pShader, CModel* pModel, CGameInstance_Proxy* pGI_Proxy,
		_uint iMesh, const _char* pConstantName, MTEX_TYPE eType, _uint iSlot, DEFAULT_TEXTURE eDefaultKind);

	_uint Resolve_Pass(MESH_LAYER_PROFILE eProfile, MESH_LAYER_RENDER_KIND eKind, const MESH_LAYER_IDX& Layer, _uint iFallbackPass);
}

NS_END