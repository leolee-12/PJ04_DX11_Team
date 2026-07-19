#include "Leopard_Slash.h"
#include "GameContent_const.h"

#include "MeshCommon.h"

CLeopard_Slash::CLeopard_Slash(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEffect_Container{ pDevice, pContext }
{
}

CLeopard_Slash::CLeopard_Slash(const CLeopard_Slash& Prototype)
	: CEffect_Container(Prototype)
{
}

HRESULT CLeopard_Slash::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CLeopard_Slash::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_EffectPartObjects()))
		return E_FAIL;

	return S_OK;
}

void CLeopard_Slash::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);
}

void CLeopard_Slash::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
}

void CLeopard_Slash::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
}

HRESULT CLeopard_Slash::Render()
{
	return __super::Render();
}

HRESULT CLeopard_Slash::Ready_EffectPartObjects()
{
	CMeshCommon::MESH_COMMON_DESC tDesc{};
	tDesc.iModelLevel = m_iPrototypeLevel;
	tDesc.bUseTextureCom = true;
	tDesc.iTextureLevel = m_iPrototypeLevel;
	
	tDesc.wstrTextureTag = Texture_LeoSlash.szProtoTag;
	tDesc.wstrModelTag = MODEL_PROTO_TAG;
	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshCommon::PROTOTYPE_TAG, L"Slash", &tDesc)))
		return E_FAIL;

	return S_OK;
}

CLeopard_Slash* CLeopard_Slash::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CLeopard_Slash* pInstance = new CLeopard_Slash(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created: CLeopard_Slash");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CLeopard_Slash::Clone(void* pArg)
{
	CLeopard_Slash* pInstance = new CLeopard_Slash(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned: CLeopard_Slash");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CLeopard_Slash::Free()
{
	__super::Free();
}
