#include "EnvVolume_Effect.h"
#include "Effect_Loader.h"
#include "Effect_Container.h"

NS_BEGIN(Client)

CEnvVolume_Effect::CEnvVolume_Effect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEnvObject_Trigger(pDevice, pContext)
	, m_vEmitPos{}
{
	m_strProtoTag = PROTOTYPE_TAG;
}

CEnvVolume_Effect::CEnvVolume_Effect(const CEnvVolume_Effect& Prototype)
	: CEnvObject_Trigger(Prototype)
	, m_strEffectId{ Prototype.m_strEffectId }
	, m_strEffectKind{ Prototype.m_strEffectKind }
	, m_vEmitPos{ Prototype.m_vEmitPos }
{
	m_strProtoTag = PROTOTYPE_TAG;
}

HRESULT CEnvVolume_Effect::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	m_strEffectKind = !m_tDesc.tEffect.strKind.empty() ? m_tDesc.tEffect.strKind : m_tDesc.wstrObjectName;
	m_vEmitPos = m_tDesc.tEffect.vEmitPos;
	m_bActive = false;
	m_pSpawnedEffect = nullptr;

	return S_OK;
}

void CEnvVolume_Effect::OnTriggerEnter(CCollider* pOther)
{
	if (false == Is_PlayerActivator(pOther))
		return;

	if (true == m_bActive || true == m_strEffectId.empty())
		return;

	_float3 vSpawnPosition{};
	XMStoreFloat3(&vSpawnPosition, m_pTransformCom->Get_State(STATE::POSITION));

	if (FAILED(CEffect_Loader::GetInstance()->Spawn(
		m_strEffectId,
		Get_LevelIndex(),
		vSpawnPosition,
		_float3(0.f, 0.f, 0.f),
		_float3(0.f, 0.f, 0.f),
		nullptr,
		&m_pSpawnedEffect)))
	{
		m_pSpawnedEffect = nullptr;
		return;
	}

	m_bActive = true;
}

void CEnvVolume_Effect::OnTriggerExit(CCollider* pOther)
{
	if (false == Is_PlayerActivator(pOther))
		return;

	if (false == m_bActive)
		return;

	if (nullptr != m_pSpawnedEffect)
		m_pSpawnedEffect->EffectContainer_StopAfterEmission();

	m_pSpawnedEffect = nullptr;
	m_bActive = false;
}

#ifdef _DEBUG
_wstring CEnvVolume_Effect::Get_DebugLabel() const
{
	const _wstring& strValue = !m_strEffectKind.empty() ? m_strEffectKind : m_strTriggerId;
	if (strValue.empty())
		return L"Effect";

	return _wstring(L"Effect : ") + strValue;
}
#endif

CEnvVolume_Effect* CEnvVolume_Effect::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CEnvVolume_Effect* pInstance = new CEnvVolume_Effect(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CEnvVolume_Effect");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CEnvVolume_Effect::Clone(void* pArg)
{
	CEnvVolume_Effect* pInstance = new CEnvVolume_Effect(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CEnvVolume_Effect");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CEnvVolume_Effect::Free()
{
	m_bActive = false;
	m_pSpawnedEffect = nullptr;

	__super::Free();
}

NS_END