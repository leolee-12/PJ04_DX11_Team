#include "Animation.h"
#include "Channel.h"


CAnimation::CAnimation()
{
}

CAnimation::CAnimation(const CAnimation& Prototype)
    : m_fDuration { Prototype.m_fDuration }
    , m_fTickPerSecond{ Prototype.m_fTickPerSecond }
    , m_fCurrentTrackPosition{ Prototype.m_fCurrentTrackPosition }
    , m_iNumChannels{ Prototype.m_iNumChannels }
    , m_Channels{ Prototype.m_Channels }
    , m_CurrentKeyFrameIndices{ Prototype.m_CurrentKeyFrameIndices }
    , m_strName{ Prototype.m_strName }
    
{	
    for (auto& pChannel : m_Channels)
        Safe_AddRef(pChannel);
}

HRESULT CAnimation::Initialize(const ANIMATION_DATA& data, class CModel* pModel)
{
    m_strName = data.strName;
    m_fDuration = data.fDuration;
    m_fTickPerSecond = data.fTickPerSecond;
    m_iNumChannels = (_uint)data.Channels.size();

    m_CurrentKeyFrameIndices.resize(m_iNumChannels);

    for (const auto& channelData : data.Channels)
    {
        CChannel* pChannel = CChannel::Create(channelData, pModel);
        if (nullptr == pChannel)
            return E_FAIL;
		m_Channels.push_back(pChannel);
    }

	return S_OK;
}

_bool CAnimation::Update_TransformationMatrices(const vector<class CBone*>& Bones, _float fTimeDelta, _bool isLoop, _float fSpeed)
{
    m_fCurrentTrackPosition += m_fTickPerSecond * fSpeed * fTimeDelta;

    _bool isFinished = false;
    if (m_fCurrentTrackPosition >= m_fDuration)
    {
        if (false == isLoop)
        {
            m_fCurrentTrackPosition = m_fDuration;
            isFinished = true;
        }
        else
            m_fCurrentTrackPosition = 0.f;
    }

    _uint       iChannelIndex = {};

    for (auto& pChannel : m_Channels)
    {
        pChannel->Update_TransformationMatrix(Bones, m_fCurrentTrackPosition, &m_CurrentKeyFrameIndices[iChannelIndex++]);
    }


    return isFinished;


}

_bool CAnimation::Compute_BoneKeyFrames(unordered_map<_uint, KEYFRAME>& Out, _float fTimeDelta, _bool isLoop, _float fSpeed)
{
    m_fCurrentTrackPosition += m_fTickPerSecond * fSpeed * fTimeDelta;

    _bool isFinished = false;
    if (m_fCurrentTrackPosition >= m_fDuration)
    {
        if (!isLoop)
        {
            m_fCurrentTrackPosition = m_fDuration;
            isFinished = true;
        }
        else
            m_fCurrentTrackPosition = 0.f;
    }

    _uint iChannelIndex = {};
    for (auto& pChannel : m_Channels)
    {
        KEYFRAME kf = pChannel->Compute_TransformationMatrix(
            m_fCurrentTrackPosition, &m_CurrentKeyFrameIndices[iChannelIndex++]);
        Out[pChannel->Get_BoneIndex()] = kf;
    }

    return isFinished;
}

void CAnimation::Reset_TrackPosition()
{
    m_fCurrentTrackPosition = 0.f;
    fill(m_CurrentKeyFrameIndices.begin(), m_CurrentKeyFrameIndices.end(), 0);
}

_float CAnimation::Advance_Position(_float fTrackPosition, _float fTimeDelta, _bool isLoop, _float fSpeed, _bool& bFinished) const
{
    bFinished = false;
    fTrackPosition += m_fTickPerSecond * fSpeed * fTimeDelta;

    if (fTrackPosition >= m_fDuration)
    {
        if (!isLoop)
        {
            fTrackPosition = m_fDuration;
            bFinished = true;
        }
        else
            fTrackPosition = 0.f;
    }
    return fTrackPosition;
}

void CAnimation::Sample_Pose(unordered_map<_uint, KEYFRAME>& Out, _float fTrackPosition, vector<_uint>& Cursors) const
{
    if (Cursors.size() < m_Channels.size())
        Cursors.resize(m_Channels.size(), 0);

    _uint iChannelIndex = {};
    for (auto& pChannel : m_Channels)
    {
        KEYFRAME kf = pChannel->Compute_TransformationMatrix(fTrackPosition, &Cursors[iChannelIndex++]);
        Out[pChannel->Get_BoneIndex()] = kf;
    }
}

void CAnimation::Get_ChannelBoneIndices(vector<_uint>& Out) const
{
    Out.clear();
    Out.reserve(m_Channels.size());
    for (auto* pChannel : m_Channels)
        if (pChannel) Out.push_back(pChannel->Get_BoneIndex());
}

CAnimation* CAnimation::Create(const ANIMATION_DATA& data, class CModel* pModel)
{
    CAnimation* pInstance = new CAnimation();

    if (FAILED(pInstance->Initialize(data, pModel)))
    {
        MSG_BOX("Failed to Created : CAnimation");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CAnimation* CAnimation::Clone()
{
    return new CAnimation(*this);
}

void CAnimation::Free()
{
	__super::Free();

    for (auto& pChannel : m_Channels)
        Safe_Release(pChannel);

    m_Channels.clear();
}
