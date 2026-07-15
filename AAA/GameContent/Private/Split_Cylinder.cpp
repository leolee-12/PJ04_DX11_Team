#include "Split_Cylinder.h"
#include "GameContent_const.h"
#include "MeshEmitterCommon.h"

CSplit_Cylinder::CSplit_Cylinder(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEffect_Container(pDevice, pContext)
{
}

CSplit_Cylinder::CSplit_Cylinder(const CSplit_Cylinder& Prototype)
	: CEffect_Container(Prototype)
{
}

HRESULT CSplit_Cylinder::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CSplit_Cylinder::Initialize(void* pArg)
{
	EFFECT_CONTAINER_DESC* pDesc = static_cast<EFFECT_CONTAINER_DESC*>(pArg);

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_EffectPartObjects()))
		return E_FAIL;

	return S_OK;
}

void CSplit_Cylinder::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);
}

void CSplit_Cylinder::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
}

void CSplit_Cylinder::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
}

HRESULT CSplit_Cylinder::Render()
{
	__super::Render();

	return S_OK;
}

HRESULT CSplit_Cylinder::Ready_EffectPartObjects()
{
	CMeshEmitterCommon::MESH_EMITTER_COMMON_DESC tDesc{};
	tDesc.iModelLevel = m_iPrototypeLevel;
	tDesc.bCustomShader = true;
	tDesc.iShaderLevel = Shader_EffectRock.iLevelID;
	tDesc.wstrShaderTag = Shader_EffectRock.szProtoTag;

	tDesc.wstrModelTag = L"Prototype_Component_Model_Cylinder_DrainM";
	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshEmitterCommon::PROTOTYPE_TAG, TEXT("DrainM"), &tDesc)))
		return E_FAIL;

	tDesc.wstrModelTag = L"Prototype_Component_Model_Cylinder_PieceM";
	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshEmitterCommon::PROTOTYPE_TAG, TEXT("PieceM"), &tDesc)))
		return E_FAIL;

	tDesc.wstrModelTag = L"Prototype_Component_Model_SmokeSphereOriginal";
	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshEmitterCommon::PROTOTYPE_TAG, TEXT("Smoke"), &tDesc)))
		return E_FAIL;

	return S_OK;
}

CSplit_Cylinder* CSplit_Cylinder::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CSplit_Cylinder* pInstance = new CSplit_Cylinder(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created: CSplit_Cylinder");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CSplit_Cylinder::Clone(void* pArg)
{
	CSplit_Cylinder* pInstance = new CSplit_Cylinder(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned: CSplit_Cylinder");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CSplit_Cylinder::Free()
{
	__super::Free();
}
