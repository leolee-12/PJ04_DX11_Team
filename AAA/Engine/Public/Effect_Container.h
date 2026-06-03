#pragma once
#include "GameObject.h"

NS_BEGIN(Engine)

class CEffect_Part;

class ENGINE_DLL CEffect_Container abstract : public CGameObject
{
    GENERATED_BODY_ABSTRACT(CEffect_Container)

    PROPERTY(_bool, m_bIsPlay,   L"IsPlay",  L"Effect_Container");

    PROPERTY(_bool, m_bLoop,     L"Loop",    L"Effect_Container");

    PROPERTY(_float, m_fDuration,   L"Duration",    L"Effect_Container");
    PROPERTY(_float, m_fAccTime,    L"AccTime",     L"Effect_Container");

public:
    struct EFFECT_CONTAINER_DESC : public CGameObject::GAMEOBJECT_DESC
    {
        const _float4x4* pParentMatrix = { nullptr };
    };

protected:
    CEffect_Container(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CEffect_Container(const CEffect_Container& Prototype);
    virtual ~CEffect_Container() = default;

protected:
    virtual HRESULT Initialize_Prototype() override;
    virtual HRESULT Initialize(void* pArg) override;

public:
    virtual void    Priority_Update(_float fTimeDelta) override;
    virtual void    Update(_float fTimeDelta) override;
    virtual void    Late_Update(_float fTimeDelta) override;
    virtual HRESULT Render() override;

    void EffectContainer_Start();

public:
    const unordered_map<_wstring, CEffect_Part*>& Get_EffectPartObject() const
    {
        return m_EffestParts;
    }

public:
    virtual json Serialize() const override;
    virtual void Deserialize(const json& j) override;

protected:
    unordered_map<_wstring, CEffect_Part*> m_EffestParts;

    const _float4x4* m_pParentMatrix{};
    _float4x4 m_CombinedWorldMatrix{};

protected:
    HRESULT Add_Effect_PartObject(_uint iPrototypeLevelIndex, const _wstring& strPrototypeTag, const _wstring& strPartTag, void* pArg = nullptr);

    void Compute_CombinedWorldMatrix();

private:
    void Init_PropetyValue();

protected:
    virtual void Free() override;
};

NS_END