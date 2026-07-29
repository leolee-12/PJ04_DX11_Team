#include "EnvVolume_Light.h"

NS_BEGIN(Client)

CEnvVolume_Light::CEnvVolume_Light(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEnvObject_Trigger(pDevice, pContext)
	, m_fInTransitionSec{ 0.f }
	, m_fOutTransitionSec{ 0.f }
{
	m_strProtoTag = PROTOTYPE_TAG;
}

CEnvVolume_Light::CEnvVolume_Light(const CEnvVolume_Light& Prototype)
	: CEnvObject_Trigger(Prototype)
	, m_strAreaLightName{ Prototype.m_strAreaLightName }
	, m_fInTransitionSec{ Prototype.m_fInTransitionSec }
	, m_fOutTransitionSec{ Prototype.m_fOutTransitionSec }
{
	m_strProtoTag = PROTOTYPE_TAG;
}

HRESULT CEnvVolume_Light::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	m_strAreaLightName = m_tDesc.tEffect.strAreaLightName;
	m_fInTransitionSec = m_tDesc.tEffect.fInTransitionSec > 0.f ? m_tDesc.tEffect.fInTransitionSec : m_tDesc.tEffect.fTransitionSec;
	m_fOutTransitionSec = m_tDesc.tEffect.fOutTransitionSec > 0.f ? m_tDesc.tEffect.fOutTransitionSec : m_tDesc.tEffect.fTransitionSec;

	return S_OK;
}

#ifdef _DEBUG
_wstring CEnvVolume_Light::Get_DebugLabel() const
{
	const _wstring& strValue = !m_strAreaLightName.empty() ? m_strAreaLightName : m_strTriggerId;
	if (strValue.empty())
		return L"Light";

	return _wstring(L"Light : ") + strValue;
}
#endif

CEnvVolume_Light* CEnvVolume_Light::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CEnvVolume_Light* pInstance = new CEnvVolume_Light(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CEnvVolume_Light");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CEnvVolume_Light::Clone(void* pArg)
{
	CEnvVolume_Light* pInstance = new CEnvVolume_Light(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CEnvVolume_Light");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CEnvVolume_Light::Free()
{
	__super::Free();
}

NS_END