#include "Effect_Part.h"

#include "GameInstance.h"

CEffect_Part::CEffect_Part(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CGameObject(pDevice, pContext)
{
    Init_PropertyValue();
}

CEffect_Part::CEffect_Part(const CEffect_Part& Prototype)
    : CGameObject(Prototype)
{
    Init_PropertyValue();
}

HRESULT CEffect_Part::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CEffect_Part::Initialize(void* pArg)
{
    EFFECT_PART_DESC* pDesc = static_cast<EFFECT_PART_DESC*>(pArg);

    // Texture
    m_bUseTextureCom = pDesc->bUseTextureCom;
    m_iTextureLevel = pDesc->iTextureLevel;
    m_wstrTextureTag = pDesc->wstrTextureTag;

    // Mask
    m_bUseMaskCom = pDesc->bUseMaskCom;
    m_iMaskLevel = pDesc->iMaskLevel;
    m_wstrMaskTag = pDesc->wstrMaskTag;

    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;
   
    if (FAILED(Ready_Components()))
        return E_FAIL;

    Apply_PropertyScale();

    return S_OK;
}

void CEffect_Part::Priority_Update(_float fTimeDelta)
{

}

void CEffect_Part::Update(_float fTimeDelta)
{
    Update_FadeOut(fTimeDelta);
    Update_Value(fTimeDelta);
}

void CEffect_Part::Late_Update(_float fTimeDelta)
{
}

HRESULT CEffect_Part::Render()
{
    return S_OK;
}

void CEffect_Part::On_Deserialized()
{
    __super::On_Deserialized();
    Apply_PropertyScale();
}

void CEffect_Part::Effect_Start()
{
    m_bIsPlay = true;
    m_bActive = false;
    m_fAccTime = 0.f;

    m_bFadeOutActive = false;
    m_bFadeOutFinished = false;
    m_fFadeOutDuration = 0.3f;
    m_fAccFadeOutTime = 0.f;
    m_fFadeOutAlpha = 1.f;

    Apply_PropertyScale();
}

void CEffect_Part::Start_FadeOut(_float fFadeOutDuration)
{
    if (m_bFadeOutActive == true)
        return;

    m_bFadeOutActive = true;
    m_bFadeOutFinished = false;
    m_fFadeOutDuration = fFadeOutDuration;
    m_fAccFadeOutTime = 0.f;
    m_fFadeOutAlpha = 1.f;

    if (m_fFadeOutDuration <= Helper::fEpsilon)
    {
        m_fFadeOutAlpha = 0.f;
        m_bFadeOutFinished = true;
    }
}

void CEffect_Part::Update_PlayValue(_bool bIsPlay, _bool bLoop, _float fDuration, _float fAccTime)
{
    m_bIsPlay = bIsPlay;
    m_bLoop = bLoop;
    m_fDuration = fDuration;
    m_fAccTime = fAccTime;
}

void CEffect_Part::Set_ParentMatrix(const _float4x4* pParentMatrix)
{
    m_pParentMatrix = pParentMatrix;
}

void CEffect_Part::Compute_CombinedWorldMatrix()
{
    if (m_pParentMatrix != nullptr)
    {
        XMStoreFloat4x4(&m_CombinedWorldMatrix,
            XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()) * XMLoadFloat4x4(m_pParentMatrix));
    }
    else
    {
        XMStoreFloat4x4(&m_CombinedWorldMatrix, XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));
    }
}

void CEffect_Part::Set_LocalPositionFromProperty()
{
    m_pTransformCom->Set_State(STATE::POSITION, XMVectorSetW(XMLoadFloat3(&m_vLocalPos), 1.f));
}

void CEffect_Part::Apply_PropertyScale(_float fUniformScale, _float fRatio)
{
    _float3 vScale = Evaluate_Float3Curve(
        fRatio, m_vScale, m_bScaleChange,
        m_vScaleStartValue, m_vScaleEndValue,
        m_bActive_Scale_Ratio_0, m_fScale_Ratio_0, m_vScale_Value_0,
        m_bActive_Scale_Ratio_1, m_fScale_Ratio_1, m_vScale_Value_1,
        false);

    if (vScale.x < Helper::fEpsilon)
        vScale.x = Helper::fEpsilon;
    if (vScale.y < Helper::fEpsilon)
        vScale.y = Helper::fEpsilon;
    if (vScale.z < Helper::fEpsilon)
        vScale.z = Helper::fEpsilon;
    if (fUniformScale < Helper::fEpsilon)
        fUniformScale = Helper::fEpsilon;

    if (m_bScaleChange == false)
        m_vScale = vScale;

    m_vAppliedPropertyScale = {
        vScale.x * fUniformScale,
        vScale.y * fUniformScale,
        vScale.z * fUniformScale
    };

    if (Use_PropertyScaleForParticleSize() == true)
        m_pTransformCom->Set_Scale(1.f, 1.f, 1.f);
    else
        m_pTransformCom->Set_Scale(
            m_vAppliedPropertyScale.x,
            m_vAppliedPropertyScale.y,
            m_vAppliedPropertyScale.z);
}

void CEffect_Part::Update_Orbit(const _float fRatio)
{
    if (m_bOrbitChange == false)
        return;

    const _float fOrbitRange = m_fOrbit_End_Ratio - m_fOrbit_Start_Ratio;
    _float fSubRatio = 0.f;

    if (fabsf(fOrbitRange) > Helper::fEpsilon)
        fSubRatio = (fRatio - m_fOrbit_Start_Ratio) / fOrbitRange;
    else if (fRatio >= m_fOrbit_Start_Ratio)
        fSubRatio = 1.f;

    Helper::FloatClamp(fSubRatio, 0.f, 1.f);

    _vector vAxis = XMLoadFloat3(&m_vOrbitAxis);
    if (XMVectorGetX(XMVector3LengthSq(vAxis)) <= Helper::fEpsilon)
        vAxis = XMVectorSet(0.f, 1.f, 0.f, 0.f);
    else
        vAxis = XMVector3Normalize(vAxis);

    const _float fRadian = XMConvertToRadians(m_fOrbitDegree * fSubRatio);
    _vector vPivot = XMLoadFloat3(&m_vOrbitPivot);
    _vector vBasePos = XMVectorSetW(m_pTransformCom->Get_State(STATE::POSITION), 0.f);
    _vector vOrbitPos = vPivot + XMVector3Rotate(vBasePos - vPivot, XMQuaternionRotationAxis(vAxis, fRadian));

    m_pTransformCom->Set_State(STATE::POSITION, XMVectorSetW(vOrbitPos, 1.f));
}

_int CEffect_Part::Resolve_ShaderPass()
{
    Helper::IntClamp(m_iShaderPass, ShaderPass::Default, ShaderPass::ShaderPass_End - 1);
    Helper::IntClamp(m_iMirror, Sampler::DEFAULT, Sampler::SAMPLER_END - 1);
    Helper::IntClamp(m_iDepthIgnore, DepthMode::DEPTH_DEFAULT, DepthMode::DEPTH_MODE_END - 1);

    const _int iDepthModeOffset =
        m_iDepthIgnore * ShaderPass::ShaderPass_End * Sampler::SAMPLER_END;

    return m_iShaderPass +
        (m_iMirror == Sampler::MIRROR ? ShaderPass::ShaderPass_End : 0) +
        iDepthModeOffset;
}

HRESULT CEffect_Part::Bind_ViewProjectionMatrices()
{
    if (FAILED(m_pShaderCom->Bind_Matrix(
        "g_ViewMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::VIEW, m_eProjType))))
        return E_FAIL;

    if (FAILED(m_pShaderCom->Bind_Matrix(
        "g_ProjMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::PROJ, m_eProjType))))
        return E_FAIL;

    return S_OK;
}

_float4x4 CEffect_Part::Make_BillboardWorldMatrix(const _float4x4& WorldMatrix) const
{
    _matrix BillboardMatrix = XMLoadFloat4x4(&WorldMatrix);
    const _matrix InverseViewMatrix = XMMatrixInverse(
        nullptr,
        XMLoadFloat4x4(m_pGameInstance_Proxy->Get_Matrix(D3DTS::VIEW, m_eProjType)));

    const _float fScaleX = XMVectorGetX(XMVector3Length(BillboardMatrix.r[0]));
    const _float fScaleY = XMVectorGetX(XMVector3Length(BillboardMatrix.r[1]));
    const _float fScaleZ = XMVectorGetX(XMVector3Length(BillboardMatrix.r[2]));

    BillboardMatrix.r[0] = XMVectorSetW(XMVector3Normalize(InverseViewMatrix.r[0]) * fScaleX, 0.f);
    BillboardMatrix.r[1] = XMVectorSetW(XMVector3Normalize(InverseViewMatrix.r[1]) * fScaleY, 0.f);
    BillboardMatrix.r[2] = XMVectorSetW(XMVector3Normalize(InverseViewMatrix.r[2]) * fScaleZ, 0.f);

    _float4x4 Result{};
    XMStoreFloat4x4(&Result, BillboardMatrix);
    return Result;
}

void CEffect_Part::Evaluate_SpriteFrame(_int iFrameX, _int iFrameY, _float fRatio, _float2& vOutUV, _float2& vOutSize)
{
    if (iFrameX < 1)
        iFrameX = 1;
    if (iFrameY < 1)
        iFrameY = 1;

    Helper::FloatClamp(fRatio, 0.f, 1.f);

    const _int iTotalCount = iFrameX * iFrameY;
    _int iFrameIndex = static_cast<_int>(static_cast<_float>(iTotalCount) * fRatio);
    if (iFrameIndex >= iTotalCount)
        iFrameIndex = iTotalCount - 1;

    vOutSize.x = 1.f / static_cast<_float>(iFrameX);
    vOutSize.y = 1.f / static_cast<_float>(iFrameY);
    vOutUV.x = vOutSize.x * static_cast<_float>(iFrameIndex % iFrameX);
    vOutUV.y = vOutSize.y * static_cast<_float>(iFrameIndex / iFrameX);
}

_float CEffect_Part::Evaluate_FloatCurve(
    _float fRatio, _float fFixedValue, _bool bChange,
    _float fStartValue, _float fEndValue,
    _bool bActiveRatio0, _float fRatio0, _float fValue0,
    _bool bActiveRatio1, _float fRatio1, _float fValue1,
    _bool bNormalizePoints)
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

    if (bNormalizePoints == true)
    {
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

        Helper::FloatClamp(fRatio, 0.f, 1.f);
    }

    for (_uint i = 0; i + 1 < iCount; ++i)
    {
        if (fRatio <= Points[i + 1].fRatio)
        {
            const _float fRange = Points[i + 1].fRatio - Points[i].fRatio;
            if (bNormalizePoints == true && fRange <= Helper::fEpsilon)
                return Points[i + 1].fValue;

            const _float fStep = Helper::FloatSmoothStep(
                Points[i].fRatio, Points[i + 1].fRatio, fRatio);
            return Points[i].fValue + (Points[i + 1].fValue - Points[i].fValue) * fStep;
        }
    }

    return bNormalizePoints == true ? Points[iCount - 1].fValue : fFixedValue;
}

_float3 CEffect_Part::Evaluate_Float3Curve(
    _float fRatio, const _float3& vFixedValue, _bool bChange,
    const _float3& vStartValue, const _float3& vEndValue,
    _bool bActiveRatio0, _float fRatio0, const _float3& vValue0,
    _bool bActiveRatio1, _float fRatio1, const _float3& vValue1,
    _bool bNormalizePoints)
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

    if (bNormalizePoints == true)
    {
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

        Helper::FloatClamp(fRatio, 0.f, 1.f);
    }

    for (_uint i = 0; i + 1 < iCount; ++i)
    {
        if (fRatio <= Points[i + 1].fRatio)
        {
            const _float fRange = Points[i + 1].fRatio - Points[i].fRatio;
            if (bNormalizePoints == true && fRange <= Helper::fEpsilon)
                return Points[i + 1].vValue;

            const _float fStep = Helper::FloatSmoothStep(
                Points[i].fRatio, Points[i + 1].fRatio, fRatio);

            _float3 vResult{};
            vResult.x = Points[i].vValue.x + (Points[i + 1].vValue.x - Points[i].vValue.x) * fStep;
            vResult.y = Points[i].vValue.y + (Points[i + 1].vValue.y - Points[i].vValue.y) * fStep;
            vResult.z = Points[i].vValue.z + (Points[i + 1].vValue.z - Points[i].vValue.z) * fStep;
            return vResult;
        }
    }

    return bNormalizePoints == true ? Points[iCount - 1].vValue : vFixedValue;
}

_float4 CEffect_Part::Evaluate_Float4Curve(
    _float fRatio, const _float4& vFixedValue, _bool bChange,
    const _float4& vStartValue, const _float4& vEndValue,
    _bool bActiveRatio0, _float fRatio0, const _float4& vValue0,
    _bool bActiveRatio1, _float fRatio1, const _float4& vValue1,
    _bool bNormalizePoints)
{
    if (bChange == false)
        return vFixedValue;

    RATIO_VALUE_FLOAT4 Points[4]{};
    _uint iCount = 0;
    Points[iCount++] = { 0.f, vStartValue };
    if (bActiveRatio0 == true)
        Points[iCount++] = { fRatio0, vValue0 };
    if (bActiveRatio1 == true)
        Points[iCount++] = { fRatio1, vValue1 };
    Points[iCount++] = { 1.f, vEndValue };

    if (bNormalizePoints == true)
    {
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

        Helper::FloatClamp(fRatio, 0.f, 1.f);
    }

    for (_uint i = 0; i + 1 < iCount; ++i)
    {
        if (fRatio <= Points[i + 1].fRatio)
        {
            const _float fRange = Points[i + 1].fRatio - Points[i].fRatio;
            if (bNormalizePoints == true && fRange <= Helper::fEpsilon)
                return Points[i + 1].vValue;

            const _float fStep = Helper::FloatSmoothStep(
                Points[i].fRatio, Points[i + 1].fRatio, fRatio);

            _float4 vResult{};
            vResult.x = Points[i].vValue.x + (Points[i + 1].vValue.x - Points[i].vValue.x) * fStep;
            vResult.y = Points[i].vValue.y + (Points[i + 1].vValue.y - Points[i].vValue.y) * fStep;
            vResult.z = Points[i].vValue.z + (Points[i + 1].vValue.z - Points[i].vValue.z) * fStep;
            vResult.w = Points[i].vValue.w + (Points[i + 1].vValue.w - Points[i].vValue.w) * fStep;
            return vResult;
        }
    }

    return bNormalizePoints == true ? Points[iCount - 1].vValue : vFixedValue;
}

void CEffect_Part::Update_UVScroll(const _float fTimeDelta, const _float fRatio)
{
    MoveUVScroll(fRatio, m_bTextureUVScroll, m_vTextureUVScrollCount, m_vTextureOffset, m_vCurTextureUVOffset);
    MoveUVScroll(fRatio, m_bMaskUVScroll, m_vMaskUVScrollCount, m_vMaskOffset, m_vCurMaskUVOffset);
}

void CEffect_Part::MoveUVScroll(const _float fRatio, const _bool bUpdate, const _float2 vScrollCount, const _float2 vBaseUV, _float2& vOutUV)
{
    if (bUpdate == false)
    {
        vOutUV = vBaseUV;
        return;
    }

    vOutUV.x = vBaseUV.x + vScrollCount.x * fRatio;
    vOutUV.y = vBaseUV.y + vScrollCount.y * fRatio;

    vOutUV.x = fmodf(vOutUV.x, 1.f);
    vOutUV.y = fmodf(vOutUV.y, 1.f);
}

HRESULT CEffect_Part::Bind_ShaderValue()
{
    if (FAILED(m_pShaderCom->Bind_RawValue("g_vEmissiveColor", &m_vEmissiveColor, sizeof(m_vEmissiveColor))))
        return E_FAIL;

    if (FAILED(m_pShaderCom->Bind_RawValue("g_fEffectIntensity", &m_fEffectIntensity, sizeof(m_fEffectIntensity))))
        return E_FAIL;

    // Texture
    if (m_pTextureCom != nullptr && m_bUseTextureCom == true)
    {
        if (FAILED(m_pTextureCom->Bind_ShaderResource(m_pShaderCom, "g_Texture", 0)))
            return E_FAIL;

        if (FAILED(m_pShaderCom->Bind_RawValue("g_bUseTexture", &m_bUseTextureCom, sizeof(m_bUseTextureCom))))
            return E_FAIL;
        if (FAILED(m_pShaderCom->Bind_RawValue("g_vTextureTiling", &m_vTextureTiling, sizeof(m_vTextureTiling))))
            return E_FAIL;
        if (FAILED(m_pShaderCom->Bind_RawValue("g_vTextureOffset", &m_vCurTextureUVOffset, sizeof(m_vCurTextureUVOffset))))
            return E_FAIL;
        if (FAILED(m_pShaderCom->Bind_RawValue("g_fTextureUVRotationDegree", &m_fTextureUVRotationDegree, sizeof(m_fTextureUVRotationDegree))))
            return E_FAIL;
    }
    else
    {
        _bool bFalse = false;
        if (FAILED(m_pShaderCom->Bind_RawValue("g_bUseTexture", &bFalse, sizeof(bFalse))))
            return E_FAIL;
    }

    // Mask
    if (m_pMaskCom != nullptr && m_bUseMaskCom == true)
    {
        if (FAILED(m_pMaskCom->Bind_ShaderResource(m_pShaderCom, "g_Mask", 0)))
            return E_FAIL;

        if (FAILED(m_pShaderCom->Bind_RawValue("g_bUseMask", &m_bUseMaskCom, sizeof(m_bUseMaskCom))))
            return E_FAIL;
        if (FAILED(m_pShaderCom->Bind_RawValue("g_vMaskTiling", &m_vMaskTiling, sizeof(m_vMaskTiling))))
            return E_FAIL;
        if (FAILED(m_pShaderCom->Bind_RawValue("g_vMaskOffset", &m_vCurMaskUVOffset, sizeof(m_vCurMaskUVOffset))))
            return E_FAIL;
        if (FAILED(m_pShaderCom->Bind_RawValue("g_fMaskUVRotationDegree", &m_fMaskUVRotationDegree, sizeof(m_fMaskUVRotationDegree))))
            return E_FAIL;

        Helper::IntClamp(m_iMaskBlendMode, MaskBlendMode::MASK_MULTIPLY, MaskBlendMode::MASK_BLEND_END - 1);
        Helper::IntClamp(m_iMaskChannel, MaskChannel::MASK_RGBA, MaskChannel::MASK_CHANNEL_END - 1);

        if (FAILED(m_pShaderCom->Bind_RawValue("g_iMaskBlendMode", &m_iMaskBlendMode, sizeof(m_iMaskBlendMode))))
            return E_FAIL;
        if (FAILED(m_pShaderCom->Bind_RawValue("g_iMaskChannel", &m_iMaskChannel, sizeof(m_iMaskChannel))))
            return E_FAIL;
        if (FAILED(m_pShaderCom->Bind_RawValue("g_bMaskInvert", &m_bMaskInvert, sizeof(m_bMaskInvert))))
            return E_FAIL;
        if (FAILED(m_pShaderCom->Bind_RawValue("g_fMaskStrength", &m_fMaskStrength, sizeof(m_fMaskStrength))))
            return E_FAIL;
        if (FAILED(m_pShaderCom->Bind_RawValue("g_bUseMaskUVDistortion", &m_bUseMaskUVDistortion, sizeof(m_bUseMaskUVDistortion))))
            return E_FAIL;
        if (m_bCustomShader == false || m_bUseMaskRGUVDistortion == true || m_bMaskUVDistortionSigned == true)
        {
            if (FAILED(m_pShaderCom->Bind_RawValue("g_bUseMaskRGUVDistortion", &m_bUseMaskRGUVDistortion, sizeof(m_bUseMaskRGUVDistortion))))
                return E_FAIL;
            if (FAILED(m_pShaderCom->Bind_RawValue("g_bMaskUVDistortionSigned", &m_bMaskUVDistortionSigned, sizeof(m_bMaskUVDistortionSigned))))
                return E_FAIL;
        }
        if (FAILED(m_pShaderCom->Bind_RawValue("g_vMaskUVDistortionStrength", &m_vMaskUVDistortionStrength, sizeof(m_vMaskUVDistortionStrength))))
            return E_FAIL;
    }
    else
    {
        _bool bFalse = false;
        if (FAILED(m_pShaderCom->Bind_RawValue("g_bUseMask", &bFalse, sizeof(bFalse))))
            return E_FAIL;
    }

    return S_OK;
}

HRESULT CEffect_Part::Ready_Components()
{
    if (m_bUseTextureCom == true)
    {
        m_pTextureCom = Add_Component<CTexture>(m_iTextureLevel, m_wstrTextureTag, TEXT("Com_Texture"));
        if (m_pTextureCom == nullptr)
            return E_FAIL;
    }

    if (m_bUseMaskCom == true)
    {
        m_pMaskCom = Add_Component<CTexture>(m_iMaskLevel, m_wstrMaskTag, TEXT("Com_Mask"));
        if (m_pMaskCom == nullptr)
            return E_FAIL;
    }
    return S_OK;
}

void CEffect_Part::Init_PropertyValue()
{
    m_iShaderPass = { 0 };
    m_iMirror = Sampler::DEFAULT;
    m_iDepthIgnore = DepthMode::DEPTH_DEFAULT;
    m_bUseGBufferOutput = false;
    m_fEffectIntensity = 1.f;
    m_vEmissiveColor = { 0.f, 0.f, 0.f, 0.f };
    m_bEmissiveChange = false;
    m_vEmissiveStartValue = { 0.f, 0.f, 0.f, 0.f };
    m_vEmissiveEndValue = { 0.f, 0.f, 0.f, 0.f };
    m_bActive_Emissive_Ratio_1 = false;
    m_fEmissive_Ratio_1 = 0.5f;
    m_vEmissive_Value_1 = { 0.f, 0.f, 0.f, 0.f };
    m_bActive_Emissive_Ratio_2 = false;
    m_fEmissive_Ratio_2 = 0.75f;
    m_vEmissive_Value_2 = { 0.f, 0.f, 0.f, 0.f };

    m_vLocalPos = { 0.f, 0.f, 0.f };
    m_vScale = { 1.f, 1.f, 1.f };
    m_bScaleChange = false;
    m_vScaleStartValue = { 1.f, 1.f, 1.f };
    m_vScaleEndValue = { 1.f, 1.f, 1.f };
    m_bActive_Scale_Ratio_0 = false;
    m_fScale_Ratio_0 = 0.5f;
    m_vScale_Value_0 = { 1.f, 1.f, 1.f };
    m_bActive_Scale_Ratio_1 = false;
    m_fScale_Ratio_1 = 0.75f;
    m_vScale_Value_1 = { 1.f, 1.f, 1.f };
    m_vAppliedPropertyScale = { 1.f, 1.f, 1.f };

    m_bOrbitChange = false;
    m_vOrbitPivot = { 0.f, 0.f, 0.f };
    m_vOrbitAxis = { 0.f, 1.f, 0.f };
    m_fOrbitDegree = 360.f;
    m_fOrbit_Start_Ratio = 0.f;
    m_fOrbit_End_Ratio = 1.f;

    m_bIsPlay = { true };

    m_bLoop = { true };

    m_fDuration = { 1.f };
    m_fAccTime = { 0.f };

    m_fStartRatio = { 0.f };
    m_fEndRatio = { 1.f };

    m_bFadeOutActive = false;
    m_bFadeOutFinished = false;
    m_fFadeOutDuration = 0.3f;
    m_fAccFadeOutTime = 0.f;
    m_fFadeOutAlpha = 1.f;

    // Texture
    m_bUseTextureCom = false;
    m_vTextureTiling = { 1.f, 1.f };
    m_vTextureOffset = { 0.f, 0.f };
    m_fTextureUVRotationDegree = 0.f;

    m_bTextureUVScroll = false;
    m_vTextureUVScrollCount = { 0.f, 0.f };

    // Mask
    m_bUseMaskCom = false;
    m_vMaskTiling = { 1.f, 1.f };
    m_vMaskOffset = { 0.f, 0.f };
    m_fMaskUVRotationDegree = 0.f;

    m_bMaskUVScroll = false;
    m_vMaskUVScrollCount = { 0.f, 0.f };

    m_iMaskBlendMode = MaskBlendMode::MASK_MULTIPLY;
    m_iMaskChannel = MaskChannel::MASK_RGBA;
    m_bMaskInvert = false;
    m_fMaskStrength = 1.f;
    m_bUseMaskUVDistortion = false;
    m_bUseMaskRGUVDistortion = false;
    m_bMaskUVDistortionSigned = false;
    m_vMaskUVDistortionStrength = { 0.f, 0.f };
}

void CEffect_Part::Update_FadeOut(_float fTimeDelta)
{
    if (m_bFadeOutActive == false || m_bFadeOutFinished == true)
        return;

    m_fAccFadeOutTime += fTimeDelta;

    _float fFadeOutRatio = m_fAccFadeOutTime / m_fFadeOutDuration;
    Helper::FloatClamp(fFadeOutRatio, 0.f, 1.f);

    m_fFadeOutAlpha = 1.f - Helper::FloatSmoothStep(0.f, 1.f, fFadeOutRatio);

    if (fFadeOutRatio >= 1.f)
    {
        m_fFadeOutAlpha = 0.f;
        m_bFadeOutFinished = true;
    }
}

void CEffect_Part::Update_Value(const _float fTimeDelta)
{
    if (m_bIsPlay == false)
        return;  

    _float fContainerRatio = 0.f;
    if (m_fDuration > Helper::fEpsilon)
        fContainerRatio = m_fAccTime / m_fDuration;

    Helper::FloatClamp(fContainerRatio, 0.f, 1.f);

    const _float fPartRange = m_fEndRatio - m_fStartRatio;

    if (fPartRange < 0.f || fContainerRatio < m_fStartRatio || fContainerRatio > m_fEndRatio)
        m_bActive = false;
    else
        m_bActive = true;

    if (m_bActive == true)
    {
        _float fPartRatio = 0.f;
        if (fPartRange > Helper::fEpsilon)
            fPartRatio = (fContainerRatio - m_fStartRatio) / fPartRange;

        Helper::FloatClamp(fPartRatio, 0.f, 1.f);
        Update_Core(fTimeDelta, fPartRatio);
    }


    if (m_fAccTime >= m_fDuration)
    {
        if (m_bLoop == true)
        {
            m_fAccTime = 0.f;
        }
        else
        {
            m_bIsPlay = false;
            m_bActive = false;
            m_fAccTime = m_fDuration;
        }
    }
}

void CEffect_Part::Update_Core(const _float fTimeDelta, const _float fRatio)
{
    Apply_PropertyScale(1.f, fRatio);

    if (m_bEmissiveChange == true)
    {
        m_vEmissiveColor = Evaluate_Float4Curve(
            fRatio, m_vEmissiveColor, true,
            m_vEmissiveStartValue, m_vEmissiveEndValue,
            m_bActive_Emissive_Ratio_1, m_fEmissive_Ratio_1, m_vEmissive_Value_1,
            m_bActive_Emissive_Ratio_2, m_fEmissive_Ratio_2, m_vEmissive_Value_2,
            false);
    }

    Update_UVScroll(fTimeDelta, fRatio);
}

_float CEffect_Part::Get_PartDuration() const
{
    _float fPartRange = m_fEndRatio - m_fStartRatio;
    if (fPartRange < 0.f)
        fPartRange = 0.f;

    return m_fDuration * fPartRange;
}

void CEffect_Part::Free()
{
    __super::Free();
}
