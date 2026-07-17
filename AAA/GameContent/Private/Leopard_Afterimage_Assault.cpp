#include "Leopard_Afterimage_Assault.h"
#include "GameContent_const.h"

#include "MeshCommon.h"

CLeopard_Afterimage_Assault::CLeopard_Afterimage_Assault(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEffect_Container{ pDevice, pContext }
{
}

CLeopard_Afterimage_Assault::CLeopard_Afterimage_Assault(const CLeopard_Afterimage_Assault& Prototype)
	: CEffect_Container(Prototype)
{
}

HRESULT CLeopard_Afterimage_Assault::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CLeopard_Afterimage_Assault::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_EffectPartObjects()))
		return E_FAIL;

	return S_OK;
}

void CLeopard_Afterimage_Assault::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);
}

void CLeopard_Afterimage_Assault::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
}

void CLeopard_Afterimage_Assault::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
}

HRESULT CLeopard_Afterimage_Assault::Render()
{
	return __super::Render();
}

HRESULT CLeopard_Afterimage_Assault::Ready_EffectPartObjects()
{
	CMeshCommon::MESH_COMMON_DESC tDesc{};
	tDesc.iModelLevel = m_iPrototypeLevel;
	tDesc.wstrModelTag = MODEL_PROTO_TAG;
	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshCommon::PROTOTYPE_TAG, L"Meteo", &tDesc)))
		return E_FAIL;

	return S_OK;
}

CLeopard_Afterimage_Assault* CLeopard_Afterimage_Assault::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CLeopard_Afterimage_Assault* pInstance = new CLeopard_Afterimage_Assault(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created: CLeopard_Afterimage_Assault");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CLeopard_Afterimage_Assault::Clone(void* pArg)
{
	CLeopard_Afterimage_Assault* pInstance = new CLeopard_Afterimage_Assault(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned: CLeopard_Afterimage_Assault");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CLeopard_Afterimage_Assault::Free()
{
	__super::Free();
}
