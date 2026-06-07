#pragma once
#include "Engine_Defines.h"
#include "Base.h"

NS_BEGIN(Engine)

class CCamera_Manager final : public CBase
{
private:
	CCamera_Manager();
	virtual ~CCamera_Manager() = default;

public:
	HRESULT Initialize(_float fWidth, _float fHeigth);

public:
	const _float4x4* Get_Matrix(D3DTS eState, PROJ_TYPE eType) const
	{
		return &m_Matrix[ETOUI(eState)][ETOUI(eType)];
	}

	const _float4x4* Get_InverseMatrix_Prespec(D3DTS eState) const
	{
		return &m_InverseMatrix[ETOUI(eState)];
	}

	const _float4* Get_CamPosition() const { return &m_vCamPosition; }
	const _float4* Get_CamLook()     const { return &m_vCamLook; } 
	const _float4* Get_CamRight()    const { return &m_vCamRight; }

public:
	void Set_Transform(D3DTS eState, PROJ_TYPE eType, _fmatrix StateMatrix) {
		XMStoreFloat4x4(&m_Matrix[ETOUI(eState)][ETOUI(eType)], StateMatrix);
	}

	void Set_Transform(D3DTS eState, PROJ_TYPE eType, const _float4x4& StateMatrix) {
		m_Matrix[ETOUI(eState)][ETOUI(eType)] = StateMatrix;
	}


public:
	void Update();

private:
	_float4x4			m_Matrix[ETOUI(D3DTS::END)][ETOUI(PROJ_TYPE::END)] = {};
	_float4x4			m_InverseMatrix[ETOUI(D3DTS::END)] = {};
	_float4				m_vCamPosition = {};
	_float4             m_vCamLook = {};  
	_float4             m_vCamRight = {};

public:
	static CCamera_Manager* Create(_float fWidth, _float fHeigth);
	virtual void Free() override;
};

NS_END

