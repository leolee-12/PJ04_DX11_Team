#include "Camera_Manager.h"

CCamera_Manager::CCamera_Manager()
{
}

HRESULT CCamera_Manager::Initialize(_float fWidth, _float fHeigth)
{
	for (size_t i = 0; i < ETOUI(D3DTS::END); ++i)
	{
		XMStoreFloat4x4(&m_Matrix[i][ETOUI(PROJ_TYPE::PERSPEC)], XMMatrixIdentity());
		XMStoreFloat4x4(&m_InverseMatrix[i], XMMatrixIdentity());	
	}
	
	_float4 vEye = { 0.f, 0.f, -1.f, 1.f };
	_float4 vAt	 = { 0.f, 0.f, 0.f, 1.f };
	_float4 vUp	 = { 0.f, 1.f, 0.f, 0.f };

	XMStoreFloat4x4(&m_Matrix[ETOUI(D3DTS::VIEW)][ETOUI(PROJ_TYPE::ORTHO)], XMMatrixIdentity());
	XMStoreFloat4x4(&m_Matrix[ETOUI(D3DTS::PROJ)][ETOUI(PROJ_TYPE::ORTHO)], XMMatrixOrthographicLH(fWidth, fHeigth, 0.1f, 2.f));
	
	return S_OK;
}

void CCamera_Manager::Update()
{
	for (size_t i = 0; i < ETOUI(D3DTS::END); ++i)
	{
		XMStoreFloat4x4(&m_InverseMatrix[i],
			XMMatrixInverse(nullptr, XMLoadFloat4x4(&m_Matrix[i][ETOUI(PROJ_TYPE::PERSPEC)])));
	}

	memcpy(&m_vCamPosition, &m_InverseMatrix[ETOUI(D3DTS::VIEW)]._41, sizeof m_vCamPosition);
}

CCamera_Manager* CCamera_Manager::Create(_float fWidth, _float fHeigth)
{
	CCamera_Manager* pInstance = new CCamera_Manager;

	if (FAILED(pInstance->Initialize(fWidth, fHeigth)))
	{
		MSG_BOX("Create Failed : CCamera_Manager");
		return nullptr;
	}
	return pInstance;
}

void CCamera_Manager::Free()
{
}
