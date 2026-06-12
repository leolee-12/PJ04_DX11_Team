#pragma once

#include "GameContent_Defines.h"
#include "UIContainerObject.h"

NS_BEGIN(Client)

class CUI_GaugeFill;
class CUI_GaugeBarCom;

class CLIENT_DLL CUI_KirbyStatus final : public CUIContainerObject
{
    GENERATED_BODY(CUI_KirbyStatus)

public:
    static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_UI_KirbyStatus";

private:
    CUI_KirbyStatus(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CUI_KirbyStatus(const CUI_KirbyStatus& Prototype);
    virtual ~CUI_KirbyStatus() = default;

public:
    virtual HRESULT Initialize_Prototype() override;
    virtual HRESULT Initialize(void* pArg) override;
    virtual void Priority_Update(_float fTimeDelta) override;
    virtual void Update(_float fTimeDelta) override;
    virtual void Late_Update(_float fTimeDelta) override;
    virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* pOut) override
    {
        pOut->strPrototypeTag = PROTOTYPE_TAG;
    }

protected:
    virtual HRESULT Ready_Events() override;

private:
    HRESULT Ready_Components();

private:
    CUI_GaugeBarCom* m_pGaugeBar = { nullptr };
    _float m_fDefaultMaxHP = { 100.f };
    _float m_fDefaultCurrHP = { 100.f };

public:
    static CUI_KirbyStatus* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CGameObject* Clone(void* pArg) override;

protected:
    virtual void Free() override;
};

NS_END
