#pragma once
#include "GameObject.h"
#include "Effect_Allocator.h"

NS_BEGIN(Engine)

class CShader;
class CTexture;

class ENGINE_DLL CEffect_Part abstract : public CGameObject
{
    GENERATED_BODY_ABSTRACT(CEffect_Part)

    PROPERTY(_int, m_iShaderPass,       L"Shader Pass", L"Effect");
    PROPERTY(_int, m_iMirror,           L"Mirror",      L"Effect");

    PROPERTY(_float3, m_vLocalPos,      L"Local Pos",   L"Effect");

    PROPERTY(_float, m_fStartRatio,     L"Start Ratio",  L"Effect");
    PROPERTY(_float, m_fEndRatio,       L"End Ratio",    L"Effect");


    // Texture
    PROPERTY(_bool, m_bUseTextureCom,           L"Use TextureCom_T",          L"Texture Com");
    PROPERTY(_float2, m_vTextureTiling,         L"Tiling_T",                  L"Texture Com");
    PROPERTY(_float2, m_vTextureOffset,         L"Offset_T",                  L"Texture Com");

    PROPERTY(_bool, m_bTextureUVScroll,         L"UV Scroll     _T",          L"Texture Com");
    PROPERTY(_float2, m_vTextureUVScrollCount,  L"UV Scroll Count_T",         L"Texture Com");

    // Mask
    PROPERTY(_bool, m_bUseMaskCom,          L"Use MaskCom_M",             L"Mask Com");
    PROPERTY(_float2, m_vMaskTiling,        L"Tiling_M",                  L"Mask Com");
    PROPERTY(_float2, m_vMaskOffset,        L"Offset_M",                  L"Mask Com");

    PROPERTY(_bool, m_bMaskUVScroll,        L"UV Scroll     _M",          L"Mask Com");
    PROPERTY(_float2, m_vMaskUVScrollCount, L"UV Scroll Count_M",         L"Mask Com");

public:
    struct EFFECT_PART_DESC : public CGameObject::GAMEOBJECT_DESC
    {
        _bool bCustomShader{};

        // TextureCom관련
        _bool bUseTextureCom{};
        _uint iTextureLevel{};
        _wstring wstrTextureTag;

        // MaskCom관련
        _bool bUseMaskCom{};
        _uint iMaskLevel{};
        _wstring wstrMaskTag;
    };

protected:
    enum Sampler { DEFAULT, MIRROR, SAMPLER_END };

private:
    struct RATIO_VALUE
    {
        _float fRatio{};
        _float fValue{};
    };

    struct RATIO_VALUE_FLOAT3
    {
        _float fRatio{};
        _float3 vValue{};
    };

#pragma region NEW
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
#pragma endregion

protected:
    CEffect_Part(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CEffect_Part(const CEffect_Part& Prototype);
    virtual ~CEffect_Part() = default;

protected:
    virtual HRESULT Initialize_Prototype() override;
    virtual HRESULT Initialize(void* pArg) override;

public:
    virtual void    Priority_Update(_float fTimeDelta) override;
    virtual void    Update(_float fTimeDelta) override;
    virtual void    Late_Update(_float fTimeDelta) override;
    virtual HRESULT Render() override;

    virtual void    Effect_Start();
    void Update_PlayValue(_bool bIsPlay, _bool bLoop, _float fDuration, _float fAccTime);

    void Set_ParentMatrix(const _float4x4* pParentMatrix);

protected:
    _bool m_bCustomShader{};
    CShader* m_pShaderCom{};
    CTexture* m_pTextureCom{};
    CTexture* m_pMaskCom{};

    // Texture
    _uint m_iTextureLevel{};
    _wstring m_wstrTextureTag;

    // Mask
    _uint m_iMaskLevel{};
    _wstring m_wstrMaskTag;

protected:
    _bool m_bActive{ true };

    _bool m_bIsPlay{};
    _bool m_bLoop{};
    _float m_fDuration{};
    _float m_fAccTime{};

    _float2 m_vCurTextureUVOffset{};
    _float2 m_vCurMaskUVOffset{};

    const _float4x4* m_pParentMatrix{};
    _float4x4 m_CombinedWorldMatrix{};

protected:
    void Compute_CombinedWorldMatrix();

    virtual void Update_UVScroll(const _float fTimeDelta, const _float fRatio);
    void MoveUVScroll(const _float fRatio, const _bool bUpdate, const _float2 vScrollCount, const _float2 vBaseUV, _float2& vOutUv);

protected:
    HRESULT Bind_ShaderValue();

protected:
    void Update_Value(const _float fTimeDelta);
    
    virtual void Update_Core(const _float fTimeDelta, const _float fRatio);
    virtual void Update_EffectPart(const _float fTimeDelta, const _float fRatio);

private:
    HRESULT Ready_Components();
    void Init_PropertyValue();

protected:
    virtual void Free() override;
};

NS_END