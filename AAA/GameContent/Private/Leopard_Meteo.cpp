#include "Leopard_Meteo.h"
#include "GameContent_const.h"

#include "MeshCommon.h"

CLeopard_Meteo::CLeopard_Meteo(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEffect_Container{ pDevice, pContext }
{
}

CLeopard_Meteo::CLeopard_Meteo(const CLeopard_Meteo& Prototype)
	: CEffect_Container(Prototype)
{
}

HRESULT CLeopard_Meteo::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CLeopard_Meteo::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_EffectPartObjects()))
		return E_FAIL;

	return S_OK;
}

void CLeopard_Meteo::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);
}

void CLeopard_Meteo::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
}

void CLeopard_Meteo::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
}

HRESULT CLeopard_Meteo::Render()
{
	return __super::Render();
}

HRESULT CLeopard_Meteo::Ready_EffectPartObjects()
{
	CMeshCommon::MESH_COMMON_DESC tDesc{};
	tDesc.iModelLevel = m_iPrototypeLevel;
	tDesc.wstrModelTag = MODEL_PROTO_TAG;
	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshCommon::PROTOTYPE_TAG, L"Meteo", &tDesc)))
		return E_FAIL;

	return S_OK;
}

CLeopard_Meteo* CLeopard_Meteo::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CLeopard_Meteo* pInstance = new CLeopard_Meteo(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created: CLeopard_Meteo");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CLeopard_Meteo::Clone(void* pArg)
{
	CLeopard_Meteo* pInstance = new CLeopard_Meteo(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned: CLeopard_Meteo");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CLeopard_Meteo::Free()
{
	__super::Free();
}
