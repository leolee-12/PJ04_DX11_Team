#include "Level_Edit.h"

#include "GameObject.h"
#include "GameInstance.h"
#include "GameInstance_Proxy.h"
#include "GameObject_Factory.h"
#include "EditCamera.h"
#include "MapStage.h"
#include "Map_EditHelper.h"

CLevel_Edit::CLevel_Edit(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CLevel{ pDevice, pContext }
{
}

HRESULT CLevel_Edit::Initialize()
{
    if (FAILED(__super::Initialize()))
        return E_FAIL;

    if (FAILED(Ready_EditLights()))
        return E_FAIL;

    if (FAILED(Ready_EditCamera()))
        return E_FAIL;

    return S_OK;
}

void CLevel_Edit::Update(_float fTimeDelta)
{
    if (m_pGameInstance_Proxy->Key_Down(DIK_F2))
        m_pGameInstance_Proxy->Toggle_DebugRender();

    if (m_bPreview && m_pCamera)
    {
        Client::CAM_POSE pose = m_Solver.Solve(XMLoadFloat3(&m_vKirby));
    }
}

HRESULT CLevel_Edit::Render()
{
    return S_OK;
}

_bool CLevel_Edit::Load_CameraDoc(const wstring& strPath)
{
    _bool b = m_Solver.Load(strPath);
    m_eSel = SEL::NONE; m_iSelArea = m_iSelRail = m_iSelNode = -1;
    return b;
}

_bool CLevel_Edit::Save_CameraDoc(const wstring& strPath)
{
    return m_Solver.Save(strPath);
}

void CLevel_Edit::Add_Layer(const wstring& strLayerTag)
{
    if (m_Layers.find(strLayerTag) == m_Layers.end())
        m_Layers[strLayerTag] = {};
}

void CLevel_Edit::Set_Preview(_bool b)
{
    m_bPreview = b;
}

void CLevel_Edit::Set_CameraActive(_bool b)
{
    if (m_pCamera)
        m_pCamera->Set_Active(b);
}

void CLevel_Edit::Get_PreviewViewProj(_float4x4* pView, _float4x4* pProj)
{
    Client::CAM_POSE p = m_Solver.Solve(XMLoadFloat3(&m_vKirby));

    _vector vEye = XMLoadFloat3(&p.eye);
    _vector vLook = XMVector3Normalize(XMLoadFloat3(&p.fwd));
    _vector vUp = XMLoadFloat3(&p.up);
    if (fabsf(XMVectorGetX(XMVector3Dot(vLook, vUp))) > 0.999f)   // look parallel to up guard
        vUp = XMVectorSet(0.f, 0.f, 1.f, 0.f);

    XMStoreFloat4x4(pView, XMMatrixLookToLH(vEye, vLook, vUp));
    _float fAspect = (_float)g_iWinSizeX / (_float)g_iWinSizeY;
    XMStoreFloat4x4(pProj, XMMatrixPerspectiveFovLH(XMConvertToRadians(p.fov), fAspect, 0.1f, 1000.f));
}

HRESULT CLevel_Edit::Ready_EditLights()
{
    LIGHT_DESC      LightDesc{};

    LightDesc.eType = LIGHT::DIRECTIONAL;
    LightDesc.vDiffuse = _float4(1.f, 1.f, 1.f, 1.f);
    LightDesc.vAmbient = _float4(0.2f, 0.2f, 0.2f, 1.f);
    LightDesc.vSpecular = _float4(1.f, 1.f, 1.f, 1.f);
    LightDesc.vDirection = _float4(0.4f, -1.f, 0.5f, 0.f);

    if (FAILED(m_pGameInstance_Proxy->Add_Light(LightDesc)))
        return E_FAIL;

    return S_OK;
}

HRESULT CLevel_Edit::Ready_EditCamera()
{
    if (!m_pGameInstance_Proxy->Has_Prototype(ETOUI(EDIT_LEVEL::STATIC), CEditCamera::PROTOTYPE_TAG))
    {
        m_pGameInstance_Proxy->Add_Prototype(
            ETOUI(EDIT_LEVEL::STATIC),
            CEditCamera::PROTOTYPE_TAG,
            CEditCamera::Create(m_pDevice, m_pContext));
    }

    // 오브젝트 매니저에 추가 (레이어는 카메라 전용)
    CGameObject* pCam = nullptr;
    CEditCamera::EDIT_CAMERA_FREE_DESC desc{};
    desc.vEye = { 0.f, 5.f, -10.f };
    desc.vAt = { 0.f, 0.f,   0.f };
    desc.fFovy = XMConvertToRadians(60.f);
    desc.fNear = 0.1f; desc.fFar = 1000.f;
    desc.fSpeedPerSec = 20.f;
    desc.fRotationPerSec = XMConvertToRadians(180.f);
    desc.fMouseSensor = 0.05f;

    m_pGameInstance_Proxy->Add_GameObject_Return(
        &pCam,
        ETOUI(EDIT_LEVEL::STATIC), CEditCamera::PROTOTYPE_TAG,
        ETOUI(EDIT_LEVEL::STATIC), L"Layer_Camera", L"Edit_Camera", &desc);

    m_pCamera = static_cast<CEditCamera*>(pCam);

    return S_OK;
}

_uint CLevel_Edit::Get_MapPreviewPresetCount() const
{
    return CMap_EditHelper::Get_MapPresetCount();
}

const _char* CLevel_Edit::Get_MapPreviewPresetLabel(_uint iPresetIndex) const
{
    return CMap_EditHelper::Get_MapPresetLabel(iPresetIndex);
}

HRESULT CLevel_Edit::Load_MapPreview(_uint iPresetIndex)
{
    Clear_MapPreview();

    Client::CMap_EditHelper::MAP_PRESET_LOAD_REPORT Report{};
    CMapStage* pLoadedStage = nullptr;
    const HRESULT hResult = CMap_EditHelper::Load_MapPreset(
        m_pDevice,
        m_pContext,
        iPresetIndex,
        ETOUI(EDIT_LEVEL::EDIT),
        ETOUI(LEVEL::GAMEPLAY),
        &On_MapPreviewObjectCreated,
        this,
        &Report,
        &pLoadedStage);

    if (FAILED(hResult))
    {
        m_pMapStage = nullptr;
        m_strLoadedMapStageName.clear();
        m_iEnvObjCreatedCount = 0;
        m_iLoadedMapPresetIndex = -1;
        m_strMapPreviewStatus = L"Map preset load failed.";
        return hResult;
    }

    m_pMapStage = pLoadedStage;
    m_strLoadedMapStageName = Report.strStageName;
    if (m_strLoadedMapStageName.empty())
        m_strLoadedMapStageName = StrToWstr(Get_MapPreviewPresetLabel(iPresetIndex));

    m_iEnvObjCreatedCount = Report.iEnvCreatedCount;
    m_iLoadedMapPresetIndex = static_cast<_int>(iPresetIndex);
    m_strMapPreviewStatus = L"Map preset loaded: " + m_strLoadedMapStageName
        + L" / env=" + to_wstring(m_iEnvObjCreatedCount);

    if (0 != Report.iEnvJsonFailedCount
        || 0 != Report.iEnvSkippedMissingModel
        || 0 != Report.iEnvSkippedCreateFailed)
    {
        m_strMapPreviewStatus += L" / warnings";
    }

    return hResult;
}

HRESULT CLevel_Edit::Load_MapPreviewStage(_uint iPresetIndex)
{
    Clear_MapPreviewStage();
    m_iLoadedMapPresetIndex = -1;

    CMapStage* pLoadedStage = nullptr;
    _wstring strStageName;
    const HRESULT hResult = CMap_EditHelper::Load_MapStage(
        m_pDevice,
        m_pContext,
        iPresetIndex,
        ETOUI(EDIT_LEVEL::EDIT),
        ETOUI(LEVEL::GAMEPLAY),
        &On_MapPreviewObjectCreated,
        this,
        &pLoadedStage,
        &strStageName);

    if (FAILED(hResult))
    {
        m_pMapStage = nullptr;
        m_strLoadedMapStageName.clear();
        m_strMapPreviewStatus = 0 != m_iEnvObjCreatedCount
            ? L"Map stage preview load failed. / env=" + to_wstring(m_iEnvObjCreatedCount)
            : L"Map stage preview load failed.";
        return hResult;
    }

    m_pMapStage = pLoadedStage;
    m_strLoadedMapStageName = strStageName;
    if (m_strLoadedMapStageName.empty())
        m_strLoadedMapStageName = StrToWstr(Get_MapPreviewPresetLabel(iPresetIndex));

    m_strMapPreviewStatus = L"Map stage preview loaded: " + m_strLoadedMapStageName
        + L" / env=" + to_wstring(m_iEnvObjCreatedCount);

    return hResult;
}

HRESULT CLevel_Edit::Load_MapPreviewEnv(_uint iPresetIndex)
{
    Clear_MapPreviewEnv();
    m_iLoadedMapPresetIndex = -1;

    Client::CMap_EditHelper::MAP_PRESET_LOAD_REPORT Report{};
    const HRESULT hResult = CMap_EditHelper::Load_EnvObject_FromJson(
        m_pDevice,
        m_pContext,
        iPresetIndex,
        ETOUI(EDIT_LEVEL::EDIT),
        &On_MapPreviewObjectCreated,
        this,
        &Report);

    if (FAILED(hResult))
    {
        m_iEnvObjCreatedCount = 0;
        m_strMapPreviewStatus = nullptr != m_pMapStage
            ? L"Environment preview load failed. / stage=" + m_strLoadedMapStageName
            : L"Environment preview load failed.";
        return hResult;
    }

    m_iEnvObjCreatedCount = Report.iEnvCreatedCount;
    m_strMapPreviewStatus = L"Environment preview loaded: env=" + to_wstring(m_iEnvObjCreatedCount);
    if (!m_strLoadedMapStageName.empty())
        m_strMapPreviewStatus += L" / stage=" + m_strLoadedMapStageName;

    if (0 != Report.iEnvJsonFailedCount
        || 0 != Report.iEnvSkippedMissingModel
        || 0 != Report.iEnvSkippedCreateFailed)
    {
        m_strMapPreviewStatus += L" / warnings";
    }

    return hResult;
}

void CLevel_Edit::Clear_MapPreview()
{
    vector<wstring> MapLayers;
    MapLayers.reserve(m_Layers.size());

    for (const auto& Pair : m_Layers)
    {
        if (CMap_EditHelper::Is_MapLayer(Pair.first))
            MapLayers.push_back(Pair.first);
    }

    for (const wstring& strLayerTag : MapLayers)
        Clear_MapPreviewLayer(strLayerTag);

    m_pMapStage = nullptr;
    m_strLoadedMapStageName.clear();
    m_iEnvObjCreatedCount = 0;
    m_iLoadedMapPresetIndex = -1;
    m_MapPreviewObjects.clear();
    m_strMapPreviewStatus = L"Map preset not loaded.";
}

void CLevel_Edit::Clear_MapPreviewStage()
{
    Clear_MapPreviewLayer(L"Layer_MapStage");

    m_pMapStage = nullptr;
    m_strLoadedMapStageName.clear();
    m_iLoadedMapPresetIndex = -1;

    if (0 != m_iEnvObjCreatedCount)
        m_strMapPreviewStatus = L"Environment preview loaded only. / env=" + to_wstring(m_iEnvObjCreatedCount);
    else
        m_strMapPreviewStatus = L"Map preset not loaded.";
}

void CLevel_Edit::Clear_MapPreviewEnv()
{
    static const _tchar* kEnvLayers[] =
    {
        L"Layer_EnvStatic",
        L"Layer_EnvInteract",
        L"Layer_EnvEffect"
    };

    for (const _tchar* pLayerTag : kEnvLayers)
        Clear_MapPreviewLayer(pLayerTag);

    m_iEnvObjCreatedCount = 0;
    m_iLoadedMapPresetIndex = -1;

    if (nullptr != m_pMapStage)
    {
        const _wstring strStageName = m_strLoadedMapStageName.empty()
            ? L"(loaded stage)"
            : m_strLoadedMapStageName;
        m_strMapPreviewStatus = L"Map stage preview loaded: " + strStageName + L" / env=0";
    }
    else
    {
        m_strMapPreviewStatus = L"Map preset not loaded.";
    }
}

void CLevel_Edit::Clear_MapPreviewLayer(const _wstring& strLayerTag)
{
    auto iter = m_Layers.find(strLayerTag);
    if (iter == m_Layers.end())
        return;

    auto& Objects = iter->second;
    for (auto ObjectIter = Objects.begin(); ObjectIter != Objects.end();)
    {
        CGameObject* pObject = ObjectIter->pObject;
        if (m_MapPreviewObjects.find(pObject) == m_MapPreviewObjects.end())
        {
            ++ObjectIter;
            continue;
        }

        if (m_pSelected == pObject)
            m_pSelected = nullptr;

        m_MapPreviewObjects.erase(pObject);
        m_pGameInstance_Proxy->Destroy_GameObject(pObject);
        ObjectIter = Objects.erase(ObjectIter);
    }

    if (Objects.empty())
        m_Layers.erase(iter);
}

void CLevel_Edit::Add_MapPreviewObjectHandle(const _wstring& strPrototypeTag, const _wstring& strLayerTag, const _wstring& strObjectTag, CGameObject* pObject)
{
    if (nullptr == pObject)
        return;

    Add_Layer(strLayerTag);
    m_Layers[strLayerTag].push_back({ strPrototypeTag, strObjectTag, pObject });
    m_MapPreviewObjects.insert(pObject);
}

void CLevel_Edit::On_MapPreviewObjectCreated(void* pContext, CGameObject* pObject,
    const _wstring& strPrototypeTag, const _wstring& strLayerTag, const _wstring& strObjectTag)
{
    CLevel_Edit* pLevel = static_cast<CLevel_Edit*>(pContext);
    if (nullptr == pLevel)
        return;

    pLevel->Add_MapPreviewObjectHandle(strPrototypeTag, strLayerTag, strObjectTag, pObject);

    if (Client::CMapStage::PROTOTYPE_TAG == strPrototypeTag)
        pLevel->m_pMapStage = dynamic_cast<Client::CMapStage*>(pObject);
}

CLevel_Edit* CLevel_Edit::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CLevel_Edit* pInstance = new CLevel_Edit(pDevice, pContext);

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Created : CLevel_Edit");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CLevel_Edit::Free()
{
    __super::Free();
}


