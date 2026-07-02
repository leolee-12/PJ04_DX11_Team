#include "Cappy_Body.h"
#include "GameInstance.h"

CCappy_Body::CCappy_Body(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CMonsterPart{ pDevice, pContext }
{
}

CCappy_Body::CCappy_Body(const CCappy_Body& Prototype)
	: CMonsterPart (Prototype)
{
}

HRESULT CCappy_Body::Initialize_Prototype()
{
	m_eProjType = PROJ_TYPE::PERSPEC;
	return S_OK;
}

HRESULT CCappy_Body::Initialize(void* pArg)
{
	auto pDesc = static_cast<CAPPY_BODY_DESC*>(pArg);
	pDesc->fSpeedPerSec = 1.f;

	if (FAILED(__super::Initialize(pDesc)))
		return E_FAIL;

	if (FAILED(Ready_Components()))
		return E_FAIL;

	m_pAnimatorCom->Play("Wait", true, true);

	return S_OK;
}

void CCappy_Body::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
}

HRESULT CCappy_Body::Ready_Components()
{
	PART_SETUP t{};
	t.tShader = Shader_Monster;
	t.szModelProtoTag = TEXT("Prototype_Component_Model_Cappy_Body");

	if (FAILED(Ready_MeshPart(t)))
		return E_FAIL;

	return S_OK;
}

CCappy_Body* CCappy_Body::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CCappy_Body* pInstance = new CCappy_Body(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CCappy_Body");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CCappy_Body::Clone(void* pArg)
{
	CCappy_Body* pInstance = new CCappy_Body(*this);
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CCappy_Body");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CCappy_Body::Free()
{
	__super::Free();
}
