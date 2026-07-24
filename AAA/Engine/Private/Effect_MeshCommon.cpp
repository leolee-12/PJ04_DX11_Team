#include "Effect_MeshCommon.h"

#include "Model.h"
#include "Shader.h"

namespace
{
    HRESULT BindDiffuseValues(Engine::CShader* pShader, const Engine::EffectMesh::VALUES& Values)
    {
        if (FAILED(pShader->Bind_RawValue("g_bUseDiffuseTexture", &Values.Diffuse.bUse, sizeof(_bool))) ||
            FAILED(pShader->Bind_RawValue("g_vDiffuseTiling", &Values.Diffuse.vTiling, sizeof(_float2))) ||
            FAILED(pShader->Bind_RawValue("g_vDiffuseOffset", &Values.Diffuse.vCurrentOffset, sizeof(_float2))) ||
            FAILED(pShader->Bind_RawValue("g_fDiffuseUVRotationDegree", &Values.Diffuse.fUVRotationDegree, sizeof(_float))))
            return E_FAIL;

        return S_OK;
    }

    HRESULT BindUnknownValues(Engine::CShader* pShader, const Engine::EffectMesh::VALUES& Values)
    {
        if (FAILED(pShader->Bind_RawValue("g_bUseUnknownTexture", &Values.Unknown.bUse, sizeof(_bool))) ||
            FAILED(pShader->Bind_RawValue("g_vUnknownTiling", &Values.Unknown.vTiling, sizeof(_float2))) ||
            FAILED(pShader->Bind_RawValue("g_vUnknownOffset", &Values.Unknown.vCurrentOffset, sizeof(_float2))) ||
            FAILED(pShader->Bind_RawValue("g_fUnknownUVRotationDegree", &Values.Unknown.fUVRotationDegree, sizeof(_float))))
            return E_FAIL;

        return S_OK;
    }

    void MoveUVScroll(const _float fRatio, const Engine::EffectMesh::TEXTURE_VALUES& Values)
    {
        if (Values.bUVScroll == false)
        {
            Values.vCurrentOffset = Values.vOffset;
            return;
        }

        Values.vCurrentOffset.x = Values.vOffset.x + Values.vUVScrollCount.x * fRatio;
        Values.vCurrentOffset.y = Values.vOffset.y + Values.vUVScrollCount.y * fRatio;
        Values.vCurrentOffset.x = fmodf(Values.vCurrentOffset.x, 1.f);
        Values.vCurrentOffset.y = fmodf(Values.vCurrentOffset.y, 1.f);
    }

    _float ComputeAnimationRatio(const _float fRatio, const _float fStartRatio, const _float fEndRatio)
    {
        const _float fRange = fEndRatio - fStartRatio;
        if (fabsf(fRange) <= Helper::fEpsilon)
            return fRatio >= fEndRatio ? 1.f : 0.f;

        _float fResult = (fRatio - fStartRatio) / fRange;
        Helper::FloatClamp(fResult, 0.f, 1.f);
        return fResult;
    }

    void UpdateCircleRatio(Engine::EffectMesh::CIRCLE_UV_VALUES& Values, const _float fRatio)
    {
        Values.fRatio = ComputeAnimationRatio(fRatio, Values.fStartRatio, Values.fEndRatio);
    }

    void UpdateLinearRatio(Engine::EffectMesh::LINEAR_UV_VALUES& Values, const _float fRatio)
    {
        Values.fRatio = ComputeAnimationRatio(fRatio, Values.fStartRatio, Values.fEndRatio);
    }

    void InitializeTexture(Engine::EffectMesh::TEXTURE_VALUES& Values)
    {
        Values.bUse = false;
        Values.vTiling = { 1.f, 1.f };
        Values.vOffset = { 0.f, 0.f };
        Values.fUVRotationDegree = 0.f;
        Values.bUVScroll = false;
        Values.vUVScrollCount = { 0.f, 0.f };
    }

    void InitializeCircle(Engine::EffectMesh::CIRCLE_UV_VALUES& Values)
    {
        Values.bUse = false;
        Values.fRatio = 1.f;
        Values.fStartRatio = 0.f;
        Values.fEndRatio = 1.f;
        Values.fStartDegree = 0.f;
        Values.bClockwise = true;
    }

    void InitializeLinear(Engine::EffectMesh::LINEAR_UV_VALUES& Values)
    {
        Values.bUse = false;
        Values.fRatio = 0.f;
        Values.fStartRatio = 0.f;
        Values.fEndRatio = 1.f;
        Values.iAxis = 0;
        Values.bReverse = false;
    }
}

HRESULT Engine::EffectMesh::Bind_ShaderValues(CShader* pShader, const VALUES& Values, _bool bBindUnknownBeforePBR)
{
    if (FAILED(pShader->Bind_RawValue("g_bTextureColorToAlpha", &Values.bTextureColorToAlpha, sizeof(_bool))) ||
        FAILED(pShader->Bind_RawValue("g_bUseTextureUVEdgeFade", &Values.TextureEdgeFade.bUse, sizeof(_bool))) ||
        FAILED(pShader->Bind_RawValue("g_iTextureUVEdgeFadeAxis", &Values.TextureEdgeFade.iAxis, sizeof(_int))) ||
        FAILED(pShader->Bind_RawValue("g_fTextureUVEdgeFadeStartRange", &Values.TextureEdgeFade.fStartRange, sizeof(_float))) ||
        FAILED(pShader->Bind_RawValue("g_fTextureUVEdgeFadeEndRange", &Values.TextureEdgeFade.fEndRange, sizeof(_float))) ||
        FAILED(pShader->Bind_RawValue("g_fTextureUVEdgeFadePower", &Values.TextureEdgeFade.fPower, sizeof(_float))) ||
        FAILED(pShader->Bind_RawValue("g_bDiffuseColorToAlpha", &Values.bDiffuseColorToAlpha, sizeof(_bool))) ||
        FAILED(pShader->Bind_RawValue("g_bUseDiffuseUVEdgeFade", &Values.DiffuseEdgeFade.bUse, sizeof(_bool))) ||
        FAILED(pShader->Bind_RawValue("g_iDiffuseUVEdgeFadeAxis", &Values.DiffuseEdgeFade.iAxis, sizeof(_int))) ||
        FAILED(pShader->Bind_RawValue("g_fDiffuseUVEdgeFadeStartRange", &Values.DiffuseEdgeFade.fStartRange, sizeof(_float))) ||
        FAILED(pShader->Bind_RawValue("g_fDiffuseUVEdgeFadeEndRange", &Values.DiffuseEdgeFade.fEndRange, sizeof(_float))) ||
        FAILED(pShader->Bind_RawValue("g_fDiffuseUVEdgeFadePower", &Values.DiffuseEdgeFade.fPower, sizeof(_float))) ||
        FAILED(pShader->Bind_RawValue("g_bUnknownColorToAlpha", &Values.bUnknownColorToAlpha, sizeof(_bool))) ||
        FAILED(pShader->Bind_RawValue("g_bUseUnknownUVEdgeFade", &Values.UnknownEdgeFade.bUse, sizeof(_bool))) ||
        FAILED(pShader->Bind_RawValue("g_iUnknownUVEdgeFadeAxis", &Values.UnknownEdgeFade.iAxis, sizeof(_int))) ||
        FAILED(pShader->Bind_RawValue("g_fUnknownUVEdgeFadeStartRange", &Values.UnknownEdgeFade.fStartRange, sizeof(_float))) ||
        FAILED(pShader->Bind_RawValue("g_fUnknownUVEdgeFadeEndRange", &Values.UnknownEdgeFade.fEndRange, sizeof(_float))) ||
        FAILED(pShader->Bind_RawValue("g_fUnknownUVEdgeFadePower", &Values.UnknownEdgeFade.fPower, sizeof(_float))))
        return E_FAIL;

    if (FAILED(BindDiffuseValues(pShader, Values)))
        return E_FAIL;

    if (bBindUnknownBeforePBR == true && FAILED(BindUnknownValues(pShader, Values)))
        return E_FAIL;

    if (FAILED(pShader->Bind_RawValue("g_bUseNormalTexture", &Values.bUseNormalTexture, sizeof(_bool))) ||
        FAILED(pShader->Bind_RawValue("g_fNormalUVRotationDegree", &Values.fNormalUVRotationDegree, sizeof(_float))) ||
        FAILED(pShader->Bind_RawValue("g_bUseMRATexture", &Values.bUseMRATexture, sizeof(_bool))) ||
        FAILED(pShader->Bind_RawValue("g_fMRAUVRotationDegree", &Values.fMRAUVRotationDegree, sizeof(_float))))
        return E_FAIL;

    if (bBindUnknownBeforePBR == false && FAILED(BindUnknownValues(pShader, Values)))
        return E_FAIL;

    if (FAILED(pShader->Bind_RawValue("g_bUseCircleUVAnim_T", &Values.TextureCircle.bUse, sizeof(_bool))) ||
        FAILED(pShader->Bind_RawValue("g_fCircleUVRatio_T", &Values.TextureCircle.fRatio, sizeof(_float))) ||
        FAILED(pShader->Bind_RawValue("g_fCircleUVStartDegree_T", &Values.TextureCircle.fStartDegree, sizeof(_float))) ||
        FAILED(pShader->Bind_RawValue("g_bCircleUVClockwise_T", &Values.TextureCircle.bClockwise, sizeof(_bool))) ||
        FAILED(pShader->Bind_RawValue("g_bUseCircleUVAnim_M", &Values.MaskCircle.bUse, sizeof(_bool))) ||
        FAILED(pShader->Bind_RawValue("g_fCircleUVRatio_M", &Values.MaskCircle.fRatio, sizeof(_float))) ||
        FAILED(pShader->Bind_RawValue("g_fCircleUVStartDegree_M", &Values.MaskCircle.fStartDegree, sizeof(_float))) ||
        FAILED(pShader->Bind_RawValue("g_bCircleUVClockwise_M", &Values.MaskCircle.bClockwise, sizeof(_bool))) ||
        FAILED(pShader->Bind_RawValue("g_bUseCircleUVAnim_D", &Values.DiffuseCircle.bUse, sizeof(_bool))) ||
        FAILED(pShader->Bind_RawValue("g_fCircleUVRatio_D", &Values.DiffuseCircle.fRatio, sizeof(_float))) ||
        FAILED(pShader->Bind_RawValue("g_fCircleUVStartDegree_D", &Values.DiffuseCircle.fStartDegree, sizeof(_float))) ||
        FAILED(pShader->Bind_RawValue("g_bCircleUVClockwise_D", &Values.DiffuseCircle.bClockwise, sizeof(_bool))) ||
        FAILED(pShader->Bind_RawValue("g_bUseCircleUVAnim_U", &Values.UnknownCircle.bUse, sizeof(_bool))) ||
        FAILED(pShader->Bind_RawValue("g_fCircleUVRatio_U", &Values.UnknownCircle.fRatio, sizeof(_float))) ||
        FAILED(pShader->Bind_RawValue("g_fCircleUVStartDegree_U", &Values.UnknownCircle.fStartDegree, sizeof(_float))) ||
        FAILED(pShader->Bind_RawValue("g_bCircleUVClockwise_U", &Values.UnknownCircle.bClockwise, sizeof(_bool))))
        return E_FAIL;

    if (FAILED(pShader->Bind_RawValue("g_bUseLinearUVAnim_T", &Values.TextureLinear.bUse, sizeof(_bool))) ||
        FAILED(pShader->Bind_RawValue("g_fLinearUVRatio_T", &Values.TextureLinear.fRatio, sizeof(_float))) ||
        FAILED(pShader->Bind_RawValue("g_iLinearUVAxis_T", &Values.TextureLinear.iAxis, sizeof(_int))) ||
        FAILED(pShader->Bind_RawValue("g_bLinearUVReverse_T", &Values.TextureLinear.bReverse, sizeof(_bool))) ||
        FAILED(pShader->Bind_RawValue("g_bUseLinearUVAnim_M", &Values.MaskLinear.bUse, sizeof(_bool))) ||
        FAILED(pShader->Bind_RawValue("g_fLinearUVRatio_M", &Values.MaskLinear.fRatio, sizeof(_float))) ||
        FAILED(pShader->Bind_RawValue("g_iLinearUVAxis_M", &Values.MaskLinear.iAxis, sizeof(_int))) ||
        FAILED(pShader->Bind_RawValue("g_bLinearUVReverse_M", &Values.MaskLinear.bReverse, sizeof(_bool))) ||
        FAILED(pShader->Bind_RawValue("g_bUseLinearUVAnim_D", &Values.DiffuseLinear.bUse, sizeof(_bool))) ||
        FAILED(pShader->Bind_RawValue("g_fLinearUVRatio_D", &Values.DiffuseLinear.fRatio, sizeof(_float))) ||
        FAILED(pShader->Bind_RawValue("g_iLinearUVAxis_D", &Values.DiffuseLinear.iAxis, sizeof(_int))) ||
        FAILED(pShader->Bind_RawValue("g_bLinearUVReverse_D", &Values.DiffuseLinear.bReverse, sizeof(_bool))) ||
        FAILED(pShader->Bind_RawValue("g_bUseLinearUVAnim_U", &Values.UnknownLinear.bUse, sizeof(_bool))) ||
        FAILED(pShader->Bind_RawValue("g_fLinearUVRatio_U", &Values.UnknownLinear.fRatio, sizeof(_float))) ||
        FAILED(pShader->Bind_RawValue("g_iLinearUVAxis_U", &Values.UnknownLinear.iAxis, sizeof(_int))) ||
        FAILED(pShader->Bind_RawValue("g_bLinearUVReverse_U", &Values.UnknownLinear.bReverse, sizeof(_bool))))
        return E_FAIL;

    return S_OK;
}

HRESULT Engine::EffectMesh::Bind_Materials(CModel* pModel, CShader* pShader, _uint iMeshIndex, const VALUES& Values)
{
    if (Values.Diffuse.bUse == true &&
        FAILED(pModel->Bind_Material(pShader, "g_DiffuseTexture", iMeshIndex, MTEX_TYPE::DIFFUSE, 0)))
        return E_FAIL;

    if (Values.bUseNormalTexture == true &&
        FAILED(pModel->Bind_Material(pShader, "g_NormalTexture", iMeshIndex, MTEX_TYPE::NORMALS, 0)))
        return E_FAIL;

    if (Values.bUseMRATexture == true &&
        FAILED(pModel->Bind_Material(pShader, "g_MRATexture", iMeshIndex, MTEX_TYPE::METALNESS, 0)))
        return E_FAIL;

    if (Values.Unknown.bUse == true &&
        FAILED(pModel->Bind_Material(pShader, "g_UnknownTexture", iMeshIndex, MTEX_TYPE::UNKNOWN, 0)))
        return E_FAIL;

    return S_OK;
}

void Engine::EffectMesh::Apply_BillboardRoll(_float4x4& BillboardWorldMatrix, _float fRoll)
{
    _matrix BillboardMatrix = XMLoadFloat4x4(&BillboardWorldMatrix);

    const _float fScaleX = XMVectorGetX(XMVector3Length(BillboardMatrix.r[0]));
    const _float fScaleY = XMVectorGetX(XMVector3Length(BillboardMatrix.r[1]));
    const _vector vRight = XMVector3Normalize(BillboardMatrix.r[0]);
    const _vector vUp = XMVector3Normalize(BillboardMatrix.r[1]);
    const _float fSin = sinf(fRoll);
    const _float fCos = cosf(fRoll);

    BillboardMatrix.r[0] = XMVectorSetW(
        (vRight * fCos + vUp * fSin) * fScaleX,
        0.f);
    BillboardMatrix.r[1] = XMVectorSetW(
        (vUp * fCos - vRight * fSin) * fScaleY,
        0.f);

    XMStoreFloat4x4(&BillboardWorldMatrix, BillboardMatrix);
}

void Engine::EffectMesh::Update_UVAnimations(VALUES& Values, _float fRatio)
{
    MoveUVScroll(fRatio, Values.Diffuse);
    MoveUVScroll(fRatio, Values.Unknown);

    UpdateCircleRatio(Values.TextureCircle, fRatio);
    UpdateCircleRatio(Values.MaskCircle, fRatio);
    UpdateCircleRatio(Values.DiffuseCircle, fRatio);
    UpdateCircleRatio(Values.UnknownCircle, fRatio);

    UpdateLinearRatio(Values.TextureLinear, fRatio);
    UpdateLinearRatio(Values.MaskLinear, fRatio);
    UpdateLinearRatio(Values.DiffuseLinear, fRatio);
    UpdateLinearRatio(Values.UnknownLinear, fRatio);
}

void Engine::EffectMesh::Initialize_DefaultValues(VALUES& Values)
{
    Values.bTextureColorToAlpha = false;
    Values.TextureEdgeFade.bUse = false;
    Values.TextureEdgeFade.iAxis = 0;
    Values.TextureEdgeFade.fStartRange = 0.1f;
    Values.TextureEdgeFade.fEndRange = 0.1f;
    Values.TextureEdgeFade.fPower = 1.f;
    InitializeTexture(Values.Diffuse);
    Values.bDiffuseColorToAlpha = false;
    Values.DiffuseEdgeFade.bUse = false;
    Values.DiffuseEdgeFade.iAxis = 0;
    Values.DiffuseEdgeFade.fStartRange = 0.1f;
    Values.DiffuseEdgeFade.fEndRange = 0.1f;
    Values.DiffuseEdgeFade.fPower = 1.f;
    Values.bUseNormalTexture = false;
    Values.fNormalUVRotationDegree = 0.f;
    Values.bUseMRATexture = false;
    Values.fMRAUVRotationDegree = 0.f;
    InitializeTexture(Values.Unknown);
    Values.bUnknownColorToAlpha = false;
    Values.UnknownEdgeFade.bUse = false;
    Values.UnknownEdgeFade.iAxis = 0;
    Values.UnknownEdgeFade.fStartRange = 0.1f;
    Values.UnknownEdgeFade.fEndRange = 0.1f;
    Values.UnknownEdgeFade.fPower = 1.f;

    InitializeCircle(Values.TextureCircle);
    InitializeCircle(Values.MaskCircle);
    InitializeCircle(Values.DiffuseCircle);
    InitializeCircle(Values.UnknownCircle);

    InitializeLinear(Values.TextureLinear);
    InitializeLinear(Values.MaskLinear);
    InitializeLinear(Values.DiffuseLinear);
    InitializeLinear(Values.UnknownLinear);
}
