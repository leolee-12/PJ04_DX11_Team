#include "Effect_MeshParticle.h"

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

HRESULT CEffect_MeshParticle::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CEffect_MeshParticle::Initialize(void* pArg)
{
    EFFECT_MESHPARTICLE_DESC* pDesc = static_cast<EFFECT_MESHPARTICLE_DESC*>(pArg);

    m_iModelLevel = pDesc->iModelLevel;
    m_wstrModelTag = pDesc->wstrModelTag;
    m_bUseDiffuseTexture = pDesc->bUseDiffuseTexture;
    m_bUseUnknownTexture = pDesc->bUseUnKnownTexture;
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

void CEffect_MeshParticle::Priority_Update(_float fTimeDelta)
{
    __super::Priority_Update(fTimeDelta);
}

void CEffect_MeshParticle::Update(_float fTimeDelta)
{
    __super::Update(fTimeDelta);
}

void CEffect_MeshParticle::Late_Update(_float fTimeDelta)
{
    __super::Late_Update(fTimeDelta);
}

HRESULT CEffect_MeshParticle::Render()
{
    if (FAILED(Bind_ShaderResources()))
        return E_FAIL;

    if (FAILED(Bind_ShaderValue()))
        return E_FAIL;

    size_t iNumMeshes = m_pModelCom->Get_NumMeshes();

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

        for (_uint i = 0; i < iNumMeshes; ++i)
        {
            if (m_bUseDiffuseTexture == true)
            {
                if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_DiffuseTexture", i, MTEX_TYPE::DIFFUSE, 0)))
                    return E_FAIL;
            }
            if (m_bUseNormalTexture == true)
            {
                if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_NormalTexture", i, MTEX_TYPE::NORMALS, 0)))
                    return E_FAIL;
            }
            if (m_bUseMRATexture == true)
            {
                if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_MRATexture", i, MTEX_TYPE::METALNESS, 0)))
                    return E_FAIL;
            }
            if (m_bUseUnknownTexture == true)
            {
                if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_UnknownTexture", i, MTEX_TYPE::UNKNOWN, 0)))
                    return E_FAIL;
            }

            Helper::IntClamp(m_iShaderPass, ShaderPass::Default, ShaderPass::ShaderPass_End - 1);
            Helper::IntClamp(m_iMirror, Sampler::DEFAULT, Sampler::SAMPLER_END - 1);

            _int iPass = m_iShaderPass + (m_iMirror == Sampler::MIRROR ? ShaderPass::ShaderPass_End : 0);

            if (FAILED(m_pShaderCom->Begin(iPass)))
                return E_FAIL;

            if (FAILED(m_pModelCom->Render(i)))
                return E_FAIL;
        }
    }

    return S_OK;
}

void CEffect_MeshParticle::Effect_Start()
{
    __super::Effect_Start();
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
    if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::VIEW, m_eProjType))))
        return E_FAIL;

    if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::PROJ, m_eProjType))))
        return E_FAIL;

    return S_OK;
}

HRESULT CEffect_MeshParticle::Bind_ShaderValue()
{
    if (FAILED(__super::Bind_ShaderValue()))
        return E_FAIL;

    if (FAILED(m_pShaderCom->Bind_RawValue("g_bUseDiffuseTexture", &m_bUseDiffuseTexture, sizeof(m_bUseDiffuseTexture))))
        return E_FAIL;

    if (FAILED(m_pShaderCom->Bind_RawValue("g_vDiffuseTiling", &m_vDiffuseTiling, sizeof(m_vDiffuseTiling))))
        return E_FAIL;

    if (FAILED(m_pShaderCom->Bind_RawValue("g_vDiffuseOffset", &m_vCurDiffuseUVOffset, sizeof(m_vCurDiffuseUVOffset))))
        return E_FAIL;

    if (FAILED(m_pShaderCom->Bind_RawValue("g_bUseUnknownTexture", &m_bUseUnknownTexture, sizeof(m_bUseUnknownTexture))))
        return E_FAIL;

    if (FAILED(m_pShaderCom->Bind_RawValue("g_vUnknownTiling", &m_vUnknownTiling, sizeof(m_vUnknownTiling))))
        return E_FAIL;

    if (FAILED(m_pShaderCom->Bind_RawValue("g_vUnknownOffset", &m_vCurUnknownUVOffset, sizeof(m_vCurUnknownUVOffset))))
        return E_FAIL;

    if (FAILED(m_pShaderCom->Bind_RawValue("g_bUseNormalTexture", &m_bUseNormalTexture, sizeof(m_bUseNormalTexture))))
        return E_FAIL;

    if (FAILED(m_pShaderCom->Bind_RawValue("g_bUseMRATexture", &m_bUseMRATexture, sizeof(m_bUseMRATexture))))
        return E_FAIL;

    if (FAILED(m_pShaderCom->Bind_RawValue("g_bUseCircleUVAnim_T", &m_bTextureCircleUVAnim, sizeof(m_bTextureCircleUVAnim))))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_RawValue("g_fCircleUVRatio_T", &m_fTextureCircleUVRatio, sizeof(m_fTextureCircleUVRatio))))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_RawValue("g_fCircleUVStartDegree_T", &m_fTextureCircleUVStartDegree, sizeof(m_fTextureCircleUVStartDegree))))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_RawValue("g_bCircleUVClockwise_T", &m_bTextureCircleUVClockwise, sizeof(m_bTextureCircleUVClockwise))))
        return E_FAIL;

    if (FAILED(m_pShaderCom->Bind_RawValue("g_bUseCircleUVAnim_M", &m_bMaskCircleUVAnim, sizeof(m_bMaskCircleUVAnim))))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_RawValue("g_fCircleUVRatio_M", &m_fMaskCircleUVRatio, sizeof(m_fMaskCircleUVRatio))))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_RawValue("g_fCircleUVStartDegree_M", &m_fMaskCircleUVStartDegree, sizeof(m_fMaskCircleUVStartDegree))))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_RawValue("g_bCircleUVClockwise_M", &m_bMaskCircleUVClockwise, sizeof(m_bMaskCircleUVClockwise))))
        return E_FAIL;

    if (FAILED(m_pShaderCom->Bind_RawValue("g_bUseCircleUVAnim_D", &m_bDiffuseCircleUVAnim, sizeof(m_bDiffuseCircleUVAnim))))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_RawValue("g_fCircleUVRatio_D", &m_fDiffuseCircleUVRatio, sizeof(m_fDiffuseCircleUVRatio))))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_RawValue("g_fCircleUVStartDegree_D", &m_fDiffuseCircleUVStartDegree, sizeof(m_fDiffuseCircleUVStartDegree))))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_RawValue("g_bCircleUVClockwise_D", &m_bDiffuseCircleUVClockwise, sizeof(m_bDiffuseCircleUVClockwise))))
        return E_FAIL;

    if (FAILED(m_pShaderCom->Bind_RawValue("g_bUseCircleUVAnim_U", &m_bUnknownCircleUVAnim, sizeof(m_bUnknownCircleUVAnim))))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_RawValue("g_fCircleUVRatio_U", &m_fUnknownCircleUVRatio, sizeof(m_fUnknownCircleUVRatio))))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_RawValue("g_fCircleUVStartDegree_U", &m_fUnknownCircleUVStartDegree, sizeof(m_fUnknownCircleUVStartDegree))))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_RawValue("g_bCircleUVClockwise_U", &m_bUnknownCircleUVClockwise, sizeof(m_bUnknownCircleUVClockwise))))
        return E_FAIL;

    if (FAILED(m_pShaderCom->Bind_RawValue("g_bUseLinearUVAnim_T", &m_bTextureLinearUVAnim, sizeof(m_bTextureLinearUVAnim))))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_RawValue("g_fLinearUVRatio_T", &m_fTextureLinearUVRatio, sizeof(m_fTextureLinearUVRatio))))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_RawValue("g_iLinearUVAxis_T", &m_iTextureLinearUVAxis, sizeof(m_iTextureLinearUVAxis))))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_RawValue("g_bLinearUVReverse_T", &m_bTextureLinearUVReverse, sizeof(m_bTextureLinearUVReverse))))
        return E_FAIL;

    if (FAILED(m_pShaderCom->Bind_RawValue("g_bUseLinearUVAnim_M", &m_bMaskLinearUVAnim, sizeof(m_bMaskLinearUVAnim))))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_RawValue("g_fLinearUVRatio_M", &m_fMaskLinearUVRatio, sizeof(m_fMaskLinearUVRatio))))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_RawValue("g_iLinearUVAxis_M", &m_iMaskLinearUVAxis, sizeof(m_iMaskLinearUVAxis))))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_RawValue("g_bLinearUVReverse_M", &m_bMaskLinearUVReverse, sizeof(m_bMaskLinearUVReverse))))
        return E_FAIL;

    if (FAILED(m_pShaderCom->Bind_RawValue("g_bUseLinearUVAnim_D", &m_bDiffuseLinearUVAnim, sizeof(m_bDiffuseLinearUVAnim))))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_RawValue("g_fLinearUVRatio_D", &m_fDiffuseLinearUVRatio, sizeof(m_fDiffuseLinearUVRatio))))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_RawValue("g_iLinearUVAxis_D", &m_iDiffuseLinearUVAxis, sizeof(m_iDiffuseLinearUVAxis))))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_RawValue("g_bLinearUVReverse_D", &m_bDiffuseLinearUVReverse, sizeof(m_bDiffuseLinearUVReverse))))
        return E_FAIL;

    if (FAILED(m_pShaderCom->Bind_RawValue("g_bUseLinearUVAnim_U", &m_bUnknownLinearUVAnim, sizeof(m_bUnknownLinearUVAnim))))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_RawValue("g_fLinearUVRatio_U", &m_fUnknownLinearUVRatio, sizeof(m_fUnknownLinearUVRatio))))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_RawValue("g_iLinearUVAxis_U", &m_iUnknownLinearUVAxis, sizeof(m_iUnknownLinearUVAxis))))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_RawValue("g_bLinearUVReverse_U", &m_bUnknownLinearUVReverse, sizeof(m_bUnknownLinearUVReverse))))
        return E_FAIL;

    return S_OK;
}

void CEffect_MeshParticle::Update_Core(const _float fTimeDelta, const _float fRatio)
{
    __super::Update_Core(fTimeDelta, fRatio);
}

void CEffect_MeshParticle::Update_UVScroll(const _float fTimeDelta, const _float fRatio)
{
    __super::Update_UVScroll(fTimeDelta, fRatio);

    MoveUVScroll(fRatio, m_vDiffuseUVScroll, m_vDiffuseUVScrollCount, m_vDiffuseOffset, m_vCurDiffuseUVOffset);
    MoveUVScroll(fRatio, m_vUnknownUVScroll, m_vUnknownUVScrollCount, m_vUnknownOffset, m_vCurUnknownUVOffset);

    const auto ComputeCircleRatio = [](_float fCurRatio, _float fStartRatio, _float fEndRatio)
        {
            _float fRange = fEndRatio - fStartRatio;

            if (fabsf(fRange) <= Helper::fEpsilon)
                return fCurRatio >= fEndRatio ? 1.f : 0.f;

            _float fResult = (fCurRatio - fStartRatio) / fRange;
            Helper::FloatClamp(fResult, 0.f, 1.f);

            return fResult;
        };

    m_fTextureCircleUVRatio = ComputeCircleRatio(fRatio, m_fTextureCircleUVStartRatio, m_fTextureCircleUVEndRatio);
    m_fMaskCircleUVRatio = ComputeCircleRatio(fRatio, m_fMaskCircleUVStartRatio, m_fMaskCircleUVEndRatio);
    m_fDiffuseCircleUVRatio = ComputeCircleRatio(fRatio, m_fDiffuseCircleUVStartRatio, m_fDiffuseCircleUVEndRatio);
    m_fUnknownCircleUVRatio = ComputeCircleRatio(fRatio, m_fUnknownCircleUVStartRatio, m_fUnknownCircleUVEndRatio);

    m_fTextureLinearUVRatio = ComputeCircleRatio(fRatio, m_fTextureLinearUVStartRatio, m_fTextureLinearUVEndRatio);
    m_fMaskLinearUVRatio = ComputeCircleRatio(fRatio, m_fMaskLinearUVStartRatio, m_fMaskLinearUVEndRatio);
    m_fDiffuseLinearUVRatio = ComputeCircleRatio(fRatio, m_fDiffuseLinearUVStartRatio, m_fDiffuseLinearUVEndRatio);
    m_fUnknownLinearUVRatio = ComputeCircleRatio(fRatio, m_fUnknownLinearUVStartRatio, m_fUnknownLinearUVEndRatio);
}

void CEffect_MeshParticle::Init_PropertyValue()
{
    m_bUseDiffuseTexture = false;
    m_vDiffuseTiling = { 1.f, 1.f };
    m_vDiffuseOffset = { 0.f, 0.f };

    m_vDiffuseUVScroll = false;
    m_vDiffuseUVScrollCount = { 0.f, 0.f };

    m_bUseUnknownTexture = false;
    m_vUnknownTiling = { 1.f, 1.f };
    m_vUnknownOffset = { 0.f, 0.f };

    m_vUnknownUVScroll = false;
    m_vUnknownUVScrollCount = { 0.f, 0.f };

    m_bTextureCircleUVAnim = false;
    m_fTextureCircleUVStartDegree = 0.f;
    m_bTextureCircleUVClockwise = true;
    m_fTextureCircleUVStartRatio = 0.f;
    m_fTextureCircleUVEndRatio = 1.f;

    m_bMaskCircleUVAnim = false;
    m_fMaskCircleUVStartDegree = 0.f;
    m_bMaskCircleUVClockwise = true;
    m_fMaskCircleUVStartRatio = 0.f;
    m_fMaskCircleUVEndRatio = 1.f;

    m_bDiffuseCircleUVAnim = false;
    m_fDiffuseCircleUVStartDegree = 0.f;
    m_bDiffuseCircleUVClockwise = true;
    m_fDiffuseCircleUVStartRatio = 0.f;
    m_fDiffuseCircleUVEndRatio = 1.f;

    m_bUnknownCircleUVAnim = false;
    m_fUnknownCircleUVStartDegree = 0.f;
    m_bUnknownCircleUVClockwise = true;
    m_fUnknownCircleUVStartRatio = 0.f;
    m_fUnknownCircleUVEndRatio = 1.f;

    m_fTextureCircleUVRatio = 1.f;
    m_fMaskCircleUVRatio = 1.f;
    m_fDiffuseCircleUVRatio = 1.f;
    m_fUnknownCircleUVRatio = 1.f;

    m_bTextureLinearUVAnim = false;
    m_iTextureLinearUVAxis = 0;
    m_bTextureLinearUVReverse = false;
    m_fTextureLinearUVStartRatio = 0.f;
    m_fTextureLinearUVEndRatio = 1.f;

    m_bMaskLinearUVAnim = false;
    m_iMaskLinearUVAxis = 0;
    m_bMaskLinearUVReverse = false;
    m_fMaskLinearUVStartRatio = 0.f;
    m_fMaskLinearUVEndRatio = 1.f;

    m_bDiffuseLinearUVAnim = false;
    m_iDiffuseLinearUVAxis = 0;
    m_bDiffuseLinearUVReverse = false;
    m_fDiffuseLinearUVStartRatio = 0.f;
    m_fDiffuseLinearUVEndRatio = 1.f;

    m_bUnknownLinearUVAnim = false;
    m_iUnknownLinearUVAxis = 0;
    m_bUnknownLinearUVReverse = false;
    m_fUnknownLinearUVStartRatio = 0.f;
    m_fUnknownLinearUVEndRatio = 1.f;

    m_fTextureLinearUVRatio = 0.f;
    m_fMaskLinearUVRatio = 0.f;
    m_fDiffuseLinearUVRatio = 0.f;
    m_fUnknownLinearUVRatio = 0.f;
}

void CEffect_MeshParticle::Free()
{
    __super::Free();
}