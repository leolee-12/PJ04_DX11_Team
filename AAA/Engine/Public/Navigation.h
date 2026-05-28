#pragma once

#include "Component.h"

NS_BEGIN(Engine)

class CCell;

class ENGINE_DLL CNavigation final : public CComponent
{
public:
	typedef struct tagNavigationDesc
	{
		_float4			 vInitPos = {};
		const _float4x4* pParentMarix = { nullptr };
	}NAVIGATION_DESC;
private:
	CNavigation(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CNavigation(const CNavigation& Prototype);
	virtual ~CNavigation() = default;

public:
	virtual HRESULT Initialize_Prototype(const _tchar* pNavigationDataFile);
	virtual HRESULT Initialize(void* pArg);

public:
	HRESULT SetUp_Neighbors(const vector<array<_uint, 3>>& cellIndices);
	_bool Query_isMove(_fvector vResultPos);
	void  Find_StartCell(_fvector vPos);
	_int  Find_CellIndex(_fvector vPos) const;

	_int Clamp_ToNaviMesh(_fvector vPos, _float3* pOutClamped) const;
	vector<_uint>  Find_CellPath(_uint iStartIdx, _uint iGoalIdx) const;
	vector<_float3> String_Pull(const vector<_uint>& cellPath, _fvector vStart, _fvector vGoal) const;

	vector<_float3> Find_Path(_fvector vStart, _fvector vGoal) const;

	_bool Compute_SlideVector(_fvector vCurrentPos, _fvector vDesiredMove, _vector* pOutSlideMove) const;

	_bool Project_OnNaviMesh(_fvector vPos, _float3* pOutProjected) const;


#ifdef _DEBUG
public:
	virtual HRESULT Render() override;
#endif

private:
	vector<CCell*>			m_Cells;
	_int					m_iCurrentCellIndex = {};

	static const _float4x4*		m_pParentMatrixPtr;
	static _float4x4			m_IdentityMatrix;

#ifdef _DEBUG
	mutable vector<_uint>   m_LastCellPath;
	mutable vector<_float3> m_LastWaypoints;
#endif

private: // A* 계산 헬퍼
	static _float XZDistance(_fvector vA, _fvector vB)
	{
		_vector d = XMVectorSetY(vB - vA, 0.f);
		return XMVectorGetX(XMVector3Length(d));
	}

	static vector<_uint> Reconstruct_Path(const vector<_uint>& parent, _uint iGoal)
	{
		vector<_uint> path;
		_uint node = iGoal;
		while (node != (_uint)-1)
		{
			path.push_back(node);
			node = parent[node];
		}
		std::reverse(path.begin(), path.end());
		return path;
	}

private:
	static _float triArea2D(_fvector a, _fvector b, _fvector c)
	{
		// 2D 외적 (XZ 평면) = (c-a).x * (b-a).z - (c-a).z * (b-a).x
		// Mononen funnel 부호 규약 : 타이트닝 ≤ 0, 찢어짐 > 0
		_fvector ab = XMVectorSubtract(b, a);
		_fvector ac = XMVectorSubtract(c, a);
		_fvector cross = XMVectorSubtract(
			XMVectorMultiply(XMVectorSwizzle<0, 0, 0, 0>(ac), XMVectorSwizzle<2, 2, 2, 2>(ab)),
			XMVectorMultiply(XMVectorSwizzle<2, 2, 2, 2>(ac), XMVectorSwizzle<0, 0, 0, 0>(ab))
		);
		return XMVectorGetX(cross);
	}

	static _vector GetClosestPointOnEdgeToIdealLine(_fvector vP0, _fvector vP1, _fvector vStart, _fvector vGoal)
	{
		// XZ 평면에서만 계산 (Y는 끝에 보간으로 들어감)
		_float ex = XMVectorGetX(vP1) - XMVectorGetX(vP0);
		_float ez = XMVectorGetZ(vP1) - XMVectorGetZ(vP0);

		_float ux = XMVectorGetX(vGoal) - XMVectorGetX(vStart);
		_float uz = XMVectorGetZ(vGoal) - XMVectorGetZ(vStart);

		_float dx = XMVectorGetX(vStart) - XMVectorGetX(vP0);
		_float dz = XMVectorGetZ(vStart) - XMVectorGetZ(vP0);

		// 2D cross: r × u
		_float denom = ex * uz - ez * ux;

		_float t;
		if (fabsf(denom) < 1e-6f)
		{
			// 평행/퇴화: 기존 동작(goal을 엣지에 투영)으로 폴백
			_vector vEdgeDir = vP1 - vP0;
			_vector vToGoal = vGoal - vP0;
			t = XMVectorGetX(XMVector3Dot(vToGoal, vEdgeDir)) /
				XMVectorGetX(XMVector3LengthSq(vEdgeDir));
		}
		else
		{
			// delta × u / r × u
			t = (dx * uz - dz * ux) / denom;
		}

		t = std::clamp(t, 0.f, 1.f);
		return vP0 + (vP1 - vP0) * t;   // Y 자동 보간
	}

	void Get_Portal(_uint iFromCell, _uint iToCell,
		_float3* pOutLeft, _float3* pOutRight) const;

#ifdef _DEBUG
private:
	class CShader* m_pShader = { nullptr };
#endif

public:
	static CNavigation* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const _tchar* pNavigationDataFile);
	virtual CComponent* Clone(void* pArg);
	virtual void Free() override;
};

NS_END