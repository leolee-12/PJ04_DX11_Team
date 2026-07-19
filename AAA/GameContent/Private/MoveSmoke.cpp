#include "MoveSmoke.h"

#include "MeshEmitterCommon.h"

CMoveSmoke::CMoveSmoke(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEffect_Container{ pDevice, pContext }
{
}

CMoveSmoke::CMoveSmoke(const CMoveSmoke& Prototype)
	: CEffect_Container(Prototype)
{
}

HRESULT CMoveSmoke::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CMoveSmoke::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_EffectPartObjects()))
		return E_FAIL;

	return S_OK;
}

void CMoveSmoke::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);
}

void CMoveSmoke::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
}

void CMoveSmoke::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
}

HRESULT CMoveSmoke::Render()
{
	return __super::Render();
}

HRESULT CMoveSmoke::Ready_EffectPartObjects()
{
	CMeshEmitterCommon::MESH_EMITTER_COMMON_DESC tDesc{};
	tDesc.iModelLevel = m_iPrototypeLevel;
	tDesc.wstrModelTag = L"Prototype_Component_Model_SmokeMesh";
	tDesc.bUseDiffuseTexture = false;
	tDesc.bUseUnknownTexture = true;
	tDesc.bUseNormalTexture = false;
	tDesc.bUseMRATexture = false;
	tDesc.bUseTextureCom = false;
	tDesc.iTextureLevel = 0;
	tDesc.wstrTextureTag = L"";
	tDesc.bUseMaskCom = false;
	tDesc.iMaskLevel = 0;
	tDesc.wstrMaskTag = L"";
	tDesc.bCustomShader = false;

	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshEmitterCommon::PROTOTYPE_TAG, L"Smoke", &tDesc)))
		return E_FAIL;

	return S_OK;
}

CMoveSmoke* CMoveSmoke::Create(ID3D11Device* pDevice,
	ID3D11DeviceContext* pContext)
{
	CMoveSmoke* pInstance = new CMoveSmoke(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created: CMoveSmoke");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CMoveSmoke::Clone(void* pArg)
{
	CMoveSmoke* pInstance = new CMoveSmoke(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned: CMoveSmoke");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CMoveSmoke::Free()
{
	__super::Free();
}
