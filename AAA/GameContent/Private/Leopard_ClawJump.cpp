#include "Leopard_ClawJump.h"
#include "GameContent_const.h"

#include "MeshCommon.h"

CLeopard_ClawJump::CLeopard_ClawJump(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEffect_Container{ pDevice, pContext }
{
}

CLeopard_ClawJump::CLeopard_ClawJump(const CLeopard_ClawJump& Prototype)
	: CEffect_Container(Prototype)
{
}

HRESULT CLeopard_ClawJump::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CLeopard_ClawJump::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_EffectPartObjects()))
		return E_FAIL;

	return S_OK;
}

void CLeopard_ClawJump::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);
}

void CLeopard_ClawJump::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
}

void CLeopard_ClawJump::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
}

HRESULT CLeopard_ClawJump::Render()
{
	return __super::Render();
}

HRESULT CLeopard_ClawJump::Ready_EffectPartObjects()
{
	CMeshCommon::MESH_COMMON_DESC tDesc{};
	tDesc.iModelLevel = m_iPrototypeLevel;
	tDesc.wstrModelTag = MODEL_PROTO_TAG;
	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshCommon::PROTOTYPE_TAG, L"Afterimage", &tDesc)))
		return E_FAIL;

	return S_OK;
}

CLeopard_ClawJump* CLeopard_ClawJump::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CLeopard_ClawJump* pInstance = new CLeopard_ClawJump(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created: CLeopard_ClawJump");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CLeopard_ClawJump::Clone(void* pArg)
{
	CLeopard_ClawJump* pInstance = new CLeopard_ClawJump(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned: CLeopard_ClawJump");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CLeopard_ClawJump::Free()
{
	__super::Free();
}
