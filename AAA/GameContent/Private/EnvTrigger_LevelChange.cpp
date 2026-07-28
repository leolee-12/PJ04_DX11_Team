#include "EnvTrigger_LevelChange.h"

CEnvTrigger_LevelChange::CEnvTrigger_LevelChange(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CEnvTrigger_EventPublisher(pDevice, pContext)
    , m_fSFXDuration{ 0.f }
{
    m_strProtoTag = PROTOTYPE_TAG;
}

CEnvTrigger_LevelChange::CEnvTrigger_LevelChange(const CEnvTrigger_LevelChange& Prototype)
    : CEnvTrigger_EventPublisher(Prototype)
    , m_fSFXDuration{Prototype.m_fSFXDuration}
{
    m_strProtoTag = PROTOTYPE_TAG;
}

void* CEnvTrigger_LevelChange::Get_EnterPayload()
{
    ePayload.fSFXDuration = m_fSFXDuration;

    return &ePayload;
}

CEnvTrigger_LevelChange* CEnvTrigger_LevelChange::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CEnvTrigger_LevelChange* pInstance = new CEnvTrigger_LevelChange(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created : CEnvTrigger_LevelChange");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CEnvTrigger_LevelChange::Clone(void* pArg)
{
    CEnvTrigger_LevelChange* pInstance = new CEnvTrigger_LevelChange(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : CEnvTrigger_LevelChange");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CEnvTrigger_LevelChange::Free()
{
    __super::Free();
}
