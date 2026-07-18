#include "Leopard_ClawAssault.h"
#include "GameContent_const.h"

#include "MeshCommon.h"

CLeopard_ClawAssault::CLeopard_ClawAssault(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEffect_Container{ pDevice, pContext }
{
}

CLeopard_ClawAssault::CLeopard_ClawAssault(const CLeopard_ClawAssault& Prototype)
	: CEffect_Container(Prototype)
{
}

HRESULT CLeopard_ClawAssault::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CLeopard_ClawAssault::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_EffectPartObjects()))
		return E_FAIL;

	return S_OK;
}

void CLeopard_ClawAssault::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);
}

void CLeopard_ClawAssault::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
}

void CLeopard_ClawAssault::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
}

HRESULT CLeopard_ClawAssault::Render()
{
	return __super::Render();
}

HRESULT CLeopard_ClawAssault::Ready_EffectPartObjects()
{
	CMeshCommon::MESH_COMMON_DESC tDesc{};
	tDesc.iModelLevel = m_iPrototypeLevel;
	tDesc.wstrModelTag = MODEL_PROTO_TAG;
	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshCommon::PROTOTYPE_TAG, L"Afterimage", &tDesc)))
		return E_FAIL;

	return S_OK;
}

CLeopard_ClawAssault* CLeopard_ClawAssault::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CLeopard_ClawAssault* pInstance = new CLeopard_ClawAssault(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created: CLeopard_ClawAssault");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CLeopard_ClawAssault::Clone(void* pArg)
{
	CLeopard_ClawAssault* pInstance = new CLeopard_ClawAssault(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned: CLeopard_ClawAssault");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CLeopard_ClawAssault::Free()
{
	__super::Free();
}
