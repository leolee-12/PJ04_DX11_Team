#include "BreakWallEffect.h"
#include "GameContent_const.h"
#include "MeshEmitterCommon.h"

CBreakWallEffect::CBreakWallEffect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEffect_Container(pDevice, pContext)
{
}

CBreakWallEffect::CBreakWallEffect(const CBreakWallEffect& Prototype)
	: CEffect_Container(Prototype)
{
}

HRESULT CBreakWallEffect::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_EffectPartObjects()))
		return E_FAIL;

	return S_OK;
}

HRESULT CBreakWallEffect::Ready_EffectPartObjects()
{
	CMeshEmitterCommon::MESH_EMITTER_COMMON_DESC tDesc{};
	tDesc.iModelLevel = m_iPrototypeLevel;
	tDesc.bCustomShader = true;
	tDesc.iShaderLevel = Shader_EffectRock.iLevelID;
	tDesc.wstrShaderTag = Shader_EffectRock.szProtoTag;

	tDesc.wstrModelTag = Model_StoneDust.szProtoTag;
	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshEmitterCommon::PROTOTYPE_TAG, TEXT("StoneDust"), &tDesc)))
		return E_FAIL;

	tDesc.wstrModelTag = Model_SmokeSphereOriginal.szProtoTag;
	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshEmitterCommon::PROTOTYPE_TAG, TEXT("Smoke"), &tDesc)))
		return E_FAIL;

	return S_OK;
}

CBreakWallEffect* CBreakWallEffect::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CBreakWallEffect* pInstance = new CBreakWallEffect(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created: CBreakWallEffect");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CBreakWallEffect::Clone(void* pArg)
{
	CBreakWallEffect* pInstance = new CBreakWallEffect(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned: CBreakWallEffect");
		Safe_Release(pInstance);
	}

	return pInstance;
}