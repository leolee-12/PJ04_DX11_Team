#include "LD_DeformObject.h"
#include "LevelDesign_Registry.h"
#include "Parsing_Utils.h"
#include "GameContent_const.h"

#include "GameInstance.h"

namespace
{
	struct LD_DEFORMOBJECT_CATALOG
	{
		const _tchar* pObjectName;
		const _tchar* pModelProtoTag;
		const _char* pModelPath;
		MODEL eModelType;
		DEFORM_TYPE eDeformType;
		_float fInteractionRadius;
	};

	static const LD_DEFORMOBJECT_CATALOG g_DeformObjectCatalog[] =
	{
		{ L"DeformCar", L"Proto_Component_Model_DeformCar", "../../Resources/Map/Gimmick/Anim/DeformCar/DeformCar.ysh", MODEL::ANIM, DEFORM_TYPE::CAR, 0.f },
		//{ L"DeformCylinder", L"Proto_Component_Model_DeformCylinder", "../../Resources/Map/Gimmick/Anim/DeformCylinder/DeformCylinder.ysh", MODEL::ANIM, DEFORM_TYPE::CYLINDER, 0.f},
		//{ L"DeformCoaster", L"Proto_Component_Model_DeformCoaster", "../../Resources/Map/Gimmick/Anim/DeformCoaster/DeformCoaster.ysh", MODEL::ANIM, DEFORM_TYPE::COASTER, 0.f },
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
{
}

HRESULT CLD_DeformObject::Initialize(void* pArg)
{
	if (nullptr == pArg)
		return E_FAIL;

	m_tDeformObjectDesc = *static_cast<const LD_DEFORMOBJECT_DESC*>(pArg);

	return __super::Initialize(pArg);
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

	if (nullptr == m_pInteractionCollider)
		return E_FAIL;

	if (!m_tDeformObjectDesc.bUseCollMesh || nullptr == m_pRigidStatic)
		return E_FAIL;

	return S_OK;
}

void CLD_DeformObject::Update(_float fTimeDelta)
{
	if (!m_bAvailable)
		return;

	__super::Update(fTimeDelta);
}

void CLD_DeformObject::Late_Update(_float fTimeDelta)
{
	if (m_bAvailable && m_pInteractionCollider->Is_Enabled())
	{
		m_pInteractionCollider->Update(XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));

#ifdef _DEBUG
		m_pGameInstance_Proxy->Add_DebugComponent(m_pInteractionCollider);
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
	Set_InteractionEnabled(false);
	Release_RigidStatic();
	Set_Active(false);

	return S_OK;
}

HRESULT CLD_DeformObject::On_DeformReleased(const _float3& vWorldPosition)
{
	if (m_bAvailable)
		return S_FALSE;

	m_pTransformCom->Set_State(
		STATE::POSITION,
		XMVectorSetW(XMLoadFloat3(&vWorldPosition), 1.f));

	if (FAILED(Ready_RigidStatic()))
		return E_FAIL;

	m_bAvailable = true;
	Set_Active(true);
	Set_InteractionEnabled(true);
	m_pInteractionCollider->Update(XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));

	return S_OK;
}

HRESULT CLD_DeformObject::Ready_Components()
{
	if (FAILED(__super::Ready_Components()))
		return E_FAIL;

	return Ready_InteractionCollider();
}

HRESULT CLD_DeformObject::Ready_InteractionCollider()
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

	m_pInteractionCollider = Add_Component<CCollider>(Collider_Sphere.iLevelID, Collider_Sphere.szProtoTag,
		TEXT("Com_InteractionCollider"), &ColliderDesc);
	if (nullptr == m_pInteractionCollider)
		return E_FAIL;

	m_pGameInstance_Proxy->Register_Collider(m_pInteractionCollider, ETOUI(COLLISION_LAYER::ENV_TRIGGER));

	return S_OK;
}

void CLD_DeformObject::Set_InteractionEnabled(_bool bEnabled)
{
	if (nullptr == m_pInteractionCollider)
		return;

	m_pInteractionCollider->Set_Enabled(bEnabled);
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