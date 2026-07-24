#pragma once
#include "UI_CurtainAnimBase.h"

NS_BEGIN(Client)

// 모션 없는 고정 크기 텍스처 커튼 파트.
// 애님 없이 항상 표시(시퀀스를 막지 않음). 크기/위치는 에디터 트랜스폼(직렬화)으로 관리.
class CLIENT_DLL CUI_CurtainStatic final : public CUI_CurtainAnimBase
{
    GENERATED_BODY(CUI_CurtainStatic)

    PROPERTY(_float4, m_vColor, L"Color", L"UI")

public:
    typedef struct tagUICurtainStaticDesc : public CUI_CurtainAnimBase::UI_CURTAINANIM_DESC
    {
        _float4 vColor = { 1.f, 1.f, 1.f, 1.f };
    } UI_CURTAINSTATIC_DESC;

    static constexpr const wchar_t* PROTOTYPE_TAG = L"Proto_UI_CurtainStatic";

protected:
    CUI_CurtainStatic(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CUI_CurtainStatic(const CUI_CurtainStatic& Prototype);
    virtual ~CUI_CurtainStatic() = default;

public:
    virtual HRESULT Initialize(void* pArg) override;
    virtual void    Update(_float fTimeDelta) override;   // 애님 스킵(트랜스폼 안 건드림)
    virtual void    Copy_PrototypeName(ENGINE_OBJECT_DATA* pOut) override { pOut->strPrototypeTag = PROTOTYPE_TAG; }

    // ICurtainPart: 정적 파트 -> 시퀀스 안 막게 즉시 완료 취급 + 항상 표시
    virtual void    Begin_Delayed() override {}
    virtual void    Anim_Stop() override {}
    virtual _bool   Is_Finished() const override { return true; }
    virtual _bool   Is_Loop() const override { return false; }
    virtual void    Reset() override {}

protected:
    virtual _uint   Render_Pass() const override { return 8; }   // CurtainFill (CUI_CurtainTexture와 동일)
    virtual HRESULT Bind_Material(CShader* pShader) override;

public:
    static CUI_CurtainStatic* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CGameObject* Clone(void* pArg) override;
protected:
    virtual void    Free() override;
};
NS_END