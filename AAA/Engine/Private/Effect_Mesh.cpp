#include "Effect_Mesh.h"

#include "Effect_MeshCommon.h"
#include "GameInstance.h"

CEffect_Mesh::CEffect_Mesh(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CEffect_NonParticle(pDevice, pContext)
{
    Init_PropertyValue();
}

CEffect_Mesh::CEffect_Mesh(const CEffect_Mesh& Prototype)
    : CEffect_NonParticle(Prototype)
{
    Init_PropertyValue();
}

EffectMesh::VALUES CEffect_Mesh::Make_MeshValues()
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
        m_bUseMRATexture,
        {
            m_bUseUnknownTexture,
            m_vUnknownTiling,
            m_vUnknownOffset,
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


HRESULT CEffect_Mesh::Initialize(void* pArg)
{
    EFFECT_MESH_DESC* pDesc = static_cast<EFFECT_MESH_DESC*>(pArg);

    // Model
    m_iModelLevel = pDesc->iModelLevel;
    m_wstrModelTag = pDesc->wstrModelTag;
    m_bUseDiffuseTexture = pDesc->bUseDiffuseTexture;
    m_bUseUnknownTexture = pDesc->bUseUnknownTexture;
    m_bUseNormalTexture = pDesc->bUseNormalTexture;
    m_bUseMRATexture = pDesc->bUseMRATexture;

    // Shader
    m_bCustomShader = pDesc->bCustomShader;
    m_iShaderLevel = pDesc->iShaderLevel;
    m_wstrShaderTag = pDesc->wstrShaderTag;


    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    if (FAILED(Ready_Components()))
        return E_FAIL;

    return S_OK;
}

HRESULT CEffect_Mesh::Render()
{
    if (FAILED(Bind_ShaderResources()))
        return E_FAIL;

    if (FAILED(Bind_ShaderValue()))
        return E_FAIL;

    size_t iNumMeshes = m_pModelCom->Get_NumMeshes();
    const _int iPass = Resolve_ShaderPass();
    auto Values = Make_MeshValues();

    for (_uint i = 0; i < iNumMeshes; ++i)
    {
        if (FAILED(EffectMesh::Bind_Materials(m_pModelCom, m_pShaderCom, i, Values)))
            return E_FAIL;

        if (FAILED(m_pShaderCom->Begin(iPass)))
            return E_FAIL;

        if (FAILED(m_pModelCom->Render(i)))
            return E_FAIL;
    }

    return S_OK;
}

HRESULT CEffect_Mesh::Ready_Components()
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

HRESULT CEffect_Mesh::Bind_ShaderResources()
{
    const _float4x4 WorldMatrix = m_bBillboard == true
        ? Make_BillboardWorldMatrix(m_CombinedWorldMatrix)
        : m_CombinedWorldMatrix;

    if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", &WorldMatrix)))
        return E_FAIL;

    if (FAILED(Bind_ViewProjectionMatrices()))
        return E_FAIL;

    return S_OK;
}

HRESULT CEffect_Mesh::Bind_ShaderValue()
{
    if (FAILED(__super::Bind_ShaderValue()))
        return E_FAIL;

    auto Values = Make_MeshValues();
    return EffectMesh::Bind_ShaderValues(m_pShaderCom, Values);
}

void CEffect_Mesh::Update_UVScroll(const _float fTimeDelta, const _float fRatio)
{
    __super::Update_UVScroll(fTimeDelta, fRatio);

    auto Values = Make_MeshValues();
    EffectMesh::Update_UVAnimations(Values, fRatio);
}

void CEffect_Mesh::Init_PropertyValue()
{
    m_bBillboard = false;

    auto Values = Make_MeshValues();
    EffectMesh::Initialize_DefaultValues(Values);
}
