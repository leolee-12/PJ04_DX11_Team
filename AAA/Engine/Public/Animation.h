#pragma once

#include "Base.h"

NS_BEGIN(Engine)

class CChannel;

class CAnimation final : public CBase
{
private:
	CAnimation();
	CAnimation(const CAnimation& Prototype);
	virtual ~CAnimation() = default;

public:
	HRESULT Initialize(const ANIMATION_DATA& data, class CModel* pModel);
	_bool Update_TransformationMatrices(const vector<class CBone*>& Bones, _float fTimeDelta, _bool isLoop);
	void  Compute_BoneKeyFrames(unordered_map<_uint, KEYFRAME>& Out, _float fTimeDelta, _bool isLoop);
	void  Reset_TrackPosition();

public:
	const string& Get_AnimationName() const { return m_strName; }

	_float Get_Progress() const
	{
		return m_fDuration > 0.f ? m_fCurrentTrackPosition / m_fDuration : 1.f;
	}

private:
	string				m_strName = {};

	_float				m_fDuration = {}; /* 현재 애니메이션트랙 총 길이.  */
	_float				m_fTickPerSecond = {}; /* 현재 트랙의 초당 재생 속도. */

	_float				m_fCurrentTrackPosition = {}; /* 현재 재생 위치. */

	_uint					m_iNumChannels = {}; /* 현재 애니메이션의 재생을 위해 상태를 제어해야하는 뼈의 갯수 */
	vector<CChannel*>		m_Channels;
	vector<_uint>			m_CurrentKeyFrameIndices;


public:
	static CAnimation* Create(const ANIMATION_DATA& data, class CModel* pModel);
	CAnimation* Clone();
	virtual void Free() override;
};

NS_END