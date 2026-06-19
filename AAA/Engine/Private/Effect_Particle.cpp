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

		if (FAILED(m_pShaderCom->Bind_RawValue("g_vColor", &Particle.vColor, sizeof(Particle.vColor))))
			return E_FAIL;

		Helper::IntClamp(m_iShaderPass, ShaderPass::Default, ShaderPass::ShaderPass_End - 1);
		Helper::IntClamp(m_iMirror, Sampler::DEFAULT, Sampler::SAMPLER_END - 1);

		_int iPass = m_iShaderPass + (m_iMirror == Sampler::MIRROR ? ShaderPass::ShaderPass_End : 0);

		if (FAILED(m_pShaderCom->Begin(iPass)))
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

	Update_TexSpriteAnimation(fTimeDelta, fRatio);
	Update_MaskSpriteAnimation(fTimeDelta, fRatio);
}

void CEffect_Particle::Update_TexSpriteAnimation(const _float fTimeDelta, const _float fRatio)
{
	if (m_bSpriteAniTexture == true)
	{
		_int iTotalCount = m_iTexFrameX * m_iTexFrameY;
		_int iCurTexFrameIndex = static_cast<_int>(static_cast<_float>(iTotalCount) * fRatio);

		if (iCurTexFrameIndex >= iTotalCount)
			iCurTexFrameIndex -= 1;

		_float fCurTexFrameX = static_cast<_float>(iCurTexFrameIndex % m_iTexFrameX);
		_float fCurTexFrameY = static_cast<_float>(iCurTexFrameIndex / m_iTexFrameX);

		m_fCurTexAniSize.x = 1.f / static_cast<_float>(m_iTexFrameX);
		m_fCurTexAniSize.y = 1.f / static_cast<_float>(m_iTexFrameY);

		m_fCurTexAniUV.x = m_fCurTexAniSize.x * fCurTexFrameX;
		m_fCurTexAniUV.y = m_fCurTexAniSize.y * fCurTexFrameY;
	}
}

void CEffect_Particle::Update_MaskSpriteAnimation(const _float fTimeDelta, const _float fRatio)
{
	if (m_bSpriteAniMask == true)
	{
		_int iTotalCount = m_iMaskFrameX * m_iMaskFrameY;
		_int iCurMaskFrameIndex = static_cast<_int>(static_cast<_float>(iTotalCount) * fRatio);

		if (iCurMaskFrameIndex >= iTotalCount)
			iCurMaskFrameIndex -= 1;

		_float fCurMaskFrameX = static_cast<_float>(iCurMaskFrameIndex % m_iMaskFrameX);
		_float fCurMaskFrameY = static_cast<_float>(iCurMaskFrameIndex / m_iMaskFrameX);

		m_fCurMaskAniSize.x = 1.f / static_cast<_float>(m_iMaskFrameX);
		m_fCurMaskAniSize.y = 1.f / static_cast<_float>(m_iMaskFrameY);

		m_fCurMaskAniUV.x = m_fCurMaskAniSize.x * fCurMaskFrameX;
		m_fCurMaskAniUV.y = m_fCurMaskAniSize.y * fCurMaskFrameY;
	}
}

void CEffect_Particle::Init_PropertyValue()
{
	m_bSpriteAniTexture = false;
	m_iTexFrameX = 1;
	m_iTexFrameY = 1;

	m_bSpriteAniMask = false;
	m_iMaskFrameX = 1;
	m_iMaskFrameY = 1;

	// Particle
	m_iParticleCount = 20;

	// Particle Spawn
	m_bParticleSpawnRandom = true;
	m_fParticleSpawnStartRatio = 0.f;
	m_fParticleSpawnEndRatio = 0.7f;
	m_fParticleLifeRatio = 0.25f;

	//  Particle Move
	m_iParticleMoveMode = PARTICLE_MOVE_SPREAD;
	m_fParticleStartSpeed = 6.f;
	m_fParticleFountainSpread = 1.f;
	m_fParticleFountainUpBias = 1.5f;
	m_fParticleFountainGravity = 9.8f;

	// Particle Alpha
	m_fParticleAlpha = 1.f;

	m_bParticleAlphaChange = true;
	m_fParticleAlphaStartValue = 0.f;
	m_fParticleAlphaEndValue = 0.f;

	m_bActive_ParticleAlpha_Ratio_0 = true;
	m_fParticleAlpha_Ratio_0 = 0.3f;
	m_fParticleAlpha_Value_0 = 1.f;

	m_bActive_ParticleAlpha_Ratio_1 = true;
	m_fParticleAlpha_Ratio_1 = 0.7f;
	m_fParticleAlpha_Value_1 = 1.f;

	// Particle Size
	m_fParticleStartSize = 0.5f;

	m_bParticleRandomSize = false;
	m_vParticleStartSizeRange = { 0.3f, 0.7f };

	m_bParticleSizeChange = true;
	m_fParticleSizeStartValue = 0.1f;
	m_fParticleSizeEndValue = 0.1f;

	m_bActive_ParticleSize_Ratio_0 = true;
	m_fParticleSize_Ratio_0 = 0.5f;
	m_fParticleSize_Value_0 = 1.f;

	m_bActive_ParticleSize_Ratio_1 = false;
	m_fParticleSize_Ratio_1 = 0.75f;
	m_fParticleSize_Value_1 = 1.f;

	// Particle Color
	m_vParticleColor = { 1.f, 1.f, 1.f };

	m_bParticleColorChange = false;
	m_vParticleColorStartValue = { 1.f, 1.f, 1.f };
	m_vParticleColorEndValue = { 1.f, 1.f, 1.f };

	m_bActive_ParticleColor_Ratio_0 = false;
	m_fParticleColor_Ratio_0 = 0.5f;
	m_vParticleColor_Value_0 = { 1.f, 1.f, 1.f };

	m_bActive_ParticleColor_Ratio_1 = false;
	m_fParticleColor_Ratio_1 = 0.75f;
	m_vParticleColor_Value_1 = { 1.f, 1.f, 1.f };
}

void CEffect_Particle::Reset_Particles()
{
	// Particle
	if (m_iParticleCount < 1)
		m_iParticleCount = 1;

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

		// Size
		_float fParticleSize = m_fParticleStartSize;

		if (m_bParticleRandomSize == true)
		{
			_float fMinSize = m_vParticleStartSizeRange.x;
			_float fMaxSize = m_vParticleStartSizeRange.y;

			if (fMaxSize < fMinSize)
				std::swap(fMinSize, fMaxSize);

			if (fMinSize < Helper::fEpsilon)
				fMinSize = Helper::fEpsilon;

			if (fMaxSize < Helper::fEpsilon)
				fMaxSize = Helper::fEpsilon;

			fParticleSize = m_pGameInstance_Proxy->RandomFloat(fMinSize, fMaxSize);
		}

		if (fParticleSize < Helper::fEpsilon)
			fParticleSize = Helper::fEpsilon;

		Particle.fBaseSize = fParticleSize;
		Particle.vScale = { fParticleSize, fParticleSize, fParticleSize };

		// Alpha
		Particle.fAlpha = m_fParticleAlpha;

		// Color
		Particle.vColor = m_vParticleColor;
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

		Update_ParticleMove(Particle, fRatio, fLocalRatio);
		Update_ParticleAlpha(Particle, fLocalRatio);
		Update_ParticleSize(Particle, fLocalRatio);
		Update_ParticleColor(Particle, fLocalRatio);
	}
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
		Particle.vLocalPos.y -= 0.5f * m_fParticleFountainGravity * fElapsedTime * fElapsedTime;
}

void CEffect_Particle::Update_ParticleAlpha(PARTICLE& Particle, _float fLocalRatio)
{
	_float fAlpha = Evaluate_ParticleFloatCurve(
		fLocalRatio, m_fParticleAlpha, m_bParticleAlphaChange,
		m_fParticleAlphaStartValue, m_fParticleAlphaEndValue,
		m_bActive_ParticleAlpha_Ratio_0, m_fParticleAlpha_Ratio_0, m_fParticleAlpha_Value_0,
		m_bActive_ParticleAlpha_Ratio_1, m_fParticleAlpha_Ratio_1, m_fParticleAlpha_Value_1);

	Helper::FloatClamp(fAlpha, 0.f, 1.f);

	Particle.fAlpha = fAlpha;
}

void CEffect_Particle::Update_ParticleSize(PARTICLE& Particle, _float fLocalRatio)
{
	_float fSizeRatio = Evaluate_ParticleFloatCurve(
		fLocalRatio, 1.f, m_bParticleSizeChange,
		m_fParticleSizeStartValue, m_fParticleSizeEndValue,
		m_bActive_ParticleSize_Ratio_0, m_fParticleSize_Ratio_0, m_fParticleSize_Value_0,
		m_bActive_ParticleSize_Ratio_1, m_fParticleSize_Ratio_1, m_fParticleSize_Value_1);

	_float fSize = Particle.fBaseSize * fSizeRatio;

	if (fSize < Helper::fEpsilon)
		fSize = Helper::fEpsilon;

	Particle.vScale = { fSize, fSize, fSize };
}

void CEffect_Particle::Update_ParticleColor(PARTICLE& Particle, _float fLocalRatio)
{
	Particle.vColor = Evaluate_ParticleFloat3Curve(
		fLocalRatio, m_vParticleColor, m_bParticleColorChange,
		m_vParticleColorStartValue, m_vParticleColorEndValue,
		m_bActive_ParticleColor_Ratio_0, m_fParticleColor_Ratio_0, m_vParticleColor_Value_0,
		m_bActive_ParticleColor_Ratio_1, m_fParticleColor_Ratio_1, m_vParticleColor_Value_1);
}

_float CEffect_Particle::Evaluate_ParticleFloatCurve(_float fLocalRatio, _float fFixedValue, _bool bChange, _float fStartValue, _float fEndValue, _bool bActiveRatio0, _float fRatio0, _float fValue0, _bool bActiveRatio1, _float fRatio1, _float fValue1) const
{
	if (bChange == false)
		return fFixedValue;

	RATIO_VALUE Points[4]{};
	_uint iCount = 0;

	Points[iCount++] = { 0.f, fStartValue };

	if (bActiveRatio0 == true)
		Points[iCount++] = { fRatio0, fValue0 };

	if (bActiveRatio1 == true)
		Points[iCount++] = { fRatio1, fValue1 };

	Points[iCount++] = { 1.f, fEndValue };

	for (_uint i = 0; i < iCount; ++i)
		Helper::FloatClamp(Points[i].fRatio, 0.f, 1.f);

	for (_uint i = 0; i < iCount; ++i)
	{
		for (_uint j = i + 1; j < iCount; ++j)
		{
			if (Points[j].fRatio < Points[i].fRatio)
				std::swap(Points[i], Points[j]);
		}
	}

	Helper::FloatClamp(fLocalRatio, 0.f, 1.f);

	for (_uint i = 0; i + 1 < iCount; ++i)
	{
		if (fLocalRatio <= Points[i + 1].fRatio)
		{
			const _float fRange = Points[i + 1].fRatio - Points[i].fRatio;

			if (fRange <= Helper::fEpsilon)
				return Points[i + 1].fValue;

			const _float fStep = Helper::FloatSmoothStep(
				Points[i].fRatio,
				Points[i + 1].fRatio,
				fLocalRatio);

			return Points[i].fValue + (Points[i + 1].fValue - Points[i].fValue) * fStep;
		}
	}

	return Points[iCount - 1].fValue;
}

_float3 CEffect_Particle::Evaluate_ParticleFloat3Curve(_float fLocalRatio, const _float3& vFixedValue, _bool bChange, const _float3& vStartValue, const _float3& vEndValue, _bool bActiveRatio0, _float fRatio0, const _float3& vValue0, _bool bActiveRatio1, _float fRatio1, const _float3& vValue1) const
{
	if (bChange == false)
		return vFixedValue;

	RATIO_VALUE_FLOAT3 Points[4]{};
	_uint iCount = 0;

	Points[iCount++] = { 0.f, vStartValue };

	if (bActiveRatio0 == true)
		Points[iCount++] = { fRatio0, vValue0 };

	if (bActiveRatio1 == true)
		Points[iCount++] = { fRatio1, vValue1 };

	Points[iCount++] = { 1.f, vEndValue };

	for (_uint i = 0; i < iCount; ++i)
		Helper::FloatClamp(Points[i].fRatio, 0.f, 1.f);

	for (_uint i = 0; i < iCount; ++i)
	{
		for (_uint j = i + 1; j < iCount; ++j)
		{
			if (Points[j].fRatio < Points[i].fRatio)
				std::swap(Points[i], Points[j]);
		}
	}

	Helper::FloatClamp(fLocalRatio, 0.f, 1.f);

	for (_uint i = 0; i + 1 < iCount; ++i)
	{
		if (fLocalRatio <= Points[i + 1].fRatio)
		{
			const _float fRange = Points[i + 1].fRatio - Points[i].fRatio;

			if (fRange <= Helper::fEpsilon)
				return Points[i + 1].vValue;

			const _float fStep = Helper::FloatSmoothStep(
				Points[i].fRatio,
				Points[i + 1].fRatio,
				fLocalRatio);

			_float3 vResult{};
			vResult.x = Points[i].vValue.x + (Points[i + 1].vValue.x - Points[i].vValue.x) * fStep;
			vResult.y = Points[i].vValue.y + (Points[i + 1].vValue.y - Points[i].vValue.y) * fStep;
			vResult.z = Points[i].vValue.z + (Points[i + 1].vValue.z - Points[i].vValue.z) * fStep;

			return vResult;
		}
	}

	return Points[iCount - 1].vValue;
}

void CEffect_Particle::Free()
{
	__super::Free();
}