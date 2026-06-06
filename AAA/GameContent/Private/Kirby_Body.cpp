#include "Kirby_Body.h"

#include "GameInstance.h"

#include "GameContent_const.h"

#include "Animator.h"

CKirby_Body::CKirby_Body(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CPartObject(pDevice, pContext)
{
}

CKirby_Body::CKirby_Body(const CKirby_Body& Prototype)
    : CPartObject(Prototype){
}

HRESULT CKirby_Body::Initialize_Prototype()
{
    m_eProjType = PROJ_TYPE::PERSPEC;
    return S_OK;
}

HRESULT CKirby_Body::Initialize(void* pArg)
{
    KIRBY_BODY_DESC* pDesc = static_cast<KIRBY_BODY_DESC*>(pArg);

    pDesc->fSpeedPerSec = 1.f;

    if (FAILED(__super::Initialize(pDesc)))
        return E_FAIL;

    if (FAILED(Ready_Components()))
        return E_FAIL;
    
    m_pAnimatorCom->Play("Wait", true, true);

    return S_OK;
}

void CKirby_Body::Priority_Update(_float fTimeDelta)
{
}

void CKirby_Body::Update(_float fTimeDelta)
{
    static _int iState = 0; // 0: Wait, 1: Run

    _float3 vDir = { 0.f, 0.f, 0.f };

    if (m_pGameInstance_Proxy->Key_Pressing(DIK_W))
    {
        vDir.z += 1.f;
    }

    if (m_pGameInstance_Proxy->Key_Pressing(DIK_S))
    {
        vDir.z -= 1.f;
    }

    if (m_pGameInstance_Proxy->Key_Pressing(DIK_A))
    {
        vDir.x -= 1.f;
    }

    if (m_pGameInstance_Proxy->Key_Pressing(DIK_D))
    {
        vDir.x += 1.f;
    }

    _bool bMove = false;

    if (vDir.x != 0.f || vDir.z != 0.f)
    {
        bMove = true;

        _vector vMoveDir = XMVector3Normalize(XMLoadFloat3(&vDir));

        // +Z를 기본 정면으로 보고, 이동 방향에 맞는 Y축 회전각 계산
        _float fRadian = atan2f(vDir.x, vDir.z);

        m_pTransformCom->Rotation(
            XMVectorSet(0.f, 1.f, 0.f, 0.f),
            fRadian
        );

        // 현재 LOOK 방향으로 이동
        m_pTransformCom->Go_Straight(fTimeDelta * 10.f);    

        if (iState != 1)
        {
            m_pAnimatorCom->Play("Run", true, true);
            iState = 1;
        }
    }
    else
    {
        if (iState != 0)
        {
            m_pAnimatorCom->Play("Wait", true, true);
            iState = 0;
        }
    }

    if (m_pGameInstance_Proxy->Key_Pressing(DIK_Z))
    {
        Update_Jump(fTimeDelta);
    }

    m_pAnimatorCom->Update(fTimeDelta);
}

void CKirby_Body::Late_Update(_float fTimeDelta)
{
    __super::Compute_CombinedWorldMatrix(XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));

    m_pGameInstance_Proxy->Add_RenderGroup(RENDERID::NONBLEND, this);
}

HRESULT CKirby_Body::Render()
{
    if (FAILED(Bind_ShaderResources()))
        return E_FAIL;

    size_t iNumMeshes = m_pModelCom->Get_NumMeshes();

    for (_uint i = 0; i < iNumMeshes; ++i)
    {
        if (i == 0 || i == 2 || i == 3 || i == 5 || i == 6 || i == 7)
            continue;

        if(FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_EyeTexture", i, MTEX_TYPE::UNKNOWN, 0)))
            return E_FAIL;
        if(FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_EyeMaskTexture", i, MTEX_TYPE::UNKNOWN, 3)))
            return E_FAIL;

        if(FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_SkinTexture", i, MTEX_TYPE::UNKNOWN, 1)))
            return E_FAIL;
        if(FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_MouthTexture", i, MTEX_TYPE::UNKNOWN, 2)))
            return E_FAIL;
        //if(FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_WetMaskTexture", i, MTEX_TYPE::UNKNOWN, 4)))
        //    return E_FAIL;
        //if(FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_NormalTexture", i, MTEX_TYPE::NORMALS, 0)))
        //    return E_FAIL;

        if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", (_uint)i)))
            return E_FAIL;
        if(FAILED(m_pShaderCom->Bind_RawValue("g_vBodyColor", &m_vBodyColor, sizeof(_float4))))
            return E_FAIL;
        if(FAILED(m_pShaderCom->Bind_RawValue("g_vFootColor", &m_vFootColor, sizeof(_float4))))
            return E_FAIL;
        if(FAILED(m_pShaderCom->Bind_RawValue("g_vBlushColor", &m_vBlushColor, sizeof(_float4))))
            return E_FAIL;

        if (FAILED(m_pShaderCom->Begin(0)))
            return E_FAIL;

        if (FAILED(m_pModelCom->Render(i)))
            return E_FAIL;
    }

    return S_OK;
}

void CKirby_Body::Update_Jump(_float fTimeDelta)
{
    if (false == m_pGameInstance_Proxy->Key_Pressing(DIK_Z))
        return;

    const _float fJumpSpeed = 8.f;

    _vector vPos = m_pTransformCom->Get_State(STATE::POSITION);

    vPos += XMVectorSet(0.f, fJumpSpeed * fTimeDelta, 0.f, 0.f);

    m_pTransformCom->Set_State(STATE::POSITION, vPos);
}

HRESULT CKirby_Body::Ready_Components()
{
    /* For.Com_Shader */
    m_pShaderCom = Add_Component<CShader>(Shader_Kirby.iLevelID, Shader_Kirby.szProtoTag, TEXT("Com_Shader"));
    if (m_pShaderCom == nullptr)
        return E_FAIL;

    /* For.Com_Model */
    m_pModelCom = Add_Component<CModel>(ETOUI(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Model_Kirby_Body"), TEXT("Com_Model"));
    if (m_pModelCom == nullptr)
        return E_FAIL;

    /* For.Com_Animator */
    CAnimator::ANIMATOR_DESC AnimDesc{};
    AnimDesc.pModel = m_pModelCom;
    //AnimDesc.strDataFile = TEXT("../Bin/Resources/Models/Test/Marb1e/Marb1e_animevents.json");

    m_pAnimatorCom = Add_Component<CAnimator>(TEXT("Com_Animator"), CAnimator::Create(m_pDevice, m_pContext));

    if (nullptr == m_pAnimatorCom || FAILED(m_pAnimatorCom->Initialize(&AnimDesc)))
        return E_FAIL;

    return S_OK;
}

HRESULT CKirby_Body::Bind_ShaderResources()
{
    if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", &m_CombinedWorldMatrix)))
        return E_FAIL;

    if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::VIEW, m_eProjType))))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::PROJ, m_eProjType))))
        return E_FAIL;

    return S_OK;
}

CKirby_Body* CKirby_Body::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CKirby_Body* pInstance = new CKirby_Body(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created: CKirby_Body");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CKirby_Body::Clone(void* pArg)
{
    CKirby_Body* pInstance = new CKirby_Body(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned: CKirby_Body");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CKirby_Body::Free()
{
    __super::Free();
}