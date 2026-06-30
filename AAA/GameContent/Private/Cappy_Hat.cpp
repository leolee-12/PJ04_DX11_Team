#include "Cappy_Hat.h"
#include "GameInstance.h"

CCappy_Hat::CCappy_Hat(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CMonsterPart{ pDevice, pContext }
{
}

CCappy_Hat::CCappy_Hat(const CCappy_Hat& Prototype)
	: CMonsterPart(Prototype)
{
}

HRESULT CCappy_Hat::Initialize_Prototype()
{
	m_eProjType = PROJ_TYPE::PERSPEC;
	return S_OK;
}

HRESULT CCappy_Hat::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_Components()))
		return E_FAIL;

	m_pAnimatorCom->Play("Wait", true, true);

	return S_OK;
}

HRESULT CCappy_Hat::Ready_Components()
{
	PART_SETUP t{};
	t.tShader = Shader_AnimMesh_PBR;
	t.szModelProtoTag = TEXT("Prototype_Component_Model_Cappy_Hat");

	if (FAILED(Ready_MeshPart(t)))
		return E_FAIL;

	return S_OK;
}

CCappy_Hat* CCappy_Hat::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CCappy_Hat* pInstance = new CCappy_Hat(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype())) 
	{
		MSG_BOX("Failed to Created: CCappy_Hat");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CCappy_Hat::Clone(void* pArg)
{
	CCappy_Hat* pInstance = new CCappy_Hat(*this);
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned: CCappy_Hat");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CCappy_Hat::Free()
{
	__super::Free();
}
