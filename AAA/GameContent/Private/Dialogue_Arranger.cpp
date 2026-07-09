#include "Dialogue_Arranger.h"
#include "GameInstance.h"
#include "GameContrnt_Events.h"

// 위치 vPos에서 vDir 방향을 바라보는 액터 월드 행렬 (스케일 1, 수평 시선)
static _float4x4 Make_ActorWorld(_fvector vPos, _fvector vDir)
{
    _vector vLook = XMVector3Normalize(XMVectorSetY(vDir, 0.f));
    _vector vUp = XMVectorSet(0.f, 1.f, 0.f, 0.f);
    _vector vRight = XMVector3Normalize(XMVector3Cross(vUp, vLook));

    _float4x4 Out{};
    XMStoreFloat4x4(&Out, XMMATRIX(vRight, vUp, vLook, XMVectorSetW(vPos, 1.f)));
    return Out;
}

CDialogue_Arranger::CDialogue_Arranger(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CGameObject{ pDevice, pContext }
{
    m_fActorGap = { 3.f };
}

CDialogue_Arranger::CDialogue_Arranger(const CDialogue_Arranger& Prototype)
    : CGameObject(Prototype)
    , m_fActorGap(Prototype.m_fActorGap)
{
}

HRESULT CDialogue_Arranger::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CDialogue_Arranger::Initialize(void* pArg)
{
    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    return S_OK;
}

HRESULT CDialogue_Arranger::Ready_Events()
{
    Subscribe_Event(EventTag::CutFade_OutDone, [this](void*)
        {
            if (m_bArranged)
                return;
            m_bArranged = true;

            // 배치된 자리(자기 트랜스폼)가 대화 씬 앵커.
            // 커비는 룩 반대편에서 웨이들디를, 웨이들디는 룩 방향에서 커비를 바라봄
            _vector vAnchor = m_pTransformCom->Get_State(STATE::POSITION);
            _vector vLook = XMVector3Normalize(XMVectorSetY(m_pTransformCom->Get_State(STATE::LOOK), 0.f));
            _float  fHalf = m_fActorGap * 0.5f;

            DIALOGUE_SETUP_DESC Desc{};
            Desc.KirbyWorld = Make_ActorWorld(vAnchor - vLook * fHalf, vLook);
            Desc.DeeWorld = Make_ActorWorld(vAnchor + vLook * fHalf, XMVectorNegate(vLook));

            m_pGameInstance_Proxy->Publish(EventTag::Dialogue_Setup, &Desc);

            // 동기 실행: 위 발행이 리턴했으면 재배치 완료. 바로 걷어냄
            m_pGameInstance_Proxy->Publish(EventTag::CutFade_In, nullptr);
        });

    return S_OK;
}

CDialogue_Arranger* CDialogue_Arranger::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CDialogue_Arranger* pInstance = new CDialogue_Arranger(pDevice, pContext);
    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created : CDialogue_Arranger");
        Safe_Release(pInstance);
    }
    return pInstance;
}

CGameObject* CDialogue_Arranger::Clone(void* pArg)
{
    CDialogue_Arranger* pInstance = new CDialogue_Arranger(*this);
    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : CDialogue_Arranger");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CDialogue_Arranger::Free()
{
    __super::Free();
}