#include "Level_Edit.h"

#include "GameObject.h"
#include "GameInstance.h"
#include "GameInstance_Proxy.h"
#include "GameObject_Factory.h"
#include "EditCamera.h"
#include "MapStage.h"
#include "Map_Loader.h"
#include "imgui.h"
#include "Kirby.h"
#include "Transform.h"

static _vector SmoothDampV(_fvector cur, _fvector target, _vector& vel, _float smoothTime, _float dt)
{
    smoothTime = max(0.0001f, smoothTime);
    _float omega = 2.f / smoothTime, x = omega * dt;
    _float e = 1.f / (1.f + x + 0.48f * x * x + 0.235f * x * x * x);
    _vector change = XMVectorSubtract(cur, target);
    _vector temp = XMVectorScale(XMVectorAdd(vel, XMVectorScale(change, omega)), dt);
    vel = XMVectorScale(XMVectorSubtract(vel, XMVectorScale(temp, omega)), e);
    return XMVectorAdd(target, XMVectorScale(XMVectorAdd(change, temp), e));
}

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

    if (FAILED(Ready_Kirby()))
        return E_FAIL;

    return S_OK;
}

void CLevel_Edit::Update(_float fTimeDelta)
{
    if (m_pGameInstance_Proxy->Key_Down(DIK_F2))
        m_pGameInstance_Proxy->Toggle_DebugRender();

    if (m_pKirby)
    {
        CTransform* pTr = m_pKirby->Get_Transform();
        if (m_bPlay)
            XMStoreFloat3(&m_vKirby, pTr->Get_State(STATE::POSITION));   // 커비 스스로 이동 > 솔버로 풀
        else
            pTr->Set_State(STATE::POSITION, XMVectorSetW(XMLoadFloat3(&m_vKirby), 1.f)); // 기즈모 > 커비로푸시(Sync_To_Controller가 컨트롤러까지 텔레포트)
    }

    m_Solver.Update(XMLoadFloat3(&m_vKirby), fTimeDelta);

    // ---- 프리뷰 카메라 포즈 스무딩 (게임과 동일) ----
    {
        const Client::CAM_POSE& pose = m_Solver.Cur_Pose();
        _vector vEye = XMLoadFloat3(&pose.eye);
        _vector vFwd = XMLoadFloat3(&pose.fwd);
        if (XMVector3IsNaN(vEye) || XMVector3IsNaN(vFwd) || XMVectorGetX(XMVector3LengthSq(vFwd)) < 1e-6f)
        {
            vEye = m_bCamInit ? XMLoadFloat3(&m_eyeCur) : XMVectorSet(0.f, 3.f, -10.f, 0.f);
            vFwd = XMVectorSet(0.f, 0.f, 1.f, 0.f);
            XMStoreFloat3(&m_eyeVel, XMVectorZero()); XMStoreFloat3(&m_atVel, XMVectorZero());
        }
        _vector vAt = XMVectorAdd(vEye, vFwd);
        if (!m_bCamInit) { XMStoreFloat3(&m_eyeCur, vEye); XMStoreFloat3(&m_atCur, vAt); m_bCamInit = true; }

        _int curArea = m_Solver.Cur_AreaIndex();
        if (curArea != m_lastArea) { m_areaBlendTimer = 0.5f; m_lastArea = curArea; }
        _float smooth = m_smoothTime;
        if (m_areaBlendTimer > 0.f) { m_areaBlendTimer -= fTimeDelta; smooth = 0.7f; }

        _vector eVel = XMLoadFloat3(&m_eyeVel), aVel = XMLoadFloat3(&m_atVel);
        XMStoreFloat3(&m_eyeCur, SmoothDampV(XMLoadFloat3(&m_eyeCur), vEye, eVel, smooth, fTimeDelta));
        XMStoreFloat3(&m_atCur, SmoothDampV(XMLoadFloat3(&m_atCur), vAt, aVel, smooth, fTimeDelta));
        XMStoreFloat3(&m_eyeVel, eVel); XMStoreFloat3(&m_atVel, aVel);
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
    _vector vEye = XMLoadFloat3(&m_eyeCur);
    _vector vDir = XMVectorSubtract(XMLoadFloat3(&m_atCur), vEye);
    if (XMVectorGetX(XMVector3LengthSq(vDir)) < 1e-6f) vDir = XMVectorSet(0.f, 0.f, 1.f, 0.f);
    _vector vLook = XMVector3Normalize(vDir);
    _vector vUpRef = XMVectorSet(0.f, 1.f, 0.f, 0.f);
    _vector vRight = XMVector3Cross(vUpRef, vLook);
    if (XMVectorGetX(XMVector3LengthSq(vRight)) < 1e-6f) vRight = XMVector3Cross(XMVectorSet(0.f, 0.f, 1.f, 0.f),
        vLook);
    vRight = XMVector3Normalize(vRight);
    _vector vUp = XMVector3Normalize(XMVector3Cross(vLook, vRight));

    XMStoreFloat4x4(pView, XMMatrixLookToLH(vEye, vLook, vUp));
    _float fAspect = (_float)g_iWinSizeX / (_float)g_iWinSizeY;
    _float fFov = m_Solver.Cur_Pose().fov;
    XMStoreFloat4x4(pProj, XMMatrixPerspectiveFovLH(XMConvertToRadians(fFov), fAspect, 0.1f, 1000.f));
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

HRESULT CLevel_Edit::Ready_Kirby()
{
    auto* pFactory = Client::CGameObject_Factory::GetInstance();
    if (nullptr == pFactory) return E_FAIL;

    // 1) 커비 바디/모델 프로토타입 등록(로더 실행) 로더가 GAMEPLAY 레벨에 넣음
    pFactory->LoadResource(Client::CKirby::PROTOTYPE_TAG, m_pGameInstance_Proxy, m_pDevice, m_pContext);

    // 2) 커비 프로토타입 등록
    if (!m_pGameInstance_Proxy->Has_Prototype(ETOUI(EDIT_LEVEL::STATIC), Client::CKirby::PROTOTYPE_TAG))
    {
        auto* pReg = pFactory->Get_Registration(Client::CKirby::PROTOTYPE_TAG);
        if (nullptr == pReg) return E_FAIL;
        m_pGameInstance_Proxy->Add_Prototype(ETOUI(EDIT_LEVEL::STATIC),
            Client::CKirby::PROTOTYPE_TAG,
            dynamic_cast<CGameObject*>(pReg->CreatorFunc(m_pDevice, m_pContext)));
    }

    // 3) 원점에 스폰
    CGameObject* pKirby = nullptr;
    Client::CKirby::KIRBY_BODY_DESC desc{};
    if (FAILED(m_pGameInstance_Proxy->Add_GameObject_Return(&pKirby,
        ETOUI(EDIT_LEVEL::STATIC), Client::CKirby::PROTOTYPE_TAG,
        ETOUI(EDIT_LEVEL::EDIT), L"Layer_Kirby", L"Kirby", &desc)))
        return E_FAIL;

    m_pKirby = pKirby;
    XMStoreFloat3(&m_vKirby, m_pKirby->Get_Transform()->Get_State(STATE::POSITION));
    return S_OK;
}

void CLevel_Edit::Set_Play(_bool b)
{
    m_bPlay = b;
    if (b) m_bPreview = true;                       // 플레이 시 게임캠 프리뷰 ON
    m_pGameInstance_Proxy->Set_EditMode(!b);        // Play=게임모드(커비 Move 활성), Stop=에디트
    if (b) m_pGameInstance_Proxy->Enable_InputDeveice();   // Play일 때만 커비가 입력받게
    else   m_pGameInstance_Proxy->Disable_InputDeveice();
}

HRESULT CLevel_Edit::Load_MapPreview(_uint iPresetIndex)
{
    Clear_MapPreview();

    _wstring strManifestPath;
    if (FAILED(Client::CMap_Loader::Get_MapPresetManifestPath(iPresetIndex, &strManifestPath)))
    {
        m_strMapPreviewStatus = L"Map preset load failed.";
        return E_FAIL;
    }

    Client::MAP_RUNTIME_LOAD_CONTEXT Context{};
    Context.pDevice = m_pDevice;
    Context.pContext = m_pContext;
    Context.iPlaceLevel = ETOUI(EDIT_LEVEL::EDIT);
    Context.iModelLevel = ETOUI(LEVEL::STATIC);
    Context.pCreatedCallback = &On_MapPreviewObjectCreated;
    Context.pCallbackContext = this;

    CMapStage* pLoadedStage = nullptr;
    HRESULT hResult = Client::CMap_Loader::Load_MapStage_Runtime(
        Context,
        strManifestPath,
        &pLoadedStage);

    if (FAILED(hResult))
    {
        m_strMapPreviewStatus = L"Map preset load failed.";
        return hResult;
    }

    Client::MAP_LOAD_REPORT Report{};
    hResult = Client::CMap_Loader::Load_Env_Runtime(
        Context,
        strManifestPath,
        nullptr,
        &Report);

    if (FAILED(hResult))
    {
        Clear_MapPreview();
        m_strMapPreviewStatus = L"Map preset load failed.";
        return hResult;
    }

    m_pMapStage = pLoadedStage;
    m_strLoadedMapStageName = nullptr != m_pMapStage ? m_pMapStage->Get_StageName() : L"";
    if (m_strLoadedMapStageName.empty())
        m_strLoadedMapStageName = StrToWstr(Client::CMap_Loader::Get_MapPresetLabel(iPresetIndex));

    m_iEnvObjCreatedCount = Report.iEnvCreatedCount;
    m_iLoadedMapPresetIndex = static_cast<_int>(iPresetIndex);
    m_strMapPreviewStatus = L"Map preset loaded: " + m_strLoadedMapStageName
        + L" / env=" + to_wstring(m_iEnvObjCreatedCount);

    if (0 != Report.iEnvSkippedMissingModel
        || 0 != Report.iEnvSkippedCreateFailed)
    {
        m_strMapPreviewStatus += L" / warnings";
    }

    return S_OK;
}

HRESULT CLevel_Edit::Load_MapPreviewStage(_uint iPresetIndex)
{
    Clear_MapPreviewStage();
    m_iLoadedMapPresetIndex = -1;

    _wstring strManifestPath;
    if (FAILED(Client::CMap_Loader::Get_MapPresetManifestPath(iPresetIndex, &strManifestPath)))
    {
        m_pMapStage = nullptr;
        m_strLoadedMapStageName.clear();
        m_strMapPreviewStatus = 0 != m_iEnvObjCreatedCount
            ? L"Map stage preview load failed. / env=" + to_wstring(m_iEnvObjCreatedCount)
            : L"Map stage preview load failed.";
        return E_FAIL;
    }

    Client::MAP_RUNTIME_LOAD_CONTEXT Context{};
    Context.pDevice = m_pDevice;
    Context.pContext = m_pContext;
    Context.iPlaceLevel = ETOUI(EDIT_LEVEL::EDIT);
    Context.iModelLevel = ETOUI(LEVEL::STATIC);
    Context.pCreatedCallback = &On_MapPreviewObjectCreated;
    Context.pCallbackContext = this;

    CMapStage* pLoadedStage = nullptr;
    const HRESULT hResult = Client::CMap_Loader::Load_MapStage_Runtime(
        Context,
        strManifestPath,
        &pLoadedStage);

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
    m_strLoadedMapStageName = nullptr != m_pMapStage ? m_pMapStage->Get_StageName() : L"";
    if (m_strLoadedMapStageName.empty())
        m_strLoadedMapStageName = StrToWstr(Client::CMap_Loader::Get_MapPresetLabel(iPresetIndex));

    m_strMapPreviewStatus = L"Map stage preview loaded: " + m_strLoadedMapStageName
        + L" / env=" + to_wstring(m_iEnvObjCreatedCount);

    return S_OK;
}

HRESULT CLevel_Edit::Load_MapPreviewEnv(_uint iPresetIndex)
{
    Clear_MapPreviewEnv();
    m_iLoadedMapPresetIndex = -1;

    _wstring strManifestPath;
    if (FAILED(Client::CMap_Loader::Get_MapPresetManifestPath(iPresetIndex, &strManifestPath)))
    {
        m_iEnvObjCreatedCount = 0;
        m_strMapPreviewStatus = nullptr != m_pMapStage
            ? L"Environment preview load failed. / stage=" + m_strLoadedMapStageName
            : L"Environment preview load failed.";
        return E_FAIL;
    }

    MAP_RUNTIME_LOAD_CONTEXT Context{};
    Context.pDevice = m_pDevice;
    Context.pContext = m_pContext;
    Context.iPlaceLevel = ETOUI(EDIT_LEVEL::EDIT);
    Context.iModelLevel = ETOUI(LEVEL::STATIC);
    Context.pCreatedCallback = &On_MapPreviewObjectCreated;
    Context.pCallbackContext = this;

    MAP_LOAD_REPORT Report{};
    const HRESULT hResult = CMap_Loader::Load_Env_Runtime(Context, strManifestPath, nullptr, &Report);

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

    if (0 != Report.iEnvSkippedMissingModel
        || 0 != Report.iEnvSkippedCreateFailed)
    {
        m_strMapPreviewStatus += L" / warnings";
    }

    return S_OK;
}

void CLevel_Edit::Clear_MapPreview()
{
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


