#include "MapObject.h"
#include "GameInstance.h"
#include "GameContent_const.h"
#include "Shader.h"
#include "Model.h"

CMapObject::CMapObject(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CGameObject{ pDevice, pContext }
{
    m_fNormalStrength = 1.f;
    m_vBaseUVScale = { 0.1f, 0.1f };
    m_bTopProjection = false;
}

CMapObject::CMapObject(const CMapObject& Prototype)
    : CGameObject(Prototype)
    , m_fNormalStrength{ Prototype.m_fNormalStrength }
    , m_vBaseUVScale{ Prototype.m_vBaseUVScale }
    , m_bTopProjection{ Prototype.m_bTopProjection }
{
}

HRESULT CMapObject::Initialize_Prototype()
{
    m_eProjType = PROJ_TYPE::PERSPEC;
    return S_OK;
}

HRESULT CMapObject::Initialize(void* pArg)
{
    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    if (FAILED(Ready_MapComponents()))
        return E_FAIL;

    return S_OK;
}

void CMapObject::Priority_Update(_float fTimeDelta) {}
void CMapObject::Update(_float fTimeDelta) {}

void CMapObject::Late_Update(_float fTimeDelta)
{
    m_pGameInstance_Proxy->Add_RenderGroup(RENDERID::NONBLEND, this);
}

HRESULT CMapObject::Render()
{
    if (FAILED(Bind_ShaderResources()))
        return E_FAIL;

    size_t n = m_pModelCom->Get_NumMeshes();

    // 1패스: 베이스 불투명 (Parts 제외)
    for (size_t i = 0; i < n; ++i)
    {
        if (Is_OverlayMesh((_uint)i))
            continue;

        m_pModelCom->Bind_Material(m_pShaderCom, "g_DiffuseTexture", (_uint)i, MTEX_TYPE::DIFFUSE, 0);
        m_pModelCom->Bind_Material(m_pShaderCom, "g_NormalTexture", (_uint)i, MTEX_TYPE::NORMALS, 0);
        m_pModelCom->Bind_Material(m_pShaderCom, "g_MRATexture", (_uint)i, MTEX_TYPE::METALNESS, 0);

        _int iZero = 0;
        m_pShaderCom->Bind_RawValue("g_iHasMoss", &iZero, sizeof(_int)); 
        m_pShaderCom->Bind_RawValue("g_iHasDirt", &iZero, sizeof(_int));

        _int iUseTop = (m_bTopProjection &&
            m_pModelCom->Get_MeshName((_uint)i).find("Top") != string::npos) ? 1 : 0;
        m_pShaderCom->Bind_RawValue("g_iUseTopProjection", &iUseTop, sizeof(_int));

        Bind_MeshLayers((_uint)i);   

        if (FAILED(m_pShaderCom->Begin(0)))
            return E_FAIL;
        if (FAILED(m_pModelCom->Render((_uint)i)))
            return E_FAIL;
    }

    // 2패스: 오버레이(DirtParts discard) 베이스 위에 얹기
    for (size_t i = 0; i < n; ++i)
    {
        if (!Is_OverlayMesh((_uint)i))
            continue;

        m_pModelCom->Bind_Material(m_pShaderCom, "g_DiffuseTexture", (_uint)i, MTEX_TYPE::DIFFUSE, 0);

        if (FAILED(m_pShaderCom->Begin(1)))
            return E_FAIL;
        if (FAILED(m_pModelCom->Render((_uint)i)))
            return E_FAIL;
    }

    return S_OK;
}

_bool CMapObject::Is_OverlayMesh(_uint iMesh) const
{
    return m_pModelCom->Get_MeshName(iMesh).find("Parts") != string::npos;
}

HRESULT CMapObject::Ready_MapComponents()
{
    m_pShaderCom = Add_Component<CShader>(Shader_Map.iLevelID, Shader_Map.szProtoTag, TEXT("Com_Shader"));
    if (nullptr == m_pShaderCom)
        return E_FAIL;

    m_pModelCom = Add_Component<CModel>(ETOUI(LEVEL::GAMEPLAY), Get_ModelProtoTag(), TEXT("Com_Model"));
    if (nullptr == m_pModelCom)
        return E_FAIL;

    return S_OK;
}

HRESULT CMapObject::Bind_ShaderResources()
{
    if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::VIEW, m_eProjType))))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::PROJ, m_eProjType))))
        return E_FAIL;

    m_pShaderCom->Bind_RawValue("g_NormalStrength", &m_fNormalStrength, sizeof(_float));
    m_pShaderCom->Bind_RawValue("g_vBaseUVScale", &m_vBaseUVScale, sizeof(_float2));

    return S_OK;
}

void CMapObject::Free()
{
    __super::Free();
}