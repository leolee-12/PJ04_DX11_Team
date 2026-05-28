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
#include "Lumia.h"

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

    m_pNavMeshEditor = CNavMesh_Editor::Create();
    if (nullptr == m_pNavMeshEditor)
        return E_FAIL;


    return S_OK;
}

void CLevel_Edit::Update(_float fTimeDelta)
{
    if (m_pGameInstance_Proxy->Key_Down(DIK_ESCAPE))
        m_pGameInstance_Proxy->Publish(TEXT("Return_Lobby"), nullptr);
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
            json jObj = handle.pObject->Serialize();

            jObj["Object_Tag"]      = WstrToStr(handle.strName);
            jObj["Prototype_Tag"]   = WstrToStr(handle.strPrototypeTag);
            jObj["Layer_Tag"]       = WstrToStr(LayerTag);

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
                Objects.erase(iter);
                m_pGameInstance_Proxy->Destroy_GameObject(pObject);
                return;
            }
        }
    }
}

void CLevel_Edit::Pick_And_Place(_fvector vOrigin, _fvector vDir)
{
    if (m_bNavEditMode)
    {
        _float3 fHitPos = {};
        _float  fDummy = {};
        if (m_pLumia->Pick_Floor(vOrigin, vDir, &fHitPos, &fDummy))
            m_pNavMeshEditor->OnClick(fHitPos);
        return;
    }

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

void CLevel_Edit::Begin_NavEditMode()
{
    // CLumia 캐싱, 없으면 모드 진입 거부
    m_pLumia = dynamic_cast<CLumia*>(
        m_pGameInstance_Proxy->Find_GameObject(ETOUI(EDIT_LEVEL::EDIT), L"Default_Layer", L"Proto_Lumia_0"));

    if (!m_pLumia) return;

    m_bNavEditMode = true;
}

void CLevel_Edit::End_NavEditMode()
{
    m_bNavEditMode = false;
    m_pLumia = nullptr;  // 소유권 없음, Release 불필요
}

void CLevel_Edit::Nav_Undo()
{
    if (m_pNavMeshEditor)
        m_pNavMeshEditor->Undo();
}

void CLevel_Edit::Save_NavMesh(const wstring& strFilePath)
{
    if (m_pNavMeshEditor)
        m_pNavMeshEditor->Save(strFilePath);
}

void CLevel_Edit::Load_NavMesh(const wstring& strFilePath)
{
    if (m_pNavMeshEditor)
        m_pNavMeshEditor->Load(strFilePath);
}

void CLevel_Edit::Nav_Redo()
{
    if (m_pNavMeshEditor)
        m_pNavMeshEditor->Redo();
}

HRESULT CLevel_Edit::Ready_EditLights()
{
    LIGHT_DESC      LightDesc{};

    LightDesc.eType = LIGHT::DIRECTIONAL;
    LightDesc.vDiffuse = _float4(1.f, 1.f, 1.f, 1.f);
    LightDesc.vAmbient = _float4(1.f, 1.f, 1.f, 1.f);
    LightDesc.vSpecular = _float4(1.f, 1.f, 1.f, 1.f);
    LightDesc.vDirection = _float4(1.f, -1.f, 1.f, 0.f);

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
    m_pGrid = CEdit_Grid::Create(m_pDevice, m_pContext, 1000, 1.f);
    return (m_pGrid == nullptr) ? E_FAIL : S_OK;
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
    Safe_Release(m_pNavMeshEditor);
}
