#include "Level_Tool.h"
#include "GameInstance.h"
#include "EditCamera.h"
#include "Edit_Grid.h"

#include "Shader.h"
#include "Model.h"
#include "Preview_Actor.h"
#include "Preview_Kirby.h"
#include "GameContent_const.h"
#include "GameObject_Factory.h"

#include "UI_Title.h"
#include "Panel_Manager.h"
#include "UIContainerObject.h"
#include "UIPartObject.h"
#include "GameObject.h"
#include "UI_GenericContainer.h"
#include "UI_SpriteAnim.h"

namespace
{
    constexpr const _char* PREVIEW_MODEL_PATH =
        "../../Resources/Models/Test/BladeKnight/BladeKnight.ysh";

    MODEL Read_YshType(const _wstring& strPath)
    {
        FILE* fp = nullptr; _wfopen_s(&fp, strPath.c_str(), L"rb");
        if (!fp) return MODEL::END;
        uint32_t magic = 0, ver = 0, type = 0;
        fread(&magic, 4, 1, fp);
        fread(&ver, 4, 1, fp);
        fread(&type, 4, 1, fp);

        fclose(fp);
        if (magic != 0x2E595348)
            return MODEL::END;

        return (type == 0) ? MODEL::NONANIM : MODEL::ANIM;
    }
}


CLevel_Tool::CLevel_Tool(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CLevel(pDevice, pContext) {
}

HRESULT CLevel_Tool::Initialize()
{
    if (FAILED(__super::Initialize()))
        return E_FAIL;

    if (FAILED(Ready_Lights()))
        return E_FAIL;

    if (FAILED(Ready_Camera()))
        return E_FAIL;

    if (FAILED(Ready_Grid()))
        return E_FAIL;

    if (FAILED(Ready_PreviewShaders()))
        return E_FAIL;

    //if (FAILED(Ready_TestUI()))
    //    return E_FAIL;

    return S_OK;
}

HRESULT CLevel_Tool::Ready_Lights()
{
    LIGHT_DESC      LightDesc{};

    LightDesc.eType = LIGHT::DIRECTIONAL;
    LightDesc.vDiffuse = _float4(1.f, 1.f, 1.f, 1.f);
    LightDesc.vAmbient = _float4(0.2f, 0.2f, 0.2f, 1.f);
    LightDesc.vSpecular = _float4(1.f, 1.f, 1.f, 1.f);
    LightDesc.vDirection = _float4(-0.3f, -1.f, -0.3f, 0.f);

    if (FAILED(m_pGameInstance_Proxy->Add_Light(LightDesc)))
        return E_FAIL;

    return S_OK;
}

HRESULT CLevel_Tool::Ready_Camera()
{
    if (!m_pGameInstance_Proxy->Has_Prototype(ETOUI(TOOL_LEVEL::STATIC), CEditCamera::PROTOTYPE_TAG))
    {
        m_pGameInstance_Proxy->Add_Prototype(
            ETOUI(TOOL_LEVEL::STATIC),
            CEditCamera::PROTOTYPE_TAG,
            CEditCamera::Create(m_pDevice, m_pContext));
    }

    CEditCamera::EDIT_CAMERA_FREE_DESC desc{};
    desc.vEye = { 0.f, 10.f, -20.f };
    desc.vAt = { 0.f,  0.f,   0.f };
    desc.fFovy = XMConvertToRadians(60.f);
    desc.fNear = 0.1f;
    desc.fFar = 1000.f;
    desc.fSpeedPerSec = 20.f;
    desc.fRotationPerSec = XMConvertToRadians(360.f);
    desc.fMouseSensor = 0.05f;

    CGameObject* pCam = nullptr;
    if (FAILED(m_pGameInstance_Proxy->Add_GameObject_Return(
        &pCam,
        ETOUI(TOOL_LEVEL::STATIC), CEditCamera::PROTOTYPE_TAG,
        ETOUI(TOOL_LEVEL::EDIT), L"Layer_Camera", L"Edit_Camera", &desc)))
        return E_FAIL;

    m_pCamera = static_cast<CEditCamera*>(pCam);
    return S_OK;
}

HRESULT CLevel_Tool::Ready_Grid()
{
    m_pGrid = CEdit_Grid::Create(m_pDevice, m_pContext, 100, 1.f);
    return (m_pGrid == nullptr) ? E_FAIL : S_OK;
}

HRESULT CLevel_Tool::Ready_PreviewShaders()
{
    const _uint L = ETOUI(TOOL_LEVEL::STATIC);
    if (!m_pGameInstance_Proxy->Has_Prototype(L, L"Proto_Shader_AnimMesh"))
        m_pGameInstance_Proxy->Add_Prototype(L, L"Proto_Shader_AnimMesh",
            CShader::Create(m_pDevice, m_pContext, Shader_AnimMesh_PBR.szFileTag,
                VTXANIMMESH::Elements, VTXANIMMESH::iNumElements));

    if (!m_pGameInstance_Proxy->Has_Prototype(L, L"Proto_Shader_NonAnimMesh"))
        m_pGameInstance_Proxy->Add_Prototype(L, L"Proto_Shader_NonAnimMesh",
            CShader::Create(m_pDevice, m_pContext, Shader_NonAnimMesh_PBR.szFileTag,
                VTXMESH::Elements, VTXMESH::iNumElements));
    return S_OK;
}

HRESULT CLevel_Tool::Ready_TestUI()
{
    auto* pReg = Client::CGameObject_Factory::GetInstance()
        ->Get_Registration(Client::CUI_Title::PROTOTYPE_TAG);
    if (!pReg)
        return E_FAIL;

    const _uint L = ETOUI(TOOL_LEVEL::STATIC);

    if (!m_pGameInstance_Proxy->Has_Prototype(L, Client::CUI_Title::PROTOTYPE_TAG))
    {
        pReg->ResourceLoader(m_pGameInstance_Proxy, m_pDevice, m_pContext);

        m_pGameInstance_Proxy->Add_Prototype(
            L,
            Client::CUI_Title::PROTOTYPE_TAG,
            pReg->CreatorFunc(m_pDevice, m_pContext));
    }

    Client::CUI_Title::UI_TITLE_DESC desc{};
    //desc.vPosition = { -250.f, 120.f, 0.f, 1.f };
    desc.vPosition = { 0.f, 0.f, 0.f, 1.f };
    desc.bCreateTitleImage = true;
    desc.szTitleImagePartTag = Client::CUI_Title::PART_TAG_TITLE_IMAGE;

    desc.TitleImageDesc.iTextureLevel = ETOUI(LEVEL::STATIC);
    desc.TitleImageDesc.szTextureProtoTag = L"Proto_Tex_TestUI";
    desc.TitleImageDesc.vSize = { 496.f, 317.f };
    desc.TitleImageDesc.vPosition = { 0.f, 0.f };
    desc.TitleImageDesc.iRenderLayer = 1;

    CGameObject* pSource = nullptr;
    if (FAILED(m_pGameInstance_Proxy->Add_GameObject_Return(&pSource, L, Client::CUI_Title::PROTOTYPE_TAG, ETOUI(TOOL_LEVEL::EDIT), L"Layer_UI", L"Test_UI_Title_Source", &desc)))
        return E_FAIL;

    Track_UIContainer(pSource);

    json jUI = pSource->Serialize();
    jUI["Transform"]["vPosition"][0] = 0.f;

    CGameObject* pLoaded = nullptr;
    if (FAILED(m_pGameInstance_Proxy->Add_GameObject_Return(&pLoaded, L, Client::CUI_Title::PROTOTYPE_TAG, ETOUI(TOOL_LEVEL::EDIT), L"Layer_UI", L"Test_UI_Title_Loaded", nullptr)))
        return E_FAIL;

    Track_UIContainer(pLoaded);

    pLoaded->Deserialize(jUI);

    return S_OK;
}

void CLevel_Tool::Track_UIContainer(CGameObject* pObject)
{
    auto* pContainer = dynamic_cast<CUIContainerObject*>(pObject);
    if (!pContainer)
        return;

    if (find(m_UIContainers.begin(), m_UIContainers.end(), pContainer) != m_UIContainers.end())
        return;

    m_UIContainers.push_back(pContainer);
}

HRESULT  CLevel_Tool::Save_UIContainer(CGameObject* pContainer, const _float2& vDesignSize, const _wstring& strFileName)
{
    if (nullptr == pContainer)
    {
        Log_Warning("Save_UIContainer: null");
        return E_FAIL;
    }

    try
    {
        json j;
        j["DesignSize"] = { vDesignSize.x, vDesignSize.y };
        j["ProtoTag"] = WstrToStr(Get_AuthoredProtoTag(pContainer));
        j["Container"] = pContainer->Serialize();

        // 텍스처 매니페스트: 이 컨테이너가 쓰는 TextureProtoTag -> path
        json jTextures = json::object();
        if (j["Container"].contains("UIPartObjects"))
        {
            for (auto& [strPartTag, jPart] :
                j["Container"]["UIPartObjects"].items())
            {
                if (!jPart.contains("TextureProtoTag"))
                    continue;
                std::string strTag =
                    jPart["TextureProtoTag"].get<std::string>();
                if (strTag.empty())
                    continue;
                auto it = m_TextureProtoPaths.find(StrToWstr(strTag));
                if (it != m_TextureProtoPaths.end())
                    jTextures[strTag] = WstrToStr(it->second);
            }
        }
        j["Textures"] = jTextures;

        namespace fs = std::filesystem;
        fs::path dir = L"../../Resources/CHJ/UI";
        std::error_code ec; fs::create_directories(dir, ec);
        fs::path path = dir / (strFileName + L"_ui.json");

        std::ofstream fout(path);
        if (!fout.is_open())
        {
            Log_Error("Save_UIContainer open fail: "
                + WstrToStr(path.wstring()));
            return E_FAIL;
        }

        // 비-UTF8 문자가 있어도 throw 대신 치환해서 기록
        fout << j.dump(2, ' ', false,
            json::error_handler_t::replace);
        fout.close();

        Log_Info("Saved UI: " + WstrToStr(path.wstring()));
        return S_OK;
    }
    catch (const std::exception& e)
    {
        Log_Error(std::string("Save_UIContainer exception: ") + e.what());
        return E_FAIL;
    }
}

CGameObject* CLevel_Tool::Load_UIContainerByPath(const _wstring& strFullPath, _float2& vOutDesignSize)
{
    namespace fs = std::filesystem;

    std::ifstream fin(strFullPath);
    if (!fin.is_open()) {
        Log_Error("Load_UIContainerByPath: open fail: " + WstrToStr(strFullPath));
        return nullptr;
    }
    json j;
    try 
    {
        fin >> j;
    }
    catch (const exception& e)
    {
        fin.close();
        Log_Error(string("Load_UIContainerByPath : json Parse Fail: ") + e.what());
        return nullptr;
    }
    fin.close();

    CGameObject* pObj = nullptr;
    try
    {
        std::wstring strAuthoredTag =
            StrToWstr(j.value("ProtoTag", std::string()));
        if (strAuthoredTag.empty())
        {
            Log_Error("Load_UIContainerByPath: no ProtoTag");
            return nullptr;
        }

        std::wstring strName = fs::path(strFullPath).stem().wstring();
        if (strName.size() > 3 &&
            0 == strName.compare(strName.size() - 3, 3, L"_ui"))
            strName = strName.substr(0, strName.size() - 3);

        const _uint L = ETOUI(TOOL_LEVEL::STATIC);

        std::wstring strSpawnTag = strAuthoredTag;
        auto* pReg = Client::CGameObject_Factory::GetInstance()
            ->Get_Registration(strSpawnTag);

        if (pReg && pReg->strCategory != L"UI_CONTAINER")
        {
            Log_Warning("Load: '" + WstrToStr(strAuthoredTag)
                + "' is not a UI_CONTAINER -> GenericContainer fallback");
            pReg = nullptr;
        }

        if (!pReg)
        {
            Log_Warning("Load: unregistered ProtoTag '"
                + WstrToStr(strAuthoredTag)
                + "' -> GenericContainer fallback");
            strSpawnTag = Client::CUI_GenericContainer::PROTOTYPE_TAG;
            pReg = Client::CGameObject_Factory::GetInstance()
                ->Get_Registration(strSpawnTag);
            if (!pReg)
            {
                Log_Error("Load: GenericContainer not registered");
                return nullptr;
            }
        }

        if (!m_pGameInstance_Proxy->Has_Prototype(L, strSpawnTag))
        {
            pReg->ResourceLoader(
                m_pGameInstance_Proxy, m_pDevice, m_pContext);
            m_pGameInstance_Proxy->Add_Prototype(L, strSpawnTag,
                pReg->CreatorFunc(m_pDevice, m_pContext));
        }

        if (FAILED(m_pGameInstance_Proxy->Add_GameObject_Return(
            &pObj, L, strSpawnTag,
            ETOUI(TOOL_LEVEL::EDIT), L"Layer_UI", strName, nullptr)))
            return nullptr;

        Track_UIContainer(pObj);
        Set_AuthoredProtoTag(pObj, strAuthoredTag);

        if (j.contains("Textures"))
        {
            for (auto& [strTag, jPath] : j["Textures"].items())
                Register_TextureProto(
                    StrToWstr(jPath.get<std::string>()));
        }
        if (j.contains("Container"))
            pObj->Deserialize(j["Container"]);

        if (j.contains("DesignSize") && j["DesignSize"].size() == 2)
            vOutDesignSize = { j["DesignSize"][0], j["DesignSize"][1] };

        Log_Info("Loaded UI: " + WstrToStr(strFullPath));
        return pObj;
    }
    catch (const std::exception& e)
    {
        Log_Error(std::string(
            "Load_UIContainerByPath: json consume fail: ") + e.what());
        if (pObj)
            Delete_UIContainer(
                dynamic_cast<CUIContainerObject*>(pObj));
        return nullptr;
    }
}

void CLevel_Tool::Delete_UIContainer(CUIContainerObject* pContainer)
{
    if (nullptr == pContainer) return;

    m_UIContainers.erase(
        std::remove(m_UIContainers.begin(), m_UIContainers.end(), pContainer),
        m_UIContainers.end());

    m_AuthoredProtoTags.erase(pContainer);

    m_pGameInstance_Proxy->Destroy_GameObject(pContainer);
    Log_Info("Deleted UI container.");
}

CGameObject* CLevel_Tool::Add_UIContainer()
{
    const _uint L = ETOUI(TOOL_LEVEL::STATIC);
    const _wstring strTag = Client::CUI_GenericContainer::PROTOTYPE_TAG;

    auto* pReg = Client::CGameObject_Factory::GetInstance()
        ->Get_Registration(strTag);
    if (!pReg)
    {
        Log_Error("Add_UIContainer: GenericContainer not registered");
        return nullptr;
    }

    if (!m_pGameInstance_Proxy->Has_Prototype(L, strTag))
    {
        pReg->ResourceLoader(m_pGameInstance_Proxy, m_pDevice, m_pContext);
        m_pGameInstance_Proxy->Add_Prototype(L, strTag,
            pReg->CreatorFunc(m_pDevice, m_pContext));
    }

    Client::CUI_GenericContainer::UI_GENERIC_CONTAINER_DESC desc{};
    desc.vPosition = { 0.f, 0.f, 0.f, 1.f };   // 센터 배치

    _wstring strName =
        L"UIContainer_" + std::to_wstring(m_iUIContainerCounter++);

    CGameObject* pObj = nullptr;
    if (FAILED(m_pGameInstance_Proxy->Add_GameObject_Return(&pObj, L, strTag,
        ETOUI(TOOL_LEVEL::EDIT), L"Layer_UI", strName, &desc)))
        return nullptr;

    Track_UIContainer(pObj);
    Set_AuthoredProtoTag(pObj, strTag);   // 기본 ProtoTag = 자기(self-host)
    Log_Info("Added UI container: " + WstrToStr(strName));
    return pObj;
}

CUIPartObject* CLevel_Tool::Add_UIPart(CGameObject* pContainer, UI_PART_TYPE eType, _wstring* pOutPartTag)
{
    auto* pGeneric =
        dynamic_cast<Client::CUI_GenericContainer*>(pContainer);
    if (!pGeneric)
    {
        Log_Warning("Add_UIPart: not a GenericContainer");
        return nullptr;
    }

    const _uint L = ETOUI(LEVEL::STATIC);
    const _wstring strProtoTag =
        (eType == UI_PART_TYPE::SPRITEANIM)
        ? Client::CUI_SpriteAnim::PROTOTYPE_TAG
        : Client::CUI_Image::PROTOTYPE_TAG;

    // 파트 프로토 보장 (SpriteAnim은 컨테이너 로더가 안 올림)
    if (!m_pGameInstance_Proxy->Has_Prototype(L, strProtoTag))
    {
        auto* pReg = Client::CGameObject_Factory::GetInstance()
            ->Get_Registration(strProtoTag);
        if (!pReg)
        {
            Log_Error("Add_UIPart: unregistered "
                + WstrToStr(strProtoTag));
            return nullptr;
        }
        pReg->ResourceLoader(m_pGameInstance_Proxy, m_pDevice, m_pContext);
        m_pGameInstance_Proxy->Add_Prototype(L, strProtoTag,
            pReg->CreatorFunc(m_pDevice, m_pContext));
    }

    _wstring strPartTag =
        L"Part_" + std::to_wstring(m_iUIPartCounter++);

    HRESULT hr = E_FAIL;

    if (eType == UI_PART_TYPE::SPRITEANIM)
    {
        Client::CUI_SpriteAnim::UI_SPRITEANIM_DESC desc{};
        desc.iTextureLevel = ETOUI(LEVEL::STATIC);
        desc.szTextureProtoTag = L"Proto_Tex_StarArray";
        desc.vSize = { 100.f, 100.f };
        desc.vPosition = { 0.f, 0.f };
        desc.iRenderLayer = 1;
        desc.fDuration = 1.f;
        desc.fEndDelay = 0.f;
        desc.bLoop = true;
        desc.bAutoPlay = true;

        hr = pGeneric->Add_Part(
            ETOUI(LEVEL::STATIC), strProtoTag, strPartTag, &desc);
    }
    else
    {
        Client::CUI_Image::UI_IMAGE_DESC desc{};
        desc.iTextureLevel = ETOUI(LEVEL::STATIC);
        desc.szTextureProtoTag = L"Proto_Tex_TestUI";
        desc.vSize = { 100.f, 100.f };
        desc.vPosition = { 0.f, 0.f };
        desc.iRenderLayer = 1;

        hr = pGeneric->Add_Part(
            ETOUI(LEVEL::STATIC), strProtoTag, strPartTag, &desc);
    }

    if (FAILED(hr))
        return nullptr;

    const auto& Parts = pGeneric->Get_UIPartObjects();
    auto it = Parts.find(strPartTag);
    CUIPartObject* pPart =
        (it != Parts.end()) ? it->second : nullptr;

    if (pPart && pOutPartTag)
        *pOutPartTag = strPartTag;

    Log_Info("Added UI part: " + WstrToStr(strPartTag));
    return pPart;
}

HRESULT CLevel_Tool::Remove_UIPart(CGameObject* pContainer, const _wstring& strPartTag)
{
    auto* pGeneric =
        dynamic_cast<Client::CUI_GenericContainer*>(pContainer);
    if (!pGeneric)
    {
        Log_Warning("Remove_UIPart: not a GenericContainer");
        return E_FAIL;
    }
    return pGeneric->Remove_Part(strPartTag);
}

_wstring CLevel_Tool::Get_AuthoredProtoTag(CGameObject* pContainer)
{
    if (nullptr == pContainer)
        return L"";

    auto it = m_AuthoredProtoTags.find(pContainer);
    if (it != m_AuthoredProtoTags.end())
        return it->second;

    // 기록 없으면 인스턴스 실제 클래스 태그로 폴백
    CGameObject::ENGINE_OBJECT_DATA data{};
    pContainer->Copy_ObjectData(&data);
    return data.strPrototypeTag;
}

void CLevel_Tool::Set_AuthoredProtoTag(CGameObject* pContainer, const _wstring& strProtoTag)
{
    if (nullptr == pContainer || strProtoTag.empty())
        return;

    m_AuthoredProtoTags[pContainer] = strProtoTag;
}

_wstring CLevel_Tool::Register_TextureProto(const _wstring& strTexturePath)
{
    if (strTexturePath.empty())
        return L"";

    // 파일 stem 기반 태그: ".../KirbyFace.png" -> "Proto_Tex_KirbyFace"
    const _wstring strStem = filesystem::path(strTexturePath).stem().wstring();
    const _wstring strProtoTag = L"Proto_Tex_" + strStem;
    m_TextureProtoPaths[strProtoTag] = strTexturePath;

    const _uint L = ETOUI(TOOL_LEVEL::STATIC);

    // 이미 등록돼 있으면 재사용 (같은 png 중복 로딩/중복 등록 방지)
    if (m_pGameInstance_Proxy->Has_Prototype(L, strProtoTag))
        return strProtoTag;

    CTexture* pTexture = CTexture::Create(m_pDevice, m_pContext, strTexturePath.c_str(), 1);
    if (nullptr == pTexture)
    {
        Log_Error("Register_TextureProto: CTexture::Create failed: " + WstrToStr(strTexturePath));
        return L"";
    }

    if (FAILED(m_pGameInstance_Proxy->Add_Prototype(L, strProtoTag, pTexture)))
    {
        Log_Error("Register_TextureProto: Add_Prototype failed: " + WstrToStr(strProtoTag));
        return L"";
    }

    Log_Info("Registered texture proto: " + WstrToStr(strProtoTag));
    return strProtoTag;
}

void CLevel_Tool::Update(_float fTimeDelta)
{
}

HRESULT CLevel_Tool::Render()
{
    if (m_bGridVisible && m_pGrid)
    {
        const _float4x4* pView =
            m_pGameInstance_Proxy->Get_Matrix(D3DTS::VIEW, PROJ_TYPE::PERSPEC);

        const _float4x4* pProj =
            m_pGameInstance_Proxy->Get_Matrix(D3DTS::PROJ, PROJ_TYPE::PERSPEC);

        m_pGrid->Render(pView, pProj);
    }

    return S_OK;
}

void CLevel_Tool::Set_CameraActive(_bool bActive)
{
    if (m_pCamera) m_pCamera->Set_Active(bActive);
}

void CLevel_Tool::Set_PreviewVisible(_bool bVisible)
{
    if (m_pPreview)
        m_pPreview->Set_Active(bVisible);
}

CGameObject* CLevel_Tool::Load_Preview(const _wstring& strYshPath)
{
    MODEL eType = Read_YshType(strYshPath);
    if (eType == MODEL::END) { Log_Error("Invalid .ysh header."); return nullptr; }

    Clear_Preview();    // 기존 1개 제거(단일 교체형)

    // 모델 proto (경로 단위 재사용)
    std::wstring strModelTag;
    auto it = m_ModelTags.find(strYshPath);
    if (it != m_ModelTags.end())
        strModelTag = it->second;
    else
    {
        strModelTag = L"Proto_Model_" + std::to_wstring(m_iTagCounter++);
        std::string sPath(strYshPath.begin(), strYshPath.end());   // ASCII 경로 가정
        if (FAILED(m_pGameInstance_Proxy->Add_Prototype(ETOUI(TOOL_LEVEL::STATIC), strModelTag,
            CModel::Create(m_pDevice, m_pContext, eType, sPath.c_str()))))
        {
            Log_Error("Model prototype create failed."); return nullptr;
        }
        m_ModelTags[strYshPath] = strModelTag;
    }

    // 액터 proto (1회)
    if (!m_pGameInstance_Proxy->Has_Prototype(ETOUI(TOOL_LEVEL::STATIC), CPreview_Actor::PROTOTYPE_TAG))
        m_pGameInstance_Proxy->Add_Prototype(ETOUI(TOOL_LEVEL::STATIC), CPreview_Actor::PROTOTYPE_TAG,
            CPreview_Actor::Create(m_pDevice, m_pContext));

    CPreview_Actor::PREVIEW_DESC desc{};
    desc.iProtoLevel = ETOUI(TOOL_LEVEL::STATIC);
    desc.eType = eType;
    desc.szModelTag = strModelTag.c_str();      // Initialize 내에서만 사용(동기 호출)
    desc.szShaderTag = (eType == MODEL::ANIM) ? L"Proto_Shader_AnimMesh" : L"Proto_Shader_NonAnimMesh";

    CGameObject* pObj = nullptr;
    if (FAILED(m_pGameInstance_Proxy->Add_GameObject_Return(&pObj,
        ETOUI(TOOL_LEVEL::STATIC), CPreview_Actor::PROTOTYPE_TAG,
        ETOUI(TOOL_LEVEL::EDIT), L"Layer_Preview", L"Preview", &desc)))
        return nullptr;

    m_pPreview = pObj;
    return pObj;
}

CGameObject* CLevel_Tool::Load_Kirby()
{
    const _uint L = ETOUI(TOOL_LEVEL::STATIC);

    Clear_Preview();    // 기존 Preview 정리

    // 1) Shader_Kirby_Proto - Kirby 는 스키닝 메쉬라 VTXANIMMESH 레이아웃 
    if (!m_pGameInstance_Proxy->Has_Prototype(L, L"Proto_Shader_Kirby"))
        m_pGameInstance_Proxy->Add_Prototype(L, L"Proto_Shader_Kirby",
            CShader::Create(m_pDevice, m_pContext, Shader_Kirby.szFileTag,
                VTXANIMMESH::Elements, VTXANIMMESH::iNumElements));

    // 2) Kirby 모델 proto - ANIM, 180도 Y 회전 (GameContent 와 동일하게 정면)
    if (!m_pGameInstance_Proxy->Has_Prototype(L, L"Proto_Model_Kirby"))
        m_pGameInstance_Proxy->Add_Prototype(L, L"Proto_Model_Kirby",
            CModel::Create(m_pDevice, m_pContext, MODEL::ANIM,
                "../../Resources/CHJ/AnimModel/Kirby/Kirby.ysh",
                XMMatrixRotationY(XMConvertToRadians(180.f))));

    // 3) Preview_Kirby proto (1회만)
    if (!m_pGameInstance_Proxy->Has_Prototype(L, CPreview_Kirby::PROTOTYPE_TAG))
        m_pGameInstance_Proxy->Add_Prototype(L, CPreview_Kirby::PROTOTYPE_TAG,
            CPreview_Kirby::Create(m_pDevice, m_pContext));

    // 4) 스폰 
    CPreview_Kirby::PREVIEW_KIRBY_DESC desc{};
    desc.iProtoLevel = L;
    desc.szShaderTag = L"Proto_Shader_Kirby";
    desc.szModelTag = L"Proto_Model_Kirby";
    desc.strAnimEvents = {};

    CGameObject* pObj = nullptr;
    if (FAILED(m_pGameInstance_Proxy->Add_GameObject_Return(&pObj,
        L, CPreview_Kirby::PROTOTYPE_TAG,
        ETOUI(TOOL_LEVEL::EDIT), L"Layer_Preview", L"Preview_Kirby", &desc)))
        return nullptr;

    m_pPreview = pObj;
    return pObj;
}

void CLevel_Tool::Clear_Preview()
{
    if (m_pPreview)
    {
        Log_Info("Clear_Preview: destroy requested.");
        m_pGameInstance_Proxy->Destroy_GameObject(m_pPreview);
        m_pPreview = nullptr;
    }
    else
        Log_Warning("Clear_Preview: m_pPreview == null");
}

void CLevel_Tool::Recalc_CameraProj()
{
    if (m_pCamera)
        m_pCamera->Recalculate_ProjMatrix();
}

CLevel_Tool* CLevel_Tool::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CLevel_Tool* pInstance = new CLevel_Tool(pDevice, pContext);
    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Created : CLevel_Tool");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CLevel_Tool::Free()
{
    __super::Free();

    m_UIContainers.clear();

    Safe_Release(m_pGrid);
}