#include "NormalEnemyWild_Body.h"
#include "GameInstance.h"

CNormalEnemyWild_Body::CNormalEnemyWild_Body(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CNormalEnemy_Body{ pDevice, pContext }
{
}

CNormalEnemyWild_Body::CNormalEnemyWild_Body(const CNormalEnemyWild_Body& Prototype)
	: CNormalEnemy_Body ( Prototype )
{
}

HRESULT CNormalEnemyWild_Body::Ready_Components()
{
    PART_SETUP t{};
    t.tShader = Shader_Monster;
    t.szModelProtoTag = TEXT("Prototype_Component_Model_NormalEnemyWild_Body");
    t.szAnimEventFile = TEXT("");
    t.szAnimEventFile = TEXT("../../Resources/CHJ/Monster/NormalEnemyWild/Body/NormalEnemyWild_AnimEvents.json");
    if (FAILED(Ready_MeshPart(t)))
        return E_FAIL;

    m_pEyeTextureCom = Add_Component<CTexture>(TEXT("Com_EyeTexture"),
        CTexture::Create(m_pDevice, m_pContext,
                            L"../../Resources/CHJ/Monster/NormalEnemyWild/Body/NormalEnemyEye.%02d.dds",  EYE_COUNT));
    if (nullptr == m_pEyeTextureCom)
        return E_FAIL;

    return S_OK;
}

CNormalEnemyWild_Body* CNormalEnemyWild_Body::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CNormalEnemyWild_Body* pInstance = new CNormalEnemyWild_Body(pDevice, pContext);
    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created: CNormalEnemyWild_Body");
        Safe_Release(pInstance);
    }
    return pInstance;
}

CGameObject* CNormalEnemyWild_Body::Clone(void* pArg)
{
    CNormalEnemyWild_Body* pInstance = new CNormalEnemyWild_Body(*this);
    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned: CNormalEnemyWild_Body");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CNormalEnemyWild_Body::Free()
{
    __super::Free();
}
