#include "Effect_Trail.h"

#include "GameInstance.h"
#include "VIBuffer_Trail.h"

namespace
{
    _float Distance(const _float3& vA, const _float3& vB)
    {
        return XMVectorGetX(XMVector3Length(XMLoadFloat3(&vB) - XMLoadFloat3(&vA)));
    }

    _float3 Lerp(const _float3& vA, const _float3& vB, _float fRatio)
    {
        _float3 vResult{};
        XMStoreFloat3(&vResult, XMVectorLerp(XMLoadFloat3(&vA), XMLoadFloat3(&vB), fRatio));
        return vResult;
    }
}

CEffect_Trail::CEffect_Trail(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CEffect_NonParticle(pDevice, pContext)
{
    Init_PropertyValue();
}

CEffect_Trail::CEffect_Trail(const CEffect_Trail& Prototype)
    : CEffect_NonParticle(Prototype)
{
    Init_PropertyValue();
}

HRESULT CEffect_Trail::Initialize(void* pArg)
{
    if (pArg == nullptr)
        return E_FAIL;

    EFFECT_TRAIL_DESC* pDesc = static_cast<EFFECT_TRAIL_DESC*>(pArg);
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

void CEffect_Trail::Update(_float fTimeDelta)
{
    __super::Update(fTimeDelta);

    if (m_pVIBuffer != nullptr)
    {
        Configure_Buffer();
        m_pVIBuffer->Update(fTimeDelta);
    }
}

void CEffect_Trail::Late_Update(_float fTimeDelta)
{
    __super::Late_Update(fTimeDelta);

    Compute_CombinedWorldMatrix();

    // sampling
    if (m_bAutoSample == true && m_bEmitting == true && m_bActive == true)
        Sample_Automatically(fTimeDelta);
    else if (m_bHasPreviousFrame == true)
        Begin_NewSegment();
    else
        Reset_SamplingState();
}

HRESULT CEffect_Trail::Render()
{
    if (m_pVIBuffer == nullptr || m_pVIBuffer->Is_Renderable() == false)
        return S_OK;

    if (FAILED(Bind_ShaderResources()))
        return E_FAIL;
    if (FAILED(Bind_ShaderValue()))
        return E_FAIL;

    if (FAILED(m_pShaderCom->Begin(Resolve_ShaderPass())))
        return E_FAIL;
    if (FAILED(m_pVIBuffer->Bind_Resources()))
        return E_FAIL;

    return m_pVIBuffer->Render();
}

void CEffect_Trail::Effect_Start()
{
    __super::Effect_Start();
    Clear_Trail();
    Start_Emission();
}

void CEffect_Trail::Start_Emission()
{
    m_bEmitting = true;
    Begin_NewSegment();
}

void CEffect_Trail::Stop_Emission()
{
    m_bEmitting = false;
    Begin_NewSegment();
}

void CEffect_Trail::Clear_Trail()
{
    if (m_pVIBuffer != nullptr)
        m_pVIBuffer->Clear();
    Reset_SamplingState();
    m_bHasLastEmitted = false;
}

void CEffect_Trail::Begin_NewSegment()
{
    if (m_pVIBuffer != nullptr)
        m_pVIBuffer->Begin_NewSegment();
    Reset_SamplingState();
    m_bHasLastEmitted = false;
}

void CEffect_Trail::Set_SourceMatrix(const _float4x4* pSourceMatrix)
{
    if (m_pSourceMatrix == pSourceMatrix)
        return;

    m_pSourceMatrix = pSourceMatrix;

    Begin_NewSegment();
}

void CEffect_Trail::Push_WorldSample(const _float3& vBase, const _float3& vTip)
{
    if (m_pVIBuffer == nullptr || m_bEmitting == false)
        return;

    Push_FilteredSample(vBase, vTip);
}

_bool CEffect_Trail::Is_TrailRenderable() const
{
    return m_pVIBuffer != nullptr && m_pVIBuffer->Is_Renderable();
}

_bool CEffect_Trail::Is_TrailFinished() const
{
    return m_bEmitting == false && (m_pVIBuffer == nullptr || m_pVIBuffer->Is_Empty());
}

void CEffect_Trail::On_Deserialized()
{
    __super::On_Deserialized();

    Configure_Buffer();
}

HRESULT CEffect_Trail::Ready_Components()
{
    if (m_bCustomShader == false)
        m_pShaderCom = m_pGameInstance_Proxy->Get_TrailShader();
    else
        m_pShaderCom = Add_Component<CShader>(m_iShaderLevel, m_wstrShaderTag, TEXT("Com_Shader"));

    if (m_pShaderCom == nullptr)
        return E_FAIL;

    CVIBuffer_Trail::TRAIL_DESC TrailDesc{};
    TrailDesc.iMaxSamples = m_iMaxSamples;
    TrailDesc.fSampleLifeTime = m_fSampleLifeTime;
    TrailDesc.iSmoothSegments = m_iSmoothSegments;
    TrailDesc.fSplineSmoothness = m_fSplineSmoothness;
    TrailDesc.bDistanceUV = m_bDistanceUV;
    TrailDesc.fTailWidthScale = m_fTailWidthScale;
    TrailDesc.fHeadWidthScale = m_fHeadWidthScale;

    m_pVIBuffer = Add_Component<CVIBuffer_Trail>(m_iVIBufferLevel, m_wstrVIBufferTag, TEXT("Com_Buffer"), &TrailDesc);

    if (m_pVIBuffer == nullptr)
        return E_FAIL;

    return S_OK;
}

HRESULT CEffect_Trail::Configure_Buffer()
{
    // Effect 프로퍼티와 버퍼의 현재 설정을 비교합니다. 모두 같으면 아무것도 하지 않고, 달라졌을 때만 버퍼의 Configure()를 호출.
    if (m_pVIBuffer == nullptr)
        return E_FAIL;

    CVIBuffer_Trail::TRAIL_DESC TrailDesc{};
    TrailDesc.iMaxSamples = m_iMaxSamples;
    TrailDesc.fSampleLifeTime = m_fSampleLifeTime;
    TrailDesc.iSmoothSegments = m_iSmoothSegments;
    TrailDesc.fSplineSmoothness = m_fSplineSmoothness;
    TrailDesc.bDistanceUV = m_bDistanceUV;
    TrailDesc.fTailWidthScale = m_fTailWidthScale;
    TrailDesc.fHeadWidthScale = m_fHeadWidthScale;

    const CVIBuffer_Trail::TRAIL_DESC& CurrentDesc = m_pVIBuffer->Get_Description();
    _float fExpectedSmoothness = TrailDesc.fSplineSmoothness;
    Helper::FloatClamp(fExpectedSmoothness, 0.f, 1.f);
    if (CurrentDesc.iMaxSamples == (std::max)(static_cast<_uint>(2), (std::min)(TrailDesc.iMaxSamples, 512u)) &&
        CurrentDesc.iSmoothSegments == (std::max)(static_cast<_uint>(1), (std::min)(TrailDesc.iSmoothSegments, 16u)) &&
        fabsf(CurrentDesc.fSampleLifeTime - (std::max)(TrailDesc.fSampleLifeTime, Helper::fEpsilon)) <= Helper::fEpsilon &&
        fabsf(CurrentDesc.fSplineSmoothness - fExpectedSmoothness) <= Helper::fEpsilon &&
        CurrentDesc.bDistanceUV == TrailDesc.bDistanceUV &&
        fabsf(CurrentDesc.fTailWidthScale - (std::max)(TrailDesc.fTailWidthScale, 0.f)) <= Helper::fEpsilon &&
        fabsf(CurrentDesc.fHeadWidthScale - (std::max)(TrailDesc.fHeadWidthScale, 0.f)) <= Helper::fEpsilon)
        return S_OK;

    return m_pVIBuffer->Configure(TrailDesc);
}

HRESULT CEffect_Trail::Bind_ShaderResources()
{
    _float4x4 matIdentity{};
    XMStoreFloat4x4(&matIdentity, XMMatrixIdentity());

    if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", &matIdentity)))
        return E_FAIL;

    return Bind_ViewProjectionMatrices();
}

HRESULT CEffect_Trail::Bind_ShaderValue()
{
    if (FAILED(__super::Bind_ShaderValue()))
        return E_FAIL;

    Helper::FloatClamp(m_fEdgeSoftness, 0.f, 1.f);
    Helper::FloatClamp(m_fHeadFadeRatio, 0.f, 1.f);
    Helper::FloatClamp(m_fTailFadeRatio, 0.f, 1.f);
    m_fSampleLifeTime = (std::max)(m_fSampleLifeTime, Helper::fEpsilon);

    if (FAILED(m_pShaderCom->Bind_RawValue("g_fTrailLifeTime", &m_fSampleLifeTime, sizeof(m_fSampleLifeTime))))
        return E_FAIL;

    if (FAILED(m_pShaderCom->Bind_RawValue("g_fTrailEdgeSoftness", &m_fEdgeSoftness, sizeof(m_fEdgeSoftness))))
        return E_FAIL;

    if (FAILED(m_pShaderCom->Bind_RawValue("g_fTrailHeadFadeRatio", &m_fHeadFadeRatio, sizeof(m_fHeadFadeRatio))))
        return E_FAIL;

    if (FAILED(m_pShaderCom->Bind_RawValue("g_fTrailTailFadeRatio", &m_fTailFadeRatio, sizeof(m_fTailFadeRatio))))
        return E_FAIL;

    return S_OK;
}

void CEffect_Trail::Sample_Automatically(_float fTimeDelta)
{
    // 프레임 사이를 일정한 시간 간격으로 나누어 Sample을 만든다.
    _float3 vCurrentBase{};
    _float3 vCurrentTip{};
    Compute_WorldEndpoints(vCurrentBase, vCurrentTip);

    if (m_bHasPreviousFrame == false)
    {
        // 너무 가까운 중복 Sample을 제거
        Push_FilteredSample(vCurrentBase, vCurrentTip);
        m_vPreviousFrameBase = vCurrentBase;
        m_vPreviousFrameTip = vCurrentTip;
        m_bHasPreviousFrame = true;
        m_fSampleAccumulator = 0.f;
        return;
    }

    const _float fFrameMovement = (std::max)(Distance(m_vPreviousFrameBase, vCurrentBase), Distance(m_vPreviousFrameTip, vCurrentTip));
    if (m_fTeleportDistance > Helper::fEpsilon && fFrameMovement > m_fTeleportDistance)
    {
        Begin_NewSegment();
        Push_FilteredSample(vCurrentBase, vCurrentTip);
        m_vPreviousFrameBase = vCurrentBase;
        m_vPreviousFrameTip = vCurrentTip;
        m_bHasPreviousFrame = true;
        return;
    }

    const _float fDelta = (std::max)(fTimeDelta, 0.f);
    const _float fInterval = (std::max)(m_fSampleInterval, 0.f);
    if (fDelta <= Helper::fEpsilon || fInterval <= Helper::fEpsilon)
    {
        Push_FilteredSample(vCurrentBase, vCurrentTip);
        m_fSampleAccumulator = 0.f;
    }
    else
    {
        const _float fOldAccumulator = m_fSampleAccumulator;
        const _float fTotalTime = fOldAccumulator + fDelta;
        _float fSampleTime = fInterval - fOldAccumulator;
        _uint iSampleCount = 0;
        const _uint iMaxSamplesPerFrame = (std::max)(static_cast<_uint>(2), m_iMaxSamples);

        while (fSampleTime <= fDelta + Helper::fEpsilon &&
            iSampleCount < iMaxSamplesPerFrame)
        {
            _float fRatio = fSampleTime / fDelta;
            Helper::FloatClamp(fRatio, 0.f, 1.f);
            Push_FilteredSample(Lerp(m_vPreviousFrameBase, vCurrentBase, fRatio),
                Lerp(m_vPreviousFrameTip, vCurrentTip, fRatio), (std::max)(fDelta - fSampleTime, 0.f));
            fSampleTime += fInterval;
            ++iSampleCount;
        }

        m_fSampleAccumulator = fmodf(fTotalTime, fInterval);
    }

    m_vPreviousFrameBase = vCurrentBase;
    m_vPreviousFrameTip = vCurrentTip;
}

void CEffect_Trail::Push_FilteredSample(const _float3& vBase, const _float3& vTip, _float fInitialAge)
{
    if (m_pVIBuffer == nullptr)
        return;

    if (m_bHasLastEmitted == true && m_fMinSampleDistance > 0.f)
    {
        const _float fMovement = (std::max)(Distance(m_vLastEmittedBase, vBase), Distance(m_vLastEmittedTip, vTip));
        if (fMovement < m_fMinSampleDistance)
            return;
    }

    m_pVIBuffer->Push_Sample(vBase, vTip, fInitialAge);
    m_vLastEmittedBase = vBase;
    m_vLastEmittedTip = vTip;
    m_bHasLastEmitted = true;
}

void CEffect_Trail::Reset_SamplingState()
{
    m_bHasPreviousFrame = false;
    m_fSampleAccumulator = 0.f;
}

void CEffect_Trail::Compute_WorldEndpoints(_float3& vBase, _float3& vTip) const
{
    // 로컬 Base/Tip Offset을 실제 월드 위치로 변환.
    _matrix matWorld{};
    if (m_pSourceMatrix != nullptr)
    {
        matWorld = XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()) * XMLoadFloat4x4(m_pSourceMatrix);
    }
    else
    {
        matWorld = XMLoadFloat4x4(&m_CombinedWorldMatrix);
    }

    XMStoreFloat3(&vBase, XMVector3TransformCoord(XMLoadFloat3(&m_vBaseOffset), matWorld));
    XMStoreFloat3(&vTip, XMVector3TransformCoord(XMLoadFloat3(&m_vTipOffset), matWorld));
}

void CEffect_Trail::Init_PropertyValue()
{
    m_bAutoSample = true;
    m_vBaseOffset = { 0.f, 0.f, 0.f };
    m_vTipOffset = { 0.f, 1.f, 0.f };
    m_iMaxSamples = 64;
    m_fSampleLifeTime = 0.3f;
    m_fSampleInterval = 1.f / 120.f;
    m_fMinSampleDistance = 0.01f;
    m_fTeleportDistance = 10.f;

    m_iSmoothSegments = 4;
    m_fSplineSmoothness = 1.f;
    m_fTailWidthScale = 0.f;
    m_fHeadWidthScale = 1.f;

    m_bDistanceUV = true;
    m_fEdgeSoftness = 0.15f;
    m_fHeadFadeRatio = 0.f;
    m_fTailFadeRatio = 0.35f;

    m_pSourceMatrix = nullptr;
    m_bEmitting = true;
    m_bHasPreviousFrame = false;
    m_bHasLastEmitted = false;
    m_fSampleAccumulator = 0.f;
}

void CEffect_Trail::Free()
{
    m_pSourceMatrix = nullptr;
    m_pVIBuffer = nullptr;

    __super::Free();
}
