#include "StarEmitter.h"

#include "GameInstance.h"

#include "GameContent_const.h"

CStarEmitter::CStarEmitter(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CEffect_MeshEmitter{ pDevice, pContext }
{
}

CStarEmitter::CStarEmitter(const CStarEmitter& Prototype)
    : CEffect_MeshEmitter(Prototype)
{
}

HRESULT CStarEmitter::Initialize_Prototype()
{
    m_eProjType = PROJ_TYPE::PERSPEC;
    return S_OK;
}

HRESULT CStarEmitter::Initialize(void* pArg)
{
    STAR_EMITTER_DESC tDesc{};

    tDesc.iModelLevel = m_iPrototypeLevel;
    tDesc.wstrModelTag = TEXT("Prototype_Component_Model_StarSmooth");

    if (pArg != nullptr)
    {
        auto pIn = static_cast<STAR_EMITTER_DESC*>(pArg);
        if (!pIn->wstrModelTag.empty())
            tDesc.wstrModelTag = pIn->wstrModelTag;
    }

    tDesc.bUseDiffuseTexture = false;
    tDesc.bUseUnknownTexture = true;

    tDesc.bUseTextureCom = false;
    tDesc.iTextureLevel = 0;
    tDesc.wstrTextureTag = L"";

    tDesc.bUseMaskCom = false;
    tDesc.iMaskLevel = 0;
    tDesc.wstrMaskTag = L"";

    tDesc.bCustomShader = false;
    //tDesc.iShaderLevel = 0;
    //tDesc.wstrShaderTag = L"";

    if (FAILED(__super::Initialize(&tDesc)))
        return E_FAIL;

    return S_OK;
}

void CStarEmitter::Priority_Update(_float fTimeDelta)
{
    __super::Priority_Update(fTimeDelta);
}

void CStarEmitter::Update(_float fTimeDelta)
{
    __super::Update(fTimeDelta);
}

void CStarEmitter::Late_Update(_float fTimeDelta)
{
    __super::Late_Update(fTimeDelta);

    if (m_bActive == false)
        return;

    Compute_CombinedWorldMatrix();
    m_pGameInstance_Proxy->Add_RenderGroup(RENDERID::NONBLEND, this);
}

HRESULT CStarEmitter::Render()
{
    __super::Render();

    return S_OK;
}

CStarEmitter* CStarEmitter::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CStarEmitter* pInstance = new CStarEmitter(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created: CStarEmitter");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CStarEmitter::Clone(void* pArg)
{
    CStarEmitter* pInstance = new CStarEmitter(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned: CStarEmitter");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CStarEmitter::Free()
{
    __super::Free();
}