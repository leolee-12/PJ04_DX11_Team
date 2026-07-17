#include "CullingState.h"

#include "Culling_Manager.h"
#include "GameInstance_Proxy.h"
#include "Model.h"

namespace
{
	constexpr _float CULL_DISTANCE = 175.f;
	constexpr _float DISTANCE_FADE_WIDTH = 10.f;
}

CCullingState::CCullingState(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CComponent(pDevice, pContext)
{
}

CCullingState::CCullingState(const CCullingState& Prototype)
	: CComponent(Prototype)
	, m_LocalBounds(Prototype.m_LocalBounds)
	, m_bHasBounds(Prototype.m_bHasBounds)
	, m_bTransformDirty(true)
	, m_bRotationInvariant(Prototype.m_bRotationInvariant)
{
}

HRESULT CCullingState::Initialize_Prototype()
{
	Reset_AllResults();

	return S_OK;
}

HRESULT CCullingState::Initialize(void* pArg)
{
	Reset_AllResults();

	return S_OK;
}

HRESULT CCullingState::Set_LocalBounds(const BoundingBox& LocalBounds)
{
	if (!GeometryUtils::Is_ValidAABB(LocalBounds))
		return E_FAIL;

	m_LocalBounds = LocalBounds;
	m_bHasBounds = true;
	m_bRotationInvariant = false;
	Mark_TransformDirty();
	Reset_AllResults();

	return S_OK;
}

HRESULT CCullingState::Set_LocalBoundsFromModel(CModel* pModel, _float fMargin)
{
	if (nullptr == pModel)
		return E_FAIL;

	_float3 vMin{}, vMax{};
	pModel->Get_ModelAABB(&vMin, &vMax);

	if (!GeometryUtils::Is_ValidAABB(vMin, vMax))
		return E_FAIL;

	BoundingBox LocalBounds = GeometryUtils::Make_AABB_FromMinMax(vMin, vMax);

	if (!GeometryUtils::Expand_AABB(&LocalBounds, fMargin))
		return E_FAIL;

	return Set_LocalBounds(LocalBounds);
}

void CCullingState::Set_RotationInvariant(_bool bEnable)
{
	if (m_bRotationInvariant == bEnable)
		return;

	m_bRotationInvariant = bEnable;
	Mark_TransformDirty();
}

void CCullingState::Mark_TransformDirty()
{
	m_bTransformDirty = true;
}

void CCullingState::Refresh_WorldBounds(const _float4x4& WorldMatrix)
{
	if (!m_bHasBounds || !m_bTransformDirty)
		return;

	if (m_bRotationInvariant)
	{
		const _vector vRight = XMVectorSet(WorldMatrix._11, WorldMatrix._12, WorldMatrix._13, 0.f);
		const _vector vUp = XMVectorSet(WorldMatrix._21, WorldMatrix._22, WorldMatrix._23, 0.f);
		const _vector vLook = XMVectorSet(WorldMatrix._31, WorldMatrix._32, WorldMatrix._33, 0.f);

		const _float fScaleX = XMVectorGetX(XMVector3Length(vRight));
		const _float fScaleY = XMVectorGetX(XMVector3Length(vUp));
		const _float fScaleZ = XMVectorGetX(XMVector3Length(vLook));
		const _float fMaxScale = max(fScaleX, max(fScaleY, fScaleZ));
		const _float fRadius = (XMVectorGetX(XMVector3Length(XMLoadFloat3(&m_LocalBounds.Center)))
			+ XMVectorGetX(XMVector3Length(XMLoadFloat3(&m_LocalBounds.Extents)))) * fMaxScale;

		const _float3 vPosition = { WorldMatrix._41, WorldMatrix._42, WorldMatrix._43 };
		m_WorldBounds = BoundingBox(vPosition, _float3(fRadius, fRadius, fRadius));
	}
	else
	{
		m_LocalBounds.Transform(m_WorldBounds, XMLoadFloat4x4(&WorldMatrix));
	}

	m_WorldSphere.Center = m_WorldBounds.Center;
	m_WorldSphere.Radius = XMVectorGetX(XMVector3Length(XMLoadFloat3(&m_WorldBounds.Extents)));
	m_bTransformDirty = false;
}

void CCullingState::Evaluate(const CULLING_EVALUATION_INPUT& Desc)
{
	Reset_AllResults();

	const _bool bEvaluateMainDistance = Desc.bEvaluateMain && Desc.Main.bUseDistance;
	const _bool bEvaluateShadowDistance = Desc.bEvaluateShadow && Desc.Shadow.bUseDistance;

	CULLING_FADE_RESULT DistanceResult{};
	_bool bHasDistanceResult = false;

	if (bEvaluateMainDistance || bEvaluateShadowDistance)
	{
		DistanceResult = m_pGameInstance_Proxy->Evaluate_DistanceFade(
			m_WorldSphere,
			CULL_DISTANCE,
			DISTANCE_FADE_WIDTH);

		bHasDistanceResult = FLT_MAX != DistanceResult.fBoundaryDistance;
	}

	if (Desc.bEvaluateMain)
		Evaluate_Channel(CHANNEL::MAIN, Desc.Main, bEvaluateMainDistance && bHasDistanceResult ? &DistanceResult : nullptr);

	if (Desc.bEvaluateShadow)
		Evaluate_Channel(CHANNEL::SHADOW, Desc.Shadow, bEvaluateShadowDistance && bHasDistanceResult ? &DistanceResult : nullptr);
}

void CCullingState::Reset_Channel(CHANNEL eChannel)
{
	if (ETOUI(eChannel) >= ETOUI(CHANNEL::COUNT))
		return;

	m_Results[ETOUI(eChannel)] = {};
}

void CCullingState::Reset_AllResults()
{
	for (_uint i = 0; i < ETOUI(CHANNEL::COUNT); ++i)
		m_Results[i] = {};
}

_bool CCullingState::Has_Bounds() const
{
	return m_bHasBounds;
}

_bool CCullingState::Is_Culled(CHANNEL eChannel) const
{
	if (ETOUI(eChannel) >= ETOUI(CHANNEL::COUNT))
		return false;

	return m_Results[ETOUI(eChannel)].bCulled;
}

_float CCullingState::Get_Dissolve(CHANNEL eChannel) const
{
	if (ETOUI(eChannel) >= ETOUI(CHANNEL::COUNT))
		return 0.f;

	return m_Results[ETOUI(eChannel)].fDissolve;
}

const BoundingBox& CCullingState::Get_LocalBounds() const
{
	return m_LocalBounds;
}

const BoundingBox& CCullingState::Get_WorldBounds() const
{
	return m_WorldBounds;
}

void CCullingState::Evaluate_Channel(CHANNEL eChannel, const CULLING_CHANNEL_QUERY& Query, const CULLING_FADE_RESULT* pDistanceResult)
{
	CHANNEL_RESULT& Result = m_Results[ETOUI(eChannel)];

	if (nullptr != pDistanceResult)
	{
		Result.fDissolve = max(Result.fDissolve, pDistanceResult->fDissolve);

		if (pDistanceResult->bCulled || pDistanceResult->fDissolve > 0.f)
			Result.iCauseFlags |= CAUSE_DISTANCE;

		if (pDistanceResult->bCulled)
		{
			Result.bCulled = true;
			Result.fDissolve = 1.f;
			return;
		}
	}

	if (!Query.bUseFrustum)
		return;

	if (Query.bUseFrustumFade)
	{
		const CULLING_FADE_RESULT FrustumResult =
			m_pGameInstance_Proxy->Evaluate_FrustumFadeAABB(
				Query.eView,
				m_WorldBounds,
				CCulling_Manager::CULLING_PLANE_MASK_MAIN_SIDE);

		Result.fDissolve = max(Result.fDissolve, FrustumResult.fDissolve);

		if (FrustumResult.bCulled || FrustumResult.fDissolve > 0.f)
			Result.iCauseFlags |= CAUSE_FRUSTUM;

		if (FrustumResult.bCulled)
		{
			Result.bCulled = true;
			Result.fDissolve = 1.f;
		}

		return;
	}

	if (m_pGameInstance_Proxy->Should_CullAABB(Query.eView, m_WorldBounds))
	{
		Result.bCulled = true;
		Result.fDissolve = 1.f;
		Result.iCauseFlags |= CAUSE_FRUSTUM;
	}
}

CCullingState* CCullingState::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CCullingState* pInstance = new CCullingState(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CCullingState");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CComponent* CCullingState::Clone(void* pArg)
{
	CCullingState* pInstance = new CCullingState(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CCullingState");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CCullingState::Free()
{
	__super::Free();
}