#pragma once
#include "GameContent_Defines.h"
NS_BEGIN(Client)

struct CAM_RAIL {
    _uint           uid = 0;
    _bool           close = false;
    vector<_float3> nodes;
};

// Base/End 공통 오프셋 (진행도 t로 보간)
struct CAM_OFFSET {
    _float3 targetOffs = {};
    _float3 snapCenter = {};
    _float3 snapRate = { 1.f,1.f,1.f };
    _float4 snapRot = { 0.f,0.f,0.f,1.f };
    _float3 eyeSnapCenter = {};
    _float3 eyeSnapRate = { 0.f,0.f,0.f };
    _float4 eyeSnapRot = { 0.f,0.f,0.f,1.f };
    _float  dist = 0.f, tilt = 0.f, rotY = 0.f, roll = 0.f, fovY = 0.f;
};

enum class CAM_MODE { NORMAL, FIXED_GAZING };

struct CAM_AREA {
    CAM_MODE mode = CAM_MODE::NORMAL;
    _float3  center = {}, size = {};
    _float4  rot = { 0.f,0.f,0.f,1.f };
    _int     priority = 0;
    _float   erpIn = 120.f, erpOut = 120.f;
    _bool    useRail = false; _uint railUid = 0;
    _bool    eyeSnap = false, targetSnap = false;
    _bool    usePanLimit = false;
    _float   panCenter = 180.f, panRange = 360.f;
    CAM_OFFSET base, end;
};

// 솔버의 유일한 출력 순수 포즈
struct CAM_POSE {
    _float3 eye = {};
    _float3 fwd = { 0.f,0.f,1.f };
    _float3 up = { 0.f,1.f,0.f };
    _float  fov = 50.f;
};

class CAreaCameraSolver
{
public:
    _bool    Load(const wstring& path);
    CAM_POSE Solve(_fvector vKirby);

    _int  Cur_AreaIndex() const { return m_curArea; }
    _bool Cur_UseRail()   const { return (m_curArea >= 0) && m_areas[m_curArea].useRail; }
    _bool Cur_Gazing()    const { return (m_curArea >= 0) && m_areas[m_curArea].mode == CAM_MODE::FIXED_GAZING; }

private:
    const CAM_RAIL* Find_Rail(_uint uid) const;
    _int    Resolve_Area(_fvector vKirby) const;
    static _bool Point_In_OBB(_fvector p, const CAM_AREA& A);
    _float  Progress_T(const CAM_AREA& A, _fvector vKirby) const;
    static CAM_OFFSET Lerp_Offset(const CAM_AREA& A, _float t);
    static _float3 Eval_Rail(const vector<_float3>& nodes, _float t);
    static _float  Project_OnRail(const vector<_float3>& nodes, _fvector vPos);
    CAM_POSE Solve_Normal(const CAM_AREA&, const CAM_OFFSET&, _fvector, const CAM_RAIL*, _float t) const;
    CAM_POSE Solve_FixedGazing(const CAM_AREA&, const CAM_OFFSET&, _fvector, const CAM_RAIL*, _float t) const;

private:
    vector<CAM_RAIL> m_rails;
    vector<CAM_AREA> m_areas;
    _int             m_curArea = -1;
};
NS_END