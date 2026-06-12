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

    PROPERTY(_float3, m_vLocalPos,      L"Local Pos",   L"Effect");

    PROPERTY(_float, m_fStartRatio,     L"Start Ratio",  L"Effect");
    PROPERTY(_float, m_fEndRatio,       L"End Ratio",    L"Effect");

    // Alpha
    PROPERTY(_float, m_fAlpha,               L"Alpha_A",              L"Alpha");

    PROPERTY(_bool, m_bFadeInOut,            L"Fade In Out_A",        L"Alpha");

    PROPERTY(_float, m_fAlphaStartValue,     L"Start_A",              L"Alpha");
    PROPERTY(_float, m_fAlphaEndValue,       L"End_A",                L"Alpha");

    PROPERTY(_bool, m_bActive_Alpha_Ratio_0, L"Active Ratio 0_A",     L"Alpha");
    PROPERTY(_float, m_fAlpha_Ratio_0,       L"Ratio 0_A",            L"Alpha");
    PROPERTY(_float, m_fAlpha_Value_0,       L"Value 0_A",            L"Alpha");

    PROPERTY(_bool, m_bActive_Alpha_Ratio_1, L"Active Ratio 1_A",     L"Alpha");
    PROPERTY(_float, m_fAlpha_Ratio_1,       L"Ratio 1_A",            L"Alpha");
    PROPERTY(_float, m_fAlpha_Value_1,       L"Value 1_A",            L"Alpha");

    // Size
    PROPERTY(_float, m_fSize,               L"Size_S",                L"Size");

    PROPERTY(_bool, m_bSizeChange,          L"Size Change_S",          L"Size");

    PROPERTY(_float, m_fSizeStartValue,     L"Start_S",               L"Size");
    PROPERTY(_float, m_fSizeEndValue,       L"End_S",                 L"Size");

    PROPERTY(_bool, m_bActive_Size_Ratio_0, L"Active Ratio 0_S",      L"Size");
    PROPERTY(_float, m_fSize_Ratio_0,       L"Ratio 0_S",             L"Size");
    PROPERTY(_float, m_fSize_Value_0,       L"Value 0_S",             L"Size");

    PROPERTY(_bool, m_bActive_Size_Ratio_1, L"Active Ratio 1_S",      L"Size");
    PROPERTY(_float, m_fSize_Ratio_1,       L"Ratio 1_S",             L"Size");
    PROPERTY(_float, m_fSize_Value_1,       L"Value 1_S",            L"Size");


    // Color
    PROPERTY(_float3, m_vColor,              L"Color     _C",        L"Color");

    PROPERTY(_bool, m_bColorChange,          L"Color Change_C",       L"Color");

    PROPERTY(_float3, m_vColorStartValue,    L"Start_C",              L"Color");
    PROPERTY(_float3, m_vColorEndValue,      L"End_C",                L"Color");

    PROPERTY(_bool, m_bActive_Color_Ratio_0, L"Active Ratio 0_C",     L"Color");
    PROPERTY(_float, m_fColor_Ratio_0,       L"Ratio 0_C",            L"Color");
    PROPERTY(_float3, m_vColor_Value_0,      L"Value 0_C",            L"Color");

    PROPERTY(_bool, m_bActive_Color_Ratio_1, L"Active Ratio 1_C",     L"Color");
    PROPERTY(_float, m_fColor_Ratio_1,       L"Ratio 1_C",            L"Color");
    PROPERTY(_float3, m_vColor_Value_1,      L"Value 1_C",            L"Color");


    // Rot
    PROPERTY(_bool, m_bRotationChange,   L"Rotation Change_R",    L"Rot");

    PROPERTY(_float, m_fRotationDegree,  L"Rotation Degree_R",    L"Rot");

    PROPERTY(_float3, m_vRotationAxis,   L"Rotation Axis_R",      L"Rot");
    PROPERTY(_float, m_fRot_Start_Ratio, L"Start_R",              L"Rot");
    PROPERTY(_float, m_fRot_End_Ratio,   L"End_R",                L"Rot");


    // Move
    PROPERTY(_bool, m_bMoveChange,          L"Move Change_M",     L"Move");
                                                                
    PROPERTY(_float3, m_vMoveDir,           L"Move Dir_M",        L"Move");
    PROPERTY(_float, m_fMoveDistance,       L"Move Distance_M",   L"Move");

    PROPERTY(_float, m_fMove_Start_Ratio,   L"Start_M",           L"Move");
    PROPERTY(_float, m_fMove_End_Ratio,     L"End_M",             L"Move");


    // Move Sin
    PROPERTY(_bool, m_bMoveSin,                L"Move Sin   _MS",                              L"Move Sin");
    PROPERTY(_float, m_fSinCyclePerDuration,   L"Cycle Per Duration(Recommend int)_MS",        L"Move Sin");
    PROPERTY(_float, m_fAmplitude,             L"Amplitude_MS",                                L"Move Sin");


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

private:
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
    CEffect_Part(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CEffect_Part(const CEffect_Part& Prototype);
    virtual ~CEffect_Part() = default;

protected:
    virtual HRESULT Initialize_Prototype() override;
    virtual HRESULT Initialize(void* pArg) override;

    void MoveUVScroll(const _float fRatio, const _bool bUpdate, const _float2 vScrollCount, const _float2 vBaseUV, _float2& vOutUv);

public:
    virtual void    Priority_Update(_float fTimeDelta) override;
    virtual void    Update(_float fTimeDelta) override;
    virtual void    Late_Update(_float fTimeDelta) override;
    virtual HRESULT Render() override;

    virtual void    Effect_Start();
    void Set_ParentMatrix(const _float4x4* pParentMatrix);

    void Update_PlayValue(_bool bIsPlay, _bool bLoop, _float fDuration, _float fAccTime);

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

    _float2 m_vCurTextureUVOffset{};
    _float2 m_vCurMaskUVOffset{};

    const _float4x4* m_pParentMatrix{};
    _float4x4 m_CombinedWorldMatrix{};


    _bool m_bIsPlay{};
    _bool m_bLoop{};
    _float m_fDuration{};
    _float m_fAccTime{};

protected:
    void Compute_CombinedWorldMatrix();

protected:
    HRESULT Bind_ShaderValue();

protected:
    void Update_Value(const _float fTimeDelta);
    
    virtual void Update_Core(const _float fTimeDelta, const _float fRatio);

    void Update_Alpha(const _float fTimeDelta, const _float fRatio);
    void Update_Size(const _float fTimeDelta, const _float fRatio);
    void Update_Color(const _float fTimeDelta, const _float fRatio);
    void Update_Rot(const _float fTimeDelta, const _float fRatio);
    void Update_Move(const _float fTimeDelta, const _float fRatio);
    void Update_MoveSin(const _float fTimeDelta, const _float fRatio);
    virtual void Update_UVScroll(const _float fTimeDelta, const _float fRatio);

    virtual void Update_EffectPart(const _float fTimeDelta, const _float fRatio);

private:
    vector<RATIO_VALUE> m_AlphaRatioValue;
    vector<RATIO_VALUE> m_SizeRatioValue;
    vector<RATIO_VALUE_FLOAT3> m_ColorRatioValue;

private:
    HRESULT Ready_Components();

    void Init_PropertyValue();

protected:
    virtual void Free() override;
};

NS_END