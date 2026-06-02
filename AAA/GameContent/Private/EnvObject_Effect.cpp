#include "EnvObject_Effect.h"

NS_BEGIN(Client)

CEnvObject_Effect::CEnvObject_Effect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEnvObject(pDevice, pContext)
{
	m_strProtoTag = PROTOTYPE_TAG;
}

CEnvObject_Effect::CEnvObject_Effect(const CEnvObject_Effect& Prototype)
	: CEnvObject(Prototype)
{
	m_strProtoTag = PROTOTYPE_TAG;
}

HRESULT CEnvObject_Effect::Initialize_Prototype()
{
	return __super::Initialize_Prototype();
}

HRESULT CEnvObject_Effect::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (!m_tDesc.strModelProtoTag.empty())
	{
		if (FAILED(Ready_RenderComponents(m_tDesc.iModelProtoLevel, m_tDesc.strModelProtoTag)))
			return E_FAIL;
	}

	return S_OK;
}

void CEnvObject_Effect::Late_Update(_float fTimeDelta)
{
	UNREFERENCED_PARAMETER(fTimeDelta);
}

HRESULT CEnvObject_Effect::Render()
{
	if (!Has_RenderModel())
		return S_OK;

	return Render_Model();
}

CGameObject* CEnvObject_Effect::Clone(void* pArg)
{
	CEnvObject_Effect* pInstance = new CEnvObject_Effect(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CEnvObject_Effect");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CEnvObject_Effect::Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData)
{
	__super::Copy_PrototypeName(pOutData);
}

CEnvObject_Effect* CEnvObject_Effect::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CEnvObject_Effect* pInstance = new CEnvObject_Effect(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CEnvObject_Effect");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CEnvObject_Effect::Free()
{
	__super::Free();
}

NS_END
