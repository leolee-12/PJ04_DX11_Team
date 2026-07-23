#include "LD_WaterArea.h"
#include "GameContent_const.h"
#include "LevelDesign_Registry.h"
#include "Water_RenderBinder.h"

#include "GameInstance.h"
#include "Parsing_Utils.h"
#include "CullingState.h"

NS_BEGIN(Client)

namespace
{
    constexpr const _char* WATER_MODEL_PATH = "../../Resources/Map/Gimmick/NonAnim/Water/Water.ysh";

    constexpr const _tchar* WATER_AREA_OBJECT_NAMES[] =
    {
            CLD_WaterArea::OBJECT_NAME,
            CLD_WaterArea::SECOND_OBJECT_NAME
    };

    _bool Is_WaterAreaObjectName(const _wstring& strObjectName)
    {
        for (const _tchar* pObjectName : WATER_AREA_OBJECT_NAMES)
        {
            if (JsonUtils::Equals_NoCase(pObjectName, strObjectName.c_str()))
                return true;
        }

        return false;
    }
}

CLD_WaterArea::CLD_WaterArea(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CLevelDesignObject(pDevice, pContext)
{
    m_bUseShadow = false;
}

CLD_WaterArea::CLD_WaterArea(const CLD_WaterArea& Prototype)
    : CLevelDesignObject(Prototype)
    , m_tSurfaceAreaDesc(Prototype.m_tSurfaceAreaDesc)
    , m_tWaterRenderDesc(Prototype.m_tWaterRenderDesc)
{
}

HRESULT CLD_WaterArea::Initialize_Prototype()
{
    return __super::Initialize_Prototype();
}

HRESULT CLD_WaterArea::Initialize(void* pArg)
{
    if (nullptr == pArg)
        return E_FAIL;

    m_tSurfaceAreaDesc = *static_cast<const LD_SURFACE_AREA_DESC*>(pArg);
    m_tWaterRenderDesc = m_tSurfaceAreaDesc.tWaterRenderDesc;
    Sanitize_WaterRenderDesc(&m_tWaterRenderDesc);

    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    if (FAILED(Ready_RenderComponents()))
        return E_FAIL;

    if (FAILED(Ready_CullingState(m_pModelCom)))
        return E_FAIL;

    if (FAILED(Validate_Initialized()))
        return E_FAIL;

    return S_OK;
}

HRESULT CLD_WaterArea::Validate_Initialized()
{
    if (FAILED(__super::Validate_Initialized()))
        return E_FAIL;

    if (!Is_WaterAreaObjectName(m_tSurfaceAreaDesc.strObjectName))
        return E_FAIL;

    if (LD_CATEGORY::VOLUME != m_tSurfaceAreaDesc.eCategory)
        return E_FAIL;

    if (MODEL::NONANIM != m_tSurfaceAreaDesc.eModelType)
        return E_FAIL;

    if (m_tSurfaceAreaDesc.wstrModelProtoTag != MODEL_PROTO_TAG)
        return E_FAIL;

    if (nullptr == m_pShaderCom || nullptr == m_pModelCom)
        return E_FAIL;

    return S_OK;
}

void CLD_WaterArea::Late_Update(_float fTimeDelta)
{
    UNREFERENCED_PARAMETER(fTimeDelta);

    if (!m_bActive || Is_Dead())
        return;

    Check_Visible();
    Submit_RenderGroups(RENDERID::BLEND_HDR);
}

HRESULT CLD_WaterArea::Render()
{
    if (FAILED(Bind_ShaderResources()))
        return E_FAIL;

    return Render_Model();
}

void CLD_WaterArea::Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData)
{
    if (nullptr == pOutData)
        return;

    pOutData->strPrototypeTag = PROTOTYPE_TAG;
}

json CLD_WaterArea::Serialize() const
{
    json j = __super::Serialize();
    json jWaterMaterial = json::object();

    jWaterMaterial["ShallowColor"] = { m_tWaterRenderDesc.vShallowColor.x, m_tWaterRenderDesc.vShallowColor.y,
    m_tWaterRenderDesc.vShallowColor.z, m_tWaterRenderDesc.vShallowColor.w };
    jWaterMaterial["DeepColor"] = { m_tWaterRenderDesc.vDeepColor.x, m_tWaterRenderDesc.vDeepColor.y, m_tWaterRenderDesc.vDeepColor.z,
    m_tWaterRenderDesc.vDeepColor.w };
    jWaterMaterial["ShallowColorStrength"] = m_tWaterRenderDesc.fShallowColorStrength;
    jWaterMaterial["Opacity"] = m_tWaterRenderDesc.fOpacity;
    jWaterMaterial["DepthFadeDistance"] = m_tWaterRenderDesc.fDepthFadeDistance;

    jWaterMaterial["NormalTiling0"] = { m_tWaterRenderDesc.vNormalTiling0.x, m_tWaterRenderDesc.vNormalTiling0.y };
    jWaterMaterial["NormalSpeed0"] = { m_tWaterRenderDesc.vNormalSpeed0.x, m_tWaterRenderDesc.vNormalSpeed0.y };
    jWaterMaterial["NormalTiling1"] = { m_tWaterRenderDesc.vNormalTiling1.x, m_tWaterRenderDesc.vNormalTiling1.y };
    jWaterMaterial["NormalSpeed1"] = { m_tWaterRenderDesc.vNormalSpeed1.x, m_tWaterRenderDesc.vNormalSpeed1.y };
    jWaterMaterial["NormalStrength"] = m_tWaterRenderDesc.fNormalStrength;

    jWaterMaterial["FresnelPower"] = m_tWaterRenderDesc.fFresnelPower;
    jWaterMaterial["ReflectionStrength"] = m_tWaterRenderDesc.fReflectionStrength;
    jWaterMaterial["RefractionStrength"] = m_tWaterRenderDesc.fRefractionStrength;
    jWaterMaterial["LightReceiveStrength"] = m_tWaterRenderDesc.fLightReceiveStrength;
    jWaterMaterial["SpecularPower"] = m_tWaterRenderDesc.fSpecularPower;
    jWaterMaterial["SpecularStrength"] = m_tWaterRenderDesc.fSpecularStrength;

    jWaterMaterial["FoamWidth"] = m_tWaterRenderDesc.fFoamWidth;
    jWaterMaterial["FoamStrength"] = m_tWaterRenderDesc.fFoamStrength;
    jWaterMaterial["FoamNoiseTiling"] = { m_tWaterRenderDesc.vFoamNoiseTiling.x, m_tWaterRenderDesc.vFoamNoiseTiling.y };
    jWaterMaterial["FoamNoiseSpeed"] = { m_tWaterRenderDesc.vFoamNoiseSpeed.x, m_tWaterRenderDesc.vFoamNoiseSpeed.y };
    jWaterMaterial["FoamNoiseStrength"] = m_tWaterRenderDesc.fFoamNoiseStrength;
    jWaterMaterial["FoamBlur"] = m_tWaterRenderDesc.fFoamBlur;

    jWaterMaterial["CausticTiling"] = { m_tWaterRenderDesc.vCausticTiling.x, m_tWaterRenderDesc.vCausticTiling.y };
    jWaterMaterial["CausticSpeed"] = { m_tWaterRenderDesc.vCausticSpeed.x, m_tWaterRenderDesc.vCausticSpeed.y };
    jWaterMaterial["CausticStrength"] = m_tWaterRenderDesc.fCausticStrength;
    jWaterMaterial["CausticNoiseStrength"] = m_tWaterRenderDesc.fCausticNoiseStrength;
    jWaterMaterial["CausticBlur"] = m_tWaterRenderDesc.fCausticBlur;

    jWaterMaterial["WaveAmplitude"] = m_tWaterRenderDesc.fWaveAmplitude;
    jWaterMaterial["WaveSpeed"] = m_tWaterRenderDesc.fWaveSpeed;

    j["WaterMaterial"] = jWaterMaterial;
    return j;
}

void CLD_WaterArea::Deserialize_Internal(const json& j)
{
    __super::Deserialize_Internal(j);

    const auto IterWaterMaterial = j.find("WaterMaterial");
    if (IterWaterMaterial == j.end() || !IterWaterMaterial->is_object())
        return;

    WATER_RENDER_DESC Desc = m_tWaterRenderDesc;
    const json& jWaterMaterial = *IterWaterMaterial;

    JsonUtils::Try_ReadFloat4Array(jWaterMaterial, "ShallowColor", &Desc.vShallowColor);
    JsonUtils::Try_ReadFloat4Array(jWaterMaterial, "DeepColor", &Desc.vDeepColor);
    JsonUtils::Try_ReadFloat(jWaterMaterial, "ShallowColorStrength", &Desc.fShallowColorStrength);
    JsonUtils::Try_ReadFloat(jWaterMaterial, "Opacity", &Desc.fOpacity);
    JsonUtils::Try_ReadFloat(jWaterMaterial, "DepthFadeDistance", &Desc.fDepthFadeDistance);

    JsonUtils::Try_ReadFloat2Array(jWaterMaterial, "NormalTiling0", &Desc.vNormalTiling0);
    JsonUtils::Try_ReadFloat2Array(jWaterMaterial, "NormalSpeed0", &Desc.vNormalSpeed0);
    JsonUtils::Try_ReadFloat2Array(jWaterMaterial, "NormalTiling1", &Desc.vNormalTiling1);
    JsonUtils::Try_ReadFloat2Array(jWaterMaterial, "NormalSpeed1", &Desc.vNormalSpeed1);
    JsonUtils::Try_ReadFloat(jWaterMaterial, "NormalStrength", &Desc.fNormalStrength);

    JsonUtils::Try_ReadFloat(jWaterMaterial, "FresnelPower", &Desc.fFresnelPower);
    JsonUtils::Try_ReadFloat(jWaterMaterial, "ReflectionStrength", &Desc.fReflectionStrength);
    JsonUtils::Try_ReadFloat(jWaterMaterial, "RefractionStrength", &Desc.fRefractionStrength);
    JsonUtils::Try_ReadFloat(jWaterMaterial, "LightReceiveStrength", &Desc.fLightReceiveStrength);
    JsonUtils::Try_ReadFloat(jWaterMaterial, "SpecularPower", &Desc.fSpecularPower);
    JsonUtils::Try_ReadFloat(jWaterMaterial, "SpecularStrength", &Desc.fSpecularStrength);

    JsonUtils::Try_ReadFloat(jWaterMaterial, "FoamWidth", &Desc.fFoamWidth);
    JsonUtils::Try_ReadFloat(jWaterMaterial, "FoamStrength", &Desc.fFoamStrength);
    JsonUtils::Try_ReadFloat2Array(jWaterMaterial, "FoamNoiseTiling", &Desc.vFoamNoiseTiling);
    JsonUtils::Try_ReadFloat2Array(jWaterMaterial, "FoamNoiseSpeed", &Desc.vFoamNoiseSpeed);
    JsonUtils::Try_ReadFloat(jWaterMaterial, "FoamNoiseStrength", &Desc.fFoamNoiseStrength);
    JsonUtils::Try_ReadFloat(jWaterMaterial, "FoamBlur", &Desc.fFoamBlur);

    JsonUtils::Try_ReadFloat2Array(jWaterMaterial, "CausticTiling", &Desc.vCausticTiling);
    JsonUtils::Try_ReadFloat2Array(jWaterMaterial, "CausticSpeed", &Desc.vCausticSpeed);
    JsonUtils::Try_ReadFloat(jWaterMaterial, "CausticStrength", &Desc.fCausticStrength);
    JsonUtils::Try_ReadFloat(jWaterMaterial, "CausticNoiseStrength", &Desc.fCausticNoiseStrength);
    JsonUtils::Try_ReadFloat(jWaterMaterial, "CausticBlur", &Desc.fCausticBlur);

    JsonUtils::Try_ReadFloat(jWaterMaterial, "WaveAmplitude", &Desc.fWaveAmplitude);
    JsonUtils::Try_ReadFloat(jWaterMaterial, "WaveSpeed", &Desc.fWaveSpeed);

    Sanitize_WaterRenderDesc(&Desc);
    m_tWaterRenderDesc = Desc;
}

#pragma region Editable
_bool CLD_WaterArea::Get_EditDesc(EDITABLE_DESC* pOutDesc) const
{
    if (!__super::Get_EditDesc(pOutDesc))
        return false;

    pOutDesc->iCapabilities |= EDIT_CAP_WATER_MATERIAL;

    EDIT_WATER_MATERIAL WaterMaterial{};
    WaterMaterial.RenderDesc = m_tWaterRenderDesc;
    pOutDesc->CustomDesc = WaterMaterial;

    return true;
}

HRESULT CLD_WaterArea::Apply_EditCustomDesc(const EDIT_CUSTOM_DESC& Desc)
{
    const EDIT_WATER_MATERIAL* pWaterMaterial = get_if<EDIT_WATER_MATERIAL>(&Desc);
    if (nullptr == pWaterMaterial)
        return E_FAIL;

    m_tWaterRenderDesc = pWaterMaterial->RenderDesc;
    Sanitize_WaterRenderDesc(&m_tWaterRenderDesc);

    return S_OK;
}
#pragma endregion

void CLD_WaterArea::Register_LevelDesignSpecs()
{
    for (const _tchar* pObjectName : WATER_AREA_OBJECT_NAMES)
    {
        LD_SPAWN_SPEC Spec{};
        Spec.strObjectName = pObjectName;
        Spec.strPrototypeTag = PROTOTYPE_TAG;
        Spec.strLayerTag = LAYER_TAG;
        Spec.eCategory = LD_CATEGORY::VOLUME;
        Spec.wstrModelProtoTag = MODEL_PROTO_TAG;
        Spec.eModelType = MODEL::NONANIM;
        Spec.pPrototypeFactory = &Create_Prototype;
        Spec.pBuildDesc = &Build_Desc;

        if (JsonUtils::Equals_NoCase(pObjectName, OBJECT_NAME))
            Spec.pMakeDefaultDesc = &Make_DefaultDesc;

        Spec.ModelRequirements =
        {
                { MODEL_PROTO_TAG, WATER_MODEL_PATH, MODEL::NONANIM, false }
        };

        CLevelDesign_Registry::Register(Spec.strObjectName, Spec);
    }
}

_bool CLD_WaterArea::Build_Desc(const LD_OBJECT_DESC& CommonDesc, const json& jEntry, const LD_SPAWN_SPEC& Spec, LD_OBJECT_ENTRY* pOutEntry)
{
    UNREFERENCED_PARAMETER(jEntry);

    if (nullptr == pOutEntry)
        return false;

    if (!Is_WaterAreaObjectName(CommonDesc.strObjectName))
        return false;

    if (!JsonUtils::Equals_NoCase(Spec.strObjectName.c_str(), CommonDesc.strObjectName.c_str()))
        return false;

    if (Spec.strPrototypeTag != PROTOTYPE_TAG || Spec.strLayerTag != LAYER_TAG)
        return false;

    if (LD_CATEGORY::VOLUME != Spec.eCategory || MODEL::NONANIM != Spec.eModelType || Spec.wstrModelProtoTag != MODEL_PROTO_TAG)
        return false;

    LD_SURFACE_AREA_DESC Desc{};
    static_cast<LD_OBJECT_DESC&>(Desc) = CommonDesc;
    Desc.eCategory = Spec.eCategory;
    Desc.eModelType = Spec.eModelType;
    Desc.wstrModelProtoTag = Spec.wstrModelProtoTag;

    *pOutEntry = std::move(Desc);
    return true;
}

_bool CLD_WaterArea::Make_DefaultDesc(const LD_OBJECT_DESC& CommonDesc, _uint iModelProtoLevel,
    const LD_SPAWN_SPEC& Spec, LD_OBJECT_ENTRY* pOutEntry)
{
    if (!Build_Desc(CommonDesc, json::object(), Spec, pOutEntry))
        return false;

    LD_SURFACE_AREA_DESC* pDesc = get_if<LD_SURFACE_AREA_DESC>(pOutEntry);
    if (nullptr == pDesc)
        return false;

    pDesc->iModelProtoLevel = iModelProtoLevel;
    return true;
}

CGameObject* CLD_WaterArea::Create_Prototype(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    return CLD_WaterArea::Create(pDevice, pContext);
}

HRESULT CLD_WaterArea::Ready_RenderComponents()
{
    if (m_tSurfaceAreaDesc.wstrModelProtoTag.empty())
        return E_FAIL;

    m_pShaderCom = Add_Component<CShader>(Shader_Fluid.iLevelID, Shader_Fluid.szProtoTag, TEXT("Com_Shader"));
    if (nullptr == m_pShaderCom)
        return E_FAIL;

    m_pModelCom = Add_Component<CModel>(m_tSurfaceAreaDesc.iModelProtoLevel, m_tSurfaceAreaDesc.wstrModelProtoTag.c_str(),
        TEXT("Com_Model"));
    if (nullptr == m_pModelCom)
        return E_FAIL;

    return S_OK;
}

HRESULT CLD_WaterArea::Bind_ShaderResources()
{
    if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
        return E_FAIL;

    if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::VIEW, m_eProjType))))
        return E_FAIL;

    if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::PROJ, m_eProjType))))
        return E_FAIL;

    if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrixInverse", m_pGameInstance_Proxy->Get_InverseMatrix_Prespec(D3DTS::VIEW))))
        return E_FAIL;

    if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrixInverse", m_pGameInstance_Proxy->Get_InverseMatrix_Prespec(D3DTS::PROJ))))
        return E_FAIL;

    if (FAILED(m_pShaderCom->Bind_RawValue("g_vCamPosition", m_pGameInstance_Proxy->Get_CamPosition(), sizeof(_float4))))
        return E_FAIL;

    if (FAILED(m_pGameInstance_Proxy->Bind_RT_ShaderResource(TEXT("Target_Scene_SSR"), m_pShaderCom, "g_SceneTexture")))
        return E_FAIL;

    if (FAILED(m_pGameInstance_Proxy->Bind_RT_ShaderResource(TEXT("Target_Depth"), m_pShaderCom, "g_DepthTexture")))
        return E_FAIL;

    const ENVIRONMENT_DESC& Environment = m_pGameInstance_Proxy->Get_CurrentEnvironment();

    if (FAILED(m_pShaderCom->Bind_SRV("g_IrradianceCube", Environment.pDiffuseSRV)))
        return E_FAIL;

    if (FAILED(m_pShaderCom->Bind_SRV("g_PrefilteredCube", Environment.pSpecularSRV)))
        return E_FAIL;

    if (FAILED(m_pShaderCom->Bind_RawValue("g_iSpecularMip", &Environment.iSpecularMip, sizeof(_uint))))
        return E_FAIL;

    if (FAILED(m_pShaderCom->Bind_RawValue("g_fIBLIntensity", &Environment.fIntensity, sizeof(_float))))
        return E_FAIL;

    if (FAILED(m_pGameInstance_Proxy->Bind_ShaderGlobals(m_pShaderCom, { "g_vLightDir", "g_vLightSpecular" })))
        return E_FAIL;

    WATER_RENDER_DESC Desc = m_tWaterRenderDesc;
    Desc.fVisibility = 1.f - m_pCullingState->Get_Dissolve(CCullingState::CHANNEL::MAIN);

    if (FAILED(Bind_WaterRenderDesc(m_pShaderCom, Desc, m_pGameInstance_Proxy->Get_GameTime())))
        return E_FAIL;

    return S_OK;
}

HRESULT CLD_WaterArea::Render_Model()
{
    const _uint iNumMeshes = static_cast<_uint>(m_pModelCom->Get_NumMeshes());

    for (_uint i = 0; i < iNumMeshes; ++i)
    {
        if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_WaterNormalTexture1", i, MTEX_TYPE::NORMALS, 0)))
            return E_FAIL;

        if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_WaterNormalTexture2", i, MTEX_TYPE::NORMALS, 1)))
            return E_FAIL;

        if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_WaterCausticTexture", i, MTEX_TYPE::UNKNOWN, 0)))
            return E_FAIL;

        if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_WaterNoiseTexture", i, MTEX_TYPE::UNKNOWN, 1)))
            return E_FAIL;

        if (FAILED(m_pShaderCom->Begin(ETOUI(WATER_PASS::SURFACE))))
            return E_FAIL;

        if (FAILED(m_pModelCom->Render(i)))
            return E_FAIL;
    }

    return S_OK;
}

CLD_WaterArea* CLD_WaterArea::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CLD_WaterArea* pInstance = new CLD_WaterArea(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created : CLD_WaterArea");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CLD_WaterArea::Clone(void* pArg)
{
    CLD_WaterArea* pInstance = new CLD_WaterArea(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : CLD_WaterArea");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CLD_WaterArea::Collect_EditModelSlots(vector<EDITABLE_MODEL_SLOT>* pOutSlots) const
{
    UNREFERENCED_PARAMETER(pOutSlots);
}

void CLD_WaterArea::Free()
{
    __super::Free();
}

NS_END