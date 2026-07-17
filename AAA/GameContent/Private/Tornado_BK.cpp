#include "Tornado_BK.h"
#include "MeshCommon.h"
#include "GameContent_const.h"

CTornado_BK::CTornado_BK(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEffect_Container{ pDevice, pContext }
{
}

CTornado_BK::CTornado_BK(const CTornado_BK& Prototype)
	: CEffect_Container(Prototype)
{
}

HRESULT CTornado_BK::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CTornado_BK::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_EffectPartObjects()))
		return E_FAIL;

	return S_OK;
}

void CTornado_BK::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);
}

void CTornado_BK::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
}

void CTornado_BK::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
}

HRESULT CTornado_BK::Render()
{
	if (FAILED(__super::Render()))
		return E_FAIL;

	return S_OK;
}

HRESULT CTornado_BK::Ready_EffectPartObjects()
{
	CMeshCommon::MESH_COMMON_DESC tDesc{};
	tDesc.iModelLevel = m_iPrototypeLevel;
	tDesc.bCustomShader = false;          
	tDesc.bUseDiffuseTexture = false;
	tDesc.bUseUnknownTexture = true;      

	// Spin01 ¸µ
	tDesc.wstrModelTag = L"Prototype_Component_Model_BK_TornadoRing_Spin01";
	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel,
		CMeshCommon::PROTOTYPE_TAG, L"Ring_Spin01", &tDesc)))
		return E_FAIL;

	// Spin02 ¸µ
	tDesc.wstrModelTag = L"Prototype_Component_Model_BK_TornadoRing_Spin02";
	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel,
		CMeshCommon::PROTOTYPE_TAG, L"Ring_Spin02", &tDesc)))
		return E_FAIL;

	return S_OK;
}

CTornado_BK* CTornado_BK::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CTornado_BK* pInstance = new CTornado_BK(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created: CTornado_BK");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CTornado_BK::Clone(void* pArg)
{
	CTornado_BK* pInstance = new CTornado_BK(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned: CTornado_BK");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CTornado_BK::Free()
{
	__super::Free();
}
