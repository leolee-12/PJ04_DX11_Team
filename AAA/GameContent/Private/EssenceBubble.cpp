#include "EssenceBubble.h"
#include "GameInstance.h"
#include "Ability_Model.h"

CEssenceBubble::CEssenceBubble(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CAbility_Bubble { pDevice, pContext }
{
}

CEssenceBubble::CEssenceBubble(const CEssenceBubble& Prototype)
	: CAbility_Bubble(Prototype)
{
}

HRESULT CEssenceBubble::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	m_eCollLayer = COLLISION_LAYER::ESSENCE_BUBBLE;

	if (FAILED(Ready_Collider()))
		return E_FAIL;

	SetUp_Collider_CallBack();

	return S_OK;
}

void CEssenceBubble::Update(_float fTimeDelta)
{
	if (!m_bActive)
		return;

	__super::Update(fTimeDelta);

	if (m_fTimer > 0.f)
	{
		m_fTimer -= fTimeDelta;
		if (m_fTimer <= 0.f)
		{
			m_fTimer = 0.f;
			Set_RenderActive(true);
			m_bAvailable = true;
		}
	}

	if (m_bIsKirbyEnter)
	{
		m_bIsKirbyEnter = false;
		m_bAvailable = false;
	}
}

void CEssenceBubble::Activate(const _float3& vPos)
{
	__super::Activate(vPos);
	m_bIsKirbyEnter = false;
}

void CEssenceBubble::SetUp_Collider_CallBack()
{
	if (m_pCollider == nullptr)
		return;

	m_pCollider->Set_OnEnter([this](CCollider* pOther) {
		if (ETOUI(COLLISION_LAYER::PLAYER_HURT) != pOther->Get_RegisteredGroup())
			return;
		Set_RenderActive(false);
		m_bIsKirbyEnter = true;
		m_fTimer = 0.f;
		});

	m_pCollider->Set_OnExit([this](CCollider* pOther) {
		if (ETOUI(COLLISION_LAYER::PLAYER_HURT) != pOther->Get_RegisteredGroup())
			return;
		m_fTimer = s_fReSpawnTime;
		});
}

CEssenceBubble* CEssenceBubble::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CEssenceBubble* pInstance = new CEssenceBubble(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CEssenceBubble");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CEssenceBubble::Clone(void* pArg)
{
	CEssenceBubble* pInstance = new CEssenceBubble(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CEssenceBubble");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CEssenceBubble::Free()
{
	__super::Free();
}
