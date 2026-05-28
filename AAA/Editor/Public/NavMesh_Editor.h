#pragma once
#include "Editor_Defines.h"
#include "Base.h"
#include <stack>

NS_BEGIN(Editor)

// 삼각형 하나 - 시계방향(CW) 정점 3개
struct NAV_TRIANGLE
{
    _float3 vPoints[3];
};

class CNavMesh_Editor final : public CBase
{
private:
    CNavMesh_Editor() {};
    ~CNavMesh_Editor() = default;

public:
    void    OnClick(const _float3& vHitPos);   
    void    Undo();            
    void    Redo();
    void    Clear();
    void    Save(const wstring& strFilePath);
    void    Load(const wstring& strFilePath);

    const vector<NAV_TRIANGLE>& Get_Triangles()     const { return m_Triangles; }
    const vector<_float3>& Get_PendingPoints() const { return m_PendingPoints; }
    _uint   Get_TriangleCount() const { return (_uint)m_Triangles.size(); }
    _uint   Get_PendingCount()  const { return (_uint)m_PendingPoints.size(); }
    float   Get_SnapRadius()    const { return m_fSnapRadius; }
    void    Set_SnapRadius(float f) { m_fSnapRadius = f; }

private:
    vector<NAV_TRIANGLE>    m_Triangles;
    vector<_float3>         m_PendingPoints;   
    float                   m_fSnapRadius = { 0.5f };

    enum class NAV_ACTION { ADD_POINT, COMPLETE_TRIANGLE };

    struct NavAction
    {
        NAV_ACTION   eType;
        _float3      vPoint;     // ADD_POINT 용 (스냅 적용 후 좌표)
        NAV_TRIANGLE tri;        // COMPLETE_TRIANGLE 용 (CW 보정 후 tri)
        _float3      arrPts[3];    // COMPLETE_TRIANGLE 용 (완성 직전 pending 3점 원본)
    };

    stack<NavAction>    m_UndoStack;
    stack<NavAction>    m_RedoStack;

private:
    bool    TrySnap(_float3& vInOutPos);
    void    EnsureCW(NAV_TRIANGLE& tri);

public:
    static CNavMesh_Editor* Create();
    virtual void Free() override;
};

NS_END

