#include "SmokeDefault.h"

#include "MeshCommon.h"

CSmokeDefault::CSmokeDefault(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEffect_Container{ pDevice, pContext }
{
}

CSmokeDefault::CSmokeDefault(const CSmokeDefault& Prototype)
	: CEffect_Container{ Prototype }
{
}

HRESULT CSmokeDefault::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CSmokeDefault::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_EffectPartObjects()))
		return E_FAIL;

	return S_OK;
}

void CSmokeDefault::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);
}

void CSmokeDefault::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
}

void CSmokeDefault::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
}

HRESULT CSmokeDefault::Render()
{
	return __super::Render();
}

HRESULT CSmokeDefault::Ready_EffectPartObjects()
{
	CMeshCommon::MESH_COMMON_DESC tDesc{};
	tDesc.iModelLevel = m_iPrototypeLevel;
	tDesc.wstrModelTag = L"Prototype_Component_Model_SmokeSphereOriginal";
	tDesc.bUseDiffuseTexture = false;
	tDesc.bUseUnknownTexture = true;
	tDesc.bUseTextureCom = false;
	tDesc.bUseMaskCom = false;
	tDesc.bCustomShader = false;

	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshCommon::PROTOTYPE_TAG,
		L"SmokeSphereOriginal", &tDesc)))
		return E_FAIL;

	return S_OK;
}

CSmokeDefault* CSmokeDefault::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CSmokeDefault* pInstance = new CSmokeDefault(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created: CSmokeDefault");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CSmokeDefault::Clone(void* pArg)
{
	CSmokeDefault* pInstance = new CSmokeDefault(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned: CSmokeDefault");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CSmokeDefault::Free()
{
	__super::Free();
}
