#pragma once

#include "GameContent_Defines.h"
#include "UIContainerObject.h"

NS_BEGIN(Client)

// 엔진 CUIContainerObject 는 무수정. 위치+틸트 이동 애니메이션만 얹은 컨테이너.
// 프로토타입으로 직접 찍어 쓸 수도 있고, 특수 컨테이너의 베이스로 상속할 수도 있음.
class CLIENT_DLL CUIMovableContainer : public CUIContainerObject
{
    GENERATED_BODY(CUIMovableContainer)

public:
    static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_UI_MovableContainer";

protected:
    // 생성자 protected: 직접 인스턴스화는 Create 로만, 파생 상속도 계속 가능
    CUIMovableContainer(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CUIMovableContainer(const CUIMovableContainer& Prototype);
    virtual ~CUIMovableContainer() = default;

public:
    virtual HRESULT Initialize_Prototype() override;
    virtual HRESULT Initialize(void* pArg) override;
    virtual void    Update(_float fTimeDelta) override;
    virtual void    Late_Update(_float fTimeDelta) override;

    virtual json Serialize() const override;
    virtual void Deserialize_Internal(const json& j) override;

    virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* pOut) override
    {
        pOut->strPrototypeTag = PROTOTYPE_TAG;
    }

public:
    void Set_GroupParentMatrix(const _float4x4* p) { m_pGroupParentMatrix = p; }
    void Set_MoveTarget(const _float3& vPos, _float fTiltXDeg, _float fTiltYDeg)
    {
        m_vMoveTargetPos = vPos;
        m_fMoveTargetTiltX = fTiltXDeg;
        m_fMoveTargetTiltY = fTiltYDeg;
    }
    const _float3& Get_MoveTargetPos()   const { return m_vMoveTargetPos; }
    _float         Get_MoveTargetTiltX() const { return m_fMoveTargetTiltX; }
    _float         Get_MoveTargetTiltY() const { return m_fMoveTargetTiltY; }

    void   Set_MoveDuration(_float fDur) { m_fMoveDuration = fDur; }
    _float Get_MoveDuration() const { return m_fMoveDuration; }

    // 현재 위치/틸트를 시작점으로, 저장된 도착 지점으로 부드럽게 이동
    // bReverse=true 면 도착 지점에서 원위치로 되돌아옴
    void Start_Move(_bool bReverse = false);

    // 임의 목표를 즉석 지정해 이동 (에디터 값 무시)
    void Move_To(const _float3& vPos, _float fTiltXDeg, _float fTiltYDeg, _float fDuration);

    _bool Is_Moving() const { return m_bMoving; }
    void  Stop_Move() { m_bMoving = false; }

    void  Play_Intro(_float fStartScaleMul, _float fAppearDuration, _float fWaitDuration);
    void Play_SlideIn(const _float3& vFromOffset, _float fDuration);
    void  Set_OnIntroFinished(function<void()> cb) { m_OnIntroFinished = cb; }
    _bool Is_IntroPlaying() const { return m_eIntro == EINTRO::APPEAR || m_eIntro == EINTRO::WAIT; }

protected:
    void Update_MoveAnim(_float fTimeDelta);
    void Update_IntroAnim(_float fTimeDelta);
    virtual void Play_IntroFade(_float fDuration) {}
    virtual void On_IntroFinished() { if (m_OnIntroFinished) m_OnIntroFinished(); }

protected:
    const _float4x4* m_pGroupParentMatrix = { nullptr };

    // 에디터가 저장하는 도착 지점(앵커)
    _float3 m_vMoveTargetPos = { 0.f, 0.f, 0.f };
    _float  m_fMoveTargetTiltX = { 0.f };
    _float  m_fMoveTargetTiltY = { 0.f };
    _float  m_fMoveDuration = { 0.5f };

    // 런타임 이동 상태
    _bool   m_bMoving = { false };
    _float  m_fMoveElapsed = { 0.f };
    _float  m_fMoveDur = { 0.f };
    _float3 m_vMoveFromPos = {};
    _float3 m_vMoveToPos = {};
    _float  m_fMoveFromTiltX = { 0.f };
    _float  m_fMoveFromTiltY = { 0.f };
    _float  m_fMoveToTiltX = { 0.f };
    _float  m_fMoveToTiltY = { 0.f };

    _bool   m_bHomeCaptured = { false };
    _float3 m_vHomePos = {};
    _float  m_fHomeTiltX = { 0.f };
    _float  m_fHomeTiltY = { 0.f };

    enum class EINTRO { NONE, APPEAR, WAIT, DONE };
    EINTRO  m_eIntro = { EINTRO::NONE };
    _float  m_fIntroElapsed = { 0.f };
    _float  m_fIntroAppearDur = { 0.f };
    _float  m_fIntroWaitDur = { 0.f };
    _float  m_fIntroStartScaleMul = { 1.f };
    _float3 m_vIntroBaseScale = { 1.f, 1.f, 1.f };
    function<void()> m_OnIntroFinished = { nullptr };

public:
    static CUIMovableContainer* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CGameObject* Clone(void* pArg) override;

protected:
    virtual void Free() override;
};

NS_END