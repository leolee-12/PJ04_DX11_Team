#include "MeshLayer_Binder.h"

#include "GameInstance_Proxy.h"
#include "Math_Utils.h"
#include "Model.h"
#include "Shader.h"
#include "Transform.h"
#include "CullingState.h"

NS_BEGIN(Client)

namespace
{
	struct MESH_LAYER_BIND_RESULT
	{
		_uint iPass = { 0u };
		_bool bSkipMesh = { false };
	};

	HRESULT Bind_Map(const MESH_LAYER_BIND_CONTEXT& Ctx, MESH_LAYER_BIND_RESULT* pOutResult);
	HRESULT Bind_World(const MESH_LAYER_BIND_CONTEXT& Ctx, MESH_LAYER_BIND_RESULT* pOutResult);
	HRESULT Bind_WorldMainTextures(const MESH_LAYER_BIND_CONTEXT& Ctx);
	HRESULT Bind_WorldCommonParams(const MESH_LAYER_BIND_CONTEXT& Ctx);
	HRESULT Bind_WorldShadow(const MESH_LAYER_BIND_CONTEXT& Ctx, MESH_LAYER_BIND_RESULT* pOutResult);
	HRESULT Bind_WorldDecal(const MESH_LAYER_BIND_CONTEXT& Ctx, MESH_LAYER_BIND_RESULT* pOutResult);

	HRESULT Bind_MapParams(const MESH_LAYER_BIND_CONTEXT& Ctx);
	HRESULT Bind_MapTextures(const MESH_LAYER_BIND_CONTEXT& Ctx);
	HRESULT Bind_MapParams_Legacy(const MESH_LAYER_BIND_CONTEXT& Ctx);
	HRESULT Bind_MapTextures_Legacy(const MESH_LAYER_BIND_CONTEXT& Ctx);
	HRESULT Bind_MapParams_Ex(const MESH_LAYER_BIND_CONTEXT& Ctx);
	HRESULT Bind_MapTextures_Ex(const MESH_LAYER_BIND_CONTEXT& Ctx);
	HRESULT Bind_MapExtraSlotSafe(const MESH_LAYER_BIND_CONTEXT& Ctx, const _char* pName, int iSlot, MTEX_TYPE eType, DEFAULT_TEXTURE eDefault);
	HRESULT Bind_MapLayerExTextureSafe(const MESH_LAYER_BIND_CONTEXT& Ctx, const _char* pName, const MESH_LAYER_TEX_BIND_EX& Bind, DEFAULT_TEXTURE eDefault);
}

static HRESULT Bind(const MESH_LAYER_BIND_CONTEXT& Ctx, MESH_LAYER_BIND_RESULT* pOutResult)
{
	if (nullptr == pOutResult)
		return E_FAIL;

	*pOutResult = {};

	if (nullptr == Ctx.pShader || nullptr == Ctx.pModel || nullptr == Ctx.pGI_Proxy || nullptr == Ctx.pLayer)
		return E_FAIL;

	switch (Ctx.eProfile)
	{
	case MESH_LAYER_PROFILE::MAP:
		return Bind_Map(Ctx, pOutResult);

	case MESH_LAYER_PROFILE::WORLD_NONANIM:
	case MESH_LAYER_PROFILE::WORLD_ANIM:
	case MESH_LAYER_PROFILE::WORLD_INSTANCE:
		return Bind_World(Ctx, pOutResult);

	default:
		return E_FAIL;
	}
}

static HRESULT Bind_TextureSafe(CShader* pShader, CModel* pModel, CGameInstance_Proxy* pGI_Proxy,
	_uint iMesh, const _char* pConstantName, MTEX_TYPE eType, _uint iSlot, DEFAULT_TEXTURE eDefaultKind)
{
	if (nullptr == pShader || nullptr == pModel || nullptr == pGI_Proxy || nullptr == pConstantName)
		return E_FAIL;

	const _uint iTextureCount = pModel->Get_MeshTextureCount(iMesh, eType);

	if (iTextureCount > 0u)
	{
		const _uint iSafeSlot = (iSlot < iTextureCount) ? iSlot : (iTextureCount - 1u);

		if (SUCCEEDED(pModel->Bind_Material(pShader, pConstantName, iMesh, eType, iSafeSlot)))
			return S_OK;
	}

	return pGI_Proxy->Bind_DefaultTextureFromHub(pShader, pConstantName, eDefaultKind);
}

static _uint Resolve_Pass(MESH_LAYER_PROFILE eProfile, MESH_LAYER_RENDER_KIND eKind, const MESH_LAYER_IDX& Layer, _uint iFallbackPass)
{
	if (MESH_LAYER_RENDER_KIND::MAIN != eKind && MESH_LAYER_RENDER_KIND::MAIN_BLEND != eKind)
		return iFallbackPass;

	switch (eProfile)
	{
	case MESH_LAYER_PROFILE::MAP:
		return Is_ValidMapPassValue(Layer.iPass) ? static_cast<_uint>(Layer.iPass) : iFallbackPass;

	case MESH_LAYER_PROFILE::WORLD_NONANIM:
	case MESH_LAYER_PROFILE::WORLD_ANIM:
	case MESH_LAYER_PROFILE::WORLD_INSTANCE:
	{
		const WORLD_PASS ePass = static_cast<WORLD_PASS>(Layer.iPass);
		return WORLD_PASS::DEFAULT != ePass && Is_ValidWorldPassValue(Layer.iPass) ? static_cast<_uint>(ePass) : iFallbackPass;
	}

	default:
		return iFallbackPass;
	}
}

HRESULT MeshLayerBinder::Bind_OrSkip(const MESH_LAYER_BIND_CONTEXT& Ctx, _uint* pOutPass)
{
	if (nullptr == pOutPass)
		return E_FAIL;

	MESH_LAYER_BIND_RESULT Result{};
	const HRESULT hrBind = Bind(Ctx, &Result);
	if (FAILED(hrBind))
		return hrBind;

	if (Result.bSkipMesh)
		return S_FALSE;

	*pOutPass = Result.iPass;
	return S_OK;
}

HRESULT MeshLayerBinder::Bind_WorldViewProj(CShader* pShader, CTransform* pTransform, CGameInstance_Proxy* pGI_Proxy, PROJ_TYPE eProjType)
{
	if (nullptr == pShader || nullptr == pTransform || nullptr == pGI_Proxy)
		return E_FAIL;

	if (FAILED(pTransform->Bind_ShaderResource(pShader, "g_WorldMatrix")))
		return E_FAIL;

	if (FAILED(pShader->Bind_Matrix("g_ViewMatrix", pGI_Proxy->Get_Matrix(D3DTS::VIEW, eProjType))))
		return E_FAIL;

	if (FAILED(pShader->Bind_Matrix("g_ProjMatrix", pGI_Proxy->Get_Matrix(D3DTS::PROJ, eProjType))))
		return E_FAIL;

	return S_OK;
}

namespace
{
	HRESULT Bind_Map(const MESH_LAYER_BIND_CONTEXT& Ctx, MESH_LAYER_BIND_RESULT* pOutResult)
	{
		if (MESH_LAYER_RENDER_KIND::MAIN != Ctx.eKind)
			return E_FAIL;

		const MESH_LAYER_IDX& Layer = *Ctx.pLayer;
		pOutResult->bSkipMesh = ETOI(MAP_PASS::DISCARD) == Layer.iPass;

		if (FAILED(Bind_MapParams(Ctx)))
			return E_FAIL;
		if (FAILED(Bind_MapTextures(Ctx)))
			return E_FAIL;

		const _uint iFallbackPass = (0u != Ctx.iFallbackPass) ? Ctx.iFallbackPass : ETOI(MAP_DEFAULT_PASS);
		pOutResult->iPass = Resolve_Pass(Ctx.eProfile, Ctx.eKind, Layer, iFallbackPass);

		return S_OK;
	}

	HRESULT Bind_World(const MESH_LAYER_BIND_CONTEXT& Ctx, MESH_LAYER_BIND_RESULT* pOutResult)
	{
		const MESH_LAYER_IDX& Layer = *Ctx.pLayer;
		const _bool bBlendMesh = Is_WorldBlendPass(Layer.iPass);
		const _bool bBlendKind = MESH_LAYER_RENDER_KIND::MAIN_BLEND == Ctx.eKind;

		pOutResult->bSkipMesh = ETOI(WORLD_PASS::DISCARD) == Layer.iPass || bBlendMesh != bBlendKind;

		switch (Ctx.eKind)
		{
		case MESH_LAYER_RENDER_KIND::MAIN:
		case MESH_LAYER_RENDER_KIND::MAIN_BLEND:
		{
			if (FAILED(Bind_WorldMainTextures(Ctx)))
				return E_FAIL;
			if (FAILED(Bind_WorldCommonParams(Ctx)))
				return E_FAIL;

			if (FAILED(Ctx.pShader->Bind_RawValue("g_vEmissiveColor", &Layer.vEmissiveColor, sizeof(_float4))))
				return E_FAIL;
			if (FAILED(Ctx.pShader->Bind_RawValue("g_vMRA", &Layer.vMRA, sizeof(_float3))))
				return E_FAIL;

			if (MESH_LAYER_RENDER_KIND::MAIN_BLEND == Ctx.eKind)
			{
				if (FAILED(Ctx.pGI_Proxy->Bind_ShaderGlobals(Ctx.pShader, { "g_vLightDir", "g_vLightDiffuse" })))
					return E_FAIL;
				if (FAILED(Ctx.pShader->Bind_RawValue("g_vCamPosition", Ctx.pGI_Proxy->Get_CamPosition(), sizeof(_float4))))
					return E_FAIL;
				if (FAILED(Ctx.pShader->Bind_Matrix("g_ViewMatrixInverse",
					Ctx.pGI_Proxy->Get_InverseMatrix_Prespec(D3DTS::VIEW))))
					return E_FAIL;
				if (FAILED(Ctx.pShader->Bind_Matrix("g_ProjMatrixInverse",
					Ctx.pGI_Proxy->Get_InverseMatrix_Prespec(D3DTS::PROJ))))
					return E_FAIL;
				if (ETOI(WORLD_PASS::BLEND_UKWN_LIGHT) == Layer.iPass ||
					ETOI(WORLD_PASS::BLEND_UKWN2_LIGHT) == Layer.iPass)
				{
					if (FAILED(Ctx.pGI_Proxy->Bind_RT_ShaderResource(TEXT("Target_Depth"), Ctx.pShader, "g_DepthTexture")))
						return E_FAIL;
					if (FAILED(Ctx.pGI_Proxy->Bind_RT_ShaderResource(TEXT("Target_Diffuse"), Ctx.pShader, "g_SceneDiffuseTexture")))
						return E_FAIL;
				}
			}

			const _uint iFallbackPass = (0u != Ctx.iFallbackPass) ? Ctx.iFallbackPass : ETOUI(WORLD_PASS::DMN);
			pOutResult->iPass = Resolve_Pass(Ctx.eProfile, Ctx.eKind, Layer, iFallbackPass);
			return S_OK;
		}

		case MESH_LAYER_RENDER_KIND::SHADOW:
			return Bind_WorldShadow(Ctx, pOutResult);

		case MESH_LAYER_RENDER_KIND::DECAL:
			return Bind_WorldDecal(Ctx, pOutResult);

		default:
			return E_FAIL;
		}
	}

	HRESULT Bind_WorldMainTextures(const MESH_LAYER_BIND_CONTEXT& Ctx)
	{
		const MESH_LAYER_IDX& Layer = *Ctx.pLayer;

		if (FAILED(Bind_TextureSafe(Ctx.pShader, Ctx.pModel, Ctx.pGI_Proxy, Ctx.iMesh, "g_DiffuseTexture", MTEX_TYPE::DIFFUSE,
			Layer.idx[ETOUI(MTEX_TYPE::DIFFUSE)], DEFAULT_TEXTURE::MAGENTA)))
			return E_FAIL;
		if (FAILED(Bind_TextureSafe(Ctx.pShader, Ctx.pModel, Ctx.pGI_Proxy, Ctx.iMesh, "g_NormalTexture", MTEX_TYPE::NORMALS,
			Layer.idx[ETOUI(MTEX_TYPE::NORMALS)], DEFAULT_TEXTURE::FLAT_NORMAL)))
			return E_FAIL;
		if (FAILED(Bind_TextureSafe(Ctx.pShader, Ctx.pModel, Ctx.pGI_Proxy, Ctx.iMesh, "g_MRATexture", MTEX_TYPE::METALNESS,
			Layer.idx[ETOUI(MTEX_TYPE::METALNESS)], DEFAULT_TEXTURE::MRA)))
			return E_FAIL;
		if (FAILED(Bind_TextureSafe(Ctx.pShader, Ctx.pModel, Ctx.pGI_Proxy, Ctx.iMesh, "g_UnknownTexture", MTEX_TYPE::UNKNOWN,
			Layer.idx[ETOUI(MTEX_TYPE::UNKNOWN)], DEFAULT_TEXTURE::BLACK)))
			return E_FAIL;

		const _uint iHasNormalTexture = (Ctx.pModel->Get_MeshTextureCount(Ctx.iMesh, MTEX_TYPE::NORMALS) > 0u) ? 1u : 0u;
		if (FAILED(Ctx.pShader->Bind_RawValue("g_iHasNormalTexture", &iHasNormalTexture, sizeof(_uint))))
			return E_FAIL;

		if (Uses_WorldExtraRSlot(Layer.iPass) &&
			FAILED(Bind_MapExtraSlotSafe(Ctx, "g_ExtraRTexture", Layer.iExtraBind[0], static_cast<MTEX_TYPE>(Layer.iExtraTexType[0]),
				DEFAULT_TEXTURE::BLACK)))
			return E_FAIL;

		if (Uses_WorldEmissiveSlot(Layer.iPass) &&
			FAILED(Bind_TextureSafe(Ctx.pShader, Ctx.pModel, Ctx.pGI_Proxy, Ctx.iMesh, "g_EmissiveTexture",
				MTEX_TYPE::EMISSIVE, Layer.idx[ETOUI(MTEX_TYPE::EMISSIVE)], DEFAULT_TEXTURE::BLACK)))
			return E_FAIL;

		if (ETOI(WORLD_PASS::LAVA_SURFACE) == Layer.iPass)
		{
			const _uint iFlowCount = Ctx.pModel->Get_MeshTextureCount(Ctx.iMesh, MTEX_TYPE::EMISSIVE);
			_uint iHasFlowTexture = 0u;

			if (0u < iFlowCount && SUCCEEDED(Ctx.pModel->Bind_Material(Ctx.pShader, "g_FlowTexture", Ctx.iMesh, MTEX_TYPE::EMISSIVE, 0u)))
				iHasFlowTexture = 1u;
			else if (FAILED(Ctx.pGI_Proxy->Bind_DefaultTextureFromHub(Ctx.pShader, "g_FlowTexture", DEFAULT_TEXTURE::BLACK)))
				return E_FAIL;

			if (FAILED(Ctx.pShader->Bind_RawValue("g_iHasFlowTexture", &iHasFlowTexture, sizeof(_uint))))
				return E_FAIL;
		}

		return S_OK;
	}

	HRESULT Bind_WorldCommonParams(const MESH_LAYER_BIND_CONTEXT& Ctx)
	{
		const MESH_LAYER_IDX& Layer = *Ctx.pLayer;

		const _uint iUVIndex = (Layer.iUVIndex <= 3u) ? Layer.iUVIndex : 0u;
		const _uint iUnknownUVIndex = (Layer.iUnknownUVIndex <= 3u) ? Layer.iUnknownUVIndex : 0u;
		const _uint iExtraRUVIndex = Layer.iExtraUVIndex[0];
		_float2 vOffset = Layer.bUseUVTransform
			? _float2{ Layer.vUVOffset.x, Layer.vUVOffset.y }
			: _float2{ 0.f, 0.f };

		if (Layer.bUseUVTransform && Layer.bUseUVScroll)
		{
			const _double dGameTime = Ctx.pGI_Proxy->Get_GameTime();
			vOffset.x += static_cast<_float>(fmod(dGameTime * Layer.vUVScrollSpeed.x, 1.0));
			vOffset.y += static_cast<_float>(fmod(dGameTime * Layer.vUVScrollSpeed.y, 1.0));
		}

		const _float4 vUVTransform = Layer.bUseUVTransform
			? _float4{ Layer.vUVScale.x, Layer.vUVScale.y, vOffset.x, vOffset.y }
			: _float4{ 1.f, 1.f, 0.f, 0.f };
		const _float4 vUVTransformNormal = Layer.bUseUVTransform
			? _float4{ Layer.vUVScaleNormal.x, Layer.vUVScaleNormal.y, vOffset.x, vOffset.y }
			: _float4{ 1.f, 1.f, 0.f, 0.f };
		const _float4 vUVTransformMaterial = Layer.bUseUVTransform
			? _float4{ Layer.vUVScaleMaterial.x, Layer.vUVScaleMaterial.y, vOffset.x, vOffset.y }
			: _float4{ 1.f, 1.f, 0.f, 0.f };
		const _float4 vUVTransformUnknown = vUVTransform;
		const _float fUVRotate = Layer.bUseUVTransform ? Layer.fUVRotate : 0.f;
		const _float fNormalStrength = Layer.fNormalStrength;
		const _float fMaskStrength = Layer.fMaskStrength;
		const _uint iUseInstanceDissolve = (MESH_LAYER_PROFILE::WORLD_INSTANCE == Ctx.eProfile) ? 1u : 0u;
		_uint iFlags = Layer.iFlags | Ctx.iExtraFlags;
		_float fDissolve = Ctx.fDissolve;

		if (nullptr != Ctx.pCullingState &&
			(MESH_LAYER_RENDER_KIND::MAIN == Ctx.eKind
				|| MESH_LAYER_RENDER_KIND::MAIN_BLEND == Ctx.eKind
				|| MESH_LAYER_RENDER_KIND::SHADOW == Ctx.eKind))
		{
			const CCullingState::CHANNEL eCullChannel =
				(MESH_LAYER_RENDER_KIND::SHADOW == Ctx.eKind)
				? CCullingState::CHANNEL::SHADOW
				: CCullingState::CHANNEL::MAIN;

			const _float fCullingDissolve = Ctx.pCullingState->Get_Dissolve(eCullChannel);
			if (fCullingDissolve > 0.f)
			{
				iFlags |= WorldShaderFlags::Dither;
				fDissolve = max(fDissolve, fCullingDissolve);
			}
		}

		if (FAILED(Ctx.pShader->Bind_RawValue("g_iUVIndex", &iUVIndex, sizeof(_uint)))) return E_FAIL;
		if (FAILED(Ctx.pShader->Bind_RawValue("g_iUnknownUVIndex", &iUnknownUVIndex, sizeof(_uint)))) return E_FAIL;

		if (Uses_WorldExtraRSlot(Layer.iPass) &&
			FAILED(Ctx.pShader->Bind_RawValue("g_iExtraR_UVIndex", &iExtraRUVIndex, sizeof(_uint))))
			return E_FAIL;

		if (FAILED(Ctx.pShader->Bind_RawValue("g_vUVTransform", &vUVTransform, sizeof(_float4)))) return E_FAIL;
		if (FAILED(Ctx.pShader->Bind_RawValue("g_vUVTransformNormal", &vUVTransformNormal, sizeof(_float4)))) return E_FAIL;
		if (FAILED(Ctx.pShader->Bind_RawValue("g_vUVTransformMaterial", &vUVTransformMaterial, sizeof(_float4)))) return E_FAIL;
		if (FAILED(Ctx.pShader->Bind_RawValue("g_vUVTransformUnknown", &vUVTransformUnknown, sizeof(_float4)))) return E_FAIL;
		if (FAILED(Ctx.pShader->Bind_RawValue("g_fUVRotate", &fUVRotate, sizeof(_float)))) return E_FAIL;
		if (FAILED(Ctx.pShader->Bind_RawValue("g_NormalStrength", &fNormalStrength, sizeof(_float)))) return E_FAIL;
		if (FAILED(Ctx.pShader->Bind_RawValue("g_MaskStrength", &fMaskStrength, sizeof(_float)))) return E_FAIL;
		if (FAILED(Ctx.pShader->Bind_RawValue("g_vColor", &Layer.vRenderColor, sizeof(_float4)))) return E_FAIL;
		if (FAILED(Ctx.pShader->Bind_RawValue("g_iUseInstanceDissolve", &iUseInstanceDissolve, sizeof(_uint)))) return E_FAIL;
		if (FAILED(Ctx.pShader->Bind_RawValue("g_iFlags", &iFlags, sizeof(_uint)))) return E_FAIL;
		if (FAILED(Ctx.pShader->Bind_RawValue("g_fDissolve", &fDissolve, sizeof(_float)))) return E_FAIL;

		if (ETOI(WORLD_PASS::LAVA_SURFACE) == Layer.iPass)
		{
			const _double dWrappedWorldTime = MathUtils::Wrap_FiniteDouble(
				Ctx.pGI_Proxy->Get_GameTime(), static_cast<_double>(Layer.vMRA.z));
			const _float fWorldTime = static_cast<_float>(dWrappedWorldTime);

			if (FAILED(Ctx.pShader->Bind_RawValue("g_fWorldTime", &fWorldTime, sizeof(_float))))
				return E_FAIL;
		}

		return S_OK;
	}

	HRESULT Bind_WorldShadow(const MESH_LAYER_BIND_CONTEXT& Ctx, MESH_LAYER_BIND_RESULT* pOutResult)
	{
		const MESH_LAYER_IDX& Layer = *Ctx.pLayer;

		if (FAILED(Bind_TextureSafe(Ctx.pShader, Ctx.pModel, Ctx.pGI_Proxy, Ctx.iMesh, "g_DiffuseTexture", MTEX_TYPE::DIFFUSE,
			Layer.idx[ETOUI(MTEX_TYPE::DIFFUSE)], DEFAULT_TEXTURE::MAGENTA)))
			return E_FAIL;
		if (FAILED(Bind_TextureSafe(Ctx.pShader, Ctx.pModel, Ctx.pGI_Proxy, Ctx.iMesh, "g_UnknownTexture", MTEX_TYPE::UNKNOWN,
			Layer.idx[ETOUI(MTEX_TYPE::UNKNOWN)], DEFAULT_TEXTURE::BLACK)))
			return E_FAIL;
		if (FAILED(Bind_WorldCommonParams(Ctx)))
			return E_FAIL;

		const _uint iShadowAlphaSource = static_cast<_uint>(Resolve_WorldShadowAlphaSource(static_cast<WORLD_PASS>(Layer.iPass)));
		if (FAILED(Ctx.pShader->Bind_RawValue("g_iShadowAlphaSource", &iShadowAlphaSource, sizeof(_uint))))
			return E_FAIL;

		pOutResult->iPass = (0u != Ctx.iFallbackPass) ? Ctx.iFallbackPass : ETOUI(WORLD_PASS::SHADOW);
		return S_OK;
	}

	HRESULT Bind_WorldDecal(const MESH_LAYER_BIND_CONTEXT& Ctx, MESH_LAYER_BIND_RESULT* pOutResult)
	{
		const MESH_LAYER_IDX& Layer = *Ctx.pLayer;

		if (FAILED(Bind_TextureSafe(Ctx.pShader, Ctx.pModel, Ctx.pGI_Proxy, Ctx.iMesh, "g_DiffuseTexture",
			MTEX_TYPE::DIFFUSE, Layer.idx[ETOUI(MTEX_TYPE::DIFFUSE)], DEFAULT_TEXTURE::MAGENTA)))
			return E_FAIL;
		if (FAILED(Bind_TextureSafe(Ctx.pShader, Ctx.pModel, Ctx.pGI_Proxy, Ctx.iMesh, "g_NormalTexture",
			MTEX_TYPE::NORMALS, Layer.idx[ETOUI(MTEX_TYPE::NORMALS)], DEFAULT_TEXTURE::FLAT_NORMAL)))
			return E_FAIL;
		if (FAILED(Bind_TextureSafe(Ctx.pShader, Ctx.pModel, Ctx.pGI_Proxy, Ctx.iMesh, "g_MRATexture",
			MTEX_TYPE::METALNESS, Layer.idx[ETOUI(MTEX_TYPE::METALNESS)], DEFAULT_TEXTURE::MRA)))
			return E_FAIL;
		if (FAILED(Bind_TextureSafe(Ctx.pShader, Ctx.pModel, Ctx.pGI_Proxy, Ctx.iMesh, "g_UnknownTexture",
			MTEX_TYPE::UNKNOWN, Layer.idx[ETOUI(MTEX_TYPE::UNKNOWN)], DEFAULT_TEXTURE::BLACK)))
			return E_FAIL;
		if (FAILED(Bind_WorldCommonParams(Ctx)))
			return E_FAIL;

		const _uint iDiffuseCount = Ctx.pModel->Get_MeshTextureCount(Ctx.iMesh, MTEX_TYPE::DIFFUSE);
		const _uint iUnknownCount = Ctx.pModel->Get_MeshTextureCount(Ctx.iMesh, MTEX_TYPE::UNKNOWN);
		const _float fUseUnknown = (0u == iDiffuseCount && iUnknownCount > 0u) ? 1.f : 0.f;
		const _float fHasNormal = (Ctx.pModel->Get_MeshTextureCount(Ctx.iMesh, MTEX_TYPE::NORMALS) > 0u) ? 1.f : 0.f;
		const _float fHasMRA = (Ctx.pModel->Get_MeshTextureCount(Ctx.iMesh, MTEX_TYPE::METALNESS) > 0u) ? 1.f : 0.f;

		if (FAILED(Ctx.pShader->Bind_RawValue("g_fDecalUseUnknown", &fUseUnknown, sizeof(_float)))) return E_FAIL;
		if (FAILED(Ctx.pShader->Bind_RawValue("g_fDecalHasNormal", &fHasNormal, sizeof(_float)))) return E_FAIL;
		if (FAILED(Ctx.pShader->Bind_RawValue("g_fDecalHasMRA", &fHasMRA, sizeof(_float)))) return E_FAIL;

		pOutResult->iPass = (0u != Ctx.iFallbackPass) ? Ctx.iFallbackPass : ETOUI(WORLD_PASS::DECAL);
		return S_OK;
	}

	HRESULT Bind_MapParams(const MESH_LAYER_BIND_CONTEXT& Ctx)
	{
		const MESH_LAYER_IDX& Layer = *Ctx.pLayer;

		if (Layer.bUseLayerEx && Ctx.bUseLayerEx)
			return Bind_MapParams_Ex(Ctx);

		return Bind_MapParams_Legacy(Ctx);
	}

	HRESULT Bind_MapTextures(const MESH_LAYER_BIND_CONTEXT& Ctx)
	{
		const MESH_LAYER_IDX& Layer = *Ctx.pLayer;

		if (Layer.bUseLayerEx && Ctx.bUseLayerEx)
			return Bind_MapTextures_Ex(Ctx);

		return Bind_MapTextures_Legacy(Ctx);
	}

	HRESULT Bind_MapParams_Legacy(const MESH_LAYER_BIND_CONTEXT& Ctx)
	{
		const MESH_LAYER_IDX& Layer = *Ctx.pLayer;

		const _uint iUseLayerEx = 0u;
		if (FAILED(Ctx.pShader->Bind_RawValue("g_bUseLayerEx", &iUseLayerEx, sizeof(_uint))))
			return E_FAIL;

		const _uint iBase_UVIndex = (Layer.iUVIndex <= 3u) ? Layer.iUVIndex : 0u;
		const _uint iUnknown_UVIndex = (Layer.iUnknownUVIndex <= 3u) ? Layer.iUnknownUVIndex : 0u;
		const _uint iExtraR_UVIndex = (Layer.iExtraUVIndex[0] <= 3u) ? Layer.iExtraUVIndex[0] : 0u;
		const _uint iExtraG_UVIndex = (Layer.iExtraUVIndex[1] <= 3u) ? Layer.iExtraUVIndex[1] : 0u;
		const _uint iExtraB_UVIndex = (Layer.iExtraUVIndex[2] <= 3u) ? Layer.iExtraUVIndex[2] : 0u;
		const _uint iExtraA_UVIndex = (Layer.iExtraUVIndex[3] <= 3u) ? Layer.iExtraUVIndex[3] : 0u;

		const _float4 vUVTransform = Layer.bUseUVTransform
			? _float4{ Layer.vUVScale.x, Layer.vUVScale.y, Layer.vUVOffset.x, Layer.vUVOffset.y }
			: _float4{ 1.f, 1.f, 0.f, 0.f };

		const _float4 vUVTransformNormal = Layer.bUseUVTransform
			? _float4{ Layer.vUVScaleNormal.x, Layer.vUVScaleNormal.y, Layer.vUVOffset.x, Layer.vUVOffset.y }
			: _float4{ 1.f, 1.f, 0.f, 0.f };

		const _float4 vUVTransformMaterial = Layer.bUseUVTransform
			? _float4{ Layer.vUVScaleMaterial.x, Layer.vUVScaleMaterial.y, Layer.vUVOffset.x, Layer.vUVOffset.y }
			: _float4{ 1.f, 1.f, 0.f, 0.f };

		const _float fUVRotate = Layer.bUseUVTransform ? Layer.fUVRotate : 0.f;
		const _float fNormalStrength = Layer.fNormalStrength;
		const _float fMaskStrength = Layer.fMaskStrength;

		const _float4 vExtraEnable =
		{
				Layer.iExtraBind[0] >= 0 ? 1.f : 0.f,
				Layer.iExtraBind[1] >= 0 ? 1.f : 0.f,
				Layer.iExtraBind[2] >= 0 ? 1.f : 0.f,
				Layer.iExtraBind[3] >= 0 ? 1.f : 0.f
		};

		if (FAILED(Ctx.pShader->Bind_RawValue("g_iBase_UVIndex", &iBase_UVIndex, sizeof(_uint)))) return E_FAIL;
		if (FAILED(Ctx.pShader->Bind_RawValue("g_iUnknown_UVIndex", &iUnknown_UVIndex, sizeof(_uint)))) return E_FAIL;
		if (FAILED(Ctx.pShader->Bind_RawValue("g_iExtraR_UVIndex", &iExtraR_UVIndex, sizeof(_uint)))) return E_FAIL;
		if (FAILED(Ctx.pShader->Bind_RawValue("g_iExtraG_UVIndex", &iExtraG_UVIndex, sizeof(_uint)))) return E_FAIL;
		if (FAILED(Ctx.pShader->Bind_RawValue("g_iExtraB_UVIndex", &iExtraB_UVIndex, sizeof(_uint)))) return E_FAIL;
		if (FAILED(Ctx.pShader->Bind_RawValue("g_iExtraA_UVIndex", &iExtraA_UVIndex, sizeof(_uint)))) return E_FAIL;
		if (FAILED(Ctx.pShader->Bind_RawValue("g_vUVTransform", &vUVTransform, sizeof(_float4)))) return E_FAIL;
		if (FAILED(Ctx.pShader->Bind_RawValue("g_vUVTransformNormal", &vUVTransformNormal, sizeof(_float4)))) return E_FAIL;
		if (FAILED(Ctx.pShader->Bind_RawValue("g_vUVTransformMaterial", &vUVTransformMaterial, sizeof(_float4)))) return E_FAIL;
		if (FAILED(Ctx.pShader->Bind_RawValue("g_fUVRotate", &fUVRotate, sizeof(_float)))) return E_FAIL;
		if (FAILED(Ctx.pShader->Bind_RawValue("g_NormalStrength", &fNormalStrength, sizeof(_float)))) return E_FAIL;
		if (FAILED(Ctx.pShader->Bind_RawValue("g_MaskStrength", &fMaskStrength, sizeof(_float)))) return E_FAIL;
		if (FAILED(Ctx.pShader->Bind_RawValue("g_vExtraEnable", &vExtraEnable, sizeof(_float4)))) return E_FAIL;

		return S_OK;
	}

	HRESULT Bind_MapTextures_Legacy(const MESH_LAYER_BIND_CONTEXT& Ctx)
	{
		const MESH_LAYER_IDX& Layer = *Ctx.pLayer;

		if (FAILED(Bind_TextureSafe(Ctx.pShader, Ctx.pModel, Ctx.pGI_Proxy, Ctx.iMesh, "g_DiffuseTexture", MTEX_TYPE::DIFFUSE,
			Layer.idx[ETOUI(MTEX_TYPE::DIFFUSE)], DEFAULT_TEXTURE::MAGENTA))) return E_FAIL;
		if (FAILED(Bind_TextureSafe(Ctx.pShader, Ctx.pModel, Ctx.pGI_Proxy, Ctx.iMesh, "g_NormalTexture", MTEX_TYPE::NORMALS,
			Layer.idx[ETOUI(MTEX_TYPE::NORMALS)], DEFAULT_TEXTURE::FLAT_NORMAL))) return E_FAIL;
		if (FAILED(Bind_TextureSafe(Ctx.pShader, Ctx.pModel, Ctx.pGI_Proxy, Ctx.iMesh, "g_MRATexture", MTEX_TYPE::METALNESS,
			Layer.idx[ETOUI(MTEX_TYPE::METALNESS)], DEFAULT_TEXTURE::MRA))) return E_FAIL;
		if (FAILED(Bind_TextureSafe(Ctx.pShader, Ctx.pModel, Ctx.pGI_Proxy, Ctx.iMesh, "g_UnknownTexture", MTEX_TYPE::UNKNOWN,
			Layer.idx[ETOUI(MTEX_TYPE::UNKNOWN)], DEFAULT_TEXTURE::BLACK))) return E_FAIL;

		if (FAILED(Bind_MapExtraSlotSafe(Ctx, "g_ExtraRTexture", Layer.iExtraBind[0], static_cast<MTEX_TYPE>(Layer.iExtraTexType[0]),
			DEFAULT_TEXTURE::BLACK))) return E_FAIL;
		if (FAILED(Bind_MapExtraSlotSafe(Ctx, "g_ExtraGTexture", Layer.iExtraBind[1], static_cast<MTEX_TYPE>(Layer.iExtraTexType[1]),
			DEFAULT_TEXTURE::BLACK))) return E_FAIL;
		if (FAILED(Bind_MapExtraSlotSafe(Ctx, "g_ExtraBTexture", Layer.iExtraBind[2], static_cast<MTEX_TYPE>(Layer.iExtraTexType[2]),
			DEFAULT_TEXTURE::BLACK))) return E_FAIL;
		if (FAILED(Bind_MapExtraSlotSafe(Ctx, "g_ExtraATexture", Layer.iExtraBind[3], static_cast<MTEX_TYPE>(Layer.iExtraTexType[3]),
			DEFAULT_TEXTURE::BLACK))) return E_FAIL;

		return S_OK;
	}

	HRESULT Bind_MapParams_Ex(const MESH_LAYER_BIND_CONTEXT& Ctx)
	{
		const MESH_LAYER_IDX& Layer = *Ctx.pLayer;

		const _uint iUseLayerEx = Layer.bUseLayerEx ? 1u : 0u;
		_uint iUVIndex[MESH_LAYER_EX_GROUP_COUNT][MESH_LAYER_EX_ENTRY_COUNT] = {};
		_float4 vEnable[MESH_LAYER_EX_GROUP_COUNT] = {};
		_float4 vUVScale[MESH_LAYER_EX_GROUP_COUNT * MESH_LAYER_EX_ENTRY_COUNT] = {};
		_float4 vUVOffsetRotate[MESH_LAYER_EX_GROUP_COUNT * MESH_LAYER_EX_ENTRY_COUNT] = {};

		for (_uint g = 0; g < MESH_LAYER_EX_GROUP_COUNT; ++g)
		{
			float EnableValues[MESH_LAYER_EX_ENTRY_COUNT] = {};

			for (_uint e = 0; e < MESH_LAYER_EX_ENTRY_COUNT; ++e)
			{
				const MESH_LAYER_TEX_BIND_EX& Bind = Layer.LayerEx[g][e];
				const _uint iFlat = g * MESH_LAYER_EX_ENTRY_COUNT + e;

				iUVIndex[g][e] = (Bind.iUVIndex <= 3u) ? Bind.iUVIndex : 0u;
				EnableValues[e] = Bind.bEnable ? 1.f : 0.f;
				vUVScale[iFlat] = _float4(Bind.vUVScale.x, Bind.vUVScale.y, 0.f, 0.f);
				vUVOffsetRotate[iFlat] = _float4(Bind.vUVOffset.x, Bind.vUVOffset.y, Bind.fUVRotate, 0.f);
			}

			vEnable[g] = _float4(EnableValues[0], EnableValues[1], EnableValues[2], EnableValues[3]);
		}

		const _float fNormalStrength = Layer.fNormalStrength;
		const _float fMaskStrength = Layer.fMaskStrength;

		if (FAILED(Ctx.pShader->Bind_RawValue("g_bUseLayerEx", &iUseLayerEx, sizeof(_uint)))) return E_FAIL;
		if (FAILED(Ctx.pShader->Bind_RawValue("g_iLayerExUVIndex", iUVIndex, sizeof(iUVIndex)))) return E_FAIL;
		if (FAILED(Ctx.pShader->Bind_RawValue("g_vLayerExEnable", vEnable, sizeof(vEnable)))) return E_FAIL;
		if (FAILED(Ctx.pShader->Bind_RawValue("g_vLayerExUVScale", vUVScale, sizeof(vUVScale)))) return E_FAIL;
		if (FAILED(Ctx.pShader->Bind_RawValue("g_vLayerExUVOffsetRotate", vUVOffsetRotate, sizeof(vUVOffsetRotate)))) return E_FAIL;
		if (FAILED(Ctx.pShader->Bind_RawValue("g_NormalStrength", &fNormalStrength, sizeof(_float)))) return E_FAIL;
		if (FAILED(Ctx.pShader->Bind_RawValue("g_MaskStrength", &fMaskStrength, sizeof(_float)))) return E_FAIL;

		return S_OK;
	}

	HRESULT Bind_MapTextures_Ex(const MESH_LAYER_BIND_CONTEXT& Ctx)
	{
		const MESH_LAYER_IDX& Layer = *Ctx.pLayer;

		for (_uint g = 0; g < MESH_LAYER_EX_GROUP_COUNT; ++g)
		{
			for (_uint e = 0; e < MESH_LAYER_EX_ENTRY_COUNT; ++e)
			{
				if (FAILED(Bind_MapLayerExTextureSafe(Ctx, kLayerExTextureNames[g][e], Layer.LayerEx[g][e], GetLayerExDefaultTexture(e))))
					return E_FAIL;
			}
		}

		return S_OK;
	}

	HRESULT Bind_MapExtraSlotSafe(const MESH_LAYER_BIND_CONTEXT& Ctx, const _char* pName, int iSlot, MTEX_TYPE eType, DEFAULT_TEXTURE eDefault)
	{
		if (iSlot < 0)
			return Ctx.pGI_Proxy->Bind_DefaultTextureFromHub(Ctx.pShader, pName, eDefault);

		const _uint iTextureCount = Ctx.pModel->Get_MeshTextureCount(Ctx.iMesh, eType);

		if (iTextureCount > 0u)
		{
			const _uint iBindSlot = static_cast<_uint>(iSlot);
			const _uint iSafeSlot = (iBindSlot < iTextureCount) ? iBindSlot : (iTextureCount - 1u);

			if (SUCCEEDED(Ctx.pModel->Bind_Material(Ctx.pShader, pName, Ctx.iMesh, eType, iSafeSlot)))
				return S_OK;
		}

		return Ctx.pGI_Proxy->Bind_DefaultTextureFromHub(Ctx.pShader, pName, eDefault);
	}

	HRESULT Bind_MapLayerExTextureSafe(const MESH_LAYER_BIND_CONTEXT& Ctx, const _char* pName, const MESH_LAYER_TEX_BIND_EX& Bind, DEFAULT_TEXTURE eDefault)
	{
		if (!Bind.bEnable || Bind.iSlot < 0)
			return Ctx.pGI_Proxy->Bind_DefaultTextureFromHub(Ctx.pShader, pName, eDefault);

		const MTEX_TYPE eType = static_cast<MTEX_TYPE>(Bind.iTexType);
		const _uint iTextureCount = Ctx.pModel->Get_MeshTextureCount(Ctx.iMesh, eType);

		if (iTextureCount > 0u)
		{
			const _uint iBindSlot = static_cast<_uint>(Bind.iSlot);
			const _uint iSafeSlot = (iBindSlot < iTextureCount) ? iBindSlot : (iTextureCount - 1u);

			if (SUCCEEDED(Ctx.pModel->Bind_Material(Ctx.pShader, pName, Ctx.iMesh, eType, iSafeSlot)))
				return S_OK;
		}

		return Ctx.pGI_Proxy->Bind_DefaultTextureFromHub(Ctx.pShader, pName, eDefault);
	}
}

NS_END