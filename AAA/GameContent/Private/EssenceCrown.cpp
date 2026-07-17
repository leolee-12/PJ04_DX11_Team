#include "EssenceCrown.h"

#include "GameInstance.h"
#include "GameContent_const.h"

CEssenceCrown::CEssenceCrown(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CEffect_Mesh{ pDevice, pContext }
    , m_bUseRingDeform{ true }
    , m_fRingHeight{ 0.45f }
    , m_fRingStartRadius{ 0.1f }
    , m_fRingEndRadius{ 0.5f }
{
}

CEssenceCrown::CEssenceCrown(const CEssenceCrown& Prototype)
    : CEffect_Mesh(Prototype)
    , m_bUseRingDeform{ Prototype.m_bUseRingDeform }
    , m_fRingHeight{ Prototype.m_fRingHeight }
    , m_fRingStartRadius{ Prototype.m_fRingStartRadius }
    , m_fRingEndRadius{ Prototype.m_fRingEndRadius }
{
}

HRESULT CEssenceCrown::Initialize_Prototype()
{
    m_eProjType = PROJ_TYPE::PERSPEC;
    return S_OK;
}

HRESULT CEssenceCrown::Initialize(void* pArg)
{
    EFFECT_MESH_DESC tDesc{};

    tDesc.iModelLevel = m_iPrototypeLevel;
    tDesc.wstrModelTag = TEXT("Prototype_Component_Model_EssenceCrown");

    tDesc.bUseDiffuseTexture = false;
    tDesc.bUseUnknownTexture = true;

    tDesc.bUseTextureCom = false;
    tDesc.iTextureLevel = 0;
    tDesc.wstrTextureTag = L"";

    tDesc.bUseMaskCom = true;
    tDesc.iMaskLevel = Texture_ChargeNoise.iLevelID;
    tDesc.wstrMaskTag = Texture_ChargeNoise.szProtoTag;

    tDesc.bCustomShader = true;
    tDesc.iShaderLevel = Shader_Ring.iLevelID;
    tDesc.wstrShaderTag = Shader_Ring.szProtoTag;

    if (FAILED(__super::Initialize(&tDesc)))
        return E_FAIL;

    return S_OK;
}

void CEssenceCrown::Priority_Update(_float fTimeDelta)
{
    __super::Priority_Update(fTimeDelta);
}

void CEssenceCrown::Update(_float fTimeDelta)
{
    __super::Update(fTimeDelta);
}

void CEssenceCrown::Late_Update(_float fTimeDelta)
{
    __super::Late_Update(fTimeDelta);

    if (m_bActive == false)
        return;

    Compute_CombinedWorldMatrix();
    m_pGameInstance_Proxy->Add_RenderGroup(RENDERID::BLEND, this);
}

HRESULT CEssenceCrown::Render()
{
    if (FAILED(m_pShaderCom->Bind_RawValue("g_bUseRingDeform", &m_bUseRingDeform,
        sizeof(m_bUseRingDeform))))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_RawValue("g_fRingHeight", &m_fRingHeight, sizeof(m_fRingHeight))))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_RawValue("g_fRingStartRadius", &m_fRingStartRadius,
        sizeof(m_fRingStartRadius))))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_RawValue("g_fRingEndRadius", &m_fRingEndRadius,
        sizeof(m_fRingEndRadius))))
        return E_FAIL;

    return __super::Render();
}

CEssenceCrown* CEssenceCrown::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CEssenceCrown* pInstance = new CEssenceCrown(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created: CEssenceCrown");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CEssenceCrown::Clone(void* pArg)
{
    CEssenceCrown* pInstance = new CEssenceCrown(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned: CEssenceCrown");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CEssenceCrown::Free()
{
    __super::Free();
}