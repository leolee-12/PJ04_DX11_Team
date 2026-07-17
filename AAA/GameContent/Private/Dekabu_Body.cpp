#include "Dekabu_Body.h"
#include "GameInstance.h"

CDekabu_Body::CDekabu_Body(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CMonsterPart{ pDevice, pContext }
{
}

CDekabu_Body::CDekabu_Body(const CDekabu_Body& Prototype)
	: CMonsterPart (Prototype)
{
}

HRESULT CDekabu_Body::Initialize_Prototype()
{
	m_eProjType = PROJ_TYPE::PERSPEC;
	return S_OK;
}

HRESULT CDekabu_Body::Initialize(void* pArg)
{
	auto pDesc = static_cast<DEKABU_BODY_DESC*>(pArg);
	pDesc->fSpeedPerSec = 1.f;

	if (FAILED(__super::Initialize(pDesc)))
		return E_FAIL;

	if (FAILED(Ready_Components()))
		return E_FAIL;

	m_pAnimatorCom->Play("Wait", true, true);

	return S_OK;
}

void CDekabu_Body::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
}

HRESULT CDekabu_Body::Render()
{
	return S_OK;
}

HRESULT CDekabu_Body::Ready_Components()
{
	return S_OK;
}

CDekabu_Body* CDekabu_Body::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	return nullptr;
}

CGameObject* CDekabu_Body::Clone(void* pArg)
{
	return nullptr;
}

void CDekabu_Body::Free()
{
}
