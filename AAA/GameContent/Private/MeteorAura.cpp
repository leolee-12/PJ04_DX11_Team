#include "MeteorAura.h"
#include "GameInstance.h"
#include "GameContent_const.h"

#include "MeshCommon.h"

CMeteorAura::CMeteorAura(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEffect_Container{ pDevice, pContext }
{
}

CMeteorAura::CMeteorAura(const CMeteorAura& Prototype)
	: CEffect_Container(Prototype)
{
}

HRESULT CMeteorAura::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CMeteorAura::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_EffectPartObjects()))
		return E_FAIL;

	return S_OK;
}

void CMeteorAura::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);
}

void CMeteorAura::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
}

void CMeteorAura::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
}

HRESULT CMeteorAura::Render()
{
	if (FAILED(__super::Render()))
		return E_FAIL;

	return S_OK;
}

HRESULT CMeteorAura::Ready_EffectPartObjects()
{
	// Meteor_Spere exposes a DIFFUSE slot (IceBaseColor.png, white dummy).
	CMeshCommon::MESH_COMMON_DESC tShell{};
	tShell.iModelLevel			= m_iPrototypeLevel;
	tShell.bUseDiffuseTexture	= true;
	tShell.bUseNormalTexture	= false;
	tShell.bUseMRATexture		= false;
	tShell.bUseUnknownTexture	= false;
	tShell.bUseTextureCom		= false;
	tShell.bUseMaskCom			= false;
	tShell.bCustomShader		= false;
	tShell.iShaderLevel			= 0;
	tShell.wstrShaderTag		= L"";
	tShell.wstrModelTag			= SHELL_MODEL_TAG;

	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel,
		CMeshCommon::PROTOTYPE_TAG, L"Shell", &tShell)))
		return E_FAIL;

	// Fire meshes expose an UNKNOWN slot only (Smoke.png dummy).
	CMeshCommon::MESH_COMMON_DESC tFire{};
	tFire.iModelLevel        = m_iPrototypeLevel;
	tFire.bUseDiffuseTexture = false;
	tFire.bUseNormalTexture  = false;
	tFire.bUseMRATexture     = false;
	tFire.bUseUnknownTexture = true;
	tFire.bUseTextureCom     = false;
	tFire.bUseMaskCom        = false;
	tFire.bCustomShader      = false;
	tFire.iShaderLevel       = 0;
	tFire.wstrShaderTag      = L"";

	tFire.wstrModelTag = FIRE01_MODEL_TAG;

	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel,
		CMeshCommon::PROTOTYPE_TAG, L"Tail_00", &tFire)))
		return E_FAIL;

	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel,
		CMeshCommon::PROTOTYPE_TAG, L"Tail_01", &tFire)))
		return E_FAIL;

	tFire.wstrModelTag = FIRE02_MODEL_TAG;

	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel,
		CMeshCommon::PROTOTYPE_TAG, L"Tail_02", &tFire)))
		return E_FAIL;

	return S_OK;
}

CMeteorAura* CMeteorAura::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CMeteorAura* pInstance = new CMeteorAura(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CMeteorAura");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CMeteorAura::Clone(void* pArg)
{
	CMeteorAura* pInstance = new CMeteorAura(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CMeteorAura");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CMeteorAura::Free()
{
	__super::Free();
}
