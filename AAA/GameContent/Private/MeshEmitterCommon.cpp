#include "MeshEmitterCommon.h"
#include "GameContent_const.h"

#include "GameInstance.h"

CMeshEmitterCommon::CMeshEmitterCommon(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEffect_MeshEmitter{ pDevice, pContext }
{
}

CMeshEmitterCommon::CMeshEmitterCommon(const CMeshEmitterCommon& Prototype)
	: CEffect_MeshEmitter(Prototype)
{
}

HRESULT CMeshEmitterCommon::Initialize_Prototype()
{
	m_eProjType = PROJ_TYPE::PERSPEC;
	return S_OK;
}

HRESULT CMeshEmitterCommon::Initialize(void* pArg)
{
	if (nullptr == pArg)
		return E_FAIL;

	MESH_EMITTER_COMMON_DESC tDesc = *static_cast<MESH_EMITTER_COMMON_DESC*>(pArg);

	if (FAILED(__super::Initialize(&tDesc)))
		return E_FAIL;

	return S_OK;
}

void CMeshEmitterCommon::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);
}

void CMeshEmitterCommon::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
}

void CMeshEmitterCommon::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);

	if (m_bActive == false)
		return;

	m_pGameInstance_Proxy->Add_RenderGroup(RENDERID::NONBLEND, this);
}

HRESULT CMeshEmitterCommon::Render()
{
	return __super::Render();
}

CMeshEmitterCommon* CMeshEmitterCommon::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CMeshEmitterCommon* pInstance = new CMeshEmitterCommon(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created: CMeshEmitterCommon");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CMeshEmitterCommon::Clone(void* pArg)
{
	CMeshEmitterCommon* pInstance = new CMeshEmitterCommon(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned: CMeshEmitterCommon");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CMeshEmitterCommon::Free()
{
	__super::Free();
}