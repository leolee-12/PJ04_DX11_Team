#pragma once

#include "Base.h"

/* assimp 뼈를 표현하는 데이터의 종류 */
/* aiNode, aiBone, aiAnimNode*/

/* 1,뼈의 상태정보를 표현한다. */ 
/* 1_1, 부모의 상태를 포함한.  */


NS_BEGIN(Engine)

class CBone final : public CBase
{
private:
	CBone();
	virtual ~CBone() = default;

public:
	_bool Compare_Name(const string& strBoneName) {
		return strBoneName == m_strName;
	}

	const _float4x4* Get_CombinedTransformationMatrixPtr() const {
		return &m_CombinedTransformationMatrix;
	}

	const _float4x4* Get_BindPoseMatrixPtr() const {
		return &m_BindPoseMatrix;
	}

	void Set_TransformationMatrix(_fmatrix TransformationMatrix) {
		XMStoreFloat4x4(&m_TransformationMatrix, TransformationMatrix);
	}

	

public:
	HRESULT Initialize(const BONE_DATA& data);
	void Update_CombinedTransformMatrices(const vector<class CBone*>& Bones, _fmatrix PreTransformMatrix);

private:
	string				m_strName = {};
	_int				m_iParentIndex = { -1 };
	_float4x4			m_BindPoseMatrix = {};
	_float4x4			m_TransformationMatrix = {};
	_float4x4			m_CombinedTransformationMatrix = {};


public:
	static CBone* Create(const BONE_DATA& data);
	CBone* Clone();
	virtual void Free() override;
};

NS_END