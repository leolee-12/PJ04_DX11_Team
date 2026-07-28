#include "Meta_Slash.h"
#include "GameContent_const.h"

#include "MeshCommon.h"

CMeta_Slash::CMeta_Slash(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEffect_Container{ pDevice, pContext }
{
}

CMeta_Slash::CMeta_Slash(const CMeta_Slash& Prototype)
	: CEffect_Container(Prototype)
{
}

HRESULT CMeta_Slash::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CMeta_Slash::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_EffectPartObjects()))
		return E_FAIL;

	return S_OK;
}

void CMeta_Slash::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);
}

void CMeta_Slash::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
}

void CMeta_Slash::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
}

HRESULT CMeta_Slash::Render()
{
	return __super::Render();
}

HRESULT CMeta_Slash::Ready_EffectPartObjects()
{
	CMeshCommon::MESH_COMMON_DESC tDesc{};
	tDesc.iModelLevel = m_iPrototypeLevel;
	tDesc.bUseTextureCom = false;
	tDesc.wstrModelTag = MODEL_PROTO_TAG;
	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshCommon::PROTOTYPE_TAG, L"Slash", &tDesc)))
		return E_FAIL;
	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshCommon::PROTOTYPE_TAG, L"Slash2", &tDesc)))
		return E_FAIL;

	tDesc.bUseTextureCom = true;
	tDesc.iTextureLevel = Texture_Meta_Slash2.iLevelID;
	tDesc.wstrTextureTag = Texture_Meta_Slash2.szProtoTag;
	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshCommon::PROTOTYPE_TAG, L"Slash3", &tDesc)))
		return E_FAIL;

	return S_OK;
}

CMeta_Slash* CMeta_Slash::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CMeta_Slash* pInstance = new CMeta_Slash(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created: CMeta_Slash");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CMeta_Slash::Clone(void* pArg)
{
	CMeta_Slash* pInstance = new CMeta_Slash(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned: CMeta_Slash");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CMeta_Slash::Free()
{
	__super::Free();
}
