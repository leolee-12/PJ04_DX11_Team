#pragma once

#include "Base.h"

NS_BEGIN(Engine)

class CChannel final : public CBase
{
private:
	CChannel();
	virtual ~CChannel() = default;

public:
	HRESULT Initialize(const CHANNEL_DATA& data, class CModel* pModel);
	void Update_TransformationMatrix(const vector<class CBone*>& Bones, _float fCurrentTrackPosition, _uint* pCurrentKeyIndex);
	KEYFRAME Compute_TransformationMatrix(_float fCurrentTrackPosition, _uint* pCurrentKeyIndex);

public:
	_uint Get_BoneIndex() const { return m_iBoneIndex; }

private:
	_uint				m_iBoneIndex = {};
	_uint				m_iNumKeyFrames = {};
	vector<KEYFRAME>	m_KeyFrames;
	

public:
	static CChannel* Create(const CHANNEL_DATA& data, class CModel* pModel);
	virtual void Free() override;
};

NS_END