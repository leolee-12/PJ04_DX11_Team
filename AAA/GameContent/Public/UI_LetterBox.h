#pragma once

#include "GameContent_Defines.h"
#include "UI_GenericContainer.h"

NS_BEGIN(Client)

class CLIENT_DLL CUI_LetterBox final : public CUI_GenericContainer
{
    GENERATED_BODY(CUI_LetterBox)

    PROPERTY(_float, m_fSlideDist, L"Slide Distance", L"LetterBox")
    PROPERTY(_float, m_fSlideDur, L"Slide Duration", L"LetterBox")

public:
    static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_UI_LetterBox";

private:
    CUI_LetterBox(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CUI_LetterBox(const CUI_LetterBox& Prototype);
    virtual ~CUI_LetterBox() = default;

public:
    virtual HRESULT Initialize(void* pArg) override;
    virtual void Update(_float fTimeDelta) override;
    virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* pOut) override
    {
        pOut->strPrototypeTag = PROTOTYPE_TAG;
    }

public:
    virtual void On_Deserialized() override;

protected:
    virtual HRESULT Ready_Events() override;

private:
    void Begin_LetterBox();
    void End_LetterBox();

private:
    _float3 m_vBasePos = {};            // 에디터 배치 원위치. End 때 여기로 복구
    _bool   m_bBaseCaptured = { false };

public:
    static CUI_LetterBox* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CGameObject* Clone(void* pArg) override;

protected:
    virtual void Free() override;
};

NS_END