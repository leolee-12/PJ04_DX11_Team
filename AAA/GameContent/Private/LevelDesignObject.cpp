#include "LevelDesignObject.h"

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
_bool CLevelDesignObject::Get_EditDesc(EDITABLE_DESC* pOutDesc) const
{
	if (nullptr == pOutDesc)
		return false;

	pOutDesc->eKind = EDITABLE_OBJECT_KIND::LEVEL_DESIGN_OBJECT;
	pOutDesc->strStableKey = m_tLevelDesignDesc.strSourceFile + L"|" + m_tLevelDesignDesc.strSection +
		L"|" + m_tLevelDesignDesc.strEntryKey + L"|" + to_wstring(m_tLevelDesignDesc.iUid);
	pOutDesc->iCapabilities = 0u;
	pOutDesc->Policy = {};
	pOutDesc->ModelSlots.clear();
	return true;
}

HRESULT CLevelDesignObject::Apply_EditPolicy(const EDIT_OBJECT_POLICY& Policy)
{
	UNREFERENCED_PARAMETER(Policy);
	return S_OK;
}

const MESH_LAYER_IDX* CLevelDesignObject::Get_EditMeshLayer(_uint iModelSlot, _uint iMesh) const
{
	UNREFERENCED_PARAMETER(iModelSlot);
	UNREFERENCED_PARAMETER(iMesh);
	return nullptr;
}

HRESULT CLevelDesignObject::Apply_EditMeshLayer(_uint iModelSlot, _uint iMesh, const MESH_LAYER_IDX&
	Layer)
{
	UNREFERENCED_PARAMETER(iModelSlot);
	UNREFERENCED_PARAMETER(iMesh);
	UNREFERENCED_PARAMETER(Layer);
	return E_FAIL;
}
#pragma endregion

void CLevelDesignObject::Free()
{
	__super::Free();
}

NS_END