#include "Armadillo_RollWind.h"
#include "GameContent_const.h"

#include "MeshCommon.h"

CArmadillo_RollWind::CArmadillo_RollWind(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEffect_Container{ pDevice, pContext }
{
}

CArmadillo_RollWind::CArmadillo_RollWind(const CArmadillo_RollWind& Prototype)
	: CEffect_Container(Prototype)
{
}

HRESULT CArmadillo_RollWind::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CArmadillo_RollWind::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_EffectPartObjects()))
		return E_FAIL;

	return S_OK;
}

void CArmadillo_RollWind::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);
}

void CArmadillo_RollWind::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
}

void CArmadillo_RollWind::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
}

HRESULT CArmadillo_RollWind::Render()
{
	return __super::Render();
}

HRESULT CArmadillo_RollWind::Ready_EffectPartObjects()
{
	CMeshCommon::MESH_COMMON_DESC tDesc{};
	tDesc.iModelLevel = m_iPrototypeLevel;

	tDesc.wstrModelTag = MODEL_PROTO_TAG;
	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshCommon::PROTOTYPE_TAG, L"RollWind", &tDesc)))
		return E_FAIL;

	return S_OK;
}

CArmadillo_RollWind* CArmadillo_RollWind::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CArmadillo_RollWind* pInstance = new CArmadillo_RollWind(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created: CArmadillo_RollWind");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CArmadillo_RollWind::Clone(void* pArg)
{
	CArmadillo_RollWind* pInstance = new CArmadillo_RollWind(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned: CArmadillo_RollWind");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CArmadillo_RollWind::Free()
{
	__super::Free();
}
