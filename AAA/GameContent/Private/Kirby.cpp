#include "Kirby.h"

#include "GameInstance.h"

#include "PartObject.h"

#include "GameContent_const.h"
#include "Kirby_Body.h"

CKirby::CKirby(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CCharacter{ pDevice, pContext }
{
}

CKirby::CKirby(const CKirby& Prototype)
    : CCharacter(Prototype)
{
}

HRESULT CKirby::Initialize_Prototype()
{
    m_eProjType = PROJ_TYPE::PERSPEC;
    return S_OK;
}

HRESULT CKirby::Initialize(void* pArg)
{
    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    if (FAILED(Ready_Components()))
        return E_FAIL;

    if (FAILED(Ready_PartObjects()))
        return E_FAIL;

    _float3 vFoot;
    XMStoreFloat3(&vFoot, m_pTransformCom->Get_State(STATE::POSITION));
    m_pController = m_pGameInstance_Proxy->Create_CapsuleController(vFoot, CCT_RADIUS, CCT_HEIGHT);

    return S_OK;
}

void CKirby::Priority_Update(_float fTimeDelta)
{
    __super::Priority_Update(fTimeDelta);
}

void CKirby::Update(_float fTimeDelta)
{
    __super::Update(fTimeDelta);

    if (nullptr == m_pController)
        return;

    if (m_pGameInstance_Proxy->Is_EditMode())
    {
        _float3 vFoot;
        XMStoreFloat3(&vFoot, m_pTransformCom->Get_State(STATE::POSITION));
        m_pGameInstance_Proxy->Set_ControllerFootPosition(m_pController, vFoot);
        m_fVerticalVelocity = 0.f;     // 낙하속도 누적 방지
        return;
    }

    // 1) 입력 → 월드축 수평 방향 (M2는 단순 WASD, 카메라상대는 다음 단계)
    _vector vDir = XMVectorZero();
    if (m_pGameInstance_Proxy->Key_Pressing(DIK_W)) vDir += XMVectorSet(0.f, 0.f, 1.f, 0.f);
    if (m_pGameInstance_Proxy->Key_Pressing(DIK_S)) vDir += XMVectorSet(0.f, 0.f, -1.f, 0.f);
    if (m_pGameInstance_Proxy->Key_Pressing(DIK_D)) vDir += XMVectorSet(1.f, 0.f, 0.f, 0.f);
    if (m_pGameInstance_Proxy->Key_Pressing(DIK_A)) vDir += XMVectorSet(-1.f, 0.f, 0.f, 0.f);

    _float3 vHoriz{ 0.f, 0.f, 0.f };
    if (!XMVector3Equal(vDir, XMVectorZero()))
    {
        vDir = XMVector3Normalize(vDir);
        XMStoreFloat3(&vHoriz, vDir * MOVE_SPEED * fTimeDelta);
    }

    // 2) 중력 누적 → 수직 변위
    m_fVerticalVelocity += GRAVITY * fTimeDelta;
    const _float fVertDisp = m_fVerticalVelocity * fTimeDelta;

    // 3) 변위 합성 → 이동
    _float3 vDisp{ vHoriz.x, fVertDisp, vHoriz.z };
    _float3 vOutFoot{};
    m_bGrounded = m_pGameInstance_Proxy->Move_Controller(m_pController, vDisp, fTimeDelta, &vOutFoot);
#ifdef _DEBUG
    {
        char buf[256];
        sprintf_s(buf, "[Kirby] foot=(%.2f,%.2f,%.2f) ground=%d vVel=%.2f\n",
            vOutFoot.x, vOutFoot.y, vOutFoot.z, (int)m_bGrounded, m_fVerticalVelocity);
        OutputDebugStringA(buf);
    }
#endif

    // 4) 바닥 닿으면 낙하속도 리셋
    if (m_bGrounded && m_fVerticalVelocity < 0.f)
        m_fVerticalVelocity = 0.f;

    // 5) 결과 발 위치 → transform 역기입
    m_pTransformCom->Set_State(STATE::POSITION,
        XMVectorSet(vOutFoot.x, vOutFoot.y, vOutFoot.z, 1.f));
}

void CKirby::Late_Update(_float fTimeDelta)
{
    __super::Late_Update(fTimeDelta);
}

HRESULT CKirby::Render()
{
    return S_OK;
}

HRESULT CKirby::Ready_Components()
{
    return S_OK;
}

HRESULT CKirby::Ready_PartObjects()
{
    CKirby_Body::KIRBY_BODY_DESC BodyDesc{};
    BodyDesc.pParentMatrix = m_pTransformCom->Get_WorldMatrixPtr();

    if (FAILED(Add_PartObject(ETOUI(LEVEL::GAMEPLAY), CKirby_Body::PROTOTYPE_TAG,
        TEXT("Body"), &BodyDesc)))
        return E_FAIL;

    m_pBody = dynamic_cast<CKirby_Body*>(m_PartObjects[TEXT("Body")]);

    return S_OK;
}

HRESULT CKirby::Bind_ShaderResources()
{
    return S_OK;
}

CKirby* CKirby::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CKirby* pInstance = new CKirby(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created: CKirby");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CKirby::Clone(void* pArg)
{
    CKirby* pInstance = new CKirby(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : CKirby");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CKirby::Free()
{
    if (m_pController)
    {
        m_pGameInstance_Proxy->Release_Controller(m_pController);
        m_pController = nullptr;
    }

    __super::Free();
}