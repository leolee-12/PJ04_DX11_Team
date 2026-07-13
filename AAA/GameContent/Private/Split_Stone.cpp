#include "Split_Stone.h"
#include "GameContent_const.h"
#include "MeshEmitterCommon.h"

CSplit_Stone::CSplit_Stone(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEffect_Container(pDevice, pContext)
{
}

CSplit_Stone::CSplit_Stone(const CSplit_Stone& Prototype)
	: CEffect_Container(Prototype)
{
}

HRESULT CSplit_Stone::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CSplit_Stone::Initialize(void* pArg)
{
	EFFECT_CONTAINER_DESC* pDesc = static_cast<EFFECT_CONTAINER_DESC*>(pArg);

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_EffectPartObjects()))
		return E_FAIL;

	return S_OK;
}

void CSplit_Stone::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);
}

void CSplit_Stone::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
}

void CSplit_Stone::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
}

HRESULT CSplit_Stone::Render()
{
	__super::Render();

	return S_OK;
}

HRESULT CSplit_Stone::Ready_EffectPartObjects()
{
	CMeshEmitterCommon::MESH_EMITTER_COMMON_DESC tDesc{};
	tDesc.iModelLevel = m_iPrototypeLevel;
	tDesc.bCustomShader = true;
	tDesc.iShaderLevel = Shader_EffectRock.iLevelID;
	tDesc.wstrShaderTag = Shader_EffectRock.szProtoTag;

	tDesc.wstrModelTag = L"Prototype_Component_Model_Nature_Piece01";
	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshEmitterCommon::PROTOTYPE_TAG, TEXT("Piece_1"), &tDesc)))
		return E_FAIL;

	tDesc.wstrModelTag = L"Prototype_Component_Model_Nature_Piece02";
	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshEmitterCommon::PROTOTYPE_TAG, TEXT("Piece_2"), &tDesc)))
		return E_FAIL;

	tDesc.wstrModelTag = L"Prototype_Component_Model_Stone";
	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshEmitterCommon::PROTOTYPE_TAG, TEXT("Stone"), &tDesc)))
		return E_FAIL;

	tDesc.wstrModelTag = L"Prototype_Component_Model_StoneDust";
	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshEmitterCommon::PROTOTYPE_TAG, TEXT("StoneDust"), &tDesc)))
		return E_FAIL;

	tDesc.wstrModelTag = L"Prototype_Component_Model_StoneHiMesh";
	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshEmitterCommon::PROTOTYPE_TAG, TEXT("StoneHiMesh"), &tDesc)))
		return E_FAIL;

	tDesc.wstrModelTag = L"Prototype_Component_Model_SmokeSphereOriginal";
	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshEmitterCommon::PROTOTYPE_TAG, TEXT("Smoke"), &tDesc)))
		return E_FAIL;

	return S_OK;
}

CSplit_Stone* CSplit_Stone::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CSplit_Stone* pInstance = new CSplit_Stone(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created: CSplit_Stone");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CSplit_Stone::Clone(void* pArg)
{
	CSplit_Stone* pInstance = new CSplit_Stone(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned: CSplit_Stone");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CSplit_Stone::Free()
{
	__super::Free();
}
