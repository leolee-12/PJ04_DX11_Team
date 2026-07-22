#include "MapGimmickSection.h"
#include "GameContent_Events.h"
#include "Shader_PassMeta.h"
#include "Effect_Loader.h"

#include "GameInstance.h"
#include "Geometry_Utils.h"
#include "GameContent_Log.h"

namespace
{
	string Get_FragmentNameFromMeshName(const string& strMeshName)
	{
		const size_t iSep = strMeshName.find("__");
		if (string::npos == iSep)
			return strMeshName;

		return strMeshName.substr(0, iSep);
	}
}

NS_BEGIN(Client)

CMapGimmickSection::CMapGimmickSection(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CMapObject{ pDevice, pContext }
{
}

CMapGimmickSection::CMapGimmickSection(const CMapGimmickSection& Prototype)
	: CMapObject(Prototype)
{
}

HRESULT CMapGimmickSection::Initialize(void* pArg)
{
	if (nullptr == pArg)
		return E_FAIL;

	const MAP_GIMMICK_SECTION_DESC* pDesc = static_cast<const MAP_GIMMICK_SECTION_DESC*>(pArg);
	const MAP_GIMMICK_SECTION_ENTRY* pEntry = pDesc->pEntry;

	if (nullptr == pEntry
		|| nullptr == pEntry->pStageName
		|| nullptr == pEntry->pSectionName
		|| nullptr == pEntry->pShellSectionName
		|| nullptr == pEntry->pModelProtoTag
		|| nullptr == pEntry->pModelPath
		|| nullptr == pEntry->pObjectTag
		|| nullptr == pEntry->pBreakEffectID
		|| nullptr == pEntry->pBreakSFX
		|| nullptr == pEntry->pFragments
		|| 0 == pEntry->iNumFragments)
	{
		Log_GameContentWarning("MapGimmickSection entry invalid");
		return E_FAIL;
	}

	for (_uint i = 0; i < pEntry->iNumFragments; ++i)
	{
		if (nullptr == pEntry->pFragments[i].pFragmentName)
		{
			Log_GameContentWarning("MapGimmickSection fragment invalid: " + WstrToStr(pEntry->pSectionName));
			return E_FAIL;
		}
	}

	m_pEntry = pEntry;
	m_iModelProtoLevel = pDesc->iModelProtoLevel;
	m_bRenderable = pDesc->bRenderable;

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_Fragments()))
		return E_FAIL;

	if (FAILED(Ready_Trigger()))
		return E_FAIL;

	if (FAILED(Validate_Initialized()))
		return E_FAIL;

	return S_OK;
}

HRESULT CMapGimmickSection::Validate_Initialized()
{
	if (FAILED(__super::Validate_Initialized()))
		return E_FAIL;

	if (nullptr == m_pEntry || nullptr == m_pTrigger)
		return E_FAIL;
	if (BREAK_STATE::INTACT != m_eBreakState)
		return E_FAIL;
	if (m_Fragments.empty())
		return E_FAIL;
	if (static_cast<_uint>(m_Fragments.size()) != m_pEntry->iNumFragments)
		return E_FAIL;

	const _uint iNumMeshes = static_cast<_uint>(m_pModelCom->Get_NumMeshes());
	if (m_MeshFragmentIndices.size() != iNumMeshes)
		return E_FAIL;

	const _uint iNumFragments = static_cast<_uint>(m_Fragments.size());
	for (const _uint iFragment : m_MeshFragmentIndices)
	{
		if (INVALID_FRAGMENT_INDEX == iFragment)
			continue;
		if (iFragment >= iNumFragments)
			return E_FAIL;
	}

	for (const BREAK_FRAGMENT& Fragment : m_Fragments)
	{
		if (Fragment.strFragmentName.empty())
			return E_FAIL;
		if (Fragment.MeshIndices.empty())
			return E_FAIL;
	}

	return S_OK;
}

void CMapGimmickSection::Update(_float fTimeDelta)
{
	if (BREAK_STATE::BREAKING == m_eBreakState)
		Update_Fragments(fTimeDelta);
}

void CMapGimmickSection::Late_Update(_float fTimeDelta)
{
	UNREFERENCED_PARAMETER(fTimeDelta);

	if (BREAK_STATE::INTACT == m_eBreakState)
		Update_Trigger();

	if (!m_bRenderable)
		return;

	if (BREAK_STATE::INTACT == m_eBreakState
		|| BREAK_STATE::BREAKING == m_eBreakState)
	{
		m_pGameInstance_Proxy->Add_RenderGroup(RENDERID::NONBLEND, this);
		m_pGameInstance_Proxy->Add_RenderGroup(RENDERID::SHADOW, this);
	}
}

HRESULT CMapGimmickSection::Render()
{
	if (BREAK_STATE::INTACT != m_eBreakState
		&& BREAK_STATE::BREAKING != m_eBreakState)
	{
		return S_OK;
	}

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::VIEW, m_eProjType))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::PROJ, m_eProjType))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_iMaterialID", &m_iMaterialID, sizeof(_uint))))
		return E_FAIL;

	const _uint iNumMeshes = static_cast<_uint>(m_pModelCom->Get_NumMeshes());
	const _matrix BreakWallWorld = XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr());

	for (_uint i = 0; i < iNumMeshes; ++i)
	{
		const BREAK_FRAGMENT* pFragment = Find_Fragment(i);
		if (nullptr == pFragment)
			continue;
		if (BREAK_STATE::BREAKING == m_eBreakState && !pFragment->bActive)
			continue;

		const _float4x4 WorldMatrix = Build_FragmentWorldMatrix(*pFragment, BreakWallWorld);

		if (FAILED(Render_MapMesh(i, &WorldMatrix)))
			return E_FAIL;
	}

	return S_OK;
}

HRESULT CMapGimmickSection::Render_Shadow()
{
	if (!m_bRenderable)
		return S_OK;

	if (BREAK_STATE::INTACT != m_eBreakState
		&& BREAK_STATE::BREAKING != m_eBreakState)
	{
		return S_OK;
	}

	if (FAILED(Bind_ShadowTransforms()))
		return E_FAIL;

	const _uint iNumMeshes = static_cast<_uint>(m_pModelCom->Get_NumMeshes());
	const _matrix BreakWallWorld = XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr());

	for (_uint i = 0; i < iNumMeshes; ++i)
	{
		const BREAK_FRAGMENT* pFragment = Find_Fragment(i);
		if (nullptr == pFragment)
			continue;
		if (BREAK_STATE::BREAKING == m_eBreakState && !pFragment->bActive)
			continue;

		const _float4x4 WorldMatrix = Build_FragmentWorldMatrix(*pFragment, BreakWallWorld);

		if (FAILED(Render_ShadowMesh(i, &WorldMatrix)))
			return E_FAIL;
	}

	return S_OK;
}

void CMapGimmickSection::Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData)
{
	if (nullptr == pOutData)
		return;

	pOutData->strPrototypeTag = PROTOTYPE_TAG;
}

const _tchar* CMapGimmickSection::Get_ModelProtoTag() const
{
	return m_pEntry->pModelProtoTag;
}

_uint CMapGimmickSection::Get_ModelProtoLevel() const
{
	return m_iModelProtoLevel;
}

HRESULT CMapGimmickSection::Ready_Fragments()
{
	m_Fragments.clear();
	m_MeshFragmentIndices.clear();

	if (nullptr == m_pEntry || nullptr == m_pEntry->pFragments || 0 == m_pEntry->iNumFragments)
		return E_FAIL;
	if (nullptr == m_pModelCom)
		return E_FAIL;

	const _uint iNumMeshes = static_cast<_uint>(m_pModelCom->Get_NumMeshes());
	m_MeshFragmentIndices.assign(iNumMeshes, INVALID_FRAGMENT_INDEX);
	m_Fragments.reserve(m_pEntry->iNumFragments);

	for (_uint i = 0; i < m_pEntry->iNumFragments; ++i)
	{
		const MAP_GIMMICK_FRAGMENT& Bind = m_pEntry->pFragments[i];

		BREAK_FRAGMENT Fragment{};
		Fragment.strFragmentName = Bind.pFragmentName;
		Fragment.vPivot = Bind.vPivot;
		m_Fragments.push_back(Fragment);
	}

	for (_uint i = 0; i < iNumMeshes; ++i)
	{
		const string strFragmentName = Get_FragmentNameFromMeshName(m_pModelCom->Get_MeshName(i));
		auto iter = find_if(m_Fragments.begin(), m_Fragments.end(),
			[&](const BREAK_FRAGMENT& Fragment)->_bool
			{
				return Fragment.strFragmentName == strFragmentName;
			});

		if (m_Fragments.end() == iter)
			continue;

		const _uint iFragment = static_cast<_uint>(iter - m_Fragments.begin());
		iter->MeshIndices.push_back(i);
		m_MeshFragmentIndices[i] = iFragment;
	}

	for (const BREAK_FRAGMENT& Fragment : m_Fragments)
	{
		if (!Fragment.MeshIndices.empty())
			continue;

		Log_GameContentWarning("MapGimmickSection fragment missing: " + WstrToStr(m_pEntry->pSectionName) + "/" + Fragment.strFragmentName);
		return E_FAIL;
	}

	return S_OK;
}

const CMapGimmickSection::BREAK_FRAGMENT* CMapGimmickSection::Find_Fragment(_uint iMesh) const
{
	const _uint iFragment = m_MeshFragmentIndices[iMesh];
	if (INVALID_FRAGMENT_INDEX == iFragment)
		return nullptr;

	return &m_Fragments[iFragment];
}

HRESULT CMapGimmickSection::Ready_Trigger()
{
	if (m_Fragments.empty())
		return E_FAIL;

	const _float3 vFirstPivot = GeometryUtils::Flip_Axis(STATE::LOOK, m_Fragments.front().vPivot);
	_float3 vMin = vFirstPivot;
	_float3 vMax = vFirstPivot;

	for (const BREAK_FRAGMENT& Fragment : m_Fragments)
	{
		const _float3 vPivot = GeometryUtils::Flip_Axis(STATE::LOOK, Fragment.vPivot);

		vMin.x = min(vMin.x, vPivot.x);
		vMin.y = min(vMin.y, vPivot.y);
		vMin.z = min(vMin.z, vPivot.z);

		vMax.x = max(vMax.x, vPivot.x);
		vMax.y = max(vMax.y, vPivot.y);
		vMax.z = max(vMax.z, vPivot.z);
	}

	BoundingBox TriggerBounds = GeometryUtils::Make_AABB_FromMinMax(vMin, vMax);

	if (!GeometryUtils::Expand_AABB(&TriggerBounds, m_pEntry->fTriggerPadding))
	{
		Log_GameContentWarning("MapGimmickSection trigger invalid: " + WstrToStr(m_pEntry->pSectionName));
		return E_FAIL;
	}

	CCollider::COLLIDER_DESC ColliderDesc{};
	ColliderDesc.pOwner = this;
	ColliderDesc.vCenter = TriggerBounds.Center;
	ColliderDesc.vSize = {
			TriggerBounds.Extents.x * 2.f,
			TriggerBounds.Extents.y * 2.f,
			TriggerBounds.Extents.z * 2.f
	};

	m_pTrigger = Add_Component<CCollider>(L"Com_Trigger", CCollider::Create(m_pDevice, m_pContext, COLLIDER::AABB));
	if (nullptr == m_pTrigger)
		return E_FAIL;

	if (FAILED(m_pTrigger->Initialize(&ColliderDesc)))
		return E_FAIL;

	m_pTrigger->Set_OnEnter([this](CCollider* pOther) { On_TriggerEnter(pOther); });
	m_pGameInstance_Proxy->Register_Collider(m_pTrigger, ETOUI(COLLISION_LAYER::ENV_TRIGGER));

	return S_OK;
}

void CMapGimmickSection::On_TriggerEnter(CCollider* pOther)
{
	if (ETOUI(m_pEntry->eTriggerLayer) != pOther->Get_RegisteredGroup())
		return;

	Start_Break();
}

void CMapGimmickSection::Update_Trigger()
{
	m_pTrigger->Update(XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));

#ifdef _DEBUG
	m_pGameInstance_Proxy->Add_DebugComponent(m_pTrigger);
#endif
}

void CMapGimmickSection::Update_Fragments(_float fTimeDelta)
{
	_bool bAnyActive = false;
	const MAP_GIMMICK_SCATTER& Scatter = m_pEntry->Scatter;

	for (BREAK_FRAGMENT& Fragment : m_Fragments)
	{
		if (!Fragment.bActive)
			continue;

		Fragment.vVelocity.y += Scatter.fGravity * fTimeDelta;
		Fragment.vOffset.x += Fragment.vVelocity.x * fTimeDelta;
		Fragment.vOffset.y += Fragment.vVelocity.y * fTimeDelta;
		Fragment.vOffset.z += Fragment.vVelocity.z * fTimeDelta;

		_vector vAngularVelocity = XMLoadFloat3(&Fragment.vAngularVelocity);
		const _float fAngularSpeed = XMVectorGetX(XMVector3Length(vAngularVelocity));

		if (fAngularSpeed > 0.f)
		{
			_vector vAxis = XMVector3Normalize(vAngularVelocity);
			_vector vDeltaRotation = XMQuaternionRotationAxis(vAxis, fAngularSpeed * fTimeDelta);
			_vector vRotation = XMQuaternionNormalize(XMQuaternionMultiply(XMLoadFloat4(&Fragment.vRotation), vDeltaRotation));
			XMStoreFloat4(&Fragment.vRotation, vRotation);
		}

		if (Fragment.vOffset.y <= -Scatter.fDespawnFallDistance)
		{
			Fragment.bActive = false;
			continue;
		}

		bAnyActive = true;
	}

	if (!bAnyActive)
		m_eBreakState = BREAK_STATE::BROKEN;
}

void CMapGimmickSection::Start_Break()
{
	if (BREAK_STATE::INTACT != m_eBreakState)
		return;

	m_eBreakState = BREAK_STATE::BREAKING;
	m_bRenderable = true;
	m_pTrigger->Set_Enabled(false);

	if (!m_Fragments.empty())
	{
		const _float3 vFirstPivot = GeometryUtils::Flip_Axis(STATE::LOOK, m_Fragments.front().vPivot);
		_float3 vMin = vFirstPivot;
		_float3 vMax = vFirstPivot;

		for (const BREAK_FRAGMENT& Fragment : m_Fragments)
		{
			const _float3 vPivot = GeometryUtils::Flip_Axis(STATE::LOOK, Fragment.vPivot);

			vMin.x = min(vMin.x, vPivot.x); vMin.y = min(vMin.y, vPivot.y); vMin.z = min(vMin.z, vPivot.z);
			vMax.x = max(vMax.x, vPivot.x); vMax.y = max(vMax.y, vPivot.y); vMax.z = max(vMax.z, vPivot.z);
		}

		const _float3 vLocalEffectPosition = {
				(vMin.x + vMax.x) * 0.5f,
				vMin.y + (vMax.y - vMin.y) * m_pEntry->fEffectHeightRatio,
				vMin.z + (vMax.z - vMin.z) * m_pEntry->fEffectFrontRatio
		};

		_float3 vEffectPosition{};
		XMStoreFloat3(&vEffectPosition, XMVector3TransformCoord(
			XMLoadFloat3(&vLocalEffectPosition),
			XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr())));

		CEffect_Loader::GetInstance()->Spawn(m_pEntry->pBreakEffectID, Get_LevelIndex(), vEffectPosition);
	}

	m_pGameInstance_Proxy->Play_SFX(m_pEntry->pBreakSFX, m_pEntry->fBreakSFXVolume, ESoundBus::SFX);

	MAP_GIMMICK_BREAK_EVENT BreakEvent{};
	BreakEvent.pEntry = m_pEntry;
	m_pGameInstance_Proxy->Publish(EventTag::MapGimmick_SectionBreak, &BreakEvent);

	CAMERA_SHAKE_DESC ShakeDesc{};
	ShakeDesc.fTrauma = m_pEntry->fShakeTrauma;
	m_pGameInstance_Proxy->Publish(EventTag::Camera_Shake, &ShakeDesc);
	m_pGameInstance_Proxy->Lerp_TimeScale(0.f, 1.f, 0.2f);

	const MAP_GIMMICK_SCATTER& Scatter = m_pEntry->Scatter;

	for (_uint i = 0; i < static_cast<_uint>(m_Fragments.size()); ++i)
	{
		BREAK_FRAGMENT& Fragment = m_Fragments[i];

		const _float fAngle = static_cast<_float>(i) * Scatter.fAngleStep;
		const _float fSpeed = Scatter.fBaseSpeed + static_cast<_float>(i % 5) * Scatter.fSpeedStep;

		Fragment.bActive = true;
		Fragment.vOffset = {};
		Fragment.vRotation = { 0.f, 0.f, 0.f, 1.f };
		Fragment.vVelocity = {
				cosf(fAngle) * fSpeed,
				Scatter.fBaseUpSpeed + static_cast<_float>(i % 4) * Scatter.fUpSpeedStep,
				sinf(fAngle) * fSpeed
		};
		Fragment.vAngularVelocity = {
				Scatter.vAngularBase.x + static_cast<_float>(i % 3) * Scatter.fAngularStep,
				Scatter.vAngularBase.y + static_cast<_float>(i % 5) * Scatter.fAngularStep,
				Scatter.vAngularBase.z + static_cast<_float>(i % 4) * Scatter.fAngularStep
		};
	}
}

_float4x4 CMapGimmickSection::Build_FragmentWorldMatrix(const BREAK_FRAGMENT& Fragment, _fmatrix BreakWallWorld) const
{
	const _float3 vEnginePivot = GeometryUtils::Flip_Axis(STATE::LOOK, Fragment.vPivot);

	const _matrix Rotation = XMMatrixRotationQuaternion(XMLoadFloat4(&Fragment.vRotation));
	const _matrix Translation = XMMatrixTranslation(
		vEnginePivot.x + Fragment.vOffset.x,
		vEnginePivot.y + Fragment.vOffset.y,
		vEnginePivot.z + Fragment.vOffset.z);

	_float4x4 WorldMatrix{};
	XMStoreFloat4x4(&WorldMatrix, Rotation * Translation * BreakWallWorld);
	return WorldMatrix;
}

CMapGimmickSection* CMapGimmickSection::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CMapGimmickSection* pInstance = new CMapGimmickSection(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CMapGimmickSection");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CMapGimmickSection::Clone(void* pArg)
{
	CMapGimmickSection* pInstance = new CMapGimmickSection(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CMapGimmickSection");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CMapGimmickSection::Free()
{
	__super::Free();
}

NS_END