#pragma once
#include "GameContent_Defines.h"
NS_BEGIN(Client)

struct CAM_RAIL {
    _uint           uid = 0;
    _bool           close = false;
    vector<_float3> nodes;          // nodes.front()=start, nodes.back()=end
};

struct CAM_FRAME {
    _float yaw = 0.f;    // deg, 수평 회전
    _float pitch = 20.f;   // deg, +면 위에서 내려봄
    _float distance = 16.f;   // 타깃 뒤로(레일=0이면 눈이 레일 위)
    _float height = 3.f;    // 추가 눈 높이
    _float fov = 50.f;
    _float aimHeight = 4.f;    // 커비 머리 위 조준(화면 하단 프레이밍)
    _float3 gazePoint = { 0.f, 0.f, 0.f };  // Point/Object 응시 좌표(에디터 마커)
    _float  gazeBlend = 1.f;
};

struct CAM_AREA {
    _float3 center = {}, size = { 20.f, 10.f, 20.f };
    _float4 rot = { 0.f, 0.f, 0.f, 1.f };    // OBB 볼륨 방향 전용(카메라엔 영향 X)
    _int    priority = 0;
    _bool   useRail = false;
    _uint   railUid = 0;
    _float3 scrollDead = { 0.f, 0.f, 0.f };  // 월드축 스크롤 데드존 half-extent
    _float  smoothBase = 0.35f;
    CAM_FRAME frame;      // 비레일 프레이밍 / 레일 start
    CAM_FRAME frameEnd;   // 레일 영역 end (레일 진행도 t로 frame→frameEnd 보간)
    _int    gazeMode = 0;     // 0=Kirby, 1=Point, 2=Object
    string  gazeTag;            // Object 모드 런타임 해석 태그
};

struct CAM_POSE {
    _float3 eye = {};
    _float3 fwd = { 0.f, 0.f, 1.f };
    _float3 up = { 0.f, 1.f, 0.f };
    _float  fov = 50.f;
};

class CLIENT_DLL CAreaCameraSolver
{
public:
    _bool    Load(const wstring& path);
    _bool    Save(const wstring& path) const;
    void     Update(_fvector vKirby, _float dt);   // 프레임당 1회
    _float   Cur_T() const { return m_curT; }

    const CAM_POSE& Cur_Pose() const { return m_lastPose; }
    _int     Cur_AreaIndex()   const { return m_curArea; }
    _bool    Cur_UseRail()     const { return (m_curArea >= 0) && m_areas[m_curArea].useRail; }

    vector<CAM_RAIL>& Rails() { return m_rails; }
    vector<CAM_AREA>& Areas() { return m_areas; }
    const vector<CAM_RAIL>& Rails() const { return m_rails; }
    const vector<CAM_AREA>& Areas() const { return m_areas; }
    void Clear() { m_rails.clear(); m_areas.clear(); m_curArea = -1; m_scrollInit = false; }

    void   Set_GazeOverride(_fvector vPos, _bool bValid) {
        XMStoreFloat3(&m_gazeOverride, vPos); 
        m_gazeOverrideValid =bValid;
    }
    _int   Cur_GazeMode() const { return (m_curArea >= 0) ? m_areas[m_curArea].gazeMode : 0; }
    string Cur_GazeTag() const { return (m_curArea >= 0) ? m_areas[m_curArea].gazeTag : string(); }
    _float Cur_SmoothBase() const { return (m_curArea >= 0) ? m_areas[m_curArea].smoothBase : 0.35f; }

private:
    _int    Resolve_Area(_fvector vKirby) const;
    static _bool Point_In_OBB(_fvector p, const CAM_AREA& A);
    const CAM_RAIL* Find_Rail(_uint uid) const;
    static _float3 Eval_Rail(const vector<_float3>& nodes, _float t);
    static _float  Project_OnRail(const vector<_float3>& nodes, _fvector vPos);
    static CAM_FRAME Lerp_Frame(const CAM_FRAME& a, const CAM_FRAME& b, _float t);
    CAM_POSE Solve_Area(const CAM_AREA& A, _fvector vKirby) const;

    static _bool Kirby_Beyond_Rail(const CAM_RAIL& R, _fvector kirby);

private:
    vector<CAM_RAIL> m_rails;
    vector<CAM_AREA> m_areas;
    _int             m_curArea = -1;

    _float3 m_gazeOverride = {};
    _bool   m_gazeOverrideValid = false;

    mutable CAM_POSE m_lastPose;
    mutable _float3  m_scrollPos = {};
    mutable _bool    m_scrollInit = false;
    mutable _float   m_curT = 0.f;
};
NS_END