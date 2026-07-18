#include "Leopard_Flash.h"
#include "GameContent_const.h"

#include "MeshCommon.h"

CLeopard_Flash::CLeopard_Flash(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEffect_Container{ pDevice, pContext }
{
}

CLeopard_Flash::CLeopard_Flash(const CLeopard_Flash& Prototype)
	: CEffect_Container(Prototype)
{
}

HRESULT CLeopard_Flash::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CLeopard_Flash::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_EffectPartObjects()))
		return E_FAIL;

	return S_OK;
}

void CLeopard_Flash::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);
}

void CLeopard_Flash::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
}

void CLeopard_Flash::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
}

HRESULT CLeopard_Flash::Render()
{
	return __super::Render();
}

HRESULT CLeopard_Flash::Ready_EffectPartObjects()
{
	CMeshCommon::MESH_COMMON_DESC tDesc{};
	tDesc.iModelLevel = m_iPrototypeLevel;
	tDesc.bUseTextureCom = true;
	tDesc.iTextureLevel = m_iPrototypeLevel;

	tDesc.wstrTextureTag = Texture_LeoSlash.szProtoTag;
	tDesc.wstrModelTag = MODEL_PROTO_TAG_RING;
	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshCommon::PROTOTYPE_TAG, L"Ring", &tDesc)))
		return E_FAIL;

	tDesc.wstrTextureTag = Texture_LeoSlash.szProtoTag;
	tDesc.wstrModelTag = MODEL_PROTO_TAG_CIRCLE;
	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshCommon::PROTOTYPE_TAG, L"Flash", &tDesc)))
		return E_FAIL;

	return S_OK;
}

CLeopard_Flash* CLeopard_Flash::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CLeopard_Flash* pInstance = new CLeopard_Flash(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created: CLeopard_Flash");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CLeopard_Flash::Clone(void* pArg)
{
	CLeopard_Flash* pInstance = new CLeopard_Flash(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned: CLeopard_Flash");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CLeopard_Flash::Free()
{
	__super::Free();
}
