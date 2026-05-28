#include "Navigation.h"

#include "Cell.h"
#include "GameInstance.h"

const _float4x4* CNavigation::m_pParentMatrixPtr = { nullptr };
_float4x4	 CNavigation::m_IdentityMatrix = {};

CNavigation::CNavigation(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CComponent(pDevice, pContext)
{
}

CNavigation::CNavigation(const CNavigation& Prototype)
    : CComponent(Prototype)
    , m_Cells { Prototype.m_Cells }
#ifdef _DEBUG
    , m_pShader { Prototype.m_pShader }
#endif
{

#ifdef _DEBUG
    Safe_AddRef(m_pShader);
#endif

    for (auto& pCell : m_Cells)
        Safe_AddRef(pCell);
}

HRESULT CNavigation::Initialize_Prototype(const _tchar* pNavigationDataFile)
{
    XMStoreFloat4x4(&m_IdentityMatrix, XMMatrixIdentity());
    m_pParentMatrixPtr = &m_IdentityMatrix;

    _ulong dwByte = {};
    HANDLE hFile = CreateFile(pNavigationDataFile, GENERIC_READ, 0, nullptr,
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (0 == hFile) return E_FAIL;

    // 버텍스 배열 읽기
    uint32_t numVerts = {};
    ReadFile(hFile, &numVerts, sizeof(uint32_t), &dwByte, nullptr);

    vector<_float3> sharedVerts(numVerts);
    ReadFile(hFile, sharedVerts.data(), sizeof(_float3) * numVerts, &dwByte, nullptr);

    // 셀 인덱스 읽기
    uint32_t numCells = {};
    ReadFile(hFile, &numCells, sizeof(uint32_t), &dwByte, nullptr);

    vector<array<_uint, 3>> cellIndices(numCells);

    for (uint32_t i = 0; i < numCells; ++i)
    {
        ReadFile(hFile, cellIndices[i].data(), sizeof(_uint) * 3, &dwByte, nullptr);
        if (0 == dwByte) break;

        // 인덱스로 위치 복원 > CCell 에는 위치만 전달
        _float3 pts[3] = {
            sharedVerts[cellIndices[i][0]],
            sharedVerts[cellIndices[i][1]],
            sharedVerts[cellIndices[i][2]]
        };

        CCell* pCell = CCell::Create(m_pDevice, m_pContext, pts, (_uint)m_Cells.size());
        if (nullptr == pCell) return E_FAIL;

        m_Cells.push_back(pCell);
    }

    CloseHandle(hFile);

    SetUp_Neighbors(cellIndices);

#ifdef _DEBUG
    m_pShader = CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_Cell.hlsl"), VTXPOS::Elements, VTXPOS::iNumElements);
    if (nullptr == m_pShader)
        return E_FAIL;
#endif
    


    return S_OK;
}

HRESULT CNavigation::Initialize(void* pArg)
{
    auto        pDesc = static_cast<NAVIGATION_DESC*>(pArg);

    if (pDesc->pParentMarix != nullptr)
        m_pParentMatrixPtr = pDesc->pParentMarix;

    m_iCurrentCellIndex = -1;
    return S_OK;
}

HRESULT CNavigation::SetUp_Neighbors(const vector<array<_uint, 3>>& cellIndices)
{
    unordered_map<uint64_t, pair<_uint, _uint>> edgeMap;

    for (_uint i = 0; i < (_uint)cellIndices.size(); ++i)
    {
        for (_uint e = 0; e < 3; ++e)
        {
            _uint va = cellIndices[i][e];
            _uint vb = cellIndices[i][(e + 1) % 3];

            // 64비트에 32비트 두개 쑤셔넣기 작은값은 << 32로 32비트 밀어버림 
            uint64_t key = ((uint64_t)min(va, vb) << 32) | (uint64_t)max(va, vb);

            auto it = edgeMap.find(key);
            if (it != edgeMap.end())
            {
                // 공유 엣지 발견 > 양방향 이웃 세팅 후 맵에서 제거
                auto [neighborIdx, neighborEdge] = it->second;
                m_Cells[i]->Set_Neighbor((NAVI_LINE)e, neighborIdx);
                m_Cells[neighborIdx]->Set_Neighbor((NAVI_LINE)neighborEdge, i);
                edgeMap.erase(it);
            }
            else
            {
                edgeMap[key] = { i, e };
            }
        }
    }

    return S_OK;
}

_bool CNavigation::Query_isMove(_fvector vResultPos)
{
    if (-1 == m_iCurrentCellIndex)
        return false;

    _uint iNeighborIndex = { (_uint)-1 };

    if (true == m_Cells[m_iCurrentCellIndex]->Query_isIn(vResultPos, &iNeighborIndex))
        return true;
    else
    {
        if (-1 != iNeighborIndex)
        {
            m_iCurrentCellIndex = iNeighborIndex;
            return true;
        }
        else
        {
            for (_uint i = 0; i < (_uint)m_Cells.size(); ++i)
            {
                _uint iDummy = {};
                if (m_Cells[i]->Query_isIn(vResultPos, &iDummy))
                {
                    m_iCurrentCellIndex = (_int)i;
                    return true;
                }
            }
            return false;
        }
    }
}

void CNavigation::Find_StartCell(_fvector vPos)
{
    for (_uint i = 0; i < (_uint)m_Cells.size(); ++i)
    {
        _uint iDummy = {};
        if (m_Cells[i]->Query_isIn(vPos, &iDummy))
        {
            m_iCurrentCellIndex = (_int)i;
            break;
        }
    }
}

_int CNavigation::Find_CellIndex(_fvector vPos) const
{
    for (_uint i = 0; i < (_uint)m_Cells.size(); ++i)
    {
        _uint iDummy = {};
        if (m_Cells[i]->Query_isIn(vPos, &iDummy))
            return (_int)i;
    }
    return -1;
}

_int CNavigation::Clamp_ToNaviMesh(_fvector vPos, _float3* pOutClamped) const
{
    _int   iBestCell = -1;
    _float fBestDistSq = FLT_MAX;
    _vector vBest = vPos;

    for (_uint i = 0; i < (_uint)m_Cells.size(); ++i)
    {
        for (_uint e = 0; e < ETOUI(NAVI_LINE::END); ++e)
        {
            // 네비매쉬 경계선만 탐색
            if (m_Cells[i]->Get_Neighbor((NAVI_LINE)e) != (_uint)-1)
                continue;

            _vector vA = m_Cells[i]->Get_Point((NAVI_POINT)e);
            _vector vB = m_Cells[i]->Get_Point((NAVI_POINT)((e + 1) % 3));

            _vector vAB = XMVectorSetY(XMVectorSubtract(vB, vA), 0.f);
            _vector vAP = XMVectorSetY(XMVectorSubtract(vPos, vA), 0.f);

            _float fABLenSq = XMVectorGetX(XMVector3LengthSq(vAB));
            if (fABLenSq < FLT_EPSILON) continue;

            _float t = XMVectorGetX(XMVector3Dot(vAP, vAB)) / fABLenSq;
            t = std::clamp(t, 0.f, 1.f);

            _vector vClosest = XMVectorAdd(vA, XMVectorScale(XMVectorSubtract(vB, vA), t));

            _vector vDiff = XMVectorSetY(XMVectorSubtract(vClosest, vPos), 0.f);
            _float fDistSq = XMVectorGetX(XMVector3LengthSq(vDiff));

            if (fDistSq < fBestDistSq)
            {
                fBestDistSq = fDistSq;
                iBestCell = (_int)i;
                vBest = vClosest;
            }
        }
    }

    if (iBestCell < 0)
        return -1;

    _float3 closestF;
    XMStoreFloat3(&closestF, vBest);
    closestF.y = m_Cells[iBestCell]->Compute_Height(vBest);

    // 셀 중앙으로 아주약간 당겨주기
    _vector vCentroid = m_Cells[iBestCell]->Get_Centroid();
    _vector vNudge = XMVectorScale(
        XMVectorSubtract(vCentroid, XMLoadFloat3(&closestF)), 0.001f);
    _vector vFinal = XMVectorAdd(XMLoadFloat3(&closestF), vNudge);

    XMStoreFloat3(pOutClamped, vFinal);
    return iBestCell;
}

vector<_uint> CNavigation::Find_CellPath(_uint iStartIdx, _uint iGoalIdx) const
{
	vector<_uint> emptyPath;

	const size_t numCells = m_Cells.size();
	if (iStartIdx >= numCells || iGoalIdx >= numCells)
		return emptyPath;

	if (iStartIdx == iGoalIdx)
		return { iStartIdx };

	// A* 상태 데이터
	vector<_float> gScore(numCells, FLT_MAX);
	vector<_uint>  parent(numCells, (_uint)-1);
	vector<_char>  closed(numCells, 0);

	// [핵심] 각 셀에 어떤 지점으로 들어왔는지 기록 (Entry Point)
	vector<_float3> entryPoints(numCells);

	using Entry = pair<_float, _uint>;
	priority_queue<Entry, vector<Entry>, greater<Entry>> openSet;

	// 시작/목표 지점 설정 (각 셀의 무게중심을 기준점으로 사용)
	_vector vStart = m_Cells[iStartIdx]->Get_Centroid();
	_vector vGoal = m_Cells[iGoalIdx]->Get_Centroid();

	gScore[iStartIdx] = 0.f;
	XMStoreFloat3(&entryPoints[iStartIdx], vStart);

	// 초기 휴리스틱 계산 (가중치 1.5f 적용)
	_float fStartH = XZDistance(vStart, vGoal);
	openSet.push({ fStartH, iStartIdx });

	while (!openSet.empty())
	{
		_uint current = openSet.top().second;
		openSet.pop();

		if (closed[current]) continue;
		closed[current] = true;

		if (current == iGoalIdx)
			return Reconstruct_Path(parent, iGoalIdx);

		for (_uint e = 0; e < ETOUI(NAVI_LINE::END); ++e)
		{
			_uint nbr = m_Cells[current]->Get_Neighbor((NAVI_LINE)e);
			if (nbr == (_uint)-1 || closed[nbr]) continue;

			_vector vP0 = m_Cells[current]->Get_Point((NAVI_POINT)e);
			_vector vP1 = m_Cells[current]->Get_Point((NAVI_POINT)((e + 1) % 3));

            _vector vBestPointOnEdge = GetClosestPointOnEdgeToIdealLine(vP0, vP1, vStart, vGoal);

            _vector vEntry = XMLoadFloat3(&entryPoints[current]);
			_float  stepCost = XZDistance(vEntry, vBestPointOnEdge);
			_float  tentativeG = gScore[current] + stepCost;

			if (tentativeG < gScore[nbr])
			{
				gScore[nbr] = tentativeG;
				parent[nbr] = current;
                
				XMStoreFloat3(&entryPoints[nbr], vBestPointOnEdge);

				_float h = XZDistance(vBestPointOnEdge, vGoal);
				openSet.push({ tentativeG + h, nbr });
			}
		}
	}

	return emptyPath;
}

vector<_float3> CNavigation::String_Pull(const vector<_uint>& cellPath, _fvector vStart, _fvector vGoal) const
{
    vector<_float3> result;
    if (cellPath.empty()) return result;

    // 시작, 도착 좌표
    _float3 vStartF, vGoalF;
    XMStoreFloat3(&vStartF, vStart);
    XMStoreFloat3(&vGoalF, vGoal);

    // 포탈들 세팅하기 A*가 도출한 셀경로순회
    vector<_float3> lefts, rights;
    for (size_t i = 0; i + 1 < cellPath.size(); ++i)
    {
        _float3 L, R;
        Get_Portal(cellPath[i], cellPath[i + 1], &L, &R);

        constexpr float fInset = 0.05f;
        //_vector vCentroid = m_Cells[cellPath[i]]->Get_Centroid();

        _vector vL = XMLoadFloat3(&L);
        _vector vR = XMLoadFloat3(&R);

        _vector vInsetL = XMVectorLerp(vL, vR, fInset);
        XMStoreFloat3(&L, vInsetL);

        _vector vInsetR = XMVectorLerp(vR, vL, fInset);
        XMStoreFloat3(&R, vInsetR);

        /*_vector vInsetL = XMVectorAdd(vL, XMVectorScale(vCentroid - vL, fInset));
        _vector vInsetR = XMVectorAdd(vR, XMVectorScale(vCentroid - vR, fInset));

        XMStoreFloat3(&L, vInsetL);
        XMStoreFloat3(&R, vInsetR);*/

        lefts.push_back(L);
        rights.push_back(R);
    }
    lefts.push_back(vGoalF);
    rights.push_back(vGoalF);

    // 퍼널 데이터 세팅
    _float3 apex = vStartF;
    _float3 left = lefts[0];
    _float3 right = rights[0];
    _uint apexIdx = 0, leftIdx = 0, rightIdx = 0;

    result.push_back(apex);

    // 3. 포털 순회
    for (_uint i = 1; i < (_uint)lefts.size(); ++i)
    {
        _vector vApex = XMLoadFloat3(&apex);
        _vector vLeft = XMLoadFloat3(&left);
        _vector vRight = XMLoadFloat3(&right);
        _vector vNL = XMLoadFloat3(&lefts[i]);
        _vector vNR = XMLoadFloat3(&rights[i]);

        if (triArea2D(vApex, vRight, vNR) <= 0.f)
        {
            if (XMVector3Equal(vApex, vRight) || triArea2D(vApex, vLeft, vNR) > 0.f)
            {
                right = rights[i];
                rightIdx = i;
            }
            else
            {
                result.push_back(left);
                apex = left;
                apexIdx = leftIdx;
                left = right = apex;
                leftIdx = rightIdx = apexIdx;
                i = apexIdx;
                continue;
            }
        }

        if (triArea2D(vApex, vLeft, vNL) >= 0.f)
        {
            if (XMVector3Equal(vApex, vLeft) || triArea2D(vApex, vRight, vNL) < 0.f)
            {
                left = lefts[i];
                leftIdx = i;
            }
            else
            {
                result.push_back(right);
                apex = right;
                apexIdx = rightIdx;
                left = right = apex;
                leftIdx = rightIdx = apexIdx;
                i = apexIdx;
                continue;
            }
        }
    }

    result.push_back(vGoalF);
    return result;
}

vector<_float3> CNavigation::Find_Path(_fvector vStart, _fvector vGoal) const
{
    _float3 vStartF, vGoalF;
    XMStoreFloat3(&vStartF, vStart);
    XMStoreFloat3(&vGoalF, vGoal);

    // 1. 시작위치가 네비밖이면 클램핑
    _int iStart = Find_CellIndex(vStart);
    if (iStart < 0)
    {
        iStart = Clamp_ToNaviMesh(vStart, &vStartF);
        if (iStart < 0) return {};
    }
    // 골도 마찬가지
    _int iGoal = Find_CellIndex(vGoal);
    if (iGoal < 0)
    {
        iGoal = Clamp_ToNaviMesh(vGoal, &vGoalF);
        if (iGoal < 0) return {};
    }

    // 2. A*
    vector<_uint> cellPath = Find_CellPath((_uint)iStart, (_uint)iGoal);
    if (cellPath.empty()) return {};

#ifdef _DEBUG
    m_LastCellPath = cellPath;          // A*가 잡은 corridor
#endif

    // 3. Funnel - 클램프된 start/goal 기준으로
    _vector vStartClamped = XMLoadFloat3(&vStartF);
    _vector vGoalClamped = XMLoadFloat3(&vGoalF);

    vector<_float3> waypoints = String_Pull(cellPath, vStartClamped, vGoalClamped);
#ifdef _DEBUG
    m_LastWaypoints = waypoints;        // funnel 결과
#endif
    if (waypoints.size() < 2) return waypoints;

    // 4. Y 보정 (프론트랑 백만 하는 이유는 스타트랑 골만 네비메쉬 위가 아닐 수 있기때문에)
    waypoints.front().y = m_Cells[iStart]->Compute_Height(XMLoadFloat3(&waypoints.front()));
    waypoints.back().y = m_Cells[iGoal]->Compute_Height(XMLoadFloat3(&waypoints.back()));

    return waypoints;
}

_bool CNavigation::Compute_SlideVector(_fvector vCurrentPos, _fvector vDesiredMove, _vector* pOutSlideMove) const
{
    if (-1 == m_iCurrentCellIndex) return false;

    CCell* pCell = m_Cells[m_iCurrentCellIndex];

    for (_uint i = 0; i < ETOUI(NAVI_LINE::END); ++i)
    {
        if (pCell->Get_Neighbor((NAVI_LINE)i) != (_uint)-1) continue;

        _float3 vNormal = pCell->Get_Normal((NAVI_LINE)i);
        _vector vN = XMLoadFloat3(&vNormal);
        _float  fDot = XMVectorGetX(XMVector3Dot(vDesiredMove, vN));

        if (fDot > 0.f)
        {
            _vector vSlide = XMVectorSubtract(vDesiredMove, XMVectorScale(vN, fDot));

            _float fDesiredLen = XMVectorGetX(XMVector3Length(vDesiredMove));
            _float fSlideLen = XMVectorGetX(XMVector3Length(vSlide));
            if (fSlideLen > FLT_EPSILON)
            {
                _vector vSlideDir = XMVector3Normalize(vSlide);
                _float fRemaining = fDesiredLen - fSlideLen;
                vSlide = XMVectorAdd(vSlide, XMVectorScale(vSlideDir, fRemaining));
            }

            *pOutSlideMove = vSlide;
            return true;
        }
    }

    return false;
}

_bool CNavigation::Project_OnNaviMesh(_fvector vPos, _float3* pOutProjected) const
{
    _int iCell = Find_CellIndex(vPos);
    if (iCell < 0) return false;

    _float3 result;
    XMStoreFloat3(&result, vPos);
    result.y = m_Cells[iCell]->Compute_Height(vPos);

    *pOutProjected = result;
    return true;
}

#ifdef _DEBUG
HRESULT CNavigation::Render()
{
    _float4x4       WorldMatrix = *m_pParentMatrixPtr;
    _float4         vColor = _float4(0.f, 1.f, 0.f, 1.f);

    m_pShader->Bind_Matrix("g_ViewMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::VIEW, PROJ_TYPE::PERSPEC));
    m_pShader->Bind_Matrix("g_ProjMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::PROJ, PROJ_TYPE::PERSPEC));

    if (-1 == m_iCurrentCellIndex)
    {
        m_pShader->Bind_Matrix("g_WorldMatrix", &WorldMatrix);
        m_pShader->Bind_RawValue("g_vColor", &vColor, sizeof vColor);
        m_pShader->Begin(0);

        for (auto& pCell : m_Cells)
        {
            if (nullptr != pCell)
                pCell->Render();
        }
    }
    else
    {
        WorldMatrix._42 += 0.05f;
        vColor = _float4(1.f, 0.f, 0.f, 1.f);

        m_pShader->Bind_Matrix("g_WorldMatrix", &WorldMatrix);
        m_pShader->Bind_RawValue("g_vColor", &vColor, sizeof vColor);
        m_pShader->Begin(0);

        m_Cells[m_iCurrentCellIndex]->Render();
    }

	_float4 vYellow = _float4(1.f, 1.f, 0.f, 1.f);
	_float4x4 W = *m_pParentMatrixPtr;
	W._42 += 0.03f;   // z-fight 방지

	m_pShader->Bind_Matrix("g_WorldMatrix", &W);
	m_pShader->Bind_RawValue("g_vColor", &vYellow, sizeof vYellow);
	m_pShader->Begin(0);

	for (_uint idx : m_LastCellPath)
		if (idx < m_Cells.size())
			m_Cells[idx]->Render();

    return S_OK;
}
#endif

void CNavigation::Get_Portal(_uint iFromCell, _uint iToCell, _float3* pOutLeft, _float3* pOutRight) const
{
    CCell* pCell = m_Cells[iFromCell];

    for (_uint e = 0; e < ETOUI(NAVI_LINE::END); ++e)
    {
        if (pCell->Get_Neighbor((NAVI_LINE)e) == iToCell)
        {
            XMStoreFloat3(pOutLeft, pCell->Get_Point((NAVI_POINT)e));
            XMStoreFloat3(pOutRight, pCell->Get_Point((NAVI_POINT)((e + 1) % 3)));
            return;
        }
    }
}

CNavigation* CNavigation::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const _tchar* pNavigationDataFile)
{
    CNavigation* pInstance = new CNavigation(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype(pNavigationDataFile)))
    {
        MSG_BOX("Failed to Created : CNavigation");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CComponent* CNavigation::Clone(void* pArg)
{
    CNavigation* pInstance = new CNavigation(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : CNavigation");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CNavigation::Free()
{
    __super::Free();

    for (auto& pCell : m_Cells)
        Safe_Release(pCell);

#ifdef _DEBUG
    Safe_Release(m_pShader);
#endif

}
