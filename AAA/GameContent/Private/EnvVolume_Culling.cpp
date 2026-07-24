#include "EnvVolume_Culling.h"

NS_BEGIN(Client)

CEnvVolume_Culling::CEnvVolume_Culling(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEnvObject_Trigger(pDevice, pContext)
{
	m_strProtoTag = PROTOTYPE_TAG;
}

CEnvVolume_Culling::CEnvVolume_Culling(const CEnvVolume_Culling& Prototype)
	: CEnvObject_Trigger(Prototype)
	, m_strHideKind{ Prototype.m_strHideKind }
{
	m_strProtoTag = PROTOTYPE_TAG;
}

HRESULT CEnvVolume_Culling::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	m_strHideKind = m_tDesc.tEffect.strHideKind;

	return S_OK;
}

#ifdef _DEBUG
_wstring CEnvVolume_Culling::Get_DebugLabel() const
{
	const _wstring& strValue = !m_strHideKind.empty() ? m_strHideKind : m_strTriggerId;
	if (strValue.empty())
		return L"Culling";

	return _wstring(L"Culling : ") + strValue;
}
#endif

CEnvVolume_Culling* CEnvVolume_Culling::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CEnvVolume_Culling* pInstance = new CEnvVolume_Culling(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CEnvVolume_Culling");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CEnvVolume_Culling::Clone(void* pArg)
{
	CEnvVolume_Culling* pInstance = new CEnvVolume_Culling(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CEnvVolume_Culling");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CEnvVolume_Culling::Free()
{
	__super::Free();
}

NS_END