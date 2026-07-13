#include "Effect_RectParticle.h"

#include "Effect_RectCommon.h"
#include "GameInstance.h"

CEffect_RectParticle::CEffect_RectParticle(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEffect_Particle(pDevice, pContext)
{
	Init_PropertyValue();
}

CEffect_RectParticle::CEffect_RectParticle(const CEffect_RectParticle& Prototype)
	: CEffect_Particle(Prototype)
{
	Init_PropertyValue();
}

HRESULT CEffect_RectParticle::Initialize(void* pArg)
{
	EFFECT_RECTPARTICLE_DESC* pDesc = static_cast<EFFECT_RECTPARTICLE_DESC*>(pArg);

	m_iVIBufferLevel = pDesc->iVIBufferLevel;
	m_wstrVIBufferTag = pDesc->wstrVIBufferTag;

	m_bCustomShader = pDesc->bCustomShader;
	m_iShaderLevel = pDesc->iShaderLevel;
	m_wstrShaderTag = pDesc->wstrShaderTag;

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_Components()))
		return E_FAIL;

	return S_OK;
}

HRESULT CEffect_RectParticle::Render()
{
	if (FAILED(Bind_ViewProjectionMatrices()))
		return E_FAIL;

	if (FAILED(Bind_ShaderValue()))
		return E_FAIL;

	if (FAILED(m_pVIBuffer->Bind_Resources()))
		return E_FAIL;

	const _int iPass = Resolve_ShaderPass();

	for (const PARTICLE& Particle : m_Particles)
	{
		if (Particle.bAlive == false)
			continue;

		const _float4x4 ParticleWorld = Make_ParticleWorldMatrix(Particle);
		if (FAILED(EffectRect::Bind_ParticleDrawValues(
			m_pShaderCom, ParticleWorld, Particle.fAlpha, Particle.vColor)))
			return E_FAIL;

		if (FAILED(Bind_ParticleRectValue(Particle)))
			return E_FAIL;

		if (FAILED(EffectRect::Begin_AndRender(m_pShaderCom, m_pVIBuffer, iPass)))
			return E_FAIL;
	}

	return S_OK;
}

HRESULT CEffect_RectParticle::Ready_Components()
{
	if (m_bCustomShader == false)
		m_pShaderCom = m_pGameInstance_Proxy->Get_2DShader();
	else
		m_pShaderCom = Add_Component<CShader>(m_iShaderLevel, m_wstrShaderTag, TEXT("Com_Shader"));
	if (m_pShaderCom == nullptr)
		return E_FAIL;

	m_pVIBuffer = Add_Component<CVIBuffer_Rect>(m_iVIBufferLevel, m_wstrVIBufferTag, TEXT("Com_Buffer"));
	if (m_pVIBuffer == nullptr)
		return E_FAIL;

	return S_OK;
}

HRESULT CEffect_RectParticle::Bind_ShaderValue()
{
	if (FAILED(__super::Bind_ShaderValue()))
		return E_FAIL;

	auto Values = Make_RectValues();
	if (FAILED(EffectRect::Bind_StaticShaderValues(m_pShaderCom, Values)) ||
		FAILED(EffectRect::Bind_SpriteShaderValues(m_pShaderCom, Values)) ||
		FAILED(EffectRect::Bind_Roll(m_pShaderCom, 0.f)))
		return E_FAIL;

	return S_OK;
}

HRESULT CEffect_RectParticle::Bind_ParticleRectValue(const PARTICLE& Particle)
{
	if (m_bUseParticleRoll == true)
	{
		const _float fRoll = XMConvertToRadians(Particle.vRotation.z);
		if (FAILED(EffectRect::Bind_Roll(m_pShaderCom, fRoll)))
			return E_FAIL;
	}

	return S_OK;
}

void CEffect_RectParticle::Update_Core(const _float fTimeDelta, const _float fRatio)
{
	__super::Update_Core(fTimeDelta, fRatio);

	auto Values = Make_RectValues();
	EffectRect::Update_SpriteAnimations(Values, fRatio);
}

void CEffect_RectParticle::Init_PropertyValue()
{
	auto Values = Make_RectValues();
	EffectRect::Initialize_DefaultValues(Values);
	m_bUseParticleRoll = false;
}

EffectRect::VALUES CEffect_RectParticle::Make_RectValues()
{
	return {
		m_bBillboard,
		m_bSpriteAniTexture, m_iTexFrameX, m_iTexFrameY, m_fCurTexAniUV, m_fCurTexAniSize,
		m_bSpriteAniMask, m_iMaskFrameX, m_iMaskFrameY, m_fCurMaskAniUV, m_fCurMaskAniSize
	};
}
