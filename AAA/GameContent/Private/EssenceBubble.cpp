#include "EssenceBubble.h"
#include "GameInstance.h"

CEssenceBubble::CEssenceBubble(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CAbility_Bubble { pDevice, pContext }
{
}

CEssenceBubble::CEssenceBubble(const CEssenceBubble& Prototype)
	: CAbility_Bubble(Prototype)
{
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

	//if (FAILED(pInstance->Initialize(pArg)))
	//{
	//	MSG_BOX("Failed to Cloned : CEssenceBubble");
	//	Safe_Release(pInstance);
	//}

	ABILITY_BUBBLE_DESC desc{};
	desc.eAbility = COPY_ABILITY_TYPE::SWORD;
	desc.szModelProtoTag = L"Prototype_Component_Model_Sword";

	if (FAILED(pInstance->Initialize(&desc)))
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
