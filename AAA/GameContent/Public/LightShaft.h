#pragma once
#include "GameObject.h"

NS_BEGIN(Engine)
class CShader;
class CModel;
NS_END

NS_BEGIN(Client)
class CLightShaft final : public CGameObject
{
    GENERATED_BODY(CLightShaft)

public:
    static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_LightShaft";

    enum EASE
    {
        EASE_LINEAR,
        EASE_SMOOTH,
    };

    struct LIGHTSHAFT_DESC : public CGameObject::GAMEOBJECT_DESC
    {
        _uint         iModelLevel = { 0 };
        const _tchar* pModelProtoTag = { nullptr };
        _float3       vColor = { 1.f, 0.5f, 0.15f };
        _float3       vScaleMin = { 1.f, 1.f, 1.f };
        _float3       vScaleMax = { 6.f, 6.f, 6.f };
        _float        fIntensity = { 1.f };
        _float        fMaxAlpha = { 0.6f };
        _int          eEase = { EASE_SMOOTH };
    };

private:
    CLightShaft(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CLightShaft(const CLightShaft& Prototype);
    virtual ~CLightShaft() = default;

public:
    virtual HRESULT Initialize_Prototype() override;
    virtual HRESULT Initialize(void* pArg) override;
    virtual void Update(_float fTimeDelta) override;
    virtual void Late_Update(_float fTimeDelta) override;
    virtual HRESULT Render() override;
    virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override
    {
        pOutData->strPrototypeTag = PROTOTYPE_TAG;
    }

    void Configure(const LIGHTSHAFT_DESC& Desc);
    void Set_Progress(_float t);
    void Set_Position(_fvector vPos);
    void Align_Up(_fvector vUpDir);

    static CLightShaft* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CGameObject* Clone(void* pArg) override;

private:
    HRESULT Ready_Components();
    HRESULT Bind_ShaderResources();

private:
    CShader* m_pShaderCom = { nullptr };
    CModel* m_pModelCom = { nullptr };

    _uint m_iModelLevel = { 0 };
    _wstring m_strModelTag;
    _float3 m_vColor = { 1.f, 0.5f, 0.15f };
    _float3 m_vScaleMin = { 1.f, 1.f, 1.f };
    _float3 m_vScaleMax = { 6.f, 6.f, 6.f };
    _float m_fIntensity = { 1.f };
    _float m_fMaxAlpha = { 0.6f };
    _int m_eEase = { EASE_SMOOTH };
    _float m_fAlpha = { 0.f };
    _float3 m_vBaseAnchor = { 0.f, 0.f, 0.f };
    _float3 m_vUpDir = { 0.f, 1.f, 0.f };

protected:
    virtual void Free() override;
};

NS_END
