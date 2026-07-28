#include "OnigorosiHammerFirst.h"

#include "GameContent_const.h"
#include "MeshCommon.h"

COnigorosiHammerFirst::COnigorosiHammerFirst(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEffect_Container{ pDevice, pContext }
{
}

COnigorosiHammerFirst::COnigorosiHammerFirst(const COnigorosiHammerFirst& Prototype)
	: CEffect_Container{ Prototype }
{
}

HRESULT COnigorosiHammerFirst::Initialize_Prototype()
{
	return S_OK;
}

HRESULT COnigorosiHammerFirst::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_EffectPartObjects()))
		return E_FAIL;

	return S_OK;
}

void COnigorosiHammerFirst::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);
}

void COnigorosiHammerFirst::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
}

void COnigorosiHammerFirst::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
}

HRESULT COnigorosiHammerFirst::Render()
{
	return __super::Render();
}

HRESULT COnigorosiHammerFirst::Ready_EffectPartObjects()
{
	CMeshCommon::MESH_COMMON_DESC MeshDesc{};
	MeshDesc.iModelLevel = m_iPrototypeLevel;
	MeshDesc.wstrModelTag = MODEL_PROTO_TAG;
	MeshDesc.bUseTextureCom = true;
	MeshDesc.iTextureLevel = Texture_OnigorosiHammerFirst.iLevelID;
	MeshDesc.wstrTextureTag = Texture_OnigorosiHammerFirst.szProtoTag;

	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshCommon::PROTOTYPE_TAG, MESH_PART_TAG, &MeshDesc)))
		return E_FAIL;

	return S_OK;
}

COnigorosiHammerFirst* COnigorosiHammerFirst::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	COnigorosiHammerFirst* pInstance = new COnigorosiHammerFirst(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created: COnigorosiHammerFirst");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* COnigorosiHammerFirst::Clone(void* pArg)
{
	COnigorosiHammerFirst* pInstance = new COnigorosiHammerFirst(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned: COnigorosiHammerFirst");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void COnigorosiHammerFirst::Free()
{
	__super::Free();
}
