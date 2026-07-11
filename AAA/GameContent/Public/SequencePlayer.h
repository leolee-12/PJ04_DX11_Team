#pragma once
#include "GameContent_Defines.h"
#include "Base.h"

NS_BEGIN(Engine)
class CGameObject;
class CGameInstance_Proxy;
NS_END

NS_BEGIN(Client)

// 연출 스텝 리스트(JSON)를 순차 실행하는 인터프리터.
// 소유자(어레인저)가 배우/앵커를 바인딩하고 매 프레임 Update로 펌프한다.
class CSequencePlayer final : public CBase
{
public:
    enum class ESTEP { WAIT, EVENT, WARP, ANIM, SAY };

    struct SEQUENCE_STEP
    {
        ESTEP   eType = { ESTEP::WAIT };
        _float  fDuration = { 0.f };     // wait
        _wstring strTag;                 // event: 발행할 이벤트 태그
        _wstring strActor;               // warp / anim
        _wstring strAnchor;              // warp
        _wstring strClip;                // anim (3단계에서 구현)
        _wstring strTextId;              // say (4단계에서 구현)
        _bool    bWait = { true };        // anim/say: 완료까지 대기 여부
        _bool    bLoop = { false };
    };

private:
    CSequencePlayer(CGameInstance_Proxy* pProxy);
    virtual ~CSequencePlayer() = default;

public:
    HRESULT Load(const _wstring& strFilePath);

    void Bind_Actor(const _wstring& strName, CGameObject* pActor);
    void Bind_Anchor(const _wstring& strName, _fmatrix AnchorWorld);

    void Play();
    void Update(_float fTimeDelta);

    _bool Is_Playing() const { return m_bPlaying; }
    _bool Is_Finished() const { return m_bFinished; }

private:
    void Enter_Step(const SEQUENCE_STEP& tStep);     // 스텝 진입 시 1회 실행
    _bool Is_StepDone(const SEQUENCE_STEP& tStep);   // 대기 조건 판정
    void Advance();

    void Execute_Warp(const SEQUENCE_STEP& tStep);
    void Execute_Anim(const SEQUENCE_STEP& tStep);

private:
    CGameInstance_Proxy* m_pGameInstance_Proxy = { nullptr };   // 소유자보다 오래 살지 않으므로 참조만

    vector<SEQUENCE_STEP> m_Steps;
    unordered_map<_wstring, CGameObject*> m_Actors;
    unordered_map<_wstring, _float4x4>    m_Anchors;

    size_t m_iCurStep = { 0 };
    _float m_fStepTime = { 0.f };
    _bool  m_bPlaying = { false };
    _bool  m_bFinished = { false };

public:
    static CSequencePlayer* Create(CGameInstance_Proxy* pProxy);
    virtual void Free() override;
};

NS_END