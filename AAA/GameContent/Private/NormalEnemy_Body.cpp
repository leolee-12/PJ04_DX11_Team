#include "NormalEnemy_Body.h"
#include "Animator.h"

CNormalEnemy_Body::CNormalEnemy_Body(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CMonsterPart { pDevice, pContext }
{
}

CNormalEnemy_Body::CNormalEnemy_Body(const CNormalEnemy_Body& Prototype)
	: CMonsterPart (Prototype)
{
}

HRESULT CNormalEnemy_Body::Initialize_Prototype()
{
	m_eProjType = PROJ_TYPE::PERSPEC;
	return S_OK;
}

HRESULT CNormalEnemy_Body::Initialize(void* pArg)
{
	auto pDesc = static_cast<NORMALENEMY_BODY_DESC*>(pArg);
	pDesc->fSpeedPerSec = 1.f;

	if (FAILED(__super::Initialize(pDesc))) 
		return E_FAIL;

	if (FAILED(Ready_Components()))         
		return E_FAIL;

	// 애니메이션 지정
	m_pAnimatorCom->Play("Wait", true, true);

	return S_OK;
}

HRESULT CNormalEnemy_Body::Render()
{


	return S_OK;
}

HRESULT CNormalEnemy_Body::Ready_Components()
{
	PART_SETUP t{};
	t.tShader = Shader_AnimMesh_PBR;
	t.szModelProtoTag = TEXT("Prototype_Component_Model_NormalEnemy_Body");
	//t.szAnimEventFile = TEXT("../../Resources/CHJ/Monster/BladeKnight/BladeKnight_Anim_Events.json");		// 이거 없어도 Animator에서 자동 로드
	return Ready_MeshPart(t);
}

CNormalEnemy_Body* CNormalEnemy_Body::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CNormalEnemy_Body* pInstance = new CNormalEnemy_Body(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype())) {
		MSG_BOX("Failed to Created: CNormalEnemy_Body");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CNormalEnemy_Body::Clone(void* pArg)
{
	CNormalEnemy_Body* pInstance = new CNormalEnemy_Body(*this);
	if (FAILED(pInstance->Initialize(pArg))) {
		MSG_BOX("Failed to Cloned: CNormalEnemy_Body");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CNormalEnemy_Body::Free()
{
	__super::Free();
}
