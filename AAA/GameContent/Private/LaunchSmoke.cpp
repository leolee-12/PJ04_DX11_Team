#include "LaunchSmoke.h"

#include "MeshEmitterCommon.h"

CLaunchSmoke::CLaunchSmoke(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEffect_Container{ pDevice, pContext }
{
}

CLaunchSmoke::CLaunchSmoke(const CLaunchSmoke& Prototype)
	: CEffect_Container(Prototype)
{
}

HRESULT CLaunchSmoke::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CLaunchSmoke::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_EffectPartObjects()))
		return E_FAIL;

	return S_OK;
}

void CLaunchSmoke::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);
}

void CLaunchSmoke::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
}

void CLaunchSmoke::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
}

HRESULT CLaunchSmoke::Render()
{
	return __super::Render();
}

HRESULT CLaunchSmoke::Ready_EffectPartObjects()
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

CLaunchSmoke* CLaunchSmoke::Create(ID3D11Device* pDevice,
	ID3D11DeviceContext* pContext)
{
	CLaunchSmoke* pInstance = new CLaunchSmoke(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created: CLaunchSmoke");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CLaunchSmoke::Clone(void* pArg)
{
	CLaunchSmoke* pInstance = new CLaunchSmoke(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned: CLaunchSmoke");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CLaunchSmoke::Free()
{
	__super::Free();
}
