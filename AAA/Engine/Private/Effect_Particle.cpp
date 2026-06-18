#include "Effect_Particle.h"

#include "GameInstance.h"

CEffect_Particle::CEffect_Particle(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEffect_Part(pDevice, pContext)
{
	Init_PropertyValue();
}

CEffect_Particle::CEffect_Particle(const CEffect_Particle& Prototype)
	: CEffect_Part(Prototype)
{
	Init_PropertyValue();
}

HRESULT CEffect_Particle::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CEffect_Particle::Initialize(void* pArg)
{
	EFFECT_PARTICLE_DESC* pDesc = static_cast<EFFECT_PARTICLE_DESC*>(pArg);

	m_iVIBufferLevel = pDesc->iVIBufferLevel;
	m_wstrVIBufferTag = pDesc->wstrVIBufferTag;

	m_bCustomShader = pDesc->bCustomShader;
	m_iShaderLevel = pDesc->iShaderLevel;
	m_wstrShaderTag = pDesc->wstrShaderTag;

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_Components()))
		return E_FAIL;

	Reset_Particles();

	return S_OK;
}

void CEffect_Particle::Priority_Update(_float fTimeDelta)
{
}

void CEffect_Particle::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
}

void CEffect_Particle::Late_Update(_float fTimeDelta)
{
	Compute_CombinedWorldMatrix();
}

HRESULT CEffect_Particle::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	if (FAILED(Bind_ShaderValue()))
		return E_FAIL;

	Helper::IntClamp(m_iShaderPass, ShaderPass::Default, ShaderPass::ShaderPass_End - 1);
	if (FAILED(m_pShaderCom->Begin(m_iShaderPass)))
		return E_FAIL;

	if (FAILED(m_pVIBuffer->Bind_Resources()))
		return E_FAIL;

	for (const PARTICLE& Particle : m_Particles)
	{
		if (Particle.bAlive == false)
			continue;

		_float4x4 ParticleWorld = Make_ParticleWorldMatrix(Particle);

		if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", &ParticleWorld)))
			return E_FAIL;

		if (FAILED(m_pShaderCom->Bind_RawValue("g_fAlpha", &Particle.fAlpha, sizeof(Particle.fAlpha))))
			return E_FAIL;

		if (FAILED(m_pShaderCom->Begin(m_iShaderPass)))
			return E_FAIL;

		if (FAILED(m_pVIBuffer->Render()))
			return E_FAIL;
	}

	return S_OK;
}

void CEffect_Particle::Effect_Start()
{
	__super::Effect_Start();

	Reset_Particles();
}

HRESULT CEffect_Particle::Ready_Components()
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

HRESULT CEffect_Particle::Bind_ShaderResources()
{
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::VIEW, m_eProjType))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::PROJ, m_eProjType))))
		return E_FAIL;

	return S_OK;
}

HRESULT CEffect_Particle::Bind_ShaderValue()
{
	if (FAILED(__super::Bind_ShaderValue()))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_RawValue("g_bSpriteAniTexture", &m_bSpriteAniTexture, sizeof(m_bSpriteAniTexture))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_vSpriteAniTexUV", &m_fCurTexAniUV, sizeof(m_fCurTexAniUV))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_vSpriteAniTexSize", &m_fCurTexAniSize, sizeof(m_fCurTexAniSize))))
		return E_FAIL;


	if (FAILED(m_pShaderCom->Bind_RawValue("g_bSpriteAniMask", &m_bSpriteAniMask, sizeof(m_bSpriteAniMask))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_vSpriteAniMaskUV", &m_fCurMaskAniUV, sizeof(m_fCurMaskAniUV))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_vSpriteAniMaskSize", &m_fCurMaskAniSize, sizeof(m_fCurMaskAniSize))))
		return E_FAIL;

	return S_OK;
}

void CEffect_Particle::Update_Core(const _float fTimeDelta, const _float fRatio)
{
	__super::Update_Core(fTimeDelta, fRatio);
	Update_Particles_ByContainerTime(fRatio);
}

void CEffect_Particle::Init_PropertyValue()
{
	m_bSpriteAniTexture = false;
	m_iTexFrameX = 1;
	m_iTexFrameY = 1;

	m_bSpriteAniMask = false;
	m_iMaskFrameX = 1;
	m_iMaskFrameY = 1;

	// Alpha
	m_bParticleFadeInOut = true;
	m_fParticleAlphaStartValue = 0.f;
	m_fParticleAlphaPeakValue = 1.f;
	m_fParticleAlphaEndValue = 0.f;
	m_fParticleFadeInRatio = 0.3f;
	m_fParticleFadeOutRatio = 0.5f;

	// Spawn
	m_bParticleSpawnRandom = true;
	m_fParticleSpawnStartRatio = 0.f;
	m_fParticleSpawnEndRatio = 0.7f;
	m_fParticleLifeRatio = 0.25f;

	// Move
	m_iParticleMoveMode = PARTICLE_MOVE_SPREAD;
	m_fParticleStartSpeed = 6.f;
	m_fParticleFountainSpread = 1.f;
	m_fParticleFountainUpBias = 1.5f;
	m_fParticleFountainGravity = 9.8f;
}

void CEffect_Particle::Reset_Particles()
{
	m_iParticleCount = PARTICLE_MAX_COUNT;

	m_Particles.clear();
	m_Particles.resize(m_iParticleCount);

	for (_uint i = 0; i < m_iParticleCount; ++i)
	{
		PARTICLE& Particle = m_Particles[i];

		// spawn/move 초기화
		_float fLifeRatio = m_fParticleLifeRatio;
		Helper::FloatClamp(fLifeRatio, Helper::fEpsilon, 1.f);

		_float fSpawnStartRatio = m_fParticleSpawnStartRatio;
		_float fSpawnEndRatio = m_fParticleSpawnEndRatio;

		const _float fMaxSpawnRatio = 1.f - fLifeRatio;

		Helper::FloatClamp(fSpawnStartRatio, 0.f, fMaxSpawnRatio);
		Helper::FloatClamp(fSpawnEndRatio, 0.f, fMaxSpawnRatio);

		if (fSpawnEndRatio < fSpawnStartRatio)
			std::swap(fSpawnStartRatio, fSpawnEndRatio);

		Particle.fStartRatio = m_bParticleSpawnRandom
			? m_pGameInstance_Proxy->RandomFloat(fSpawnStartRatio, fSpawnEndRatio)
			: fSpawnStartRatio;

		Particle.fEndRatio = Particle.fStartRatio + fLifeRatio;

		// 방향/속도 세팅
		Helper::IntClamp(m_iParticleMoveMode, PARTICLE_MOVE_SPREAD, PARTICLE_MOVE_END - 1);

		_vector vDir = XMVectorZero();

		switch (m_iParticleMoveMode)
		{
		case PARTICLE_MOVE_FOUNTAIN:
			vDir = Make_FountainDirection();
			break;

		case PARTICLE_MOVE_SPREAD:
		default:
			vDir = Make_SpreadDirection3D();
			break;
		}

		XMStoreFloat3(&Particle.vVelocity, vDir * m_fParticleStartSpeed);


		// Alpha
		Particle.fFadeInRatio = m_fParticleFadeInRatio;
		Particle.fFadeOutRatio = m_fParticleFadeOutRatio;
		Particle.fAlphaStartValue = m_fParticleAlphaStartValue;
		Particle.fAlphaPeakValue = m_fParticleAlphaPeakValue;
		Particle.fAlphaEndValue = m_fParticleAlphaEndValue;
		Particle.fAlpha = Particle.fAlphaStartValue;

		Particle.vLocalPos = m_fPivot;
		XMStoreFloat3(&Particle.vVelocity, vDir * m_fParticleStartSpeed);
		Particle.vScale = { m_fParticleSize, m_fParticleSize, m_fParticleSize };
	}
}

_float4x4 CEffect_Particle::Make_ParticleWorldMatrix(const PARTICLE& Particle) const
{
	_matrix matScale = XMMatrixScaling(Particle.vScale.x, Particle.vScale.y, Particle.vScale.z);

	_matrix matTranslation = XMMatrixTranslation(Particle.vLocalPos.x, Particle.vLocalPos.y, Particle.vLocalPos.z);

	_matrix matWorld = matScale * matTranslation * XMLoadFloat4x4(&m_CombinedWorldMatrix);

	_float4x4 ParticleWorld{};
	XMStoreFloat4x4(&ParticleWorld, matWorld);

	return ParticleWorld;
}

void CEffect_Particle::Update_Particles_ByContainerTime(_float fRatio)
{
	for (PARTICLE& Particle : m_Particles)
	{
		_float fLocalRatio = (fRatio - Particle.fStartRatio) / (Particle.fEndRatio - Particle.fStartRatio);

		if (fLocalRatio < 0.f || fLocalRatio > 1.f)
		{
			Particle.bAlive = false;
			continue;
		}

		Particle.bAlive = true;

		Particle.fAlpha = Evaluate_ParticleAlpha(Particle, fLocalRatio);





		Update_ParticleMove(Particle, fRatio, fLocalRatio);
	}
}

_float CEffect_Particle::Evaluate_ParticleAlpha(const PARTICLE& Particle, _float fLocalRatio) const
{
	if (m_bParticleFadeInOut == false)
		return 1.f;

	Helper::FloatClamp(fLocalRatio, 0.f, 1.f);

	_float fFadeInRatio = Particle.fFadeInRatio;
	_float fFadeOutRatio = Particle.fFadeOutRatio;

	Helper::FloatClamp(fFadeInRatio, 0.f, 1.f);
	Helper::FloatClamp(fFadeOutRatio, 0.f, 1.f);

	const _float fFadeTotal = fFadeInRatio + fFadeOutRatio;
	if (fFadeTotal > 1.f)
	{
		fFadeInRatio /= fFadeTotal;
		fFadeOutRatio /= fFadeTotal;
	}

	_float fAlpha = Particle.fAlphaPeakValue;

	if (fFadeInRatio > Helper::fEpsilon && fLocalRatio < fFadeInRatio)
	{
		const _float fStep = Helper::FloatSmoothStep(0.f, fFadeInRatio, fLocalRatio);
		fAlpha = Particle.fAlphaStartValue + (Particle.fAlphaPeakValue - Particle.fAlphaStartValue) * fStep;
	}
	else if (fFadeOutRatio > Helper::fEpsilon && fLocalRatio > 1.f - fFadeOutRatio)
	{
		const _float fStep = Helper::FloatSmoothStep(1.f - fFadeOutRatio, 1.f, fLocalRatio);
		fAlpha = Particle.fAlphaPeakValue + (Particle.fAlphaEndValue - Particle.fAlphaPeakValue) * fStep;
	}

	Helper::FloatClamp(fAlpha, 0.f, 1.f);
	return fAlpha;
}

_vector CEffect_Particle::Make_SpreadDirection3D() const
{
	const _float fY = m_pGameInstance_Proxy->RandomFloat(-1.f, 1.f);
	const _float fTheta = m_pGameInstance_Proxy->RandomFloat(0.f, XM_2PI);

	const _float fRadius = sqrtf(max(0.f, 1.f - fY * fY));
	const _float fX = cosf(fTheta) * fRadius;
	const _float fZ = sinf(fTheta) * fRadius;

	return XMVector3Normalize(XMVectorSet(fX, fY, fZ, 0.f));
}

_vector CEffect_Particle::Make_FountainDirection() const
{
	_float fSpread = m_fParticleFountainSpread;
	_float fUpBias = m_fParticleFountainUpBias;

	Helper::FloatClamp(fSpread, 0.f, 10.f);
	if (fUpBias < Helper::fEpsilon)
		fUpBias = Helper::fEpsilon;

	const _float fTheta = m_pGameInstance_Proxy->RandomFloat(0.f, XM_2PI);
	const _float fHorizontal = m_pGameInstance_Proxy->RandomFloat(0.f, fSpread);

	const _float fX = cosf(fTheta) * fHorizontal;
	const _float fZ = sinf(fTheta) * fHorizontal;

	return XMVector3Normalize(XMVectorSet(fX, fUpBias, fZ, 0.f));
}

void CEffect_Particle::Update_ParticleMove(PARTICLE& Particle, _float fRatio, _float fLocalRatio)
{
	Helper::FloatClamp(fLocalRatio, 0.f, 1.f);
	Helper::IntClamp(m_iParticleMoveMode, PARTICLE_MOVE_SPREAD, PARTICLE_MOVE_END - 1);

	const _float fElapsedRatio = fRatio - Particle.fStartRatio;
	const _float fElapsedTime = fElapsedRatio * m_fDuration;

	Particle.vLocalPos.x = m_fPivot.x + Particle.vVelocity.x * fElapsedTime;
	Particle.vLocalPos.y = m_fPivot.y + Particle.vVelocity.y * fElapsedTime;
	Particle.vLocalPos.z = m_fPivot.z + Particle.vVelocity.z * fElapsedTime;

	if (m_iParticleMoveMode == PARTICLE_MOVE_FOUNTAIN)
	{
		Particle.vLocalPos.y -=
			0.5f * m_fParticleFountainGravity * fElapsedTime * fElapsedTime;
	}
}

void CEffect_Particle::Free()
{
	__super::Free();
}