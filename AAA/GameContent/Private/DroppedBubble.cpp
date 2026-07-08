#include "DroppedBubble.h"

CDroppedBubble::CDroppedBubble(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CAbility_Bubble{ pDevice, pContext }
{
}

CDroppedBubble::CDroppedBubble(const CDroppedBubble& Prototype)
	: CAbility_Bubble(Prototype)
{
}

HRESULT CDroppedBubble::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	m_eCollLayer = COLLISION_LAYER::DROPPED_BUBBLE;

	if (FAILED(Ready_Collider()))
		return E_FAIL;

	SetUp_Collider_CallBack();

	return S_OK;
}

void CDroppedBubble::Update(_float fTimeDelta)
{
}

_bool CDroppedBubble::Can_BeInhaled(const INHALE_QUERY& q) const
{
	return true;
}

void CDroppedBubble::Be_Captured(CGameObject* pInhaler)
{
}

void CDroppedBubble::SetUp_Collider_CallBack()
{
}

CDroppedBubble* CDroppedBubble::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CDroppedBubble* pInstance = new CDroppedBubble(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CDroppedBubble");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CDroppedBubble::Clone(void* pArg)
{
	CDroppedBubble* pInstance = new CDroppedBubble(*this);
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CDroppedBubble");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CDroppedBubble::Free()
{
	__super::Free();
}
