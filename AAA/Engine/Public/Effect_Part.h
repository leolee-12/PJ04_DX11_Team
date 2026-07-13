#pragma once
#include "GameObject.h"
#include "Effect_Allocator.h"

NS_BEGIN(Engine)

class CShader;
class CTexture;

class ENGINE_DLL CEffect_Part abstract : public CGameObject
{
    GENERATED_BODY_ABSTRACT(CEffect_Part)

    // Effect
    PROPERTY(_int, m_iShaderPass,       L"Shader Pass", L"Effect");
    PROPERTY(_int, m_iMirror,           L"Mirror",      L"Effect");
    PROPERTY(_int, m_iDepthIgnore,      L"Depth Ignore", L"Effect");
    PROPERTY(_float, m_fEffectIntensity, L"Effect Intensity_E", L"Effect");
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

    PROPERTY(_int, m_iMaskBlendMode,               L"Blend Mode_M",                 L"Mask Com");
    PROPERTY(_int, m_iMaskChannel,                 L"Channel_M",                    L"Mask Com");
    PROPERTY(_bool, m_bMaskInvert,                 L"Invert_M",                     L"Mask Com");
    PROPERTY(_float, m_fMaskStrength,              L"Strength_M",                   L"Mask Com");
    PROPERTY(_bool, m_bUseMaskUVDistortion,        L"Use UV Distortion_M",          L"Mask Com");
    PROPERTY(_float2, m_vMaskUVDistortionStrength, L"UV Distortion Strength_M",     L"Mask Com");

    // Emissive
    PROPERTY(_float4, m_vEmissiveColor, L"Emissive Color", L"Emissive");
    PROPERTY(_bool, m_bEmissiveChange, L"Emissive Change_EM", L"Emissive");
    PROPERTY(_float4, m_vEmissiveStartValue, L"Start_EM", L"Emissive");
    PROPERTY(_float4, m_vEmissiveEndValue, L"End_EM", L"Emissive");

    PROPERTY(_bool, m_bActive_Emissive_Ratio_1, L"Active Ratio 1_EM", L"Emissive");
    PROPERTY(_float, m_fEmissive_Ratio_1, L"Ratio 1_EM", L"Emissive");
    PROPERTY(_float4, m_vEmissive_Value_1, L"Value 1_EM", L"Emissive");

    PROPERTY(_bool, m_bActive_Emissive_Ratio_2, L"Active Ratio 2_EM", L"Emissive");
    PROPERTY(_float, m_fEmissive_Ratio_2, L"Ratio 2_EM", L"Emissive");
    PROPERTY(_float4, m_vEmissive_Value_2, L"Value 2_EM", L"Emissive");

    // Orbit
    PROPERTY(_bool,   m_bOrbitChange,       L"Orbit Change_O", L"Orbit");
    PROPERTY(_float3, m_vOrbitPivot,        L"Pivot_O",        L"Orbit");
    PROPERTY(_float3, m_vOrbitAxis,         L"Axis_O",         L"Orbit");
    PROPERTY(_float,  m_fOrbitDegree,       L"Degree_O",       L"Orbit");
    PROPERTY(_float,  m_fOrbit_Start_Ratio, L"Start_O",        L"Orbit");
    PROPERTY(_float,  m_fOrbit_End_Ratio,   L"End_O",          L"Orbit");

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
    enum ShaderPass { Default, AlphaBlend, Additive, ShaderPass_End };
    enum Sampler { DEFAULT, MIRROR, SAMPLER_END };
    enum DepthMode { DEPTH_DEFAULT, DEPTH_IGNORE, DEPTH_MODE_END };
    enum MaskBlendMode { MASK_MULTIPLY, MASK_ADD, MASK_SUBTRACT, MASK_REPLACE, MASK_BLEND_END };
    enum MaskChannel { MASK_RGBA, MASK_RED, MASK_GREEN, MASK_BLUE, MASK_ALPHA, MASK_CHANNEL_END };

protected:
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

    struct RATIO_VALUE_FLOAT4
    {
        _float fRatio{};
        _float4 vValue{};
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
    virtual void    On_EffectLoop() {} // 컨테이너가 한 바퀴 루프했다는 사실을 각 Part에 알려주는 공통 가상 함수
    virtual _bool   Stop_Emission() { return false; }
    virtual _bool   Is_EmissionFinished() const { return true; }
    void Start_FadeOut(_float fFadeOutDuration = 0.3f);
    _bool Is_FadingOut() const { return m_bFadeOutActive; }
    _bool Is_FadeOutFinished() const { return m_bFadeOutFinished; }

    void Update_PlayValue(_bool bIsPlay, _bool bLoop, _float fDuration, _float fAccTime);

    _bool Is_EffectPartActive() const { return m_bActive; }

    void Set_ParentMatrix(const _float4x4* pParentMatrix);

    virtual void Set_IsPlay(_bool bPlay) { m_bIsPlay = bPlay; }
    _bool Get_IsPlay() { return m_bIsPlay; }

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
    void Set_LocalPositionFromProperty();
    void Update_Orbit(const _float fRatio);

    _int Resolve_ShaderPass();
    HRESULT Bind_ViewProjectionMatrices();

    static void Evaluate_SpriteFrame(_int iFrameX, _int iFrameY, _float fRatio, _float2& vOutUV, _float2& vOutSize);

    static _float Evaluate_FloatCurve(
        _float fRatio, _float fFixedValue, _bool bChange,
        _float fStartValue, _float fEndValue,
        _bool bActiveRatio0, _float fRatio0, _float fValue0,
        _bool bActiveRatio1, _float fRatio1, _float fValue1,
        _bool bNormalizePoints);

    static _float3 Evaluate_Float3Curve(
        _float fRatio, const _float3& vFixedValue, _bool bChange,
        const _float3& vStartValue, const _float3& vEndValue,
        _bool bActiveRatio0, _float fRatio0, const _float3& vValue0,
        _bool bActiveRatio1, _float fRatio1, const _float3& vValue1,
        _bool bNormalizePoints);

    static _float4 Evaluate_Float4Curve(
        _float fRatio, const _float4& vFixedValue, _bool bChange,
        const _float4& vStartValue, const _float4& vEndValue,
        _bool bActiveRatio0, _float fRatio0, const _float4& vValue0,
        _bool bActiveRatio1, _float fRatio1, const _float4& vValue1,
        _bool bNormalizePoints);

    _float Get_FadeOutAlpha() const { return m_fFadeOutAlpha; }

    virtual void Update_UVScroll(const _float fTimeDelta, const _float fRatio);
    void MoveUVScroll(const _float fRatio, const _bool bUpdate, const _float2 vScrollCount, const _float2 vBaseUV, _float2& vOutUv);

protected:
    HRESULT Bind_ShaderValue();

protected:
    void Update_Value(const _float fTimeDelta);
    
    virtual void Update_Core(const _float fTimeDelta, const _float fRatio);

    _float Get_PartDuration() const;

private:
    HRESULT Ready_Components();
    void Init_PropertyValue();
    void Update_FadeOut(_float fTimeDelta);

private:
    _bool m_bFadeOutActive{};
    _bool m_bFadeOutFinished{};
    _float m_fFadeOutDuration{ 0.3f };
    _float m_fAccFadeOutTime{};
    _float m_fFadeOutAlpha{ 1.f };

protected:
    virtual void Free() override;
};

NS_END
