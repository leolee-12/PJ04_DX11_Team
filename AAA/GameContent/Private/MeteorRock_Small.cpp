#include "MeteorRock_Small.h"
#include "GameInstance.h"

CMeteorRock_Small::CMeteorRock_Small(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CMeteorRock{ pDevice, pContext }
{
}

CMeteorRock_Small::CMeteorRock_Small(const CMeteorRock_Small& Prototype)
    : CMeteorRock(Prototype)
{
}

CMeteorRock_Small* CMeteorRock_Small::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CMeteorRock_Small* pInstance =  new CMeteorRock_Small(pDevice, pContext);
    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created : CMeteorRock_Small");
        Safe_Release(pInstance);
    }
    return pInstance;
}

CGameObject* CMeteorRock_Small::Clone(void* pArg)
{
    CMeteorRock_Small* pInstance = new CMeteorRock_Small(*this);
    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : CMeteorRock_Small");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CMeteorRock_Small::Free()
{
    __super::Free();
}
