#pragma once
#include "GameObject.h"

NS_BEGIN(Engine)

class CShader;
class CTexture;

class ENGINE_DLL CEffect_Part abstract : public CGameObject
{
    GENERATED_BODY_ABSTRACT(CEffect_Part)

    PROPERTY(_int, m_iShdaerPass,       L"Shader Pass", L"Effect");

    PROPERTY(_float3, m_vLocalPos,      L"Local Pos",   L"Effect");

    PROPERTY(_bool, m_bIsPlay,          L"Play",        L"Effect");

    PROPERTY(_bool, m_bLoop,            L"Loop",        L"Effect");

    PROPERTY(_float, m_fDuration,       L"Duration",    L"Effect");
    PROPERTY(_float, m_fAccTime,        L"AccTime",     L"Effect");

    PROPERTY(_float, m_fStartRatio,     L"StartRatio",  L"Effect");
    PROPERTY(_float, m_fEndRatio,       L"EndRatio",    L"Effect");

    // Alpha
    PROPERTY(_float, m_fAlpha,               L"Alpha",              L"Alpha");

    PROPERTY(_bool, m_bFadeInOut,            L"Fade In Out",        L"Alpha");

    PROPERTY(_float, m_fAlphaStartValue,     L"Start",              L"Alpha");
    PROPERTY(_float, m_fAlphaEndValue,       L"End",                L"Alpha");

    PROPERTY(_bool, m_bActive_Alpha_Ratio_0, L"Active Ratio 0",     L"Alpha");
    PROPERTY(_float, m_fAlpha_Ratio_0,       L"Ratio 0",            L"Alpha");
    PROPERTY(_float, m_fAlpha_Value_0,       L"Value 0",            L"Alpha");

    PROPERTY(_bool, m_bActive_Alpha_Ratio_1, L"Active Ratio 1",     L"Alpha");
    PROPERTY(_float, m_fAlpha_Ratio_1,       L"Ratio 1",            L"Alpha");
    PROPERTY(_float, m_fAlpha_Value_1,       L"Value 1",            L"Alpha");

    // Size
    PROPERTY(_float, m_fSize,               L"Size",                L"Size");

    PROPERTY(_bool, m_bSizeChange,          L"SizeChange",          L"Size");

    PROPERTY(_float, m_fSizeStartValue,     L"Start",               L"Size");
    PROPERTY(_float, m_fSizeEndValue,       L"End",                 L"Size");

    PROPERTY(_bool, m_bActive_Size_Ratio_0, L"Active Ratio 0",      L"Size");
    PROPERTY(_float, m_fSize_Ratio_0,       L"Ratio 0",             L"Size");
    PROPERTY(_float, m_fSize_Value_0,       L"Value 0",             L"Size");

    PROPERTY(_bool, m_bActive_Size_Ratio_1, L"Active Ratio 1",      L"Size");
    PROPERTY(_float, m_fSize_Ratio_1,       L"Ratio 1",             L"Size");
    PROPERTY(_float, m_fSize_Value_1,       L"Value 1 ",            L"Size");


    // Color
    PROPERTY(_float3, m_vColor,              L"Color     _",        L"Color");

    PROPERTY(_bool, m_bColorChange,          L"Color Change",       L"Color");

    PROPERTY(_float3, m_vColorStartValue,    L"Start",              L"Color");
    PROPERTY(_float3, m_vColorEndValue,      L"End",                L"Color");

    PROPERTY(_bool, m_bActive_Color_Ratio_0, L"Active Ratio 0",     L"Color");
    PROPERTY(_float, m_fColor_Ratio_0,       L"Ratio 0",            L"Color");
    PROPERTY(_float3, m_vColor_Value_0,      L"Value 0",            L"Color");

    PROPERTY(_bool, m_bActive_Color_Ratio_1, L"Active Ratio 1",     L"Color");
    PROPERTY(_float, m_fColor_Ratio_1,       L"Ratio 1",            L"Color");
    PROPERTY(_float3, m_vColor_Value_1,      L"Value 1",            L"Color");


    // Rot
    PROPERTY(_bool, m_bRotationChange,   L"Rotation Change",    L"Rot");

    PROPERTY(_float, m_fRotationDegree,  L"Rotation Degree",    L"Rot");

    PROPERTY(_float3, m_vRotationAxis,   L"Rotation Axis",      L"Rot");
    PROPERTY(_float, m_fRot_Start_Ratio, L"Start",              L"Rot");
    PROPERTY(_float, m_fRot_End_Ratio,   L"End",                L"Rot");


    // Move
    PROPERTY(_bool, m_bMoveChange,          L"Move Change",     L"Move");
                                                                
    PROPERTY(_float3, m_vMoveDir,           L"Move Dir",        L"Move");
    PROPERTY(_float, m_fMoveDistance,       L"Move Distance",   L"Move");

    PROPERTY(_float, m_fMove_Start_Ratio,   L"Start",           L"Move");
    PROPERTY(_float, m_fMove_End_Ratio,     L"End",             L"Move");


    // Move Sin
    PROPERTY(_bool, m_bMoveSin,                L"Move Sin   _",                             L"Move Sin");
    PROPERTY(_float, m_fSinCyclePerDuration,   L"Cycle Per Duration(Recommend int)",        L"Move Sin");
    PROPERTY(_float, m_fAmplitude,             L"Amplitude",                                L"Move Sin");


    // Texture
    PROPERTY(_bool, m_bUseTextureCom,           L"Use TextureCom",          L"Texture Com");
    PROPERTY(_float2, m_vTextureTiling,         L"Tiling",                  L"Texture Com");
    PROPERTY(_float2, m_vTextureOffset,         L"Offset",                  L"Texture Com");

    PROPERTY(_bool, m_bTextureUVScroll,         L"UV Scroll     _",         L"Texture Com");
    PROPERTY(_float2, m_vTextureUVScrollCount,  L"UV Scroll Count",         L"Texture Com");

    // Mask
    PROPERTY(_bool, m_bUseMaskCom,          L"Use MaskCom",             L"Mask Com");
    PROPERTY(_float2, m_vMaskTiling,        L"Tiling",                  L"Mask Com");
    PROPERTY(_float2, m_vMaskOffset,        L"Offset",                  L"Mask Com");

    PROPERTY(_bool, m_bMaskUVScroll,        L"UV Scroll     _",         L"Mask Com");
    PROPERTY(_float2, m_vMaskUVScrollCount, L"UV Scroll Count",         L"Mask Com");

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

protected:
    void Compute_CombinedWorldMatrix();

protected:
    HRESULT Bind_ShaderValue();

protected:
    void Update_Value(const _float fTimeDelta);

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