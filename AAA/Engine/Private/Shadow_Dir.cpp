#include "Shadow_Dir.h"
#include "GameInstance.h"

CShadow_Dir::CShadow_Dir()
    : m_pGameInstance_Proxy{ CGameInstance::GetProxy() }
{
}

HRESULT CShadow_Dir::Add_ShadowLight(const SHADOW_LIGHT_DESC& ShadowDesc)
{
    XMStoreFloat4x4(&m_TransformStateMatrices[ETOUI(D3DTS::VIEW)],
        XMMatrixLookAtLH(XMLoadFloat4(&ShadowDesc.vEye), XMLoadFloat4(&ShadowDesc.vAt), XMVectorSet(0.f, 1.f, 0.f, 0.f)));

    XMStoreFloat4x4(&m_TransformStateMatrices[ETOUI(D3DTS::PROJ)],
        XMMatrixOrthographicLH(ShadowDesc.fWidth, ShadowDesc.fHeight, ShadowDesc.fNear, ShadowDesc.fFar));

    return S_OK;
}

CShadow_Dir* CShadow_Dir::Create()
{
    return new CShadow_Dir();
}

void CShadow_Dir::Free()
{
    __super::Free();

    Safe_Release(m_pGameInstance_Proxy);
}
