#include "LD_DeformObject.h"
#include "LevelDesign_Registry.h"
#include "Parsing_Utils.h"
#include "GameContent_const.h"

#include "GameInstance.h"

namespace
{
	// 회전 & 빨려들어가는 물리 관련
	static constexpr _float s_fPullInitSpeed = { 4.f };
	static constexpr _float s_fPullAccel = { 18.f };
	static constexpr _float s_fAcquireDistance = { 1.2f };
	static constexpr _float s_fAlignRotSpeedDegree = { 72.f };
	static constexpr _float s_fPullTargetFwd = { 1.8f };
	static constexpr _float s_fReleaseFwd = { 3.f };

	struct LD_DEFORMOBJECT_CATALOG
	{
		const _tchar* pObjectName;
		const _tchar* pModelProtoTag;
		const _char* pModelPath;
		MODEL eModelType;
		DEFORM_TYPE eDeformType;
		DEFORM_OBJECT_KIND eKind;
		_float fInteractionRadius;
	};

	static const LD_DEFORMOBJECT_CATALOG g_DeformObjectCatalog[] =
	{
		{ L"DeformCar", L"Proto_Component_Model_DeformCar", "../../Resources/Map/Gimmick/Anim/DeformCar/DeformCar.ysh", MODEL::ANIM, DEFORM_TYPE::CAR, DEFORM_OBJECT_KIND::MOBILE, 6.5f },
		//{ L"DeformCylinder", L"Proto_Component_Model_DeformCylinder", "../../Resources/Map/Gimmick/Anim/DeformCylinder/DeformCylinder.ysh", MODEL::ANIM, DEFORM_TYPE::CYLINDER, DEFORM_OBJECT_KIND::MOBILE, 0.f},
		//{ L"DeformCoaster", L"Proto_Component_Model_DeformCoaster", "../../Resources/Map/Gimmick/Anim/DeformCoaster/DeformCoaster.ysh", MODEL::ANIM, DEFORM_TYPE::COASTER, DEFORM_OBJECT_KIND::FIXED, 0.f },
	};

	const LD_DEFORMOBJECT_CATALOG* Find_DeformObjectCatalog(const _wstring& strObjectName)
	{
		for (const LD_DEFORMOBJECT_CATALOG& Entry : g_DeformObjectCatalog)
		{
			if (JsonUtils::Equals_NoCase(Entry.pObjectName, strObjectName.c_str()))
				return &Entry;
		}

		return nullptr;
	}

	_bool Is_SupportedCatalog(const LD_DEFORMOBJECT_CATALOG& Entry)
	{
		return nullptr != Entry.pModelProtoTag
			&& nullptr != Entry.pModelPath
			&& MODEL::END != Entry.eModelType
			&& DEFORM_TYPE::NONE != Entry.eDeformType;
	}
}

NS_BEGIN(Client)

CLD_DeformObject::CLD_DeformObject(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CLD_EventObject(pDevice, pContext)
{
}

CLD_DeformObject::CLD_DeformObject(const CLD_DeformObject& Prototype)
	: CLD_EventObject(Prototype)
	, m_tDeformObjectDesc(Prototype.m_tDeformObjectDesc)
	, m_bAvailable(Prototype.m_bAvailable)
	, m_eState(Prototype.m_eState)
	, m_eKind(Prototype.m_eKind)
{
}

HRESULT CLD_DeformObject::Initialize(void* pArg)
{
	if (nullptr == pArg)
		return E_FAIL;

	m_tDeformObjectDesc = *static_cast<const LD_DEFORMOBJECT_DESC*>(pArg);

	const LD_DEFORMOBJECT_CATALOG* pCatalog = Find_DeformObjectCatalog(m_tDeformObjectDesc.strObjectName);
	if (nullptr == pCatalog)
		return E_FAIL;

	m_eKind = pCatalog->eKind;

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	m_bUseShadow = true;

	return S_OK;
}

HRESULT CLD_DeformObject::Validate_Initialized()
{
	if (FAILED(__super::Validate_Initialized()))
		return E_FAIL;

	const LD_DEFORMOBJECT_CATALOG* pCatalog = Find_DeformObjectCatalog(m_tDeformObjectDesc.strObjectName);
	if (nullptr == pCatalog || !Is_SupportedCatalog(*pCatalog))
		return E_FAIL;

	if (m_tDeformObjectDesc.wstrModelProtoTag != pCatalog->pModelProtoTag
		|| m_tDeformObjectDesc.eModelType != pCatalog->eModelType
		|| m_tDeformObjectDesc.eDeformType != pCatalog->eDeformType)
		return E_FAIL;

	if (m_tDeformObjectDesc.fInteractionRadius < 0.f)
		return E_FAIL;

	if (nullptr == m_pTrigger)
		return E_FAIL;

	if (!m_tDeformObjectDesc.bUseCollMesh || nullptr == m_pRigidStatic)
		return E_FAIL;

	return S_OK;
}

void CLD_DeformObject::Update(_float fTimeDelta)
{
	switch (m_eState)
	{
	case DEFORM_OBJECT_STATE::IDLE:
	{
		if (!m_bAvailable)
			return;

		__super::Update(fTimeDelta);
		break;
	}
	case DEFORM_OBJECT_STATE::CAPTURED:
	{
		Update_Captured(fTimeDelta);

		if (DEFORM_OBJECT_STATE::CAPTURED == m_eState)
			__super::Update(fTimeDelta);

		break;
	}
	case DEFORM_OBJECT_STATE::ACQUIRED:
		break;
	}
}

void CLD_DeformObject::Late_Update(_float fTimeDelta)
{
	if (m_bAvailable && m_pTrigger->Is_Enabled())
	{
		m_pTrigger->Update(XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));

#ifdef _DEBUG
		m_pGameInstance_Proxy->Add_DebugComponent(m_pTrigger);
#endif
	}

	__super::Late_Update(fTimeDelta);
}

void CLD_DeformObject::Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData)
{
	if (nullptr == pOutData)
		return;

	pOutData->strPrototypeTag = PROTOTYPE_TAG;
}

void CLD_DeformObject::Register_LevelDesignSpecs()
{
	for (const LD_DEFORMOBJECT_CATALOG& Entry : g_DeformObjectCatalog)
	{
		if (!Is_SupportedCatalog(Entry))
			continue;

		LD_SPAWN_SPEC Spec{};
		Spec.strObjectName = Entry.pObjectName;
		Spec.strPrototypeTag = PROTOTYPE_TAG;
		Spec.strLayerTag = LAYER_TAG;
		Spec.eCategory = LD_CATEGORY::GIMMICK;
		Spec.wstrModelProtoTag = Entry.pModelProtoTag;
		Spec.eModelType = Entry.eModelType;
		Spec.pPrototypeFactory = &Create_Prototype;
		Spec.pBuildDesc = &Build_Desc;
		Spec.ModelRequirements = { { Entry.pModelProtoTag, Entry.pModelPath, Entry.eModelType, true }, };

		CLevelDesign_Registry::Register(Spec.strObjectName, Spec);
	}
}

_bool CLD_DeformObject::Build_Desc(const LD_OBJECT_DESC& CommonDesc, const json& jEntry, const LD_SPAWN_SPEC& Spec, LD_OBJECT_ENTRY* pOutEntry)
{
	UNREFERENCED_PARAMETER(jEntry);

	if (nullptr == pOutEntry)
		return false;

	const LD_DEFORMOBJECT_CATALOG* pCatalog = Find_DeformObjectCatalog(CommonDesc.strObjectName);
	if (nullptr == pCatalog || !Is_SupportedCatalog(*pCatalog))
		return false;

	if (Spec.strObjectName != pCatalog->pObjectName
		|| Spec.strPrototypeTag != PROTOTYPE_TAG
		|| Spec.strLayerTag != LAYER_TAG
		|| Spec.eCategory != LD_CATEGORY::GIMMICK
		|| Spec.wstrModelProtoTag != pCatalog->pModelProtoTag
		|| Spec.eModelType != pCatalog->eModelType)
		return false;

	LD_DEFORMOBJECT_DESC Desc{};
	static_cast<LD_OBJECT_DESC&>(Desc) = CommonDesc;
	Desc.eCategory = Spec.eCategory;
	Desc.eModelType = pCatalog->eModelType;
	Desc.wstrModelProtoTag = pCatalog->pModelProtoTag;
	Desc.bUseCollMesh = true;
	Desc.strAnimEventFile.clear();
	Desc.eDeformType = pCatalog->eDeformType;
	Desc.fInteractionRadius = pCatalog->fInteractionRadius;

	*pOutEntry = Desc;
	return true;
}

CGameObject* CLD_DeformObject::Create_Prototype(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	return CLD_DeformObject::Create(pDevice, pContext);
}

HRESULT CLD_DeformObject::On_DeformAcquired()
{
	if (!m_bAvailable)
		return S_FALSE;

	m_bAvailable = false;
	Set_TriggerEnabled(false);
	Release_RigidStatic();
	Set_Active(false);

	return S_OK;
}

HRESULT CLD_DeformObject::On_DeformReleased(const _float3& vWorldPosition)
{
	if (m_bAvailable)
		return S_FALSE;

	m_pTransformCom->Set_State(STATE::POSITION,
		XMVectorSetW(XMLoadFloat3(&vWorldPosition), 1.f));

	if (FAILED(Ready_RigidStatic()))
		return E_FAIL;

	m_bAvailable = true;
	Set_Active(true);
	Set_TriggerEnabled(true);
	m_pTrigger->Update(XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));

	return S_OK;
}

#pragma region Deformable
_bool CLD_DeformObject::Request_Deform(const _float4x4* AnchorWorld)
{
	if (!m_bAvailable || DEFORM_OBJECT_STATE::IDLE != m_eState)
		return false;

	if (DEFORM_OBJECT_KIND::MOBILE != m_eKind)
		return false;

	m_eState = DEFORM_OBJECT_STATE::CAPTURED;
	m_AnchorWorld = *AnchorWorld;
	m_bAlignDone = false;

	Set_TriggerEnabled(false);
	Release_RigidStatic();

	m_fPullSpeed = s_fPullInitSpeed;
	m_pTransformCom->Set_RotationPerSec(s_fAlignRotSpeedDegree);

	return true;
}

void CLD_DeformObject::End_Deform(const _float4x4* AnchorWorld)
{
	if (DEFORM_OBJECT_STATE::ACQUIRED != m_eState)
		return;

	_matrix Anchor = XMLoadFloat4x4(AnchorWorld);

	_vector vLook = XMVectorSetY(Anchor.r[2], 0.f);
	if (XMVectorGetX(XMVector3LengthSq(vLook)) <= FLT_EPSILON)
		vLook = XMVectorSet(0.f, 0.f, 1.f, 0.f);
	vLook = XMVector3Normalize(vLook);

	const _vector vReleasePos = XMVectorSetW(Anchor.r[3] + vLook * s_fReleaseFwd, 1.f);

	m_pTransformCom->Set_State(STATE::POSITION, vReleasePos);
	m_pTransformCom->LookAt(vReleasePos + vLook);

	m_eState = DEFORM_OBJECT_STATE::IDLE;

	_float3 vPosition{};
	XMStoreFloat3(&vPosition, vReleasePos);
	On_DeformReleased(vPosition);
}
#pragma endregion

void CLD_DeformObject::Update_Captured(_float fTimeDelta)
{
	_matrix AnchorWorld = XMLoadFloat4x4(&m_AnchorWorld);
	_vector vAnchorPos = AnchorWorld.r[3];

	if (!m_bAlignDone)
	{
		m_bAlignDone = m_pTransformCom->LookAt_Smooth(vAnchorPos, fTimeDelta);
		return;
	}

	_vector vLook = XMVectorSetY(AnchorWorld.r[2], 0.f);
	if (XMVectorGetX(XMVector3LengthSq(vLook)) <= FLT_EPSILON)
		vLook = XMVectorSet(0.f, 0.f, 1.f, 0.f);
	vLook = XMVector3Normalize(vLook);

	_vector vSelf = m_pTransformCom->Get_State(STATE::POSITION);

	_vector vTarget = vAnchorPos + vLook * s_fPullTargetFwd;
	vTarget = XMVectorSetY(vTarget, XMVectorGetY(vSelf));
	vTarget = XMVectorSetW(vTarget, 1.f);

	_vector vDir = vTarget - vSelf;
	_float fDist = XMVectorGetX(XMVector3Length(vDir));

	if (fDist <= s_fAcquireDistance)
	{
		m_eState = DEFORM_OBJECT_STATE::ACQUIRED;
		On_DeformAcquired();

		DEFORM_ACQUIRED_EVENT Payload{};
		Payload.eType = Get_DeformType();
		Payload.pSource = this;
		m_pGameInstance_Proxy->Publish(EventTag::Deform_Acquired, &Payload);

		return;
	}

	m_fPullSpeed += s_fPullAccel * fTimeDelta;
	_float fMove = min(m_fPullSpeed * fTimeDelta, fDist);
	m_pTransformCom->Set_State(STATE::POSITION, vSelf + XMVector3Normalize(vDir) * fMove);
}

HRESULT CLD_DeformObject::Ready_Components()
{
	if (FAILED(__super::Ready_Components()))
		return E_FAIL;

	return Ready_Trigger();
}

void CLD_DeformObject::On_Deserialized()
{
	__super::On_Deserialized();

	if (!m_bAvailable)
		return;

	if (FAILED(Ready_RigidStatic()))
		return;

	m_pTrigger->Update(XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));
}

HRESULT CLD_DeformObject::Ready_Trigger()
{
	_float3 vMin = {};
	_float3 vMax = {};
	m_pModelCom->Get_ModelAABB(&vMin, &vMax);

	if (vMin.x > vMax.x || vMin.y > vMax.y || vMin.z > vMax.z)
		return E_FAIL;

	const _float3 vCenter = { (vMin.x + vMax.x) * 0.5f, (vMin.y + vMax.y) * 0.5f, (vMin.z + vMax.z) * 0.5f };
	const _float3 vHalfExtents = { (vMax.x - vMin.x) * 0.5f, (vMax.y - vMin.y) * 0.5f, (vMax.z - vMin.z) * 0.5f };
	const _float fBoundsRadius = XMVectorGetX(XMVector3Length(XMVectorSet(vHalfExtents.x, vHalfExtents.y, vHalfExtents.z, 0.f)));
	const _float fInteractionRadius = 0.f < m_tDeformObjectDesc.fInteractionRadius ? m_tDeformObjectDesc.fInteractionRadius : fBoundsRadius;

	if (fInteractionRadius <= 0.f)
		return E_FAIL;

	CCollider::COLLIDER_DESC ColliderDesc{};
	ColliderDesc.pOwner = this;
	ColliderDesc.vCenter = vCenter;
	ColliderDesc.fRadius = fInteractionRadius;

	m_pTrigger = Add_Component<CCollider>(Collider_Sphere.iLevelID, Collider_Sphere.szProtoTag,
		TEXT("Com_Trigger"), &ColliderDesc);
	if (nullptr == m_pTrigger)
		return E_FAIL;

	m_pGameInstance_Proxy->Register_Collider(m_pTrigger, ETOUI(COLLISION_LAYER::DEFORM_OBJECT));

	return S_OK;
}

void CLD_DeformObject::Set_TriggerEnabled(_bool bEnabled)
{
	if (nullptr == m_pTrigger)
		return;

	m_pTrigger->Set_Enabled(bEnabled);
}

CLD_DeformObject* CLD_DeformObject::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CLD_DeformObject* pInstance = new CLD_DeformObject(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CLD_DeformObject");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CLD_DeformObject::Clone(void* pArg)
{
	CLD_DeformObject* pInstance = new CLD_DeformObject(*this);

	LD_DEFORMOBJECT_DESC TempDesc{};
	if (nullptr == pArg)
	{
		const LD_DEFORMOBJECT_CATALOG* pCatalog = nullptr;
		for (const LD_DEFORMOBJECT_CATALOG& Entry : g_DeformObjectCatalog)
		{
			if (Is_SupportedCatalog(Entry))
			{
				pCatalog = &Entry;
				break;
			}
		}

		if (nullptr == pCatalog)
		{
			MSG_BOX("Failed to Cloned : CLD_DeformObject");
			Safe_Release(pInstance);
			return nullptr;
		}

		TempDesc.strObjectName = pCatalog->pObjectName;
		TempDesc.strKind = pCatalog->pObjectName;
		TempDesc.eCategory = LD_CATEGORY::GIMMICK;
		TempDesc.iModelProtoLevel = m_iPrototypeLevel;
		TempDesc.eModelType = pCatalog->eModelType;
		TempDesc.wstrModelProtoTag = pCatalog->pModelProtoTag;
		TempDesc.bUseCollMesh = true;
		TempDesc.strAnimEventFile.clear();
		TempDesc.eDeformType = pCatalog->eDeformType;
		TempDesc.fInteractionRadius = pCatalog->fInteractionRadius;

		pArg = &TempDesc;
	}

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CLD_DeformObject");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CLD_DeformObject::Free()
{
	__super::Free();
}

NS_END
