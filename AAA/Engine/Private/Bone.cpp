#include "Bone.h"

CBone::CBone()
{
}

HRESULT CBone::Initialize(const BONE_DATA& data)
{   
	m_strName = data.strName;
	m_iParentIndex = data.iParentIndex;
	m_TransformationMatrix = data.TransformationMatrix;
    m_BindPoseMatrix = data.TransformationMatrix;
    XMStoreFloat4x4(&m_CombinedTransformationMatrix, XMMatrixIdentity());

    return S_OK;
}

void CBone::Update_CombinedTransformMatrices(const vector<class CBone*>& Bones, _fmatrix PreTransformMatrix)
{
    if (-1 == m_iParentIndex)
        XMStoreFloat4x4(&m_CombinedTransformationMatrix, 
            PreTransformMatrix * XMLoadFloat4x4(&m_TransformationMatrix));
    else
        XMStoreFloat4x4(&m_CombinedTransformationMatrix,
            XMLoadFloat4x4(&m_TransformationMatrix) * XMLoadFloat4x4(&Bones[m_iParentIndex]->m_CombinedTransformationMatrix));

}

CBone* CBone::Create(const BONE_DATA& data)
{
    CBone* pInstance = new CBone();

    if (FAILED(pInstance->Initialize(data)))
    {
        MSG_BOX("Failed to Created : CBone");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CBone* CBone::Clone()
{
    return new CBone(*this);
}

void CBone::Free()
{
    __super::Free();
}
