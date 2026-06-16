#include "TestMarb1eMap.h"
#include "GameInstance.h"
#include "GameContent_const.h"

CTestMarb1eMap::CTestMarb1eMap(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CGameObject{ pDevice, pContext }
{
    m_fNormalStrength = 1.f;
    m_fAOStrength = 1.f;
    m_bTopProjection = false;
    m_vTopUVScale = { 1.f, 1.f };
    m_fTopUVRotate = 0.f;
    m_vTopUVOffset = { 0.f, 0.f };
    m_vBaseUVScale = { 0.1f, 0.1f };
    m_fMossAmount = 1.f;
    m_fDirtAmount = 1.f;
}

CTestMarb1eMap::CTestMarb1eMap(const CTestMarb1eMap& Prototype)
    : CGameObject(Prototype)
    , m_fNormalStrength{ Prototype.m_fNormalStrength }
    , m_fAOStrength{ Prototype.m_fAOStrength }
    , m_bTopProjection{ Prototype.m_bTopProjection }
    , m_vTopUVScale{ Prototype.m_vTopUVScale }
    , m_fTopUVRotate{ Prototype.m_fTopUVRotate }
    , m_vTopUVOffset{ Prototype.m_vTopUVOffset }
    , m_vBaseUVScale{ Prototype.m_vBaseUVScale }
    , m_fMossAmount{ Prototype.m_fMossAmount }
    , m_fDirtAmount{ Prototype.m_fDirtAmount }
{
}

HRESULT CTestMarb1eMap::Initialize_Prototype()
{
    m_eProjType = PROJ_TYPE::PERSPEC;
    return S_OK;
}

HRESULT CTestMarb1eMap::Initialize(void* pArg)
{
    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    if (FAILED(Ready_Components()))
        return E_FAIL;

    return S_OK;
}

void CTestMarb1eMap::Priority_Update(_float fTimeDelta) {}
void CTestMarb1eMap::Update(_float fTimeDelta) {}

void CTestMarb1eMap::Late_Update(_float fTimeDelta)
{
    m_pGameInstance_Proxy->Add_RenderGroup(RENDERID::NONBLEND, this);
}

HRESULT CTestMarb1eMap::Render()
{
    if (FAILED(Bind_ShaderResources()))
        return E_FAIL;

    size_t iNumMeshes = m_pModelCom->Get_NumMeshes();

    // 1패스: 일반 베이스 메시 (Parts 제외)
    for (size_t i = 0; i < iNumMeshes; i++)
    {
        if (m_pModelCom->Get_MeshName((_uint)i).find("Parts") != string::npos)
            continue;

        m_pModelCom->Bind_Material(m_pShaderCom, "g_DiffuseTexture", (_uint)i, MTEX_TYPE::DIFFUSE, 0);
        m_pModelCom->Bind_Material(m_pShaderCom, "g_NormalTexture", (_uint)i, MTEX_TYPE::NORMALS, 0);
        m_pModelCom->Bind_Material(m_pShaderCom, "g_MRATexture", (_uint)i, MTEX_TYPE::METALNESS, 0);

        HRESULT hrMoss = m_pModelCom->Bind_Material(m_pShaderCom, "g_MossTexture", (_uint)i, MTEX_TYPE::DIFFUSE, 1);
        _int iHasMoss = SUCCEEDED(hrMoss) ? 1 : 0;
        m_pShaderCom->Bind_RawValue("g_iHasMoss", &iHasMoss, sizeof(_int));

        _int iUseTop = (m_bTopProjection &&
            m_pModelCom->Get_MeshName((_uint)i).find("Top") != string::npos) ? 1 : 0;
        m_pShaderCom->Bind_RawValue("g_iUseTopProjection", &iUseTop, sizeof(_int));

        if (FAILED(m_pShaderCom->Begin(0)))   return E_FAIL;
        if (FAILED(m_pModelCom->Render((_uint)i))) return E_FAIL;
    }

    for (size_t i = 0; i < iNumMeshes; i++)
    {
        if (m_pModelCom->Get_MeshName((_uint)i).find("Parts") == string::npos)
            continue;

        m_pModelCom->Bind_Material(m_pShaderCom, "g_DiffuseTexture", (_uint)i, MTEX_TYPE::DIFFUSE, 0);   

        if (FAILED(m_pShaderCom->Begin(1)))   return E_FAIL; 
        if (FAILED(m_pModelCom->Render((_uint)i))) return E_FAIL;
    }

    return S_OK;
}

HRESULT CTestMarb1eMap::Ready_Components()
{
    m_pShaderCom = Add_Component<CShader>(Shader_Map.iLevelID, Shader_Map.szProtoTag, TEXT("Com_Shader"));
    if (nullptr == m_pShaderCom)
        return E_FAIL;

    m_pModelCom = Add_Component<CModel>(m_iPrototypeLevel, TEXT("Prototype_Component_Model_Map"), TEXT("Com_Model"));
    if (nullptr == m_pModelCom)
        return E_FAIL;

    return S_OK;
}

HRESULT CTestMarb1eMap::Bind_ShaderResources()
{
    if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::VIEW, m_eProjType))))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::PROJ, m_eProjType))))
        return E_FAIL;

    m_pShaderCom->Bind_RawValue("g_NormalStrength", &m_fNormalStrength, sizeof(_float));
    m_pShaderCom->Bind_RawValue("g_fAOStrength", &m_fAOStrength, sizeof(_float));
    m_pShaderCom->Bind_RawValue("g_TopUVScale", &m_vTopUVScale, sizeof(_float2));
    m_pShaderCom->Bind_RawValue("g_TopUVRotate", &m_fTopUVRotate, sizeof(_float));
    m_pShaderCom->Bind_RawValue("g_TopUVOffset", &m_vTopUVOffset, sizeof(_float2));
    m_pShaderCom->Bind_RawValue("g_vBaseUVScale", &m_vBaseUVScale, sizeof(_float2));
    m_pShaderCom->Bind_RawValue("g_fMossAmount", &m_fMossAmount, sizeof(_float));
    m_pShaderCom->Bind_RawValue("g_fDirtAmount", &m_fDirtAmount, sizeof(_float));

    return S_OK;
}

CTestMarb1eMap* CTestMarb1eMap::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CTestMarb1eMap* pInstance = new CTestMarb1eMap(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created : CTestMarb1eMap");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CTestMarb1eMap::Clone(void* pArg)
{
    CTestMarb1eMap* pInstance = new CTestMarb1eMap(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : CTestMarb1eMap");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CTestMarb1eMap::Free()
{
    __super::Free();
}