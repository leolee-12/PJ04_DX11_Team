#include "Channel.h"
#include "Model.h"
#include "Bone.h"

CChannel::CChannel()
{
}

HRESULT CChannel::Initialize(const CHANNEL_DATA& data, class CModel* pModel)
{
    m_iBoneIndex = pModel->Get_BoneIndex(data.strBoneName);

    m_iNumKeyFrames = (_uint)data.KeyFrames.size();

    for (const auto& src : data.KeyFrames)
    {
        KEYFRAME KeyFrameDesc = {};
        KeyFrameDesc.vScale = src.vScale;
        KeyFrameDesc.vRotation = src.vRotation;
        KeyFrameDesc.vTranslation = src.vTranslation;
        KeyFrameDesc.fTrackPosition = src.fTrackPosition;
		m_KeyFrames.push_back(KeyFrameDesc);
    }

	return S_OK;
}

void CChannel::Update_TransformationMatrix(const vector<CBone*>& Bones, _float fCurrentTrackPosition, _uint* pCurrentKeyIndex)
{
    if (0.f == fCurrentTrackPosition)
        *pCurrentKeyIndex = 0;
    
    KEYFRAME        LastKeyFrameDesc = m_KeyFrames.back();

    _vector         vScale = {};
    _vector         vRotation = {};
    _vector         vTranslation = {};

    if (fCurrentTrackPosition >= LastKeyFrameDesc.fTrackPosition)
    {
        vScale = XMLoadFloat3(&LastKeyFrameDesc.vScale);
        vRotation = XMLoadFloat4(&LastKeyFrameDesc.vRotation);
        vTranslation = XMVectorSetW(XMLoadFloat3(&LastKeyFrameDesc.vTranslation), 1.f);
    }
    else /* 무조건 보간이 필요한 상태. */
    {
        while (fCurrentTrackPosition >= m_KeyFrames[*pCurrentKeyIndex + 1].fTrackPosition)
            ++*pCurrentKeyIndex;        

        _vector vSourScale = XMLoadFloat3(&m_KeyFrames[*pCurrentKeyIndex].vScale);
        _vector vDestScale = XMLoadFloat3(&m_KeyFrames[*pCurrentKeyIndex + 1].vScale);

        _vector vSourRotation = XMLoadFloat4(&m_KeyFrames[*pCurrentKeyIndex].vRotation);
        _vector vDestRotation = XMLoadFloat4(&m_KeyFrames[*pCurrentKeyIndex + 1].vRotation);
        
        _vector vSourTranslation = XMVectorSetW(XMLoadFloat3(&m_KeyFrames[*pCurrentKeyIndex].vTranslation), 1.f);
        _vector vDestTranslation = XMVectorSetW(XMLoadFloat3(&m_KeyFrames[*pCurrentKeyIndex + 1].vTranslation), 1.f);
        
        _float  fRatio = (fCurrentTrackPosition - m_KeyFrames[*pCurrentKeyIndex].fTrackPosition) /
            (m_KeyFrames[*pCurrentKeyIndex + 1].fTrackPosition - m_KeyFrames[*pCurrentKeyIndex].fTrackPosition);


        vScale = XMVectorLerp(vSourScale, vDestScale, fRatio); // (vSourScale + vDestScale) * fRatio;
        vRotation = XMQuaternionSlerp(vSourRotation, vDestRotation, fRatio);
        vTranslation = XMVectorLerp(vSourTranslation, vDestTranslation, fRatio);        
    }

    _matrix     TransformationMatrix = XMMatrixAffineTransformation(vScale, XMVectorSet(0.f, 0.f, 0.f, 1.f), vRotation, vTranslation);

    Bones[m_iBoneIndex]->Set_TransformationMatrix(TransformationMatrix);
}

KEYFRAME CChannel::Compute_TransformationMatrix(_float fCurrentTrackPosition, _uint* pCurrentKeyIndex)
{
    if (0.f == fCurrentTrackPosition)
        *pCurrentKeyIndex = 0;

    KEYFRAME        LastKeyFrameDesc = m_KeyFrames.back();

    _vector         vScale = {};
    _vector         vRotation = {};
    _vector         vTranslation = {};

    if (fCurrentTrackPosition >= LastKeyFrameDesc.fTrackPosition)
    {
        vScale = XMLoadFloat3(&LastKeyFrameDesc.vScale);
        vRotation = XMLoadFloat4(&LastKeyFrameDesc.vRotation);
        vTranslation = XMVectorSetW(XMLoadFloat3(&LastKeyFrameDesc.vTranslation), 1.f);
    }
    else /* 무조건 보간이 필요한 상태. */
    {
        while (fCurrentTrackPosition >= m_KeyFrames[*pCurrentKeyIndex + 1].fTrackPosition)
            ++*pCurrentKeyIndex;

        _vector vSourScale = XMLoadFloat3(&m_KeyFrames[*pCurrentKeyIndex].vScale);
        _vector vDestScale = XMLoadFloat3(&m_KeyFrames[*pCurrentKeyIndex + 1].vScale);

        _vector vSourRotation = XMLoadFloat4(&m_KeyFrames[*pCurrentKeyIndex].vRotation);
        _vector vDestRotation = XMLoadFloat4(&m_KeyFrames[*pCurrentKeyIndex + 1].vRotation);

        _vector vSourTranslation = XMVectorSetW(XMLoadFloat3(&m_KeyFrames[*pCurrentKeyIndex].vTranslation), 1.f);
        _vector vDestTranslation = XMVectorSetW(XMLoadFloat3(&m_KeyFrames[*pCurrentKeyIndex + 1].vTranslation), 1.f);

        _float  fRatio = (fCurrentTrackPosition - m_KeyFrames[*pCurrentKeyIndex].fTrackPosition) /
            (m_KeyFrames[*pCurrentKeyIndex + 1].fTrackPosition - m_KeyFrames[*pCurrentKeyIndex].fTrackPosition);


        vScale = XMVectorLerp(vSourScale, vDestScale, fRatio); // (vSourScale + vDestScale) * fRatio;
        vRotation = XMQuaternionSlerp(vSourRotation, vDestRotation, fRatio);
        vTranslation = XMVectorLerp(vSourTranslation, vDestTranslation, fRatio);
    }

    KEYFRAME tResult = {};
    XMStoreFloat3(&tResult.vScale, vScale);
    XMStoreFloat4(&tResult.vRotation, vRotation);
    XMStoreFloat3(&tResult.vTranslation, vTranslation);

    return tResult;
}

CChannel* CChannel::Create(const CHANNEL_DATA& data, class CModel* pModel)
{
    CChannel* pInstance = new CChannel();

    if (FAILED(pInstance->Initialize(data, pModel)))
    {
        MSG_BOX("Failed to Created : CChannel");
        Safe_Release(pInstance);
    }

    return pInstance;
}


void CChannel::Free()
{
    __super::Free();
}
