#pragma once
#include "CutsceneActor.h"

NS_BEGIN(Client)

class CCutsceneGorilla final : public CCutsceneActor
{
    GENERATED_BODY(CCutsceneGorilla)

public:
    static constexpr const wchar_t* PROTOTYPE_TAG = L"Proto_CutsceneGorilla";

private:
    CCutsceneGorilla(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CCutsceneGorilla(const CCutsceneGorilla& Prototype);
    virtual ~CCutsceneGorilla() = default;

public:
    virtual void    Update(_float fTimeDelta) override;
    virtual HRESULT Render() override;   // 전투 고릴라와 동일한 멀티패스
    virtual void    Copy_PrototypeName(ENGINE_OBJECT_DATA* p) override { p->strPrototypeTag = PROTOTYPE_TAG; }

private:
    enum class EPhase { ReadyWait, Appearing1, Appearing2, Done };

    EPhase m_ePhase = { EPhase::ReadyWait };
    _bool  m_bGrabFired = { false };

    static constexpr const _char* CLIP_READY = "DemoAppearReadyWait";
    static constexpr const _char* CLIP_APPEAR1 = "DemoAppear1";
    static constexpr const _char* CLIP_APPEAR2 = "DemoAppear2";
    static constexpr const _char* GRAB_BONE = "RHaveL";

protected:
    virtual HRESULT Ready_Components() override;
    virtual HRESULT Ready_Events() override;
    virtual void    On_AnimEvent(const ANIM_EVENT& e, ANIM_EVENT_PHASE p) override;

private:
    void Begin_Appear(); 
    void Fire_Grab();    
    void Do_Handoff();

public:
    static CCutsceneGorilla* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CGameObject* Clone(void* pArg) override;
    virtual void Free() override;
};

NS_END