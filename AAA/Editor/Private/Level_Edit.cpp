#include "Level_Edit.h"
#include "Loader.h"

#include "GameObject.h"
#include "GameInstance.h"
#include "GameInstance_Proxy.h"

#include "GameObject_Factory.h"
#include "DataExporter.h"
#include "DataLoader.h"
#include "EditCamera.h"
#include "Edit_Grid.h"
#include "NavMesh_Editor.h"
#include "MapStage.h"
#include "Map_EditHelper.h"

#include "Effect_Container.h"

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

    if (FAILED(Ready_EditGrid()))
        return E_FAIL;

    return S_OK;
}

void CLevel_Edit::Update(_float fTimeDelta)
{
    if (m_pGameInstance_Proxy->Key_Down(DIK_F2))
        m_pGameInstance_Proxy->Toggle_DebugRender();
}

HRESULT CLevel_Edit::Render()
{
    if (m_pGrid)
    {
        const _float4x4* pView = m_pGameInstance_Proxy->Get_Matrix(D3DTS::VIEW, PROJ_TYPE::PERSPEC);
        const _float4x4* pProj = m_pGameInstance_Proxy->Get_Matrix(D3DTS::PROJ, PROJ_TYPE::PERSPEC);
        m_pGrid->Render(pView, pProj);
    }
    return S_OK;
}

CGameObject* CLevel_Edit::Spawn_Object(const wstring& strProtoTag, const wstring& strLayerTag, const wstring& strName, void* pArg)
{
    auto* pReg = CGameObject_Factory::GetInstance()->Get_Registration(strProtoTag);
    if (!pReg) return nullptr;

    if (!m_pGameInstance_Proxy->Has_Prototype(ETOUI(EDIT_LEVEL::EDIT), strProtoTag))
    {
        pReg->ResourceLoader(m_pGameInstance_Proxy, m_pDevice, m_pContext);

        m_pGameInstance_Proxy->Add_Prototype(ETOUI(EDIT_LEVEL::EDIT), strProtoTag.c_str(),
            pReg->CreatorFunc(m_pDevice, m_pContext));
    }

    wstring strFinalLayer = strLayerTag;
    if (pReg->strCategory == L"CAMERA_OBJECT")
        strFinalLayer = L"Layer_Camera";
    else if (pReg->strCategory == L"Effect_Container") ////////
        strFinalLayer = L"Layer_Effect";

    Engine::CGameObject* pObj = { nullptr };
    m_pGameInstance_Proxy->Add_GameObject_Return(
        &pObj,
        ETOUI(EDIT_LEVEL::EDIT), strProtoTag.c_str(),
        ETOUI(EDIT_LEVEL::EDIT), strFinalLayer.c_str(), strName, pArg);

    Add_Layer(strFinalLayer);
    m_Layers[strFinalLayer].push_back({ strProtoTag, strName, pObj });

    return pObj;
}

void CLevel_Edit::Save_Level(const wstring& strFilePath, const wstring& strLevelTag)
{
    json jLevel;
    jLevel["Level_Tag"] = WstrToStr(strLevelTag);

    json jObjects = json::array();

    for (auto& [LayerTag, Objects] : m_Layers)
    {
        for (auto& handle : Objects)
        {
            if (m_MapPreviewObjects.find(handle.pObject) != m_MapPreviewObjects.end())
                continue;

            json jObj = handle.pObject->Serialize();

            jObj["Object_Tag"]      = WstrToStr(handle.strName);
            jObj["Prototype_Tag"]   = WstrToStr(handle.strPrototypeTag);
            jObj["Layer_Tag"]       = WstrToStr(LayerTag);

            jObjects.push_back(jObj);
        }
    }

    jLevel["Objects"] = jObjects;

    if (0 <= m_iLoadedMapPresetIndex)
    {
        jLevel["MapContent"] = { { "PresetIndex", m_iLoadedMapPresetIndex } };
    }

    CDataExporter::Write_JsonFile(strFilePath.c_str(), jLevel);
}

void CLevel_Edit::Load_Level(const wstring& strFilePath)
{
    m_pSelected = nullptr;
    m_Layers.clear();
    m_pGameInstance_Proxy->Clear_Objects(ETOUI(EDIT_LEVEL::EDIT));

    m_pMapStage = nullptr;
    m_strLoadedMapStageName.clear();
    m_iEnvObjCreatedCount = 0;
    m_iLoadedMapPresetIndex = -1;
    m_MapPreviewObjects.clear();
    m_strMapPreviewStatus = L"Map preset not loaded.";

    string strContent = {};
    if (FAILED(CDataLoader::Read_Json(strFilePath.c_str(), &strContent)))
        return;

    try
    {
        json jLevel = json::parse(strContent);

        for (auto& jObj : jLevel["Objects"])
        {
            string strProto = jObj["Prototype_Tag"].get<string>();
            string strLayer = jObj["Layer_Tag"].get<string>();
            string strName = jObj["Object_Tag"].get<string>();

            wstring wProto = StrToWstr(strProto);
            wstring wLayer = StrToWstr(strLayer);
            wstring wName  = StrToWstr(strName);

            CGameObject* pObj = Spawn_Object(wProto, wLayer, wName);

            if(pObj)
                pObj->Deserialize(jObj);
        }

        if (jLevel.contains("MapContent") && jLevel["MapContent"].is_object())
        {
            const json& jMap = jLevel["MapContent"];
            if (jMap.contains("PresetIndex") && jMap["PresetIndex"].is_number_integer())
            {
                const int iPresetIndex = jMap["PresetIndex"].get<int>();
                if (0 <= iPresetIndex)
                    Load_MapPreview(static_cast<_uint>(iPresetIndex));
            }
        }
    }
    catch (json::exception&)
    {
        MSG_BOX("JSON PARSE ERROR");
    }
}

void CLevel_Edit::Save_LiveObjects(const wstring& strFilePath, const wstring& strLevelTag)
{
    json jLevel;
    jLevel["Level_Tag"] = WstrToStr(strLevelTag);

    json jObjects = json::array();

    // 오브젝트 레이어만 직렬화 (맵/카메라/이펙트/UI 레이어 제외)
    auto iter = m_Layers.find(OBJECT_LAYER_TAG);
    if (iter != m_Layers.end())
    {
        const wstring& LayerTag = iter->first;
        for (auto& handle : iter->second)
        {
            // 맵 프리뷰로 올라온 객체는 저장 대상 아님
            if (m_MapPreviewObjects.find(handle.pObject) != m_MapPreviewObjects.end())
                continue;

            json jObj = handle.pObject->Serialize();

            jObj["Object_Tag"] = WstrToStr(handle.strName);
            jObj["Prototype_Tag"] = WstrToStr(handle.strPrototypeTag);
            jObj["Layer_Tag"] = WstrToStr(LayerTag);

            jObjects.push_back(jObj);
        }
    }

    jLevel["Objects"] = jObjects;

    CDataExporter::Write_JsonFile(strFilePath.c_str(), jLevel);
}

void CLevel_Edit::Load_LiveObjects(const wstring& strFilePath)
{
    string strContent = {};
    if (FAILED(CDataLoader::Read_Json(strFilePath.c_str(), &strContent)))
        return;

    // 기존 오브젝트 레이어만 비우기 (맵 프리뷰/카메라/이펙트/UI는 유지)
    auto iter = m_Layers.find(L"Layer_Object");
    if (iter != m_Layers.end())
    {
        for (auto& handle : iter->second)
        {
            if (m_pSelected == handle.pObject)
                m_pSelected = nullptr;

            // 혹시라도 맵 프리뷰로 추적 중인 객체면 집합에서도 제거
            m_MapPreviewObjects.erase(handle.pObject);

            m_pGameInstance_Proxy->Destroy_GameObject(handle.pObject);
        }
        iter->second.clear();
    }

    try
    {
        json jLevel = json::parse(strContent);

        for (auto& jObj : jLevel["Objects"])
        {
            string strProto = jObj["Prototype_Tag"].get<string>();
            string strName = jObj["Object_Tag"].get<string>();

            wstring wProto = StrToWstr(strProto);
            wstring wName = StrToWstr(strName);

            // 레이어는 항상 오브젝트 레이어로 고정해서 스폰
            CGameObject* pObj = Spawn_Object(wProto, OBJECT_LAYER_TAG, wName);

            if (pObj)
                pObj->Deserialize(jObj);
        }
    }
    catch (json::exception&)
    {
        MSG_BOX("JSON PARSE ERROR");
    }
}

void CLevel_Edit::Add_Layer(const wstring& strLayerTag)
{
    if (m_Layers.find(strLayerTag) == m_Layers.end())
        m_Layers[strLayerTag] = {};
}

HRESULT CLevel_Edit::Remove_Layer(const wstring& strLayerTag)
{
    auto iter = m_Layers.find(strLayerTag);
    if (iter == m_Layers.end()) return E_FAIL;

    if (!iter->second.empty()) {
        MSG_BOX("The layer contains objects");
        return E_FAIL;
    }

    m_Layers.erase(iter);
    return S_OK;
}

void CLevel_Edit::Change_ObjectLayer(CGameObject* pObject, const wstring& strNewLayer)
{
    if (!pObject) return;
    if (m_MapPreviewObjects.find(pObject) != m_MapPreviewObjects.end()) return;
    if (!m_MapPreviewObjects.empty() && CMap_EditHelper::Is_MapLayer(strNewLayer)) return;

    Add_Layer(strNewLayer);

    for (auto& [LayerTag, Objects] : m_Layers)
    {
        for (auto iter = Objects.begin(); iter != Objects.end(); ++iter)
        {
            if (iter->pObject == pObject)
            {
                if (LayerTag == strNewLayer) return;

                EDITOR_OBJECT_HANDLE handle = *iter;

                Objects.erase(iter);

                m_Layers[strNewLayer].push_back(handle);

                return;
            }
        }
    }
}

void CLevel_Edit::Delete_Object(CGameObject* pObject)
{
    if (!pObject) return;

    if (m_pSelected == pObject)
        m_pSelected = nullptr;

    for (auto& [LayerTag, Objects] : m_Layers)
    {
        for (auto iter = Objects.begin(); iter != Objects.end(); ++iter)
        {
            if (iter->pObject == pObject)
            {
                const _bool bWasMapPreviewObject = 0 < m_MapPreviewObjects.erase(pObject);
                if (bWasMapPreviewObject)
                {
                    if (pObject == m_pMapStage)
                    {
                        m_pMapStage = nullptr;
                        m_strLoadedMapStageName.clear();
                    }
                    else if (m_iEnvObjCreatedCount > 0)
                    {
                        --m_iEnvObjCreatedCount;
                    }

                    m_strMapPreviewStatus = L"Map preset object removed.";
                }

                Objects.erase(iter);
                m_pGameInstance_Proxy->Destroy_GameObject(pObject);
                return;
            }
        }
    }
}

void CLevel_Edit::Pick_And_Place(_fvector vOrigin, _fvector vDir)
{
    /*if (m_bNavEditMode)
    {
        _float3 fHitPos = {};
        _float  fDummy = {};
        if (m_pLumia->Pick_Floor(vOrigin, vDir, &fHitPos, &fDummy))
            m_pNavMeshEditor->OnClick(fHitPos);
        return;
    }*/

    _float3 fHitPos = {};
    if (!m_pGameInstance_Proxy->Pick_RayToPlane(vOrigin, vDir, &fHitPos))
        return;

    if (!Is_PlaceMode()) return;
    fHitPos.x = roundf(fHitPos.x);
    fHitPos.z = roundf(fHitPos.z);
    fHitPos.y = 0.f;
    Place_Object_At(fHitPos);
}

void CLevel_Edit::Place_Object_At(const _float3& vPos)
{
    wstring strName = m_strPendingProto + L"_" + to_wstring(m_iPlaceCount++);

    CGameObject* pObj = Spawn_Object(m_strPendingProto, m_strPendingLayer, strName);
    if (pObj)
    {
        pObj->Get_Transform()->Set_State(
            STATE::POSITION,
            XMVectorSet(vPos.x, vPos.y, vPos.z, 1.f)
        );
        pObj->Initialize_NaviPlacement();
    }
}

void CLevel_Edit::Begin_PlaceMode(const wstring& strProtoTag, const wstring& strLayerTag)
{
    m_ePlaceMode = PLACE_MODE::PENDING;
    m_strPendingLayer = strLayerTag;
    m_strPendingProto = strProtoTag;
}

void CLevel_Edit::End_PlaceMode()
{
    m_ePlaceMode = PLACE_MODE::NONE;
    m_strPendingLayer = {};
    m_strPendingProto = {};
}

void CLevel_Edit::Set_CameraActive(_bool b)
{
    if (m_pCamera)
        m_pCamera->Set_Active(b);
}

void CLevel_Edit::Preview_Camera(CGameObject* pCam)
{
    if (!pCam) return;

    Set_CameraActive(false);

    // 카메라 레이어 전부 Off
    auto it = m_Layers.find(L"Layer_Camera");
    if (it != m_Layers.end())
    {
        for (auto& handle : it->second)
        {
            if (handle.pObject)
                handle.pObject->Set_Active(false);
        }
    }

    pCam->Set_Active(true);
}

void CLevel_Edit::Back_To_Edit()
{
    auto it = m_Layers.find(L"Layer_Camera");
    if (it != m_Layers.end())
    {
        for (auto& handle : it->second)
        {
            if (handle.pObject && handle.pObject != m_pCamera)
                handle.pObject->Set_Active(false);
        }
    }

    Set_CameraActive(true);
}

const vector<CLevel_Edit::EDITOR_OBJECT_HANDLE>* CLevel_Edit::Get_CameraLayer() const
{
    auto it = m_Layers.find(L"Layer_Camera");
    return (it != m_Layers.end()) ? &it->second : nullptr;
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

HRESULT CLevel_Edit::Ready_EditGrid()
{
    m_pGrid = CEdit_Grid::Create(m_pDevice, m_pContext, 100, 1.f);
    return (m_pGrid == nullptr) ? E_FAIL : S_OK;
}

_uint CLevel_Edit::Get_MapPreviewPresetCount() const
{
    return CMap_EditHelper::Get_MapPresetCount();
}

const _char* CLevel_Edit::Get_MapPreviewPresetLabel(_uint iPresetIndex) const
{
    return CMap_EditHelper::Get_MapPresetLabel(iPresetIndex);
}

HRESULT CLevel_Edit::Save_Selected_Effect(const wstring& strFilePath)
{
    CEffect_Container* pEffect = dynamic_cast<CEffect_Container*>(m_pSelected);
    if (pEffect == nullptr)
        return E_FAIL;

    wstring strPrototypeTag;
    wstring strObjectTag;
    wstring strLayerTag;
    _bool bFound = false;

    for (auto& [LayerTag, Objects] : m_Layers)
    {
        for (auto& handle : Objects)
        {
            if (handle.pObject != m_pSelected)
                continue;

            strPrototypeTag = handle.strPrototypeTag;
            strObjectTag = handle.strName;
            strLayerTag = LayerTag;
            bFound = true;
            break;
        }

        if (bFound)
            break;
    }

    if (bFound == false)
        return E_FAIL;

    json jEffect = pEffect->Serialize();
    jEffect["Prototype_Tag"] = WstrToStr(strPrototypeTag);
    jEffect["Object_Tag"] = WstrToStr(strObjectTag);
    jEffect["Layer_Tag"] = WstrToStr(strLayerTag);

    return CDataExporter::Write_JsonFile(strFilePath.c_str(), jEffect);
}

HRESULT CLevel_Edit::Load_Selected_Effect(const wstring& strFilePath)
{
    string strContent;
    if (FAILED(CDataLoader::Read_Json(strFilePath.c_str(), &strContent)))
        return E_FAIL;

    try
    {
        json jEffect = json::parse(strContent);

        if (!jEffect.contains("Prototype_Tag"))
            return E_FAIL;

        wstring strProtoTag = StrToWstr(jEffect["Prototype_Tag"].get<string>());

        wstring strLayerTag = L"Layer_Effect";
        if (jEffect.contains("Layer_Tag"))
            strLayerTag = StrToWstr(jEffect["Layer_Tag"].get<string>());

        wstring strObjectTag = strProtoTag + L"_Loaded";
        if (jEffect.contains("Object_Tag"))
            strObjectTag = StrToWstr(jEffect["Object_Tag"].get<string>());

        // 이름 충돌 방지
        wstring strFinalObjectTag = strObjectTag;
        _uint iSuffix = 1;

        auto IsNameUsed = [&](const wstring& strName) -> _bool
            {
                for (auto& [LayerTag, Objects] : m_Layers)
                {
                    for (auto& handle : Objects)
                    {
                        if (handle.strName == strName)
                            return true;
                    }
                }
                return false;
            };

        while (IsNameUsed(strFinalObjectTag))
            strFinalObjectTag = strObjectTag + L"_" + to_wstring(iSuffix++);

        CGameObject* pObj = Spawn_Object(strProtoTag, strLayerTag, strFinalObjectTag);
        if (nullptr == pObj)
            return E_FAIL;

        pObj->Deserialize(jEffect);
        m_pSelected = pObj;
    }
    catch (json::exception&)
    {
        MSG_BOX("JSON PARSE ERROR");
        return E_FAIL;
    }

    return S_OK;
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
    Safe_Release(m_pGrid);
}


