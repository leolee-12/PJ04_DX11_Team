#include "Effect_MeshParticle.h"

#include "Effect_MeshCommon.h"
#include "GameInstance.h"

CEffect_MeshParticle::CEffect_MeshParticle(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CEffect_Particle(pDevice, pContext)
{
    Init_PropertyValue();
}

CEffect_MeshParticle::CEffect_MeshParticle(const CEffect_MeshParticle& Prototype)
    : CEffect_Particle(Prototype)
{
    Init_PropertyValue();
}

EffectMesh::VALUES CEffect_MeshParticle::Make_MeshValues()
{
    return {
        m_bTextureColorToAlpha,
        {
            m_bUseTextureUVEdgeFade,
            m_iTextureUVEdgeFadeAxis,
            m_fTextureUVEdgeFadeStartRange,
            m_fTextureUVEdgeFadeEndRange,
            m_fTextureUVEdgeFadePower
        },
        {
            m_bUseDiffuseTexture,
            m_vDiffuseTiling,
            m_vDiffuseOffset,
            m_fDiffuseUVRotationDegree,
            m_vDiffuseUVScroll,
            m_vDiffuseUVScrollCount,
            m_vCurDiffuseUVOffset
        },
        m_bDiffuseColorToAlpha,
        {
            m_bUseDiffuseUVEdgeFade,
            m_iDiffuseUVEdgeFadeAxis,
            m_fDiffuseUVEdgeFadeStartRange,
            m_fDiffuseUVEdgeFadeEndRange,
            m_fDiffuseUVEdgeFadePower
        },
        m_bUseNormalTexture,
        m_fNormalUVRotationDegree,
        m_bUseMRATexture,
        m_fMRAUVRotationDegree,
        {
            m_bUseUnknownTexture,
            m_vUnknownTiling,
            m_vUnknownOffset,
            m_fUnknownUVRotationDegree,
            m_vUnknownUVScroll,
            m_vUnknownUVScrollCount,
            m_vCurUnknownUVOffset
        },
        m_bUnknownColorToAlpha,
        {
            m_bUseUnknownUVEdgeFade,
            m_iUnknownUVEdgeFadeAxis,
            m_fUnknownUVEdgeFadeStartRange,
            m_fUnknownUVEdgeFadeEndRange,
            m_fUnknownUVEdgeFadePower
        },
        {
            m_bTextureCircleUVAnim,
            m_fTextureCircleUVRatio,
            m_fTextureCircleUVStartRatio,
            m_fTextureCircleUVEndRatio,
            m_fTextureCircleUVStartDegree,
            m_bTextureCircleUVClockwise
        },
        {
            m_bMaskCircleUVAnim,
            m_fMaskCircleUVRatio,
            m_fMaskCircleUVStartRatio,
            m_fMaskCircleUVEndRatio,
            m_fMaskCircleUVStartDegree,
            m_bMaskCircleUVClockwise
        },
        {
            m_bDiffuseCircleUVAnim,
            m_fDiffuseCircleUVRatio,
            m_fDiffuseCircleUVStartRatio,
            m_fDiffuseCircleUVEndRatio,
            m_fDiffuseCircleUVStartDegree,
            m_bDiffuseCircleUVClockwise
        },
        {
            m_bUnknownCircleUVAnim,
            m_fUnknownCircleUVRatio,
            m_fUnknownCircleUVStartRatio,
            m_fUnknownCircleUVEndRatio,
            m_fUnknownCircleUVStartDegree,
            m_bUnknownCircleUVClockwise
        },
        {
            m_bTextureLinearUVAnim,
            m_fTextureLinearUVRatio,
            m_fTextureLinearUVStartRatio,
            m_fTextureLinearUVEndRatio,
            m_iTextureLinearUVAxis,
            m_bTextureLinearUVReverse
        },
        {
            m_bMaskLinearUVAnim,
            m_fMaskLinearUVRatio,
            m_fMaskLinearUVStartRatio,
            m_fMaskLinearUVEndRatio,
            m_iMaskLinearUVAxis,
            m_bMaskLinearUVReverse
        },
        {
            m_bDiffuseLinearUVAnim,
            m_fDiffuseLinearUVRatio,
            m_fDiffuseLinearUVStartRatio,
            m_fDiffuseLinearUVEndRatio,
            m_iDiffuseLinearUVAxis,
            m_bDiffuseLinearUVReverse
        },
        {
            m_bUnknownLinearUVAnim,
            m_fUnknownLinearUVRatio,
            m_fUnknownLinearUVStartRatio,
            m_fUnknownLinearUVEndRatio,
            m_iUnknownLinearUVAxis,
            m_bUnknownLinearUVReverse
        }
    };
}


HRESULT CEffect_MeshParticle::Initialize(void* pArg)
{
    EFFECT_MESHPARTICLE_DESC* pDesc = static_cast<EFFECT_MESHPARTICLE_DESC*>(pArg);

    m_iModelLevel = pDesc->iModelLevel;
    m_wstrModelTag = pDesc->wstrModelTag;
    m_bUseDiffuseTexture = pDesc->bUseDiffuseTexture;
    m_bUseUnknownTexture = pDesc->bUseUnknownTexture;
    m_bUseNormalTexture = pDesc->bUseNormalTexture;
    m_bUseMRATexture = pDesc->bUseMRATexture;

    m_bCustomShader = pDesc->bCustomShader;
    m_iShaderLevel = pDesc->iShaderLevel;
    m_wstrShaderTag = pDesc->wstrShaderTag;

    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    if (FAILED(Ready_Components()))
        return E_FAIL;

    return S_OK;
}

HRESULT CEffect_MeshParticle::Render()
{
    if (FAILED(Bind_ShaderResources()))
        return E_FAIL;

    if (FAILED(Bind_ShaderValue()))
        return E_FAIL;

    size_t iNumMeshes = m_pModelCom->Get_NumMeshes();
    const _int iPass = Resolve_ShaderPass();
    auto Values = Make_MeshValues();

    for (const PARTICLE& Particle : m_Particles)
    {
        if (Particle.bAlive == false)
            continue;

        _float4x4 ParticleWorld = Make_ParticleWorldMatrix(Particle);
        if (m_bBillboard == true)
            ParticleWorld = Make_BillboardWorldMatrix(ParticleWorld);

        if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", &ParticleWorld)))
            return E_FAIL;

        if (FAILED(m_pShaderCom->Bind_RawValue("g_fAlpha", &Particle.fAlpha, sizeof(Particle.fAlpha))))
            return E_FAIL;

        if (FAILED(m_pShaderCom->Bind_RawValue("g_vColor", &Particle.vColor, sizeof(Particle.vColor))))
            return E_FAIL;

        for (_uint i = 0; i < iNumMeshes; ++i)
        {
            if (FAILED(EffectMesh::Bind_Materials(m_pModelCom, m_pShaderCom, i, Values)))
                return E_FAIL;

            if (FAILED(m_pShaderCom->Begin(iPass)))
                return E_FAIL;

            if (FAILED(m_pModelCom->Render(i)))
                return E_FAIL;
        }
    }

    return S_OK;
}

HRESULT CEffect_MeshParticle::Ready_Components()
{
    if (m_bCustomShader == false)
        m_pShaderCom = m_pGameInstance_Proxy->Get_MeshShader();
    else
        m_pShaderCom = Add_Component<CShader>(m_iShaderLevel, m_wstrShaderTag, TEXT("Com_Shader"));

    if (m_pShaderCom == nullptr)
        return E_FAIL;

    m_pModelCom = Add_Component<CModel>(m_iModelLevel, m_wstrModelTag, TEXT("Com_Model"));
    if (m_pModelCom == nullptr)
        return E_FAIL;

    return S_OK;
}

HRESULT CEffect_MeshParticle::Bind_ShaderResources()
{
    if (FAILED(Bind_ViewProjectionMatrices()))
        return E_FAIL;

    return S_OK;
}

HRESULT CEffect_MeshParticle::Bind_ShaderValue()
{
    if (FAILED(__super::Bind_ShaderValue()))
        return E_FAIL;

    auto Values = Make_MeshValues();
    return EffectMesh::Bind_ShaderValues(m_pShaderCom, Values, true);
}

void CEffect_MeshParticle::Update_UVScroll(const _float fTimeDelta, const _float fRatio)
{
    __super::Update_UVScroll(fTimeDelta, fRatio);

    auto Values = Make_MeshValues();
    EffectMesh::Update_UVAnimations(Values, fRatio);
}

void CEffect_MeshParticle::Init_PropertyValue()
{
    m_bBillboard = false;

    auto Values = Make_MeshValues();
    EffectMesh::Initialize_DefaultValues(Values);
}
