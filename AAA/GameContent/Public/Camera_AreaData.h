#pragma once
#include "GameContent_Defines.h"
NS_BEGIN(Client)

struct CAM_RAIL {
    string  srcKey;                 // key in doc["Custom"]; "" = newly added
    _uint   uid = 0;
    _bool   close = false;          // IsClose
    vector<_float3> nodes;          // Node[].Pos (engine coords)
    // restored from binary
    _float  radius = 2.f;           // Radius (attach range)
    _float  bezierCtrlLen = 5.f;    // BezierControlLength
    _float3 centerPos = {};         // CenterPos
    _bool   clockwise = false;      // IsClockwise
    string  kind = "Rail";          // Kind
    string  erpType = "Line";       // ErpType
    string  objectName = "Rail";    // Basic.ObjectName
};

struct CAM_OFFSET {
    _float3 targetOffs = {};
    _float3 snapCenter = {};
    _float3 snapRate = { 1.f,1.f,1.f };
    _float4 snapRot = { 0.f,0.f,0.f,1.f };
    _float3 eyeSnapCenter = {};
    _float3 eyeSnapRate = { 0.f,0.f,0.f };
    _float4 eyeSnapRot = { 0.f,0.f,0.f,1.f };
    _float  dist = 0.f, tilt = 0.f, rotY = 0.f, roll = 0.f, fovY = 0.f;
    // restored
    _float  dofFarDepth = 0.f;      // DofFarDepthOffs
    _float  dofFarRate = 0.f;       // DofFarRateOffs
};

enum class CAM_MODE { NORMAL, FIXED_GAZING };

struct CAM_AREA {
    string   srcKey;                // key in doc["Standard"]; "" = newly added
    CAM_MODE mode = CAM_MODE::NORMAL;
    _float3  center = {}, size = { 20.f,10.f,20.f };
    _float4  rot = { 0.f,0.f,0.f,1.f };
    _int     priority = 0;
    _float   erpIn = 120.f, erpOut = 120.f;
    _bool    useRail = false; _uint railUid = 0;
    _bool    eyeSnap = false, targetSnap = false;
    _bool    usePanLimit = false;
    _float   panCenter = 180.f, panRange = 360.f;
    CAM_OFFSET base, end;
    // restored from binary
    string   shapeKind = "Cube";        // AreaShapeKind (Normal|Cube)
    string   kind = "AreaCamera";       // Kind
    string   objectName = "AreaCamera"; // Basic.ObjectName
    _float   checkMargin = 2.f;         // AreaCheckMargin
    _bool    useCustomMargin = false;   // UseCustomAreaCheckMargin
    _int     reactPreWait = 0;          // ReactPreWaitFrame
    _bool    initActiveForEvent = false;// IsInitActiveForUseEvent
    _bool    useDof = false;            // UseDofParam
    _bool    useEventTrigger = false;   // UseEventTrigger
    _int     railNodeIndex = 0;         // Basic.RailUser.NodeIndex
    // MainComponent-level snaps (separate from base/end offsets)
    _float3  mcSnapCenter = {};         _float4 mcSnapRot = { 0.f,0.f,0.f,1.f };
    _float3  mcEyeSnapCenter = {};      _float4 mcEyeSnapRot = { 0.f,0.f,0.f,1.f };
};

struct CAM_POSE {
    _float3 eye = {};
    _float3 fwd = { 0.f,0.f,1.f };
    _float3 up = { 0.f,1.f,0.f };
    _float  fov = 50.f;
};

class CLIENT_DLL CAreaCameraSolver
{
public:
    _bool    Load(const wstring& path);
    CAM_POSE Solve(_fvector vKirby);

    _int  Cur_AreaIndex() const { return m_curArea; }
    _bool Cur_UseRail()   const { return (m_curArea >= 0) && m_areas[m_curArea].useRail; }
    _bool Cur_Gazing()    const { return (m_curArea >= 0) && m_areas[m_curArea].mode == CAM_MODE::FIXED_GAZING; }

    // editor access
    _bool                   Save(const wstring& path) const;
    vector<CAM_RAIL>& Rails() { return m_rails; }
    vector<CAM_AREA>& Areas() { return m_areas; }
    const vector<CAM_RAIL>& Rails() const { return m_rails; }
    const vector<CAM_AREA>& Areas() const { return m_areas; }
    void                    Clear() {
        m_rails.clear(); m_areas.clear(); m_docText.clear(); m_curArea = -1;
        m_railEngaged = false;
    }

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

    mutable _float3  m_lastRailFwd = { 0.f,0.f,1.f };
    mutable _float3  m_lastRailEyeOffset = {};
    mutable _bool    m_railEngaged = false;

    string           m_docText;     // full binary dump (verbatim), edited fields rewritten on Save
};
NS_END