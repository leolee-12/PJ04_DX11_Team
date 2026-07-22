#include "MeteorRock_Large.h"
#include "GameInstance.h"

CMeteorRock_Large::CMeteorRock_Large(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CMeteorRock{ pDevice, pContext }
{
}

CMeteorRock_Large::CMeteorRock_Large(const CMeteorRock_Large& Prototype)
    : CMeteorRock(Prototype)
{
}

CMeteorRock_Large* CMeteorRock_Large::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CMeteorRock_Large* pInstance = new CMeteorRock_Large(pDevice, pContext);
    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created : CMeteorRock_Large");
        Safe_Release(pInstance);
    }
    return pInstance;
}

CGameObject* CMeteorRock_Large::Clone(void* pArg)
{
    CMeteorRock_Large* pInstance = new CMeteorRock_Large(*this);
    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : CMeteorRock_Large");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CMeteorRock_Large::Free()
{
    __super::Free();
}