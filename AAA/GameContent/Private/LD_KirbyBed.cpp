#include "LD_KirbyBed.h"
#include "LevelDesign_Registry.h"
#include "MeshLayer_Binder.h"
#include "Parsing_Utils.h"
#include "GameContent_const.h"

#include "Model.h"
#include "GameInstance.h"

NS_BEGIN(Client)

CLD_KirbyBed::CLD_KirbyBed(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CLevelDesignObject(pDevice, pContext)
{
}

CLD_KirbyBed::CLD_KirbyBed(const CLD_KirbyBed& Prototype)
    : CLevelDesignObject(Prototype)
    , m_tStaticModelDesc(Prototype.m_tStaticModelDesc)
{
}

HRESULT CLD_KirbyBed::Initialize_Prototype()
{
    return __super::Initialize_Prototype();
}

HRESULT CLD_KirbyBed::Initialize(void* pArg)
{
    if (nullptr == pArg)
        return E_FAIL;

    m_tStaticModelDesc = *static_cast<const LD_STATIC_MODEL_DESC*>(pArg);

    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    if (FAILED(Ready_RenderComponents()))
        return E_FAIL;

    if (FAILED(Ready_CullingState(m_pModelCom)))
        return E_FAIL;

    if (FAILED(Ready_RigidStatic()))
        return E_FAIL;

    m_bUseShadow = true;

    return Validate_Initialized();
}

HRESULT CLD_KirbyBed::Validate_Initialized()
{
    if (FAILED(__super::Validate_Initialized()))
        return E_FAIL;

    if (!JsonUtils::Equals_NoCase(OBJECT_NAME, m_tStaticModelDesc.strObjectName.c_str()))
        return E_FAIL;

    if (LD_CATEGORY::GIMMICK != m_tStaticModelDesc.eCategory)
        return E_FAIL;

    if (m_tStaticModelDesc.wstrModelProtoTag != MODEL_PROTO_TAG)
        return E_FAIL;

    if (nullptr == m_pShaderCom || nullptr == m_pModelCom)
        return E_FAIL;

    if (m_tStaticModelDesc.bUseCollMesh != (nullptr != m_pRigidStatic))
        return E_FAIL;

    return S_OK;
}

void CLD_KirbyBed::Late_Update(_float fTimeDelta)
{
    UNREFERENCED_PARAMETER(fTimeDelta);

    if (!m_bActive || Is_Dead())
        return;

    Check_Visible();
    Submit_RenderGroups();
}

HRESULT CLD_KirbyBed::Render()
{
    if (FAILED(Bind_ShaderResources()))
        return E_FAIL;

    return Render_Model();
}

HRESULT CLD_KirbyBed::Render_Shadow()
{
    return Render_ShadowModel(m_pShaderCom, m_pModelCom, MESH_LAYER_PROFILE::WORLD_NONANIM);
}

HRESULT CLD_KirbyBed::On_EditTransformChanged()
{
    if (FAILED(__super::On_EditTransformChanged()))
        return E_FAIL;

    if (!m_tStaticModelDesc.bUseCollMesh || nullptr == m_pRigidStatic)
        return S_OK;

    if (nullptr == m_pGameInstance_Proxy || nullptr == m_pTransformCom || nullptr == m_pModelCom)
        return E_FAIL;

    physx::PxTriangleMesh* pCollisionMesh = m_pModelCom->Get_CollisionMesh();
    if (nullptr == pCollisionMesh)
        return E_FAIL;

    physx::PxRigidStatic* pNewRigidStatic = m_pGameInstance_Proxy->Create_StaticActor(
        pCollisionMesh,
        XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));

    if (nullptr == pNewRigidStatic)
        return E_FAIL;

    m_pGameInstance_Proxy->Remove_StaticActor(m_pRigidStatic);
    m_pRigidStatic = pNewRigidStatic;

    return S_OK;
}

void CLD_KirbyBed::Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData)
{
    if (nullptr == pOutData)
        return;

    pOutData->strPrototypeTag = PROTOTYPE_TAG;
}

void CLD_KirbyBed::Register_LevelDesignSpecs()
{
    LD_SPAWN_SPEC Spec{};
    Spec.strObjectName = OBJECT_NAME;
    Spec.strPrototypeTag = PROTOTYPE_TAG;
    Spec.strLayerTag = LAYER_TAG;
    Spec.eCategory = LD_CATEGORY::GIMMICK;
    Spec.wstrModelProtoTag = MODEL_PROTO_TAG;
    Spec.eModelType = MODEL::NONANIM;
    Spec.pPrototypeFactory = &Create_Prototype;
    Spec.pBuildDesc = &Build_Desc;
    Spec.ModelRequirements =
    {
            { MODEL_PROTO_TAG, MODEL_PATH, MODEL::NONANIM, true },
    };

    CLevelDesign_Registry::Register(Spec.strObjectName, Spec);
}

_bool CLD_KirbyBed::Build_Desc(const LD_OBJECT_DESC& CommonDesc, const json& jEntry, const LD_SPAWN_SPEC& Spec, LD_OBJECT_ENTRY* pOutEntry)
{
    if (nullptr == pOutEntry)
        return false;

    if (!JsonUtils::Equals_NoCase(OBJECT_NAME, CommonDesc.strObjectName.c_str()))
        return false;

    if (Spec.strPrototypeTag != PROTOTYPE_TAG || Spec.strLayerTag != LAYER_TAG)
        return false;

    if (Spec.eCategory != LD_CATEGORY::GIMMICK
        || Spec.eModelType != MODEL::NONANIM
        || Spec.wstrModelProtoTag != MODEL_PROTO_TAG)
    {
        return false;
    }

    LD_STATIC_MODEL_DESC Desc{};
    static_cast<LD_OBJECT_DESC&>(Desc) = CommonDesc;
    Desc.eCategory = Spec.eCategory;
    Desc.wstrModelProtoTag = Spec.wstrModelProtoTag;

    _bool bInvalidCollision = false;
    JsonUtils::Try_ReadBoolFromNumeric(jEntry, "Basic.Model.IsInvalidCollision", &bInvalidCollision);
    Desc.bUseCollMesh = !bInvalidCollision;

    *pOutEntry = Desc;
    return true;
}

CGameObject* CLD_KirbyBed::Create_Prototype(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    return CLD_KirbyBed::Create(pDevice, pContext);
}

HRESULT CLD_KirbyBed::Ready_RenderComponents()
{
    if (m_tStaticModelDesc.wstrModelProtoTag.empty())
        return E_FAIL;

    m_pShaderCom = Add_Component<CShader>(Shader_World_NonAnim.iLevelID, Shader_World_NonAnim.szProtoTag, TEXT("Com_Shader"));
    if (nullptr == m_pShaderCom)
        return E_FAIL;

    m_pModelCom = Add_Component<CModel>(
        m_tStaticModelDesc.iModelProtoLevel,
        m_tStaticModelDesc.wstrModelProtoTag,
        TEXT("Com_Model"));

    if (nullptr == m_pModelCom)
        return E_FAIL;

    return S_OK;
}

HRESULT CLD_KirbyBed::Ready_RigidStatic()
{
    if (!m_tStaticModelDesc.bUseCollMesh)
        return S_OK;

    if (nullptr == m_pGameInstance_Proxy || nullptr == m_pTransformCom || nullptr == m_pModelCom)
        return E_FAIL;

    physx::PxTriangleMesh* pCollisionMesh = m_pModelCom->Get_CollisionMesh();
    if (nullptr == pCollisionMesh)
        return E_FAIL;

    m_pRigidStatic = m_pGameInstance_Proxy->Create_StaticActor(
        pCollisionMesh,
        XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));

    return (nullptr != m_pRigidStatic) ? S_OK : E_FAIL;
}

void CLD_KirbyBed::Release_RigidStatic()
{
    if (nullptr == m_pRigidStatic)
        return;

    if (nullptr != m_pGameInstance_Proxy)
        m_pGameInstance_Proxy->Remove_StaticActor(m_pRigidStatic);

    m_pRigidStatic = nullptr;
}

HRESULT CLD_KirbyBed::Bind_ShaderResources()
{
    if (FAILED(MeshLayerBinder::Bind_WorldViewProj(m_pShaderCom, m_pTransformCom, m_pGameInstance_Proxy, m_eProjType)))
        return E_FAIL;

    if (FAILED(m_pShaderCom->Bind_RawValue("g_iMaterialID", &m_iMaterialID, sizeof(_uint))))
        return E_FAIL;

    return S_OK;
}

HRESULT CLD_KirbyBed::Render_Model()
{
    const _uint iNumMeshes = static_cast<_uint>(m_pModelCom->Get_NumMeshes());

    for (_uint i = 0; i < iNumMeshes; ++i)
    {
        const MESH_LAYER_IDX& Layer = m_pModelCom->Get_MeshLayer(i);
        const _bool bUseColorPass = (0u == m_pModelCom->Get_MeshTextureCount(i, MTEX_TYPE::DIFFUSE));

        MESH_LAYER_BIND_CONTEXT Ctx{};
        Ctx.Set_Renderer(m_pShaderCom, m_pModelCom, m_pGameInstance_Proxy, m_pCullingState);
        Ctx.iMesh = i;
        Ctx.pLayer = &Layer;
        Ctx.eProfile = MESH_LAYER_PROFILE::WORLD_NONANIM;
        Ctx.eKind = MESH_LAYER_RENDER_KIND::MAIN;
        Ctx.iFallbackPass = bUseColorPass ? ETOUI(WORLD_PASS::COLOR_CONST_MRA) : ETOUI(WORLD_PASS::DMN);

        _uint iPass = 0u;
        const HRESULT hrBind = MeshLayerBinder::Bind_OrSkip(Ctx, &iPass);
        if (FAILED(hrBind))
            return E_FAIL;
        if (S_FALSE == hrBind)
            continue;

        if (FAILED(m_pShaderCom->Begin(iPass)))
            return E_FAIL;
        if (FAILED(m_pModelCom->Render(i)))
            return E_FAIL;
    }

    return S_OK;
}

CLD_KirbyBed* CLD_KirbyBed::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CLD_KirbyBed* pInstance = new CLD_KirbyBed(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created : CLD_KirbyBed");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CLD_KirbyBed::Clone(void* pArg)
{
    CLD_KirbyBed* pInstance = new CLD_KirbyBed(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : CLD_KirbyBed");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CLD_KirbyBed::Free()
{
    Release_RigidStatic();

    __super::Free();
}

NS_END