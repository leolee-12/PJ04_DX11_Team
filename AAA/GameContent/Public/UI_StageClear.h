#pragma once

#include "GameContent_Defines.h"
#include "UI_GenericContainer.h"

NS_BEGIN(Client)

class CLIENT_DLL CUI_StageClear final : public CUI_GenericContainer
{
    GENERATED_BODY(CUI_StageClear)

    PROPERTY(_float, m_fStartScaleMul, L"Intro Start Scale", L"StageClear Intro")
    PROPERTY(_float, m_fAppearDuration, L"Intro Appear Time", L"StageClear Intro")
    PROPERTY(_float, m_fWaitDuration, L"Intro Wait Time", L"StageClear Intro")

public:
    static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_UI_StageClear";

private:
    CUI_StageClear(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CUI_StageClear(const CUI_StageClear& Prototype);
    virtual ~CUI_StageClear() = default;

public:
    virtual HRESULT Initialize_Prototype() override;
    virtual HRESULT Initialize(void* pArg) override;
    virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* pOut) override
    {
        pOut->strPrototypeTag = PROTOTYPE_TAG;
    }

public:
    virtual void On_Deserialized() override;

protected:
    virtual HRESULT Ready_Events() override;
    virtual void    On_IntroFinished() override;

private:
    _bool m_bIntroTriggered = { false };
    CUIPartObject* m_pEffectLines[2] = { nullptr };

public:
    static CUI_StageClear* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CGameObject* Clone(void* pArg) override;

protected:
    virtual void Free() override;
};

NS_END