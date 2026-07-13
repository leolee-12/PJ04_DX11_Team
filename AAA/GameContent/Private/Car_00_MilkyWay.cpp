#include "Car_00_MilkyWay.h"

#include "GameInstance.h"

#include "GameContent_const.h"

CCar_00_MilkyWay::CCar_00_MilkyWay(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CEffect_Mesh{ pDevice, pContext }
{
}

CCar_00_MilkyWay::CCar_00_MilkyWay(const CCar_00_MilkyWay& Prototype)
    : CEffect_Mesh(Prototype)
{
}

HRESULT CCar_00_MilkyWay::Initialize_Prototype()
{
    m_eProjType = PROJ_TYPE::PERSPEC;
    return S_OK;
}

HRESULT CCar_00_MilkyWay::Initialize(void* pArg)
{
    EFFECT_MESH_DESC tDesc{};

    tDesc.iModelLevel = m_iPrototypeLevel;
    tDesc.wstrModelTag = TEXT("Prototype_Component_Model_Car_00_MilkyWay");

    tDesc.bUseDiffuseTexture = true;
    tDesc.bUseUnknownTexture = false;

    tDesc.bUseTextureCom = true;
    tDesc.iTextureLevel = Texture_Gradiant.iLevelID;
    tDesc.wstrTextureTag = Texture_Gradiant.szProtoTag;

    tDesc.bUseMaskCom = true;
    tDesc.iMaskLevel = Texture_MilkyWayMask.iLevelID;
    tDesc.wstrMaskTag = Texture_MilkyWayMask.szProtoTag;

    tDesc.bCustomShader = false;
    //tDesc.iShaderLevel = 0;
    //tDesc.wstrShaderTag = L"";

    if (FAILED(__super::Initialize(&tDesc)))
        return E_FAIL;

    return S_OK;
}

void CCar_00_MilkyWay::Priority_Update(_float fTimeDelta)
{
    __super::Priority_Update(fTimeDelta);
}

void CCar_00_MilkyWay::Update(_float fTimeDelta)
{
    __super::Update(fTimeDelta);
}

void CCar_00_MilkyWay::Late_Update(_float fTimeDelta)
{
    __super::Late_Update(fTimeDelta);

    if (m_bActive == false)
        return;

    Compute_CombinedWorldMatrix();
    m_pGameInstance_Proxy->Add_RenderGroup(RENDERID::BLEND, this);
}

HRESULT CCar_00_MilkyWay::Render()
{
    __super::Render();

    return S_OK;
}

CCar_00_MilkyWay* CCar_00_MilkyWay::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CCar_00_MilkyWay* pInstance = new CCar_00_MilkyWay(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created: CCar_00_MilkyWay");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CCar_00_MilkyWay::Clone(void* pArg)
{
    CCar_00_MilkyWay* pInstance = new CCar_00_MilkyWay(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned: CCar_00_MilkyWay");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CCar_00_MilkyWay::Free()
{
    __super::Free();
}