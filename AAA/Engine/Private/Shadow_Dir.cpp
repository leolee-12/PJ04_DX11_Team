#include "Shadow_Dir.h"
#include "GameInstance.h"

CShadow_Dir::CShadow_Dir()
    : m_pGameInstance_Proxy{ CGameInstance::GetProxy() }
{
}

HRESULT CShadow_Dir::Add_ShadowLight(const SHADOW_LIGHT_DESC& d)
{
    _vector vEye = XMLoadFloat4(&d.vEye);
    _vector vAt = XMLoadFloat4(&d.vAt);
    _vector vDir = XMVector3Normalize(XMVectorSubtract(vAt, vEye));
    // look이 월드 up과 거의 평행(수직)이면 다른 up 사용
    _vector vUp = (fabsf(XMVectorGetY(vDir)) > 0.99f)
        ? XMVectorSet(0.f, 0.f, 1.f, 0.f)
        : XMVectorSet(0.f, 1.f, 0.f, 0.f);
    XMStoreFloat4x4(&m_TransformStateMatrices[ETOUI(D3DTS::VIEW)], XMMatrixLookAtLH(vEye, vAt, vUp));
    XMStoreFloat4x4(&m_TransformStateMatrices[ETOUI(D3DTS::PROJ)],
        XMMatrixOrthographicLH(d.fWidth, d.fHeight, d.fNear, d.fFar));
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
