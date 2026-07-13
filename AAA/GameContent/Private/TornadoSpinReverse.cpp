#include "TornadoSpinReverse.h"

#include "GameInstance.h"

#include "GameContent_const.h"

CTornadoSpinReverse::CTornadoSpinReverse(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CEffect_Mesh{ pDevice, pContext }
{
}

CTornadoSpinReverse::CTornadoSpinReverse(const CTornadoSpinReverse& Prototype)
    : CEffect_Mesh(Prototype)
{
}

HRESULT CTornadoSpinReverse::Initialize_Prototype()
{
    m_eProjType = PROJ_TYPE::PERSPEC;
    return S_OK;
}

HRESULT CTornadoSpinReverse::Initialize(void* pArg)
{
    TORNADO_SPIN_REVERSE_DESC tDesc{};

    tDesc.iModelLevel = m_iPrototypeLevel;
    tDesc.wstrModelTag = TEXT("Prototype_Component_Model_TornadoSpinReverse");

    tDesc.bUseDiffuseTexture = false;
    tDesc.bUseUnknownTexture = true;

    tDesc.bUseTextureCom = false;
    tDesc.iTextureLevel = 0;
    tDesc.wstrTextureTag = L"";

    tDesc.bUseMaskCom = false;
    tDesc.iMaskLevel = 0;
    tDesc.wstrMaskTag = L"";

    tDesc.bCustomShader = false;
    tDesc.iShaderLevel = 0;
    tDesc.wstrShaderTag = L"";

    if (FAILED(__super::Initialize(&tDesc)))
        return E_FAIL;

    return S_OK;
}

void CTornadoSpinReverse::Priority_Update(_float fTimeDelta)
{
    __super::Priority_Update(fTimeDelta);
}

void CTornadoSpinReverse::Update(_float fTimeDelta)
{
    __super::Update(fTimeDelta);
}

void CTornadoSpinReverse::Late_Update(_float fTimeDelta)
{
    __super::Late_Update(fTimeDelta);

    if (m_bActive == false)
        return;

    Compute_CombinedWorldMatrix();
    m_pGameInstance_Proxy->Add_RenderGroup(RENDERID::BLEND, this);
}

HRESULT CTornadoSpinReverse::Render()
{
    __super::Render();

    return S_OK;
}

CTornadoSpinReverse* CTornadoSpinReverse::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CTornadoSpinReverse* pInstance = new CTornadoSpinReverse(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created: CTornadoSpinReverse");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CTornadoSpinReverse::Clone(void* pArg)
{
    CTornadoSpinReverse* pInstance = new CTornadoSpinReverse(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned: CTornadoSpinReverse");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CTornadoSpinReverse::Free()
{
    __super::Free();
}