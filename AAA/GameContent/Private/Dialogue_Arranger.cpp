#include "Dialogue_Arranger.h"
#include "GameInstance.h"
#include "GameContrnt_Events.h"
#include "SequencePlayer.h"

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
    m_fActorBackset = { 0.6f };
    m_fFocusPull = { 0.3f };
}

CDialogue_Arranger::CDialogue_Arranger(const CDialogue_Arranger& Prototype)
    : CGameObject(Prototype)
    , m_fActorGap(Prototype.m_fActorGap)
    , m_fActorBackset(Prototype.m_fActorBackset)
    , m_fFocusPull(Prototype.m_fFocusPull)
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

void CDialogue_Arranger::Update(_float fTimeDelta)
{
    __super::Update(fTimeDelta);

    if (nullptr == m_pPlayer)
        return;

    m_pPlayer->Update(fTimeDelta);

    if (m_pPlayer->Is_Finished() && !m_bFinishNotified)
    {
        m_bFinishNotified = true;
        m_pGameInstance_Proxy->Publish(EventTag::Dialogue_Finished, nullptr);
    }
}

HRESULT CDialogue_Arranger::Ready_Events()
{
    Subscribe_Event(EventTag::Dialogue_Arrange, [this](void*)
        {
            if (m_bArranged)
                return;
            m_bArranged = true;

            m_pPlayer = CSequencePlayer::Create(m_pGameInstance_Proxy);

            const _wstring strPath =
                L"../../Resources/YSH/SequenceData/" + m_strDialogueId + L".json";
            if (FAILED(m_pPlayer->Load(strPath)))
            {
                MSG_BOX("Dialogue_Arranger: sequence load failed");
                return;
            }

            _vector vAnchor = m_pTransformCom->Get_State(STATE::POSITION);
            _vector vLook = XMVector3Normalize(XMVectorSetY(m_pTransformCom->Get_State(STATE::LOOK), 0.f));
            _vector vUp = XMVectorSet(0.f, 1.f, 0.f, 0.f);
            _vector vSide = XMVector3Normalize(XMVector3Cross(vUp, vLook));   // + 방향 = 카메라 자리
            _float  fHalf = m_fActorGap * 0.5f;

            _vector vPosA = vAnchor - vLook * fHalf - vSide * m_fActorBackset;   // 커비: 뒤로 살짝
            _vector vPosB = vAnchor + vLook * fHalf - vSide * m_fActorBackset;   // 상대: 뒤로 살짝
            _vector vFocus = vAnchor + vSide * m_fFocusPull;

            _float4x4 AnchorA = Make_ActorWorld(vPosA, vFocus - vPosA);
            _float4x4 AnchorB = Make_ActorWorld(vPosB, vFocus - vPosB);
            _float4x4 AnchorC = Make_ActorWorld(vAnchor, vLook);

            m_pPlayer->Bind_Anchor(L"A", XMLoadFloat4x4(&AnchorA));
            m_pPlayer->Bind_Anchor(L"B", XMLoadFloat4x4(&AnchorB));
            m_pPlayer->Bind_Anchor(L"Center", XMLoadFloat4x4(&AnchorC));

            PLAYER_QUERY q{};
            m_pGameInstance_Proxy->Publish(EventTag::Query_Player, &q);
            m_pPlayer->Bind_Actor(L"Kirby", q.pPlayer);

            CGameObject* pDee = m_pGameInstance_Proxy->Find_GameObject(Get_LevelIndex(), TEXT("Layer_LiveObject"), TEXT("DialogueDee"));
            if (pDee)
                pDee->Set_Active(true);
            m_pPlayer->Bind_Actor(L"Dee", pDee);

            DIALOGUE_CAMERA_DESC CamDesc{};
            CamDesc.pAnchorWorld = &AnchorC;
            XMStoreFloat3(&CamDesc.vPosA, XMLoadFloat4x4(&AnchorA).r[3]);
            XMStoreFloat3(&CamDesc.vPosB, XMLoadFloat4x4(&AnchorB).r[3]);
            m_pGameInstance_Proxy->Publish(EventTag::Dialogue_CamBegin, &CamDesc);

            m_pPlayer->Play();
        });

    Subscribe_Event(EventTag::Dialogue_SayDone, [this](void*)
        {
            if (m_pPlayer)
                m_pPlayer->Notify_SayDone();
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
    Safe_Release(m_pPlayer);
    __super::Free();
}