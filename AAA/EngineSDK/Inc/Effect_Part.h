#pragma once
#include "GameObject.h"

NS_BEGIN(Engine)

class CShader;
class CTexture;

class ENGINE_DLL CEffect_Part abstract : public CGameObject
{
    GENERATED_BODY_ABSTRACT(CEffect_Part)

    PROPERTY(_float3, m_vLocalPos,  L"LocalPos",    L"Effect");

    PROPERTY(_bool, m_bIsPlay,      L"Play",        L"Effect");

    PROPERTY(_bool, m_bLoop,        L"Loop",        L"Effect");

    PROPERTY(_float, m_fDuration,   L"Duration",    L"Effect");
    PROPERTY(_float, m_fAccTime,    L"AccTime",     L"Effect");
    PROPERTY(_float, m_fDelayTime,  L"DelayTime",   L"Effect");

    // Alpha
    PROPERTY(_float, m_fAlpha,               L"Alpha",                L"Effect_Alpha");

    PROPERTY(_bool, m_bFadeInOut,            L"FadeInOut",            L"Effect_Alpha");

    PROPERTY(_float, m_fAlphaStartValue,     L"m_fAlphaStartValue",   L"Effect_Alpha");
    PROPERTY(_float, m_fAlphaEndValue,       L"m_fAlphaEndValue",     L"Effect_Alpha");

    PROPERTY(_bool, m_bActive_Alpha_Ratio_0, L"Active_Alpha_Ratio_0", L"Effect_Alpha");
    PROPERTY(_float, m_fAlpha_Ratio_0,       L"Alpha_Ratio_0",        L"Effect_Alpha");
    PROPERTY(_float, m_fAlpha_Value_0,       L"m_fAlpha_Value_0",     L"Effect_Alpha");

    PROPERTY(_bool, m_bActive_Alpha_Ratio_1, L"Active_Alpha_Ratio_1", L"Effect_Alpha");
    PROPERTY(_float, m_fAlpha_Ratio_1,       L"Alpha_Ratio_1",        L"Effect_Alpha");
    PROPERTY(_float, m_fAlpha_Value_1,       L"m_fAlpha_Value_1",     L"Effect_Alpha");

    // Size
    PROPERTY(_float, m_fSize,               L"Size",                L"Effect_Size");

    PROPERTY(_bool, m_bSizeChange,          L"SizeChange",          L"Effect_Size");

    PROPERTY(_float, m_fSizeStartValue,     L"m_fSizeStartValue",   L"Effect_Size");
    PROPERTY(_float, m_fSizeEndValue,       L"m_fSizeEndValue",     L"Effect_Size");

    PROPERTY(_bool, m_bActive_Size_Ratio_0, L"Active_Size_Ratio_0", L"Effect_Size");
    PROPERTY(_float, m_fSize_Ratio_0,       L"Size_Ratio_0",        L"Effect_Size");
    PROPERTY(_float, m_fSize_Value_0,       L"m_fSize_Value_0",     L"Effect_Size");

    PROPERTY(_bool, m_bActive_Size_Ratio_1, L"Active_Size_Ratio_1", L"Effect_Size");
    PROPERTY(_float, m_fSize_Ratio_1,       L"Size_Ratio_1",        L"Effect_Size");
    PROPERTY(_float, m_fSize_Value_1,       L"m_fSize_Value_1",     L"Effect_Size");


    // Color
    PROPERTY(_float3, m_vColor,              L"Color",                  L"Effect_Color");

    PROPERTY(_bool, m_bColorChange,          L"ColorChange",            L"Effect_Color");

    PROPERTY(_float3, m_vColorStartValue,    L"m_vColorStartValue",     L"Effect_Color");
    PROPERTY(_float3, m_vColorEndValue,      L"m_vColorEndValue",       L"Effect_Color");

    PROPERTY(_bool, m_bActive_Color_Ratio_0, L"Active_Color_Ratio_0",   L"Effect_Color");
    PROPERTY(_float, m_fColor_Ratio_0,       L"Color_Ratio_0",          L"Effect_Color");
    PROPERTY(_float3, m_vColor_Value_0,      L"m_vColor_Value_0",       L"Effect_Color");

    PROPERTY(_bool, m_bActive_Color_Ratio_1, L"Active_Color_Ratio_1",   L"Effect_Color");
    PROPERTY(_float, m_fColor_Ratio_1,       L"Color_Ratio_1",          L"Effect_Color");
    PROPERTY(_float3, m_vColor_Value_1,      L"m_vColor_Value_1",       L"Effect_Color");


    // Rot
    PROPERTY(_bool, m_bRotationChange,   L"RotationChange",    L"Effect_Rot");

    PROPERTY(_float3, m_vRotationAxis,   L"m_vRotationAxis",   L"Effect_Rot");
    PROPERTY(_float, m_fRot_Start_Ratio, L"Rot_Start_Ratio",   L"Effect_Rot");
    PROPERTY(_float, m_fRot_End_Ratio,   L"Rot_End_Ratio",     L"Effect_Rot");


    // Move
    PROPERTY(_bool, m_bMoveChange,          L"MoveChange",          L"Effect_Move");
    PROPERTY(_float, m_fMoveSpeed,          L"MoveSpeed",           L"Effect_Move");

    PROPERTY(_float3, m_vMoveDir,           L"MoveDir",             L"Effect_Move");
    PROPERTY(_float, m_fMove_Start_Ratio,   L"Move_Start_Ratio",    L"Effect_Move");
    PROPERTY(_float, m_fMove_End_Ratio,     L"Move_End_Ratio",      L"Effect_Move");

    // Move Sin
    PROPERTY(_bool, m_bMoveSin,                L"MoveSin",                               L"Effect_MoveSin");
    PROPERTY(_float, m_fSinCyclePerDuration,   L"Cycle_Per_Duration(Recommend int)",     L"Effect_MoveSin");
    PROPERTY(_float, m_fAmplitude,             L"Amplitude",                             L"Effect_MoveSin");

public:
    struct EFFECT_PART_DESC : public CGameObject::GAMEOBJECT_DESC
    {
    };

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

public:
    virtual void    Priority_Update(_float fTimeDelta) override;
    virtual void    Update(_float fTimeDelta) override;
    virtual void    Late_Update(_float fTimeDelta) override;
    virtual HRESULT Render() override;

protected:
    CShader* m_pShaderCom{};
    CTexture* m_pTextureCom{};

    _bool m_bActive{ true };

    _float m_fPreOffsetY{};

protected:
    HRESULT Bind_ShaderValue();

    virtual void Update_EffectPart(const _float fTimeDelta, const _float fActiveTime, const _float fRatio);

private:
    vector<RATIO_VALUE> m_AlphaRatioValue;
    vector<RATIO_VALUE> m_SizeRatioValue;
    vector<RATIO_VALUE_FLOAT3> m_ColorRatioValue;

private:
    void Init_PropertyValue();
    void Update_Value(_float fTimeDelta);
    void Update_Alpha(const _float fTimeDelta, const _float fActiveTime, const _float fRatio);
    void Update_Size(const _float fTimeDelta, const _float fActiveTime, const _float fRatio);
    void Update_Color(const _float fTimeDelta, const _float fActiveTime, const _float fRatio);
    void Update_Rot(const _float fTimeDelta, const _float fActiveTime, const _float fRatio);
    void Update_Move(const _float fTimeDelta, const _float fActiveTime, const _float fRatio);
    void Update_MoveSin(const _float fTimeDelta, const _float fActiveTime, const _float fRatio);

protected:
    virtual void Free() override;
};

NS_END