#include "BossNamePlate.h"
#include "GameInstance.h"
#include "Shader.h"
#include "Model.h"

CBossNamePlate::CBossNamePlate(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CGameObject(pDevice, pContext) {
}
CBossNamePlate::CBossNamePlate(const CBossNamePlate& Prototype)
    : CGameObject(Prototype) {
}

HRESULT CBossNamePlate::Initialize(void* pArg)
{
    if (FAILED(__super::Initialize(pArg))) return E_FAIL;   // 내부에서 Ready_Events 호출
    if (FAILED(Ready_Components()))        return E_FAIL;

    // [임시 테스트] 스폰 즉시 표시 + 디졸브 안 함
    //Activate();              // Active=true, dissolve=0
    //m_fHoldTime = 1e9f;      // Dissolve 단계로 안 넘어감 -> 영구 표시
    return S_OK;
}

HRESULT CBossNamePlate::Ready_Components()
{
    m_pShaderCom = Add_Component<CShader>(ETOUI(LEVEL::STATIC),
        Get_ShaderProtoTag(), TEXT("Com_Shader"));
    if (!m_pShaderCom) return E_FAIL;

    m_pModelCom = Add_Component<CModel>(m_iPrototypeLevel,
        Get_ModelProtoTag(), TEXT("Com_Model"));          // 자식이 지정한 모델
    if (!m_pModelCom) return E_FAIL;

    return S_OK;
}

HRESULT CBossNamePlate::Ready_Events()
{
    Set_Active(false);                                     // 처음엔 꺼둠
    Subscribe_Event(Get_ActivateEventTag(), [this](void*) { Activate(); });
    return S_OK;
}

void CBossNamePlate::Activate()
{
    Set_Active(true);
    m_eState = EState::Hold;
    m_fTimer = 0.f;
    m_fDissolve = 0.f;

    m_pGameInstance_Proxy->Play_SFX(L"CharaBossBasic_DemoFlash.wav", 1.f, ESoundBus::UI);
}

void CBossNamePlate::Update(_float fTimeDelta)
{
    if (!m_bActive) return;

    switch (m_eState)
    {
        case EState::Hold:
            m_fTimer += fTimeDelta;
            if (m_fTimer >= m_fHoldTime) { m_eState = EState::Dissolve; m_fTimer = 0.f; }
            break;

        case EState::Dissolve:
            m_fTimer += fTimeDelta;
            m_fDissolve = min(1.f, m_fTimer / m_fFadeTime);
            if (m_fDissolve >= 1.f) { m_eState = EState::Idle; Set_Active(false); }
            break;

        default: break;
    }
}

void CBossNamePlate::Late_Update(_float fTimeDelta)
{
    if (!m_bActive) return;
    m_pGameInstance_Proxy->Add_RenderGroup(RENDERID::NONBLEND, this);
}

HRESULT CBossNamePlate::Bind_ShaderResources()
{
    if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", m_pTransformCom->Get_WorldMatrixPtr())))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix",
        m_pGameInstance_Proxy->Get_Matrix(D3DTS::VIEW, PROJ_TYPE::PERSPEC))))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix",
        m_pGameInstance_Proxy->Get_Matrix(D3DTS::PROJ, PROJ_TYPE::PERSPEC))))
        return E_FAIL;

    if (FAILED(m_pShaderCom->Bind_RawValue("g_fDissolve", &m_fDissolve, sizeof(_float))))
        return E_FAIL;

    return S_OK;
}

void CBossNamePlate::Free() { __super::Free(); }