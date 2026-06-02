#include "Preview_Actor.h"
#include "GameInstance.h"
#include "GameContent_const.h"   
#include "Shader.h"
#include "Model.h"
#include "Animator.h"
#include "GameContent_AnimEvents.h"


CPreview_Actor::CPreview_Actor(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
	, m_iAnimationIndex { 0 }
{
}

CPreview_Actor::CPreview_Actor(const CPreview_Actor& Prototype)
	: CGameObject{ Prototype }
	, m_iAnimationIndex { Prototype.m_iAnimationIndex }
{
}

HRESULT CPreview_Actor::Initialize_Prototype()
{
    m_eProjType = PROJ_TYPE::PERSPEC;
    return S_OK;
}

HRESULT CPreview_Actor::Initialize(void* pArg)
{
    if (pArg)
        m_Desc = *static_cast<PREVIEW_DESC*>(pArg);

    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    if (FAILED(Ready_Components()))
        return E_FAIL;

    return S_OK;
}

void CPreview_Actor::Late_Update(_float fTimeDelta)
{
    m_pGameInstance_Proxy->Add_RenderGroup(RENDERID::NONBLEND, this);
}

HRESULT CPreview_Actor::Render()
{
    m_pShaderCom = Get_ResolvedShader();
    if (nullptr == m_pShaderCom)
        return E_FAIL;

    if (FAILED(Bind_ShaderResources()))
        return E_FAIL;

    const _bool bAnimModel = (m_Desc.eType == MODEL::ANIM);
    const _bool bUsingAnimShader = (m_pShaderCom == m_pAnimShaderCom);
    const _uint iPass = Get_ResolvedPassIndex();

    const _uint iNum = m_pModelCom->Get_NumMeshes();
    for (_uint i = 0; i < iNum; ++i)
    {
        if (!Is_MeshVisible((_uint)i))
            continue;

        if (FAILED(Bind_MaterialOrClear("g_DiffuseTexture", (_uint)i, MTEX_TYPE::DIFFUSE)))
            return E_FAIL;

        if (FAILED(Bind_MaterialOrClear("g_NormalTexture", (_uint)i, MTEX_TYPE::NORMALS)))
            return E_FAIL;

        if (FAILED(Bind_MaterialOrClear("g_MRATexture", (_uint)i, MTEX_TYPE::METALNESS)))
            return E_FAIL;

        if (FAILED(Bind_MaterialOrClear("g_UnkownTexture", (_uint)i, MTEX_TYPE::UNKNOWN)))
            return E_FAIL;

        if (bAnimModel && bUsingAnimShader)
        {
            if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", (_uint)i)))
                return E_FAIL;
        }

        if (FAILED(m_pShaderCom->Begin(iPass)))
            return E_FAIL;

        if (FAILED(m_pModelCom->Render((_uint)i)))
            return E_FAIL;
    }

    return S_OK;
}

const _char* CPreview_Actor::Get_ResolvedShaderName() const
{
    switch (m_ePreviewShaderMode)
    {
    case PREVIEW_SHADER_MODE::ANIM_MESH:
        return "AnimMesh";

    case PREVIEW_SHADER_MODE::NONANIM_MESH:
        return "NonAnimMesh";

    case PREVIEW_SHADER_MODE::AUTO:
    default:
        return (m_Desc.eType == MODEL::ANIM)
            ? "Auto -> AnimMesh"
            : "Auto -> NonAnimMesh";
    }
}

_uint CPreview_Actor::Get_ResolvedPassIndex() const
{
    if (m_iPreviewPassOverride >= 0)
        return (_uint)m_iPreviewPassOverride;

    if (m_ePreviewShaderMode == PREVIEW_SHADER_MODE::NONANIM_MESH)
        return 0;

    if (m_ePreviewShaderMode == PREVIEW_SHADER_MODE::ANIM_MESH)
        return (m_Desc.eType == MODEL::ANIM) ? 1 : 0;

    return (m_Desc.eType == MODEL::ANIM) ? 1 : 0;
}

void CPreview_Actor::Reset_MeshVisibility()
{
    const _uint iNumMeshes = m_pModelCom ? m_pModelCom->Get_NumMeshes() : 0;
    m_MeshVisible.assign(iNumMeshes, true);
}

_bool CPreview_Actor::Is_MeshVisible(_uint iMesh) const
{
    if (iMesh >= m_MeshVisible.size())
        return true;

    return m_MeshVisible[iMesh] != 0;
}

void CPreview_Actor::Set_MeshVisible(_uint iMesh, _bool bVisible)
{
    if (iMesh >= m_MeshVisible.size())
        return;

    m_MeshVisible[iMesh] = bVisible ? 1 : 0;
}

void CPreview_Actor::Set_AllMeshVisible(_bool bVisible)
{
    for (size_t i = 0; i < m_MeshVisible.size(); ++i)
        m_MeshVisible[i] = bVisible;
}

void CPreview_Actor::Set_SoloMesh(_uint iMesh) 
{
    for (size_t i = 0; i < m_MeshVisible.size(); ++i)
        m_MeshVisible[i] = (i == iMesh);
}

HRESULT CPreview_Actor::Ready_Components()
{
    m_pAnimShaderCom = Add_Component<CShader>(
        m_Desc.iProtoLevel,
        L"Proto_Shader_AnimMesh",
        TEXT("Com_Shader_Anim"));

    if (nullptr == m_pAnimShaderCom)
        return E_FAIL;

    m_pNonAnimShaderCom = Add_Component<CShader>(
        m_Desc.iProtoLevel,
        L"Proto_Shader_NonAnimMesh",
        TEXT("Com_Shader_NonAnim"));

    if (nullptr == m_pNonAnimShaderCom)
        return E_FAIL;

    m_pShaderCom = Get_ResolvedShader();
    if (nullptr == m_pShaderCom)
        return E_FAIL;

    m_pModelCom = Add_Component<CModel>(
        m_Desc.iProtoLevel,
        m_Desc.szModelTag,
        TEXT("Com_Model"));

    if (nullptr == m_pModelCom)
        return E_FAIL;

    Reset_MeshVisibility();

    if (m_Desc.eType == MODEL::ANIM)
    {
        CAnimator::ANIMATOR_DESC AnimDesc{};
        AnimDesc.pModel = m_pModelCom;
        AnimDesc.strDataFile = m_Desc.strAnimEvents;

        m_pAnimatorCom = Add_Component<CAnimator>(
            TEXT("Com_Animator"),
            CAnimator::Create(m_pDevice, m_pContext));

        if (nullptr == m_pAnimatorCom || FAILED(m_pAnimatorCom->Initialize(&AnimDesc)))
            return E_FAIL;
    }

    if (m_pAnimatorCom)
    {
        m_pAnimatorCom->Set_EventCallback(
            [this](const ANIM_EVENT& e, ANIM_EVENT_PHASE phase)
            {
                const _char* szPhase = "";

                switch (static_cast<EANIM_EVENT>(e.iEventType))
                {
                case EANIM_EVENT::Fx:
                    if (phase == ANIM_EVENT_PHASE::POINT)
                    {
                        szPhase = "POINT";
                    }
                    break;
                case EANIM_EVENT::Hitbox:
                    if (phase == ANIM_EVENT_PHASE::BEGIN)
                        szPhase = "BEGIN";
                    if (phase == ANIM_EVENT_PHASE::END)
                        szPhase = "END";
                    break;
                default:
                    break;
                }

                Log_Info("AnimEvent Fired : type=" +
                    to_string(e.iEventType) +
                    ", phase=" +
                    szPhase);
            });
    }

    return S_OK;
}

HRESULT CPreview_Actor::Bind_ShaderResources()
{
    if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
        return E_FAIL;

    if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::VIEW, m_eProjType))))
        return E_FAIL;

    if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::PROJ, m_eProjType))))
        return E_FAIL;

    return S_OK;
}

HRESULT CPreview_Actor::Bind_MaterialOrClear(const _char* pConstantName, _uint iMeshIndex, MTEX_TYPE eType, _uint iIndex)
{
    if (FAILED(m_pModelCom->Bind_Material(
        m_pShaderCom,
        pConstantName,
        iMeshIndex,
        eType,
        iIndex)))
    {
        return m_pShaderCom->Bind_SRV(pConstantName, nullptr);
    }

    return S_OK;
}

CShader* CPreview_Actor::Get_ResolvedShader() const
{
    switch (m_ePreviewShaderMode)
    {
    case PREVIEW_SHADER_MODE::ANIM_MESH:
        return m_pAnimShaderCom;

    case PREVIEW_SHADER_MODE::NONANIM_MESH:
        return m_pNonAnimShaderCom;

    case PREVIEW_SHADER_MODE::AUTO:
    default:
        return (m_Desc.eType == MODEL::ANIM)
            ? m_pAnimShaderCom
            : m_pNonAnimShaderCom;
    }
}

CPreview_Actor* CPreview_Actor::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CPreview_Actor* pInstance = new CPreview_Actor(pDevice, pContext);
    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created : CPreview_Actor");
        Safe_Release(pInstance);
    }
    return pInstance;
}

CGameObject* CPreview_Actor::Clone(void* pArg)
{
    CPreview_Actor* pInstance = new CPreview_Actor(*this);
    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : CPreview_Actor");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CPreview_Actor::Free()
{
    __super::Free();
}