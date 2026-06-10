#include "UIPartObject.h"
#include "GameInstance.h"

CUIPartObject::CUIPartObject(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CUIObject(pDevice, pContext)
{
}

CUIPartObject::CUIPartObject(const CUIPartObject& Prototype)
	: CUIObject(Prototype)
{
}

HRESULT CUIPartObject::Initialize_Prototype()
{
	if (FAILED(__super::Initialize_Prototype()))
		return E_FAIL;

	return S_OK;
}

HRESULT CUIPartObject::Initialize(void* pArg)
{
	if (pArg)
	{
		auto pDesc = static_cast<UI_PARTOBJECT_DESC*>(pArg);
		m_pParentMatrix = pDesc->pParentMatrix;
	}

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	return S_OK;
}

void CUIPartObject::Priority_Update(_float fTimeDelta)
{
}

void CUIPartObject::Update(_float fTimeDelta)
{
}

void CUIPartObject::Late_Update(_float fTimeDelta)
{
}

HRESULT CUIPartObject::Render()
{
	return S_OK;
}

void CUIPartObject::Set_ParentMatrix(const _float4x4* pParentMatrix)
{
	m_pParentMatrix = pParentMatrix;
}

void CUIPartObject::Compute_CombinedWorldMatrix(_fmatrix ChildMatrix)
{
	XMStoreFloat4x4(&m_CombinedWorldMatrix, ChildMatrix * XMLoadFloat4x4(m_pParentMatrix));
}

void CUIPartObject::Free()
{
	__super::Free();
}
