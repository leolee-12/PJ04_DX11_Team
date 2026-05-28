#include "NavMesh_Editor.h"
#include <fstream>

NS_BEGIN(Editor)

void CNavMesh_Editor::OnClick(const _float3& vHitPos)
{
    _float3 vPos = vHitPos;
    TrySnap(vPos);

    // 새 액션 발생 → redo 스택 초기화
    while (!m_RedoStack.empty()) m_RedoStack.pop();

    if (m_PendingPoints.size() < 2)
    {
        // 점 하나 추가
        m_PendingPoints.push_back(vPos);

        NavAction action;
        action.eType = NAV_ACTION::ADD_POINT;
        action.vPoint = vPos;
        m_UndoStack.push(action);
    }
    else
    {
        // 세 번째 점 → 삼각형 완성
        NavAction action;
        action.eType = NAV_ACTION::COMPLETE_TRIANGLE;
        action.arrPts[0] = m_PendingPoints[0];
        action.arrPts[1] = m_PendingPoints[1];
        action.arrPts[2] = vPos;

        m_PendingPoints.push_back(vPos);

        NAV_TRIANGLE tri;
        tri.vPoints[0] = m_PendingPoints[0];
        tri.vPoints[1] = m_PendingPoints[1];
        tri.vPoints[2] = m_PendingPoints[2];
        EnsureCW(tri);

        action.tri = tri;

        m_Triangles.push_back(tri);
        m_PendingPoints.clear();

        m_UndoStack.push(action);
    }
}

void CNavMesh_Editor::Undo()
{
    if (m_UndoStack.empty()) return;

    NavAction action = m_UndoStack.top();
    m_UndoStack.pop();

    if (action.eType == NAV_ACTION::ADD_POINT)
    {
        // pending 마지막 점 제거
        if (!m_PendingPoints.empty())
            m_PendingPoints.pop_back();
    }
    else // COMPLETE_TRIANGLE
    {
        // 삼각형 제거 후 pending 3점 복원 (원본 순서)
        if (!m_Triangles.empty())
            m_Triangles.pop_back();

        m_PendingPoints.clear();
        m_PendingPoints.push_back(action.arrPts[0]);
        m_PendingPoints.push_back(action.arrPts[1]);
        m_PendingPoints.push_back(action.arrPts[2]);
        m_PendingPoints.pop_back(); // 세 번째 점은 undo 대상이므로 제거
    }

    m_RedoStack.push(action);
}

void CNavMesh_Editor::Redo()
{
    if (m_RedoStack.empty()) return;

    NavAction action = m_RedoStack.top();
    m_RedoStack.pop();

    if (action.eType == NAV_ACTION::ADD_POINT)
    {
        m_PendingPoints.push_back(action.vPoint);
    }
    else // COMPLETE_TRIANGLE
    {
        m_PendingPoints.clear();
        m_Triangles.push_back(action.tri);
    }

    m_UndoStack.push(action);
}

void CNavMesh_Editor::Clear()
{
    m_Triangles.clear();
    m_PendingPoints.clear();
}

bool CNavMesh_Editor::TrySnap(_float3& vInOutPos)
{
    float fRadSq = m_fSnapRadius * m_fSnapRadius;

    // 완성된 삼각형 버텍스 검사
    for (auto& tri : m_Triangles)
        for (int i = 0; i < 3; ++i)
        {
            XMVECTOR diff = XMVectorSubtract(XMLoadFloat3(&tri.vPoints[i]),
                XMLoadFloat3(&vInOutPos));
            float distSq = XMVectorGetX(XMVector3Dot(diff, diff));
            if (distSq <= fRadSq)
            {
                vInOutPos = tri.vPoints[i];
                return true;
            }
        }

    // 현재 pending 점도 검사
    for (auto& pt : m_PendingPoints)
    {
        XMVECTOR diff = XMVectorSubtract(XMLoadFloat3(&pt),
            XMLoadFloat3(&vInOutPos));
        float distSq = XMVectorGetX(XMVector3Dot(diff, diff));
        if (distSq <= fRadSq)
        {
            vInOutPos = pt;
            return true;
        }
    }

    return false;
}

void CNavMesh_Editor::EnsureCW(NAV_TRIANGLE& tri)
{
    // cross(v1-v0, v2-v0).y > 0 이면 CW (DX11 left-handed Y-up)
    // 음수면 CCW -> v1, v2 교환
    XMVECTOR v0 = XMLoadFloat3(&tri.vPoints[0]);
    XMVECTOR v1 = XMLoadFloat3(&tri.vPoints[1]);
    XMVECTOR v2 = XMLoadFloat3(&tri.vPoints[2]);

    XMVECTOR normal = XMVector3Cross(
        XMVectorSubtract(v1, v0),
        XMVectorSubtract(v2, v0)
    );

    if (XMVectorGetY(normal) < 0.f)
        swap(tri.vPoints[1], tri.vPoints[2]);
}

void CNavMesh_Editor::Save(const wstring& strFilePath)
{
    ofstream ofs(strFilePath, ios::binary);
    if (!ofs) return;

    _uint iCount = (_uint)m_Triangles.size();
    ofs.write(reinterpret_cast<const char*>(&iCount), sizeof(_uint));
    for (auto& tri : m_Triangles)
        ofs.write(reinterpret_cast<const char*>(tri.vPoints), sizeof(_float3) * 3);
}

void CNavMesh_Editor::Load(const wstring& strFilePath)
{
    ifstream ifs(strFilePath, ios::binary);
    if (!ifs) return;

    m_Triangles.clear();
    m_PendingPoints.clear();

    _uint iCount = 0;
    ifs.read(reinterpret_cast<char*>(&iCount), sizeof(_uint));
    m_Triangles.resize(iCount);
    for (auto& tri : m_Triangles)
        ifs.read(reinterpret_cast<char*>(tri.vPoints), sizeof(_float3) * 3);
}

CNavMesh_Editor* CNavMesh_Editor::Create()
{
    return new CNavMesh_Editor();
}

void CNavMesh_Editor::Free()
{
    __super::Free();
}

NS_END