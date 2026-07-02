#include "SwordCharge.h"

#include "GameInstance.h"

#include "GameContent_const.h"

CSwordCharge::CSwordCharge(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CEffect_Mesh{ pDevice, pContext }
    , m_bUseRingDeform{ true }
    , m_fRingHeight{ 0.2f }
    , m_fRingStartRadius{ 0.166626f }
    , m_fRingEndRadius{ 0.5f }
{
}

CSwordCharge::CSwordCharge(const CSwordCharge& Prototype)
    : CEffect_Mesh(Prototype)
    , m_bUseRingDeform{ Prototype.m_bUseRingDeform }
    , m_fRingHeight{ Prototype.m_fRingHeight }
    , m_fRingStartRadius{ Prototype.m_fRingStartRadius }
    , m_fRingEndRadius{ Prototype.m_fRingEndRadius }
{
}

HRESULT CSwordCharge::Initialize_Prototype()
{
    m_eProjType = PROJ_TYPE::PERSPEC;
    return S_OK;
}

HRESULT CSwordCharge::Initialize(void* pArg)
{
    EFFECT_MESH_DESC tDesc{};

    tDesc.iModelLevel = m_iPrototypeLevel;
    tDesc.wstrModelTag = TEXT("Prototype_Component_Model_SwordCharge");

    tDesc.bUseDiffuseTexture = false;
    tDesc.bUseUnKnownTexture = true;

    tDesc.bUseTextureCom = false;
    tDesc.iTextureLevel = 0;
    tDesc.wstrTextureTag = L"";

    tDesc.bUseMaskCom = false;
    tDesc.iMaskLevel = 0;
    tDesc.wstrMaskTag = L"";

    tDesc.bCustomShader = true;
    tDesc.iShaderLevel = Shader_Ring.iLevelID;
    tDesc.wstrShaderTag = Shader_Ring.szProtoTag;

    if (FAILED(__super::Initialize(&tDesc)))
        return E_FAIL;

    return S_OK;
}

void CSwordCharge::Priority_Update(_float fTimeDelta)
{
    __super::Priority_Update(fTimeDelta);
}

void CSwordCharge::Update(_float fTimeDelta)
{
    __super::Update(fTimeDelta);
}

void CSwordCharge::Late_Update(_float fTimeDelta)
{
    __super::Late_Update(fTimeDelta);

    if (m_bActive == false)
        return;

    Compute_CombinedWorldMatrix();
    m_pGameInstance_Proxy->Add_RenderGroup(RENDERID::BLEND, this);
}

HRESULT CSwordCharge::Render()
{
    if (FAILED(m_pShaderCom->Bind_RawValue("g_bUseRingDeform", &m_bUseRingDeform, sizeof(m_bUseRingDeform))))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_RawValue("g_fRingHeight", &m_fRingHeight, sizeof(m_fRingHeight))))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_RawValue("g_fRingStartRadius", &m_fRingStartRadius, sizeof(m_fRingStartRadius))))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_RawValue("g_fRingEndRadius", &m_fRingEndRadius, sizeof(m_fRingEndRadius))))
        return E_FAIL;

    return __super::Render();
}

CSwordCharge* CSwordCharge::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CSwordCharge* pInstance = new CSwordCharge(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created: CSwordCharge");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CSwordCharge::Clone(void* pArg)
{
    CSwordCharge* pInstance = new CSwordCharge(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned: CSwordCharge");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CSwordCharge::Free()
{
    __super::Free();
}
