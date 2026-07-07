#include "Level_Edit.h"
#include "Loader.h"

#include "GameObject.h"
#include "GameInstance.h"

#include "GameObject_Factory.h"
#include "DataExporter.h"
#include "DataLoader.h"
#include "EditCamera.h"
#include "Edit_Grid.h"
#include "MapStage.h"
#include "Map_Loader.h"
#include "Map_EditSession.h"
#include "LevelDesign_Registry.h"
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

    m_pMapPreviewSession = CMap_EditSession::Create();
    if (nullptr == m_pMapPreviewSession)
        return E_FAIL;

    if (FAILED(CMap_Loader::Ready_TexHub(m_pGameInstance_Proxy)))
        return E_FAIL;

    return S_OK;
}

void CLevel_Edit::Update(_float fTimeDelta)
{
#ifdef _DEBUG
    if (m_pGameInstance_Proxy->Key_Down(DIK_F2))
        m_pGameInstance_Proxy->Toggle_DebugRender();

    if (m_pGameInstance_Proxy->Key_Down(DIK_F7))
    {
        KIRBY_ATTACHMENT_END_DESC Desc{};
        Desc.eType =KIRBY_ATTACHMENT_END_REASON::GORILLA_SCENE_HANDOFF;

        m_pGameInstance_Proxy->Publish(EventTag::Kirby_AttachmentEnd, &Desc);
    }
#endif
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
        pReg->ResourceLoader(m_pGameInstance_Proxy, m_pDevice, m_pContext, ETOUI(EDIT_LEVEL::EDIT));

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
            if (Should_SkipMapObjectForLevelSave(handle.pObject))
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

void CLevel_Edit::Load_Level(const wstring& strFilePath)
{
    m_pSelected = nullptr;
    m_Layers.clear();
    m_pGameInstance_Proxy->Clear_Objects(ETOUI(EDIT_LEVEL::EDIT));

    if (nullptr != m_pMapPreviewSession)
        m_pMapPreviewSession->Reset();

    m_pMapStage = nullptr;
    m_MapPreviewObjects.clear();

    string strContent = {};
    if (FAILED(CDataLoader::Read_Json(strFilePath.c_str(), &strContent)))
        return;

    try
    {
        json jLevel = json::parse(strContent);

        if (jLevel.contains("Objects") && jLevel["Objects"].is_array())
        {
            for (auto& jObj : jLevel["Objects"])
            {
                string strProto = jObj["Prototype_Tag"].get<string>();
                string strLayer = jObj["Layer_Tag"].get<string>();
                string strName = jObj["Object_Tag"].get<string>();

                wstring wProto = StrToWstr(strProto);
                wstring wLayer = StrToWstr(strLayer);
                wstring wName = StrToWstr(strName);

                CGameObject* pObj = Spawn_Object(wProto, wLayer, wName);

                if (pObj)
                    pObj->Deserialize(jObj);
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
    auto iter = m_Layers.find(OBJECT_LAYER_TAG);
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
    if (!m_MapPreviewObjects.empty() && CMap_Loader::Is_MapLayer(strNewLayer)) return;

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
                Handle_MapSpecificDeletion(pObject);

                Objects.erase(iter);
                m_pGameInstance_Proxy->Destroy_GameObject(pObject);
                return;
            }
        }
    }
}

void CLevel_Edit::Pick_And_Place(_fvector vOrigin, _fvector vDir)
{
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
    LIGHT_DESC LightDesc{};

    LightDesc.eType = LIGHT::DIRECTIONAL;
    LightDesc.vDiffuse = _float4(6.2f, 6.15f, 5.76f, 1.f);
    //LightDesc.vAmbient = _float4(1.72f, 0.82f, 0.41f, 1.f);
    LightDesc.vAmbient = _float4(0.f, 0.f, 0.f, 1.f);
    LightDesc.vSpecular = _float4(1.f, 1.f, 1.f, 1.f);
    LightDesc.vDirection = _float4(0.557f, -0.766f, 0.321f, 0.f);

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

_bool CLevel_Edit::Is_MapPreviewLoaded() const
{
    return nullptr != m_pMapPreviewSession
        && m_pMapPreviewSession->Is_PreviewLoaded();
}

const _wstring& CLevel_Edit::Get_MapPreviewStatus() const
{
    static const _wstring s_strDefault = L"Map preset not loaded.";
    return nullptr != m_pMapPreviewSession
        ? m_pMapPreviewSession->Get_PreviewStatus()
        : s_strDefault;
}

const _wstring& CLevel_Edit::Get_LoadedMapPreviewStageName() const
{
    static const _wstring s_strEmpty;
    return nullptr != m_pMapPreviewSession
        ? m_pMapPreviewSession->Get_LoadedStageName()
        : s_strEmpty;
}

_uint CLevel_Edit::Get_MapPreviewEnvCreatedCount() const
{
    return nullptr != m_pMapPreviewSession
        ? m_pMapPreviewSession->Get_EnvCreatedCount()
        : 0;
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
    if (nullptr != m_pMapPreviewSession)
    {
        const auto& MapContentDesc = m_pMapPreviewSession->Get_EditData();
        if (!MapContentDesc.bHasMapContent
            || MapContentDesc.iPresetIndex != static_cast<_int>(iPresetIndex))
        {
            m_pMapPreviewSession->Reset();
        }
    }

    Clear_MapPreview();

    Client::MAP_EDIT_DATA ResolvedMapContentDesc{};
    if (nullptr != m_pMapPreviewSession)
        ResolvedMapContentDesc = m_pMapPreviewSession->Get_EditData();

    ResolvedMapContentDesc.bHasMapContent = true;
    ResolvedMapContentDesc.iPresetIndex = static_cast<_int>(iPresetIndex);

    const HRESULT hSavedOverride = CMap_Loader::Load_PresetEditFile(iPresetIndex, ResolvedMapContentDesc.strManifestPath, &ResolvedMapContentDesc);

    if (FAILED(hSavedOverride))
    {
        if (nullptr != m_pMapPreviewSession)
            m_pMapPreviewSession->Set_PreviewStatus(L"Map edit file parse failed.");
        return E_FAIL;
    }

    if (ResolvedMapContentDesc.strManifestPath.empty())
    {
        if (FAILED(CMap_Loader::Get_MapManifestPath(
            iPresetIndex,
            &ResolvedMapContentDesc.strManifestPath)))
        {
            if (nullptr != m_pMapPreviewSession)
                m_pMapPreviewSession->Set_PreviewStatus(L"Map preset load failed.");
            return E_FAIL;
        }
    }

    if (nullptr != m_pMapPreviewSession)
        m_pMapPreviewSession->Set_EditData(ResolvedMapContentDesc);

    Client::MAP_RUNTIME_LOAD_CONTEXT Context{};
    Context.pDevice = m_pDevice;
    Context.pContext = m_pContext;
    Context.iPlaceLevel = ETOUI(EDIT_LEVEL::EDIT);
    Context.iModelLevel = ETOUI(LEVEL::STATIC);
    Context.pCreatedCallback = &On_MapPreviewObjectCreated;
    Context.pCallbackContext = this;

    CMapStage* pLoadedStage = nullptr;
    HRESULT hResult = CMap_Loader::Load_MapStage_Runtime(
        Context,
        ResolvedMapContentDesc.strManifestPath,
        &pLoadedStage);

    if (FAILED(hResult))
    {
        m_pMapStage = nullptr;

        if (nullptr != m_pMapPreviewSession)
        {
            m_pMapPreviewSession->Set_LoadStageEnabled(false);
            m_pMapPreviewSession->Set_LoadEnvEnabled(false);
            m_pMapPreviewSession->Clear_RuntimeState();
            m_pMapPreviewSession->Set_PreviewStatus(L"Map preset load failed.");
        }

        return hResult;
    }

    Client::MAP_LOAD_RESULT Report{};
    vector<Client::ENV_OBJECT_DESC> DeletedEnvDescs;
    hResult = CMap_Loader::Load_Env_Runtime(
        Context,
        ResolvedMapContentDesc.strManifestPath,
        &ResolvedMapContentDesc.OverrideDesc,
        &Report,
        &DeletedEnvDescs);

    if (FAILED(hResult))
    {
        Clear_MapPreview();

        if (nullptr != m_pMapPreviewSession)
            m_pMapPreviewSession->Set_PreviewStatus(L"Map preset load failed.");

        return hResult;
    }

    if (FAILED(Load_LDPreview(iPresetIndex)))
    {
        if (nullptr != m_pMapPreviewSession)
            m_pMapPreviewSession->Set_PreviewStatus(L"Map preset LevelDesign load failed.");
        return E_FAIL;
    }

    m_pMapStage = pLoadedStage;

    if (nullptr != m_pMapPreviewSession)
    {
        _wstring strStageName =
            (nullptr != m_pMapStage) ? m_pMapStage->Get_StageName() : Report.strStageName;

        if (strStageName.empty())
            strStageName = StrToWstr(CMap_Loader::Get_MapName(iPresetIndex));

        m_pMapPreviewSession->Set_PresetIndex(static_cast<_int>(iPresetIndex));
        m_pMapPreviewSession->Set_ManifestPath(ResolvedMapContentDesc.strManifestPath);
        m_pMapPreviewSession->Set_LoadStageEnabled(true);
        m_pMapPreviewSession->Set_LoadEnvEnabled(true);
        m_pMapPreviewSession->Rebuild_DeletedEnvItems(DeletedEnvDescs);
        m_pMapPreviewSession->Set_LoadedStageName(strStageName);
        m_pMapPreviewSession->Set_EnvLoaded(0 < Report.iEnvCreatedCount);
        m_pMapPreviewSession->Set_EnvCreatedCount(Report.iEnvCreatedCount);

        _wstring strStatus = L"Map preset loaded: " + strStageName
            + L" / env=" + to_wstring(Report.iEnvCreatedCount)
            + L" / levelDesign=loaded";

        if (S_OK == hSavedOverride)
            strStatus += L" / edit-file";

        if (0 != Report.iEnvSkippedMissingModel
            || 0 != Report.iEnvSkippedCreateFailed)
        {
            strStatus += L" / warnings";
        }

        m_pMapPreviewSession->Set_PreviewStatus(strStatus);
    }

    return hResult;
}

HRESULT CLevel_Edit::Load_MapPreviewStage(_uint iPresetIndex)
{
    _bool bPresetChanged = false;
    _bool bPreviousLoadEnv = false;

    if (nullptr != m_pMapPreviewSession)
    {
        const auto& MapContentDesc = m_pMapPreviewSession->Get_EditData();
        bPreviousLoadEnv = MapContentDesc.bLoadEnv;
        bPresetChanged = !MapContentDesc.bHasMapContent
            || MapContentDesc.iPresetIndex != static_cast<_int>(iPresetIndex);

        if (bPresetChanged)
            m_pMapPreviewSession->Reset();
    }

    if (bPresetChanged)
        Clear_MapPreview();
    else
        Clear_MapPreviewStage();

    Client::MAP_EDIT_DATA ResolvedMapContentDesc{};
    if (nullptr != m_pMapPreviewSession)
        ResolvedMapContentDesc = m_pMapPreviewSession->Get_EditData();

    ResolvedMapContentDesc.bHasMapContent = true;
    ResolvedMapContentDesc.iPresetIndex = static_cast<_int>(iPresetIndex);

    const HRESULT hSavedOverride = CMap_Loader::Load_PresetEditFile(iPresetIndex, ResolvedMapContentDesc.strManifestPath, &ResolvedMapContentDesc);

    if (FAILED(hSavedOverride))
    {
        if (nullptr != m_pMapPreviewSession)
            m_pMapPreviewSession->Set_PreviewStatus(L"Map edit file parse failed.");
        return E_FAIL;
    }

    if (ResolvedMapContentDesc.strManifestPath.empty())
    {
        if (FAILED(CMap_Loader::Get_MapManifestPath(
            iPresetIndex,
            &ResolvedMapContentDesc.strManifestPath)))
        {
            if (nullptr != m_pMapPreviewSession)
                m_pMapPreviewSession->Set_PreviewStatus(L"Map stage preview load failed.");
            return E_FAIL;
        }
    }

    if (nullptr != m_pMapPreviewSession)
        m_pMapPreviewSession->Set_EditData(ResolvedMapContentDesc);

    Client::MAP_RUNTIME_LOAD_CONTEXT Context{};
    Context.pDevice = m_pDevice;
    Context.pContext = m_pContext;
    Context.iPlaceLevel = ETOUI(EDIT_LEVEL::EDIT);
    Context.iModelLevel = ETOUI(LEVEL::STATIC);
    Context.pCreatedCallback = &On_MapPreviewObjectCreated;
    Context.pCallbackContext = this;

    CMapStage* pLoadedStage = nullptr;
    const HRESULT hResult = CMap_Loader::Load_MapStage_Runtime(
        Context,
        ResolvedMapContentDesc.strManifestPath,
        &pLoadedStage);

    if (FAILED(hResult))
    {
        if (bPresetChanged)
            Clear_MapPreview();
        else
            Clear_MapPreviewStage();

        m_pMapStage = nullptr;

        if (nullptr != m_pMapPreviewSession)
        {
            const _uint iEnvCount = m_pMapPreviewSession->Get_EnvCreatedCount();
            m_pMapPreviewSession->Clear_LoadedStage();
            m_pMapPreviewSession->Set_LoadStageEnabled(false);
            m_pMapPreviewSession->Set_LoadEnvEnabled(bPresetChanged ? false : bPreviousLoadEnv);

            if (0 != iEnvCount)
            {
                m_pMapPreviewSession->Set_PreviewStatus(
                    L"Map stage preview load failed. / env=" + to_wstring(iEnvCount));
            }
            else
            {
                m_pMapPreviewSession->Set_PreviewStatus(L"Map stage preview load failed.");
            }
        }

        return hResult;
    }

    m_pMapStage = pLoadedStage;

    if (nullptr != m_pMapPreviewSession)
    {
        _wstring strLoadedStageName =
            (nullptr != m_pMapStage) ? m_pMapStage->Get_StageName() : L"";

        if (strLoadedStageName.empty())
            strLoadedStageName = StrToWstr(CMap_Loader::Get_MapName(iPresetIndex));

        m_pMapPreviewSession->Set_PresetIndex(static_cast<_int>(iPresetIndex));
        m_pMapPreviewSession->Set_ManifestPath(ResolvedMapContentDesc.strManifestPath);
        m_pMapPreviewSession->Set_LoadStageEnabled(true);
        m_pMapPreviewSession->Set_LoadEnvEnabled(bPresetChanged ? false : bPreviousLoadEnv);
        m_pMapPreviewSession->Set_LoadedStageName(strLoadedStageName);

        _wstring strStatus =
            L"Map stage preview loaded: " + strLoadedStageName
            + L" / env=" + to_wstring(m_pMapPreviewSession->Get_EnvCreatedCount());

        if (S_OK == hSavedOverride)
            strStatus += L" / edit-file";

        m_pMapPreviewSession->Set_PreviewStatus(strStatus);
    }

    return hResult;
}

HRESULT CLevel_Edit::Load_MapPreviewEnv(_uint iPresetIndex)
{
    _bool bPresetChanged = false;
    if (nullptr != m_pMapPreviewSession)
    {
        const auto& MapContentDesc = m_pMapPreviewSession->Get_EditData();
        bPresetChanged = !MapContentDesc.bHasMapContent
            || MapContentDesc.iPresetIndex != static_cast<_int>(iPresetIndex);

        if (bPresetChanged)
            m_pMapPreviewSession->Reset();
    }

    if (bPresetChanged)
        Clear_MapPreview();
    else
        Clear_MapPreviewEnv();

    Client::MAP_EDIT_DATA ResolvedMapContentDesc{};
    if (nullptr != m_pMapPreviewSession)
        ResolvedMapContentDesc = m_pMapPreviewSession->Get_EditData();

    ResolvedMapContentDesc.bHasMapContent = true;
    ResolvedMapContentDesc.iPresetIndex = static_cast<_int>(iPresetIndex);

    const HRESULT hSavedOverride = CMap_Loader::Load_PresetEditFile(
        iPresetIndex,
        ResolvedMapContentDesc.strManifestPath,
        &ResolvedMapContentDesc);

    if (FAILED(hSavedOverride))
    {
        if (nullptr != m_pMapPreviewSession)
            m_pMapPreviewSession->Set_PreviewStatus(L"Map edit file parse failed.");
        return E_FAIL;
    }

    if (ResolvedMapContentDesc.strManifestPath.empty())
    {
        if (FAILED(CMap_Loader::Get_MapManifestPath(
            iPresetIndex,
            &ResolvedMapContentDesc.strManifestPath)))
        {
            if (nullptr != m_pMapPreviewSession)
                m_pMapPreviewSession->Set_PreviewStatus(L"Environment preview load failed.");
            return E_FAIL;
        }
    }

    if (nullptr != m_pMapPreviewSession)
        m_pMapPreviewSession->Set_EditData(ResolvedMapContentDesc);

    Client::MAP_RUNTIME_LOAD_CONTEXT Context{};
    Context.pDevice = m_pDevice;
    Context.pContext = m_pContext;
    Context.iPlaceLevel = ETOUI(EDIT_LEVEL::EDIT);
    Context.iModelLevel = ETOUI(LEVEL::STATIC);
    Context.pCreatedCallback = &On_MapPreviewObjectCreated;
    Context.pCallbackContext = this;

    Client::MAP_LOAD_RESULT Report{};
    vector<Client::ENV_OBJECT_DESC> DeletedEnvDescs;
    const HRESULT hResult = CMap_Loader::Load_Env_Runtime(
        Context,
        ResolvedMapContentDesc.strManifestPath,
        &ResolvedMapContentDesc.OverrideDesc,
        &Report,
        &DeletedEnvDescs);

    if (FAILED(hResult))
    {
        if (bPresetChanged)
            Clear_MapPreview();
        else
            Clear_MapPreviewEnv();

        if (nullptr != m_pMapPreviewSession)
        {
            m_pMapPreviewSession->Set_EnvLoaded(false);
            m_pMapPreviewSession->Set_EnvCreatedCount(0);

            if (nullptr != m_pMapStage)
            {
                const _wstring& strStageName = m_pMapPreviewSession->Get_LoadedStageName();
                const _wstring strDisplayStageName =
                    strStageName.empty() ? L"(loaded stage)" : strStageName;

                m_pMapPreviewSession->Set_PreviewStatus(
                    L"Environment preview load failed. / stage=" + strDisplayStageName);
            }
            else
            {
                m_pMapPreviewSession->Set_PreviewStatus(L"Environment preview load failed.");
            }
        }

        return hResult;
    }

    if (nullptr != m_pMapPreviewSession)
    {
        m_pMapPreviewSession->Set_PresetIndex(static_cast<_int>(iPresetIndex));
        m_pMapPreviewSession->Set_ManifestPath(ResolvedMapContentDesc.strManifestPath);
        m_pMapPreviewSession->Set_LoadStageEnabled(nullptr != m_pMapStage);
        m_pMapPreviewSession->Set_LoadEnvEnabled(true);
        m_pMapPreviewSession->Rebuild_DeletedEnvItems(DeletedEnvDescs);
        m_pMapPreviewSession->Set_EnvLoaded(0 < Report.iEnvCreatedCount);
        m_pMapPreviewSession->Set_EnvCreatedCount(Report.iEnvCreatedCount);

        _wstring strStatus = L"Environment preview loaded: env="
            + to_wstring(Report.iEnvCreatedCount);

        const _wstring& strStageName = m_pMapPreviewSession->Get_LoadedStageName();
        if (!strStageName.empty())
            strStatus += L" / stage=" + strStageName;

        if (S_OK == hSavedOverride)
            strStatus += L" / edit-file";

        if (0 != Report.iEnvSkippedMissingModel
            || 0 != Report.iEnvSkippedCreateFailed)
        {
            strStatus += L" / warnings";
        }

        m_pMapPreviewSession->Set_PreviewStatus(strStatus);
    }

    return hResult;
}

HRESULT CLevel_Edit::Load_LDPreview(_uint iPresetIndex)
{
    _wstring strManifestPath;
    if (FAILED(CMap_Loader::Get_MapManifestPath(iPresetIndex, &strManifestPath)))
    {
        if (nullptr != m_pMapPreviewSession)
            m_pMapPreviewSession->Set_PreviewStatus(L"LevelDesign manifest resolve failed.");
        return E_FAIL;
    }

    Clear_LDPreview();

    Client::MAP_RUNTIME_LOAD_CONTEXT Context{};
    Context.pDevice = m_pDevice;
    Context.pContext = m_pContext;
    Context.iPlaceLevel = ETOUI(EDIT_LEVEL::EDIT);
    Context.iModelLevel = ETOUI(LEVEL::STATIC);
    Context.pCreatedCallback = &On_MapPreviewObjectCreated;
    Context.pCallbackContext = this;

    Client::MAP_LOAD_RESULT Report{};
    if (FAILED(CMap_Loader::Load_LevelDesign_Runtime(Context, strManifestPath, &Report)))
    {
        if (nullptr != m_pMapPreviewSession)
            m_pMapPreviewSession->Set_PreviewStatus(L"LevelDesign preview load failed.");
        return E_FAIL;
    }

    if (nullptr != m_pMapPreviewSession)
    {
        m_pMapPreviewSession->Set_PreviewStatus(
            L"LevelDesign preview loaded: created="
            + to_wstring(Report.iLevelDesignCreatedCount)
            + L" / fallback="
            + to_wstring(Report.iLevelDesignFallbackSpecCount)
            + L" / failed="
            + to_wstring(Report.iLevelDesignSkippedCreateFailedCount));
    }

    return S_OK;
}

void CLevel_Edit::Clear_MapPreview()
{
    Clear_LDPreview();

    vector<wstring> MapLayers;
    MapLayers.reserve(m_Layers.size());

    for (const auto& Pair : m_Layers)
    {
        if (CMap_Loader::Is_MapLayer(Pair.first))
            MapLayers.push_back(Pair.first);
    }

    for (const wstring& strLayerTag : MapLayers)
        Clear_MapPreviewLayer(strLayerTag);

    m_pMapStage = nullptr;
    m_MapPreviewObjects.clear();

    if (nullptr != m_pMapPreviewSession)
    {
        m_pMapPreviewSession->Set_LoadStageEnabled(false);
        m_pMapPreviewSession->Set_LoadEnvEnabled(false);
        m_pMapPreviewSession->Clear_RuntimeState();
    }
}

void CLevel_Edit::Clear_MapPreviewStage()
{
    Clear_MapPreviewLayer(L"Layer_MapStage");

    m_pMapStage = nullptr;

    if (nullptr != m_pMapPreviewSession)
    {
        const _uint iEnvCount = m_pMapPreviewSession->Get_EnvCreatedCount();
        m_pMapPreviewSession->Set_LoadStageEnabled(false);
        m_pMapPreviewSession->Clear_LoadedStage();

        if (0 < iEnvCount)
            m_pMapPreviewSession->Set_PreviewStatus(
                L"Environment preview loaded only. / env=" + to_wstring(iEnvCount));
        else
            m_pMapPreviewSession->Set_PreviewStatus(L"Map preset not loaded.");
    }
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

    if (nullptr != m_pMapPreviewSession)
    {
        const _bool bStageLoaded = nullptr != m_pMapStage;
        const _wstring strStageName = m_pMapPreviewSession->Get_LoadedStageName();

        m_pMapPreviewSession->Set_LoadEnvEnabled(false);
        m_pMapPreviewSession->Set_EnvLoaded(false);
        m_pMapPreviewSession->Set_EnvCreatedCount(0);

        if (bStageLoaded)
        {
            const _wstring strDisplayStageName =
                strStageName.empty() ? L"(loaded stage)" : strStageName;
            m_pMapPreviewSession->Set_PreviewStatus(
                L"Map stage preview loaded: " + strDisplayStageName + L" / env=0");
        }
        else
        {
            m_pMapPreviewSession->Set_PreviewStatus(L"Map preset not loaded.");
        }
    }
}

void CLevel_Edit::Clear_LDPreview()
{
    vector<wstring> LevelDesignLayers;
    LevelDesignLayers.reserve(m_Layers.size());

    for (const auto& Pair : m_Layers)
    {
        if (CLevelDesign_Registry::Is_LevelDesignLayer(Pair.first))
            LevelDesignLayers.push_back(Pair.first);
    }

    for (const wstring& strLayerTag : LevelDesignLayers)
        Clear_MapPreviewLayer(strLayerTag);

    if (nullptr != m_pMapPreviewSession)
    {
        const _uint iEnvCount = m_pMapPreviewSession->Get_EnvCreatedCount();
        const _wstring strStageName = m_pMapPreviewSession->Get_LoadedStageName();

        if (nullptr != m_pMapStage)
        {
            const _wstring strDisplayStageName = strStageName.empty() ? L"(loaded stage)" : strStageName;
            m_pMapPreviewSession->Set_PreviewStatus(
                L"Map preview loaded without LevelDesign: stage=" + strDisplayStageName
                + L" / env=" + to_wstring(iEnvCount));
        }
        else if (0 < iEnvCount)
        {
            m_pMapPreviewSession->Set_PreviewStatus(
                L"Environment preview loaded only. / env=" + to_wstring(iEnvCount));
        }
        else
        {
            m_pMapPreviewSession->Set_PreviewStatus(L"LevelDesign preview cleared.");
        }
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

        if (nullptr != m_pMapPreviewSession)
        {
            m_pMapPreviewSession->Unregister_PreviewObject(pObject);
            m_pMapPreviewSession->Unregister_AddedObject(pObject);
        }

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

    if (nullptr != m_pMapPreviewSession)
    {
        m_pMapPreviewSession->Register_PreviewObject(strLayerTag, strObjectTag, pObject);

        Try_RegisterLoadedAddedMapObject(pObject, strPrototypeTag, strLayerTag, strObjectTag);
    }
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

_bool CLevel_Edit::Should_SkipMapObjectForLevelSave(CGameObject* pObject) const
{
    if (nullptr == pObject)
        return false;

    if (m_MapPreviewObjects.find(pObject) != m_MapPreviewObjects.end())
        return true;

    if (nullptr != m_pMapPreviewSession
        && m_pMapPreviewSession->Is_AddedObject(pObject))
    {
        return true;
    }

    return false;
}

_bool CLevel_Edit::Handle_MapSpecificDeletion(CGameObject* pObject)
{
    const _bool bWasMapPreviewObject =
        m_MapPreviewObjects.find(pObject) != m_MapPreviewObjects.end();

    const _bool bWasAddedMapObject =
        (nullptr != m_pMapPreviewSession)
        && m_pMapPreviewSession->Is_AddedObject(pObject);

    if (!bWasMapPreviewObject && !bWasAddedMapObject)
        return false;

    if (nullptr != m_pMapPreviewSession)
    {
        m_pMapPreviewSession->Unregister_PreviewObject(pObject);

        if (bWasAddedMapObject)
            m_pMapPreviewSession->Unregister_AddedObject(pObject);
    }

    m_MapPreviewObjects.erase(pObject);

    if (pObject == m_pMapStage)
    {
        m_pMapStage = nullptr;

        if (nullptr != m_pMapPreviewSession)
            m_pMapPreviewSession->Clear_LoadedStage();
    }
    else if (nullptr != m_pMapPreviewSession && !bWasAddedMapObject)
    {
        const _uint iEnvCount = m_pMapPreviewSession->Get_EnvCreatedCount();

        if (0 < iEnvCount)
        {
            const _uint iNextEnvCount = iEnvCount - 1;
            m_pMapPreviewSession->Set_EnvLoaded(0 < iNextEnvCount);
            m_pMapPreviewSession->Set_EnvCreatedCount(iNextEnvCount);
        }
        else
        {
            m_pMapPreviewSession->Set_EnvLoaded(false);
        }
    }

    if (nullptr != m_pMapPreviewSession)
    {
        if (pObject == m_pMapStage)
            m_pMapPreviewSession->Set_PreviewStatus(L"Map stage preview cleared.");
        else
            m_pMapPreviewSession->Set_PreviewStatus(L"Map preview object removed.");
    }

    return true;
}

void CLevel_Edit::Try_RegisterLoadedAddedMapObject(
    CGameObject* pObject,
    const _wstring& strPrototypeTag,
    const _wstring& strLayerTag,
    const _wstring& strObjectTag)
{
    if (nullptr == pObject || nullptr == m_pMapPreviewSession)
        return;

    const auto& AddedMapObjects =
        m_pMapPreviewSession->Get_EditData().OverrideDesc.AddedMapObjects;

    for (const Client::MAP_ADD_OBJECT& AddedDesc : AddedMapObjects)
    {
        if (AddedDesc.strPrototypeTag != strPrototypeTag)
            continue;
        if (AddedDesc.strLayerTag != strLayerTag)
            continue;
        if (AddedDesc.strObjectTag != strObjectTag)
            continue;

        const _wstring strDisplayName =
            strObjectTag.empty() ? strPrototypeTag : strObjectTag;

        m_pMapPreviewSession->Register_AddedObject(
            pObject,
            AddedDesc,
            strDisplayName);
        break;
    }
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

    Safe_Release(m_pMapPreviewSession);
    Safe_Release(m_pGrid);
}