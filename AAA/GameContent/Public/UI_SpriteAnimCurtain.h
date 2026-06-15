#pragma once
#include "UI_SpriteAnim.h"
#include "ICurtainPart.h"

NS_BEGIN(Client)

// 커튼 RT용 스프라이트애님(pass 9) + FadeOut 타임라인(StartDelay/렌더스킵/완료).
class CLIENT_DLL CUI_SpriteAnimCurtain final : public CUI_SpriteAnim, public ICurtainPart
{
    GENERATED_BODY(CUI_SpriteAnimCurtain)

    PROPERTY(_float, m_fStartDelay, L"StartDelay", L"Anim");
    PROPERTY(_bool,  m_bDisableOnFinish, L"DisableOnFinish", L"Anim");
    PROPERTY(_bool,  m_bShowWhileWaiting, L"ShowWhileWaiting", L"Anim");

public:
    static constexpr const wchar_t* PROTOTYPE_TAG = L"Proto_UI_SpriteAnim_Curtain";

protected:
    CUI_SpriteAnimCurtain(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CUI_SpriteAnimCurtain(const CUI_SpriteAnimCurtain& Prototype);
    virtual ~CUI_SpriteAnimCurtain() = default;

public:
    virtual void    Update(_float fTimeDelta) override;
    virtual void    Late_Update(_float fTimeDelta) override;
    virtual void    Copy_PrototypeName(ENGINE_OBJECT_DATA* pOut) override { pOut->strPrototypeTag = PROTOTYPE_TAG; }

    virtual void    Begin_Delayed() override { m_bArmed = true; m_fDelayAcc = 0.f; Stop(); }
    virtual void    Anim_Stop()     override { Stop(); }
    virtual _bool   Is_Finished() const override { return CUI_SpriteAnim::Is_Finished(); }
    virtual _bool   Is_Loop()     const override { return CUI_SpriteAnim::Is_Loop(); }
    virtual void    Reset() override 
    {
        m_bIsPlay = false;
        m_bFinished = true;
        m_fDelayAcc = 0.f;
    }

protected:
    virtual _uint   Render_Pass() const override { return 9; }      // CurtainFillAnim

private:
    _float  m_fDelayAcc = { 0.f };
    _bool   m_bArmed = { false };

public:
    static CUI_SpriteAnimCurtain* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CGameObject* Clone(void* pArg) override;
protected:
    virtual void    Free() override;
};
NS_END