#pragma once
#include "GameContent_Defines.h"
#include "UIContainerObject.h"

NS_BEGIN(Client)
class ICurtainPart;

// 시퀀스 커튼 컨테이너 공통 베이스.
// 파트(ICurtainPart) 수집 → Play()로 StartDelay 순서 연출 → done 감지 또는 주기 기반 루프.
// 파생은 기본 플래그값과 On_SequenceDone()(완료 신호) 만 다르게.
class CLIENT_DLL CUI_CurtainSequenceBase abstract : public CUIContainerObject
{
    GENERATED_BODY_ABSTRACT(CUI_CurtainSequenceBase)

    PROPERTY(_bool,  m_bStartHidden, L"StartHidden", L"Sequence");
    PROPERTY(_bool,  m_bAutoStart,   L"AutoStart",   L"Sequence");
    PROPERTY(_bool,  m_bLoop,        L"Loop",        L"Sequence");
    PROPERTY(_float, m_fLoopPeriod,  L"LoopPeriod",  L"Sequence"); // >0: 주기 기반 루프, 0: done 기반
    PROPERTY(_wstring, m_strTriggerEvent, L"TriggerEvent", L"Sequence");

protected:
    CUI_CurtainSequenceBase(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CUI_CurtainSequenceBase(const CUI_CurtainSequenceBase& Prototype);
    virtual ~CUI_CurtainSequenceBase() = default;

public:
    virtual HRESULT Initialize_Prototype() override;
    virtual HRESULT Initialize(void* pArg) override;
    virtual void    Priority_Update(_float fTimeDelta) override;
    virtual void    Update(_float fTimeDelta) override;
    virtual void    Late_Update(_float fTimeDelta) override;

public:
    void    Play();
    void    Stop();
    _bool   Is_Done()    const { return m_bDone; }
    _bool   Is_Playing() const { return m_bStarted && !m_bDone; }
    void    Reset_Containor();

    virtual void On_Deserialized() override;

protected:
    // non-loop 파트 전부 종료 시 1회 호출. 기본 no-op. 페이드인/아웃이 신호 발행 override.
    virtual void    On_SequenceDone() {}
    void            Refresh_Subscription();

private:
    void    Collect_Parts();

protected:
    vector<ICurtainPart*>   m_Parts;
    _bool   m_bCollected = { false };
    _bool   m_bStarted = { false };
    _bool   m_bDone = { false };
    _float  m_fLoopAcc = { 0.f };

    SUBHANDLE m_hTrigger{};
    _bool     m_bSubscribed = { false };

protected:
    virtual void    Free() override;
};
NS_END