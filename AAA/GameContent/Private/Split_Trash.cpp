#include "Split_Trash.h"
#include "GameContent_const.h"
#include "MeshEmitterCommon.h"

CSplit_Trash::CSplit_Trash(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEffect_Container(pDevice, pContext)
{
}

CSplit_Trash::CSplit_Trash(const CSplit_Trash& Prototype)
	: CEffect_Container(Prototype)
{
}

HRESULT CSplit_Trash::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_EffectPartObjects()))
		return E_FAIL;

	return S_OK;
}

HRESULT CSplit_Trash::Ready_EffectPartObjects()
{
    CMeshEmitterCommon::MESH_EMITTER_COMMON_DESC tDesc{};
    tDesc.iModelLevel = m_iPrototypeLevel;
    tDesc.bCustomShader = true;
    tDesc.iShaderLevel = Shader_EffectRock.iLevelID;
    tDesc.wstrShaderTag = Shader_EffectRock.szProtoTag;

    tDesc.wstrModelTag = L"Prototype_Component_Model_EmptyCanGreen";
    if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshEmitterCommon::PROTOTYPE_TAG, TEXT("EmptyCanGreen"), &tDesc)))
        return E_FAIL;

    tDesc.wstrModelTag = L"Prototype_Component_Model_EmptyCanOrange";
    if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshEmitterCommon::PROTOTYPE_TAG, TEXT("EmptyCanOrange"), &tDesc)))
        return E_FAIL;

    tDesc.wstrModelTag = L"Prototype_Component_Model_EmptyCanPurple";
    if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshEmitterCommon::PROTOTYPE_TAG, TEXT("EmptyCanPurple"), &tDesc)))
        return E_FAIL;

    tDesc.wstrModelTag = L"Prototype_Component_Model_EmptyCanRed";
    if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshEmitterCommon::PROTOTYPE_TAG, TEXT("EmptyCanRed"), &tDesc)))
        return E_FAIL;

    tDesc.wstrModelTag = L"Prototype_Component_Model_EmptyCanYellow";
    if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshEmitterCommon::PROTOTYPE_TAG, TEXT("EmptyCanYellow"), &tDesc)))
        return E_FAIL;

    tDesc.wstrModelTag = L"Prototype_Component_Model_TrashA";
    if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshEmitterCommon::PROTOTYPE_TAG, TEXT("TrashA"), &tDesc)))
        return E_FAIL;

    tDesc.wstrModelTag = Model_StoneDust.szProtoTag;
    if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshEmitterCommon::PROTOTYPE_TAG, TEXT("StoneDust"), &tDesc)))
        return E_FAIL;

    return S_OK;
}

CSplit_Trash* CSplit_Trash::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CSplit_Trash* pInstance = new CSplit_Trash(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created: CSplit_Trash");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CSplit_Trash::Clone(void* pArg)
{
	CSplit_Trash* pInstance = new CSplit_Trash(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned: CSplit_Trash");
		Safe_Release(pInstance);
	}

	return pInstance;
}