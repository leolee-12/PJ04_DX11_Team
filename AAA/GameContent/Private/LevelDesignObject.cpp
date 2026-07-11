#include "LevelDesignObject.h"

#include "Model.h"

NS_BEGIN(Client)

CLevelDesignObject::CLevelDesignObject(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject(pDevice, pContext)
{
}

CLevelDesignObject::CLevelDesignObject(const CLevelDesignObject& Prototype)
	: CGameObject(Prototype)
	, m_tLevelDesignDesc(Prototype.m_tLevelDesignDesc)
{
}

HRESULT CLevelDesignObject::Initialize_Prototype()
{
	m_eProjType = PROJ_TYPE::PERSPEC;
	return S_OK;
}

HRESULT CLevelDesignObject::Initialize(void* pArg)
{
	if (nullptr == pArg)
		return E_FAIL;

	const LD_OBJECT_DESC* pDesc = static_cast<const LD_OBJECT_DESC*>(pArg);

	m_tLevelDesignDesc = *pDesc;

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	m_iMaterialID = 0;

	return S_OK;
}

HRESULT CLevelDesignObject::Validate_Initialized()
{
	if (nullptr == m_pGameInstance_Proxy || nullptr == m_pTransformCom)
		return E_FAIL;
	if (m_tLevelDesignDesc.strObjectName.empty())
		return E_FAIL;
	if (ETOUI(m_tLevelDesignDesc.eCategory) >= ETOUI(LD_CATEGORY::END))
		return E_FAIL;

	return S_OK;
}

_wstring CLevelDesignObject::Make_LevelDesignObjectKey() const
{
	if (m_tLevelDesignDesc.iUid != 0)
		return m_tLevelDesignDesc.strSection + L":" + to_wstring(m_tLevelDesignDesc.iUid);

	return m_tLevelDesignDesc.strSection
		+ L":"
		+ m_tLevelDesignDesc.strEntryKey
		+ L":"
		+ m_tLevelDesignDesc.strObjectName;
}

#pragma region Editable
void CLevelDesignObject::Add_EditModelSlot(vector<EDITABLE_MODEL_SLOT>* pOutSlots, const _tchar* pLabel, EDITABLE_MODEL_KIND eKind, CModel* pModel) const
{
	if (nullptr == pOutSlots || nullptr == pModel)
		return;

	EDITABLE_MODEL_SLOT Slot{};
	Slot.strLabel = nullptr != pLabel ? pLabel : L"Model";
	Slot.eKind = eKind;
	Slot.pModel = pModel;
	Slot.iMeshCount = static_cast<_uint>(pModel->Get_NumMeshes());

	pOutSlots->push_back(Slot);
}
void CLevelDesignObject::Build_EditCapabilities(_uint* pOutCaps, EDIT_OBJECT_POLICY* pOutPolicy) const
{
	if (nullptr != pOutCaps)
		*pOutCaps = 0u;

	if (nullptr != pOutPolicy)
		*pOutPolicy = {};
}

void CLevelDesignObject::Collect_EditModelSlots(vector<EDITABLE_MODEL_SLOT>* pOutSlots) const
{
	if (nullptr == pOutSlots)
		return;

	const auto& Components = Get_Components();
	const auto Iter = Components.find(TEXT("Com_Model"));
	if (Iter == Components.end())
		return;

	CModel* pModel = dynamic_cast<CModel*>(Iter->second);
	if (nullptr == pModel)
		return;

	const EDITABLE_MODEL_KIND eKind = 0u < pModel->Get_NumAnimations()
		? EDITABLE_MODEL_KIND::ANIM
		: EDITABLE_MODEL_KIND::NONANIM;

	Add_EditModelSlot(pOutSlots, TEXT("Model"), eKind, pModel);
}

HRESULT CLevelDesignObject::On_ApplyEditPolicy(const EDIT_OBJECT_POLICY& Policy)
{
	UNREFERENCED_PARAMETER(Policy);
	return S_OK;
}

_bool CLevelDesignObject::Get_EditDesc(EDITABLE_DESC* pOutDesc) const
{
	if (nullptr == pOutDesc)
		return false;

	*pOutDesc = {};
	pOutDesc->eKind = EDITABLE_OBJECT_KIND::LEVEL_DESIGN_OBJECT;
	pOutDesc->strStableKey = m_tLevelDesignDesc.strSourceFile + L"|" + m_tLevelDesignDesc.strSection + L"|" + m_tLevelDesignDesc.strEntryKey + L"|"
		+ to_wstring(m_tLevelDesignDesc.iUid);

	Build_EditCapabilities(&pOutDesc->iCapabilities, &pOutDesc->Policy);
	Collect_EditModelSlots(&pOutDesc->ModelSlots);

	if (!pOutDesc->ModelSlots.empty())
	{
		pOutDesc->iCapabilities |= EDIT_CAP_MESH_LAYER;

		for (const EDITABLE_MODEL_SLOT& Slot : pOutDesc->ModelSlots)
		{
			if (EDITABLE_MODEL_KIND::ANIM == Slot.eKind)
			{
				pOutDesc->iCapabilities |= EDIT_CAP_ANIMATION;
				break;
			}
		}
	}

	return true;
}

HRESULT CLevelDesignObject::Apply_EditPolicy(const EDIT_OBJECT_POLICY& Policy)
{
	return On_ApplyEditPolicy(Policy);
}

HRESULT CLevelDesignObject::On_EditTransformChanged()
{
	return S_OK;
}

const MESH_LAYER_IDX* CLevelDesignObject::Get_EditMeshLayer(_uint iModelSlot, _uint iMesh) const
{
	vector<EDITABLE_MODEL_SLOT> Slots;
	Collect_EditModelSlots(&Slots);

	if (iModelSlot >= Slots.size())
		return nullptr;

	CModel* pModel = Slots[iModelSlot].pModel;
	if (nullptr == pModel || iMesh >= pModel->Get_NumMeshes())
		return nullptr;

	return &pModel->Get_MeshLayer(iMesh);
}

HRESULT CLevelDesignObject::Apply_EditMeshLayer(_uint iModelSlot, _uint iMesh, const MESH_LAYER_IDX& Layer)
{
	vector<EDITABLE_MODEL_SLOT> Slots;
	Collect_EditModelSlots(&Slots);

	if (iModelSlot >= Slots.size())
		return E_FAIL;

	CModel* pModel = Slots[iModelSlot].pModel;
	if (nullptr == pModel || iMesh >= pModel->Get_NumMeshes())
		return E_FAIL;

	pModel->Set_MeshLayer(iMesh, Layer);
	return S_OK;
}
#pragma endregion

void CLevelDesignObject::Free()
{
	__super::Free();
}

NS_END