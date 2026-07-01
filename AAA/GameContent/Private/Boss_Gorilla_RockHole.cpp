#include "Boss_Gorilla_RockHole.h"
#include "GameInstance.h"
#include "GameContent_const.h"

CBoss_Gorilla_RockHole::CBoss_Gorilla_RockHole(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CPartObject(pDevice, pContext)
{
}

CBoss_Gorilla_RockHole::CBoss_Gorilla_RockHole(const CBoss_Gorilla_RockHole& Prototype)
	: CPartObject(Prototype)
{
}

HRESULT CBoss_Gorilla_RockHole::Initialize(void* pArg)
{
    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    if (FAILED(Ready_Components()))
        return E_FAIL;

    m_bActive = false;

	return S_OK;
}

void CBoss_Gorilla_RockHole::Update(_float fTimeDelta)
{
    if (!m_bActive)
        return;

    switch (m_eState)
    {
        case HOLE_STATE::START:
            Update_StateStart(fTimeDelta);
            break;
        case HOLE_STATE::WAIT:
            Update_StateWait(fTimeDelta);
            break;
        case HOLE_STATE::END:
            Update_StateEnd(fTimeDelta);
            break;
    }
}

void CBoss_Gorilla_RockHole::Late_Update(_float fTimeDelta)
{
    if (!m_bActive)
        return;

    m_pGameInstance_Proxy->Add_RenderGroup(RENDERID::NONBLEND, this);
}

HRESULT CBoss_Gorilla_RockHole::Render()
{
    if (!m_bActive)
        return S_OK;

    if (FAILED(Bind_ShaderResources()))
        return E_FAIL;

    const _uint iNumMeshes = static_cast<_uint>(m_pModelCom->Get_NumMeshes());
    for (_uint i = 0; i < iNumMeshes; ++i)
    {
        if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_DiffuseTexture", i, MTEX_TYPE::DIFFUSE, 1)))
            return E_FAIL;
        if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_MRATexture", i, MTEX_TYPE::METALNESS, 0)))
            return E_FAIL;
        if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_NormalTexture", i, MTEX_TYPE::NORMALS, 0)))
            return E_FAIL;
        if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_RockHoleMaskTexture", i, MTEX_TYPE::DIFFUSE, 0)))
            return E_FAIL;


        if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i)))
            return E_FAIL;

        if (FAILED(m_pShaderCom->Begin(4)))
            return E_FAIL;

        if (FAILED(m_pModelCom->Render(i)))
            return E_FAIL;
    }

	return S_OK;
}

void CBoss_Gorilla_RockHole::Active_Effect(_fvector vPos)
{
    m_bActive = true;
    m_eState = HOLE_STATE::START;
    m_fWaitTimer = 0.f;
    m_fDissolve = 0.f;
    m_pAnimatorCom->Play("Start", false, true, 0.f, 1.5f);

    m_pTransformCom->Set_State(STATE::POSITION, vPos);

    //start -> wait 4초유지 -> end(end 진입시 디더 스타트)
}

HRESULT CBoss_Gorilla_RockHole::Ready_Components()
{
    m_pShaderCom = Add_Component<CShader>(Shader_Gorilla.iLevelID, Shader_Gorilla.szProtoTag, TEXT("Com_Shader"));
    if (nullptr == m_pShaderCom) return E_FAIL;

    m_pModelCom = Add_Component<CModel>(m_iPrototypeLevel, MODEL_PROTO_TAG, TEXT("Com_Model"));
    if (nullptr == m_pModelCom)  return E_FAIL;

   CAnimator::ANIMATOR_DESC AnimDesc{};
   AnimDesc.pModel = m_pModelCom;

   m_pAnimatorCom = Add_Component<CAnimator>(TEXT("Com_Animator"), CAnimator::Create(m_pDevice, m_pContext));
   if (nullptr == m_pAnimatorCom || FAILED(m_pAnimatorCom->Initialize(&AnimDesc)))
       return E_FAIL;

    return S_OK;
}

HRESULT CBoss_Gorilla_RockHole::Bind_ShaderResources()
{
    if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", m_pTransformCom->Get_WorldMatrixPtr())))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::VIEW,
        m_eProjType))))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::PROJ,
        m_eProjType))))
        return E_FAIL;

    if (FAILED(m_pShaderCom->Bind_RawValue("g_fDissolve", &m_fDissolve, sizeof(_float))))
        return E_FAIL;

    return S_OK;
}

void CBoss_Gorilla_RockHole::Update_StateStart(_float fTimeDelta)
{
    if (m_pAnimatorCom->Is_Finished())
    {
        m_eState = HOLE_STATE::WAIT;
        m_pAnimatorCom->Play("Wait", true, false, 0.2f, 1.5f);

        m_fWaitTimer = s_fWaitTime;
    }

    m_pAnimatorCom->Update(fTimeDelta);
}

void CBoss_Gorilla_RockHole::Update_StateWait(_float fTimeDelta)
{
    m_fWaitTimer -= fTimeDelta;

    if (m_fWaitTimer <= 0.f)
    {
        m_eState = HOLE_STATE::END;
        m_pAnimatorCom->Play("End", false, false, 0.2f, 1.f);
    }

    m_pAnimatorCom->Update(fTimeDelta);
}

void CBoss_Gorilla_RockHole::Update_StateEnd(_float fTimeDelta)
{
    if (m_pAnimatorCom->Is_Finished())
    {
        m_bActive = false;
        return;
    }
    
    m_fDissolve = m_pAnimatorCom->Get_Progress();

    m_pAnimatorCom->Update(fTimeDelta);
}

CBoss_Gorilla_RockHole* CBoss_Gorilla_RockHole::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CBoss_Gorilla_RockHole* pInstance = new CBoss_Gorilla_RockHole(pDevice, pContext);
    if (FAILED(pInstance->Initialize_Prototype())) {
        MSG_BOX("Failed to Created: CBoss_Gorilla_RockHole");
        Safe_Release(pInstance);
    }
    return pInstance;
}

CBoss_Gorilla_RockHole* CBoss_Gorilla_RockHole::Clone(void* pArg)
{
    CBoss_Gorilla_RockHole* pInstance = new CBoss_Gorilla_RockHole(*this);
    if (FAILED(pInstance->Initialize(pArg))) {
        MSG_BOX("Failed to Cloned: CBoss_Gorilla_RockHole");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CBoss_Gorilla_RockHole::Free() { __super::Free(); }
