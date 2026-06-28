#pragma once
#include "GameObject.h"
#include "Effect_Allocator.h"

NS_BEGIN(Engine)

class CEffect_Part;
class CEffect_Manager;

class ENGINE_DLL CEffect_Container abstract : public CGameObject
{
    GENERATED_BODY_ABSTRACT(CEffect_Container)

    // Debug
    PROPERTY(_bool, m_bResetPlayDoubleCheck,    L"ResetPlay_DoubleCheck",   L"Effect_Container");

    PROPERTY(_bool, m_bIsPlay,                  L"IsPlay",                  L"Effect_Container");

    PROPERTY(_bool, m_bLoop,                    L"Loop",                    L"Effect_Container");

    PROPERTY(_float, m_fDuration,               L"Duration",                L"Effect_Container");
    PROPERTY(_float, m_fAccTime,                L"AccTime",                 L"Effect_Container");
    
public:
    struct EFFECT_CONTAINER_DESC : public CGameObject::GAMEOBJECT_DESC
    {
    };

protected:
    CEffect_Container(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CEffect_Container(const CEffect_Container& Prototype);
    virtual ~CEffect_Container() = default;

public:
#pragma push_macro("new")
#undef new
    static void* operator new(size_t n)
    {
        return CEffect_Allocator::IsPrototypePass()
            ? ::operator new(n)                            
            : CEffect_Allocator::GetInstance()->Alloc(n);  
    }
    static void  operator delete(void* p) { CEffect_Allocator::GetInstance()->Dealloc(p); }
#ifdef _DEBUG
    static void* operator new(size_t n, int, const char*, int)
    {
        return CEffect_Allocator::IsPrototypePass()
            ? ::operator new(n)
            : CEffect_Allocator::GetInstance()->Alloc(n);
    }
    static void  operator delete(void* p, int, const char*, int) { CEffect_Allocator::GetInstance()->Dealloc(p); }
#endif
    static void* operator new[](size_t) = delete;
    static void  operator delete[](void*) = delete;
#pragma pop_macro("new")

protected:
    virtual HRESULT Initialize_Prototype() override;
    virtual HRESULT Initialize(void* pArg) override;

public:
    virtual void    Priority_Update(_float fTimeDelta) override;
    virtual void    Update(_float fTimeDelta) override;
    virtual void    Late_Update(_float fTimeDelta) override;
    virtual HRESULT Render() override;

    void EffectContainer_Start(const _float3& vSpawnPos, const _float3& vLookDir = {0.f, 0.f, 0.f}, const _float4x4* pParentMatrix = nullptr);
    void EffectContainer_Stop();
    void Start_FadeOut(_float fFadeOutDuration = 0.3f);

    void Set_ParentMatrix(const _float4x4* pParentMatrix);

    _bool Set_EffectPartPlay(const _wstring& strPartTag, _bool bPlay);
    _bool Is_EffectPartPlay(const _wstring& strPartTag) const;

public:
    const unordered_map<_wstring, CEffect_Part*>& Get_EffectPartObject() const { return m_EffestParts; }

public:
    //윤석현추가
    virtual json Serialize() const override;
    virtual void Deserialize_Internal(const json& j) override;

    void Set_Pool(CEffect_Manager* pPool, _uint iLevel, const _wstring& strEffectKey);

protected:
    HRESULT Add_Effect_PartObject(_uint iPrototypeLevelIndex, const _wstring& strPrototypeTag, const _wstring& strPartTag, void* pArg = nullptr);

    void Compute_CombinedWorldMatrix();

protected:
    unordered_map<_wstring, CEffect_Part*> m_EffestParts;

    const _float4x4* m_pParentMatrix = {};
    _float4x4 m_CombinedWorldMatrix = {};

    CEffect_Manager* m_pPool = { nullptr };   // 약참조(반납처)
    _uint    m_iPoolLevel = {};
    _wstring m_strPoolKey;

    _bool m_bFadeOutRequested{};

private:
    void Update_FadeOut();

    // Debug
    _bool m_bPreResetPlayDoubleCheck{};
    void Debug_ResetPlay();

private:
    void Init_PropetyValue();

protected:
    virtual void Free() override;
};

NS_END