#include "Leopard_Impact.h"
#include "GameContent_const.h"

#include "MeshCommon.h"

CLeopard_Impact::CLeopard_Impact(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEffect_Container{ pDevice, pContext }
{
}

CLeopard_Impact::CLeopard_Impact(const CLeopard_Impact& Prototype)
	: CEffect_Container(Prototype)
{
}

HRESULT CLeopard_Impact::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CLeopard_Impact::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_EffectPartObjects()))
		return E_FAIL;

	return S_OK;
}

void CLeopard_Impact::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);
}

void CLeopard_Impact::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
}

void CLeopard_Impact::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
}

HRESULT CLeopard_Impact::Render()
{
	return __super::Render();
}

HRESULT CLeopard_Impact::Ready_EffectPartObjects()
{
	CMeshCommon::MESH_COMMON_DESC tDesc{};
	tDesc.iModelLevel = m_iPrototypeLevel;
	tDesc.bUseTextureCom = true;
	tDesc.iTextureLevel = m_iPrototypeLevel;

	tDesc.wstrTextureTag = Texture_ImpactRing.szProtoTag;
	tDesc.wstrModelTag = MODEL_PROTO_TAG_RING;
	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshCommon::PROTOTYPE_TAG, L"Ring", &tDesc)))
		return E_FAIL;

	tDesc.wstrTextureTag = Texture_ImpactCircle.szProtoTag;
	tDesc.wstrModelTag = MODEL_PROTO_TAG_CIRCLE;
	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshCommon::PROTOTYPE_TAG, L"Circle", &tDesc)))
		return E_FAIL;

	return S_OK;
}

CLeopard_Impact* CLeopard_Impact::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CLeopard_Impact* pInstance = new CLeopard_Impact(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created: CLeopard_Impact");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CLeopard_Impact::Clone(void* pArg)
{
	CLeopard_Impact* pInstance = new CLeopard_Impact(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned: CLeopard_Impact");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CLeopard_Impact::Free()
{
	__super::Free();
}
