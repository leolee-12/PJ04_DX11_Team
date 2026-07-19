#pragma once

#include "Effect_Part.h"

#include "Effect_Allocator.h"

NS_BEGIN(Engine)

class CShader;
class CTexture;

class ENGINE_DLL CEffect_NonParticle abstract : public CEffect_Part
{
    GENERATED_BODY_ABSTRACT(CEffect_NonParticle)

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
    PROPERTY(_float, m_fSize_Value_1,       L"Value 1_S",             L"Size");


    // Color
    PROPERTY(_float3, m_vColor,              L"Color     _C",         L"Color");

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
    PROPERTY(_float3, m_vBaseRotationDegree, L"Base Rotation_R", L"Rot");

    PROPERTY(_bool, m_bRandomBaseRotation, L"Random Base Rotation_R", L"Rot");
    PROPERTY(_float3, m_vRandomBaseRotationMin, L"Random Rotation Min_R", L"Rot");
    PROPERTY(_float3, m_vRandomBaseRotationMax, L"Random Rotation Max_R", L"Rot");

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

public:
    struct EFFECT_NONEPARTICLE_DESC : public CEffect_Part::EFFECT_PART_DESC
    {
    };

protected:
    CEffect_NonParticle(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CEffect_NonParticle(const CEffect_NonParticle& Prototype);
    virtual ~CEffect_NonParticle() = default;

public:
    virtual void Effect_Start() override;

    // Color
    void Set_Color(const _float3& vColor) { m_vColor = vColor; }
    void Set_ColorChange(_bool bColorChange) { m_bColorChange = bColorChange; }

protected:
    HRESULT Bind_ShaderValue();

protected:
    virtual void On_Deserialized() override;
    virtual void Update_Core(const _float fTimeDelta, const _float fRatio) override;
    virtual _bool Use_LocalRotationAxis() const { return false; }

    void Update_Alpha(const _float fTimeDelta, const _float fRatio);
    void Update_Size(const _float fTimeDelta, const _float fRatio);
    void Update_Color(const _float fTimeDelta, const _float fRatio);
    void Update_Rot(const _float fTimeDelta, const _float fRatio);
    void Update_Move(const _float fTimeDelta, const _float fRatio);
    void Update_MoveSin(const _float fTimeDelta, const _float fRatio);

protected:
    _float              m_fRoll{};
    _float3             m_vResolvedBaseRotationDegree{};

private:
    void Init_PropertyValue();
    void Resolve_BaseRotation();
};

NS_END
