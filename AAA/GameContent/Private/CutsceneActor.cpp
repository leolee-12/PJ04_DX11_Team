#include "CutsceneActor.h"
#include "GameInstance.h"
#include "Shader.h"
#include "Model.h"
#include "Animator.h"

CCutsceneActor::CCutsceneActor(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CGameObject(pDevice, pContext) {
}
CCutsceneActor::CCutsceneActor(const CCutsceneActor& Prototype)
    : CGameObject(Prototype) {
}

HRESULT CCutsceneActor::Initialize(void* pArg)
{
    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;
    if (FAILED(Ready_Components()))          
        return E_FAIL;
    return S_OK;
}

void CCutsceneActor::Update(_float fTimeDelta)
{
    if (!m_bActive)
        return;
    if (m_pAnimatorCom)
        m_pAnimatorCom->Update(fTimeDelta);
}

void CCutsceneActor::Late_Update(_float fTimeDelta)
{
    if (!m_bActive)
        return;

    m_RenderWorld = *m_pTransformCom->Get_WorldMatrixPtr();

    m_pGameInstance_Proxy->Add_RenderGroup(RENDERID::NONBLEND, this);
}

HRESULT CCutsceneActor::Ready_Visual(const VISUAL_SETUP& t)
{
    m_pShaderCom = Add_Component<CShader>(t.tShader.iLevelID, t.tShader.szProtoTag, TEXT("Com_Shader"));
    if (!m_pShaderCom) return E_FAIL;

    m_pModelCom = Add_Component<CModel>(m_iPrototypeLevel, t.szModelProtoTag, TEXT("Com_Model"));
    if (!m_pModelCom)  return E_FAIL;

    CAnimator::ANIMATOR_DESC AnimDesc{};
    AnimDesc.pModel = m_pModelCom;
    AnimDesc.strDataFile = t.szAnimEventFile ? t.szAnimEventFile : TEXT("");

    m_pAnimatorCom = Add_Component<CAnimator>(TEXT("Com_Animator"), CAnimator::Create(m_pDevice, m_pContext));
    if (!m_pAnimatorCom || FAILED(m_pAnimatorCom->Initialize(&AnimDesc)))
        return E_FAIL;

    m_pAnimatorCom->Set_EventCallback(
        [this](const ANIM_EVENT& e, ANIM_EVENT_PHASE p) { On_AnimEvent(e, p); });

    return S_OK;
}

HRESULT CCutsceneActor::Bind_ShaderResources()
{
    if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", &m_RenderWorld)))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix",
        m_pGameInstance_Proxy->Get_Matrix(D3DTS::VIEW, m_eProjType))))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix",
        m_pGameInstance_Proxy->Get_Matrix(D3DTS::PROJ, m_eProjType))))
        return E_FAIL;
    return S_OK;
}

HRESULT CCutsceneActor::Render()
{
    if (FAILED(Bind_ShaderResources()))
        return E_FAIL;

    const _uint iNumMeshes = static_cast<_uint>(m_pModelCom->Get_NumMeshes());
    for (_uint i = 0; i < iNumMeshes; ++i)
    {
        m_pModelCom->Bind_Material(m_pShaderCom, "g_DiffuseTexture", i, MTEX_TYPE::DIFFUSE, 0);
        m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i);
        if (FAILED(m_pShaderCom->Begin(0))) return E_FAIL;
        if (FAILED(m_pModelCom->Render(i)))  return E_FAIL;
    }
    return S_OK;
}

const _float4x4* CCutsceneActor::Get_BoneMatrixPtr(const _char* pBoneName) const
{
    if (!m_pModelCom || !pBoneName) return nullptr;
    return m_pModelCom->Get_BoneMatrixPtr(pBoneName);
}

void CCutsceneActor::Play(const _char* szAnim, _bool bLoop, _float fBlend, _float fSpeed)
{
    if (m_pAnimatorCom)
        m_pAnimatorCom->Play(szAnim, bLoop, false, fBlend, fSpeed);
}

void CCutsceneActor::Free() { __super::Free(); }