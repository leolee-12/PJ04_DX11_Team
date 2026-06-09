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
#include "Loader_Prototype.h"

#include "UI_Title.h"
#include "Panel_Manager.h"
#include "UIContainerObject.h"
#include "UIPartObject.h"
#include "GameObject.h"
#include "UI_GenericContainer.h"
#include "UI_SpriteAnim.h"
#include "UI_Text.h"

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
    LightDesc.vDirection = _float4(0.4f, -1.f, 0.5f, 0.f);

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

void CLevel_Tool::Track_UIContainer(CGameObject* pObject, const _wstring& strPath, const _float2& vDesignSize)
{
    auto* pContainer = dynamic_cast<CUIContainerObject*>(pObject);
    if (nullptr == pContainer)
        return;

    _wstring strLayerTag = pContainer->Get_LayerTag();
    if (strLayerTag.empty())
        strLayerTag = L"Layer_UI";

    auto it = std::find_if(
        m_UIContainers.begin(),
        m_UIContainers.end(),
        [pContainer](const UI_CONTAINER_ENTRY& Entry)
        {
            return Entry.pContainer == pContainer;
        });

    if (it != m_UIContainers.end())
    {
        if (!strPath.empty())
            it->strPath = strPath;

        it->strLayerTag = strLayerTag;
        it->vDesignSize = vDesignSize;
        return;
    }

    UI_CONTAINER_ENTRY Entry{};
    Entry.pContainer = pContainer;
    Entry.strPath = strPath;
    Entry.strLayerTag = strLayerTag;
    Entry.vDesignSize = vDesignSize;
    Entry.bExport = true;

    m_UIContainers.push_back(Entry);
}

void CLevel_Tool::UnTrack_UIContainer(CGameObject* pObject)
{
    auto* pContainer = dynamic_cast<CUIContainerObject*>(pObject);
    if (nullptr == pContainer)
        return;

    m_UIContainers.erase(
        std::remove_if(
            m_UIContainers.begin(),
            m_UIContainers.end(),
            [pContainer](const UI_CONTAINER_ENTRY& Entry)
            {
                return Entry.pContainer == pContainer;
            }),
        m_UIContainers.end());
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
                {
                    jTextures[strTag] = WstrToStr(it->second);
                }
                else
                {
                    Log_Error("Save_UIContainer: missing texture path for " + strTag);
                    return E_FAIL;
                }
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

        Track_UIContainer(pContainer, path.wstring(), vDesignSize);

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

    const _uint iContainerProtoLevel = ETOUI(TOOL_LEVEL::EDIT);
    const _uint iObjectLevel = ETOUI(TOOL_LEVEL::EDIT);
    const _uint iPartProtoLevel = ETOUI(TOOL_LEVEL::STATIC);

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

        if (!m_pGameInstance_Proxy->Has_Prototype(iContainerProtoLevel, strSpawnTag))
        {
            pReg->ResourceLoader(
                m_pGameInstance_Proxy, m_pDevice, m_pContext);
            m_pGameInstance_Proxy->Add_Prototype(iContainerProtoLevel, strSpawnTag,
                pReg->CreatorFunc(m_pDevice, m_pContext));
        }

        if (FAILED(m_pGameInstance_Proxy->Add_GameObject_Return(
            &pObj, iContainerProtoLevel, strSpawnTag,
            ETOUI(TOOL_LEVEL::EDIT), L"Layer_UI", strName, nullptr)))
            return nullptr;

        vOutDesignSize = _float2{ 1600.f, 900.f };

        if (j.contains("DesignSize") &&
            j["DesignSize"].is_array() &&
            j["DesignSize"].size() == 2)
        {
            vOutDesignSize = {
                j["DesignSize"][0].get<float>(),
                j["DesignSize"][1].get<float>()
            };
        }

        Set_AuthoredProtoTag(pObj, strAuthoredTag);

        if (j.contains("Textures") && j["Textures"].is_object())
        {
            for (auto& [strTag, jPath] : j["Textures"].items())
            {
                Register_TextureProto(
                    StrToWstr(strTag),
                    StrToWstr(jPath.get<std::string>()),
                    iObjectLevel);
            }
        }

        if (j.contains("Container"))
        {
            json jContainer = j["Container"];

            if (jContainer.contains("UIPartObjects") &&
                jContainer["UIPartObjects"].is_object())
            {
                for (auto& [strPartTag, jPart] :
                    jContainer["UIPartObjects"].items())
                {
                    jPart["ProtoLevel"] = iPartProtoLevel;

                    if (jPart.contains("TextureProtoTag") &&
                        !jPart.value("TextureProtoTag", std::string()).empty())
                    {
                        jPart["TextureLevel"] = iObjectLevel;
                    }
                }
            }

            pObj->Deserialize(jContainer);
        }

        Track_UIContainer(pObj, strFullPath, vOutDesignSize);

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
    if (nullptr == pContainer)
        return;

    UnTrack_UIContainer(pContainer);

    m_AuthoredProtoTags.erase(pContainer);

    m_pGameInstance_Proxy->Destroy_GameObject(pContainer);
    Log_Info("Deleted UI container.");
}

CGameObject* CLevel_Tool::Add_UIContainer()
{
    const _uint iContainerProtoLevel = ETOUI(TOOL_LEVEL::EDIT);
    const _uint iObjectLevel = ETOUI(TOOL_LEVEL::EDIT);
    const _wstring strTag = Client::CUI_GenericContainer::PROTOTYPE_TAG;

    auto* pReg = Client::CGameObject_Factory::GetInstance()
        ->Get_Registration(strTag);
    if (!pReg)
    {
        Log_Error("Add_UIContainer: GenericContainer not registered");
        return nullptr;
    }

    if (!m_pGameInstance_Proxy->Has_Prototype(iContainerProtoLevel, strTag))
    {
        pReg->ResourceLoader(m_pGameInstance_Proxy, m_pDevice, m_pContext);
        if (FAILED(m_pGameInstance_Proxy->Add_Prototype(iContainerProtoLevel, strTag,
            pReg->CreatorFunc(m_pDevice, m_pContext))))
            return nullptr;
    }

    Client::CUI_GenericContainer::UI_GENERIC_CONTAINER_DESC desc{};
    desc.vPosition = { 0.f, 0.f, 0.f, 1.f };   // 센터 배치

    _wstring strName =
        L"UIContainer_" + std::to_wstring(m_iUIContainerCounter++);

    CGameObject* pObj = nullptr;
    if (FAILED(m_pGameInstance_Proxy->Add_GameObject_Return(&pObj, iContainerProtoLevel, strTag,
        iObjectLevel, L"Layer_UI", strName, &desc)))
        return nullptr;

    Track_UIContainer(pObj, L"", _float2{1600.f, 900.f});
    Set_AuthoredProtoTag(pObj, strTag);   // 기본 ProtoTag = 자기(self-host)
    Log_Info("Added UI container: " + WstrToStr(strName));
    return pObj;
}

CUIPartObject* CLevel_Tool::Add_UIPart(CGameObject* pContainer, UI_PART_TYPE eType, _wstring* pOutPartTag)
{
    const _uint iPartProtoLevel = ETOUI(TOOL_LEVEL::STATIC);
    const _uint iTextureLevel = ETOUI(TOOL_LEVEL::EDIT);

    auto* pGeneric =
        dynamic_cast<Client::CUI_GenericContainer*>(pContainer);
    if (!pGeneric)
    {
        Log_Warning("Add_UIPart: not a GenericContainer");
        return nullptr;
    }

    _wstring strProtoTag = L"";

    if (eType == UI_PART_TYPE::IMAGE)
        strProtoTag = Client::CUI_Image::PROTOTYPE_TAG;
    else if (eType == UI_PART_TYPE::SPRITEANIM)
        strProtoTag = Client::CUI_SpriteAnim::PROTOTYPE_TAG;
    else if (eType == UI_PART_TYPE::TEXT)
        strProtoTag = Client::CUI_Text::PROTOTYPE_TAG;

    // 파트 프로토 보장 (SpriteAnim은 컨테이너 로더가 안 올림)
    if (!m_pGameInstance_Proxy->Has_Prototype(iPartProtoLevel, strProtoTag))
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
        if (FAILED(m_pGameInstance_Proxy->Add_Prototype(iPartProtoLevel, strProtoTag, pReg->CreatorFunc(m_pDevice, m_pContext))))
        {
            return nullptr;
        }
    }

    _wstring strPartTag =
        L"Part_" + std::to_wstring(m_iUIPartCounter++);

    HRESULT hr = E_FAIL;

    switch (eType)
    {
    case UI_PART_TYPE::IMAGE:
    {
        Client::CUI_Image::UI_IMAGE_DESC desc{};
        desc.iTextureLevel = iTextureLevel;
        desc.szTextureProtoTag = { nullptr };
        desc.vSize = { 100.f, 100.f };
        desc.vPosition = { 0.f, 0.f };
        desc.iRenderLayer = 1;

        hr = pGeneric->Add_Part(iPartProtoLevel, strProtoTag, strPartTag, &desc);
        break;
    }

    case UI_PART_TYPE::SPRITEANIM:
    {
        Client::CUI_SpriteAnim::UI_SPRITEANIM_DESC desc{};
        desc.iTextureLevel = iTextureLevel;
        desc.szTextureProtoTag = { nullptr };
        desc.vSize = { 100.f, 100.f };
        desc.vPosition = { 0.f, 0.f };
        desc.iRenderLayer = 1;
        desc.fDuration = 1.f;
        desc.fEndDelay = 0.f;
        desc.bLoop = true;
        desc.bAutoPlay = true;

        hr = pGeneric->Add_Part(iPartProtoLevel, strProtoTag, strPartTag, &desc);
        break;
    }

    case UI_PART_TYPE::TEXT:
    {
        Client::CUI_Text::UI_TEXT_DESC desc{};
        desc.szText = L"Text";
        desc.szFontTag = L"KOR-FOT-RodinNTLGPro-B";
        desc.vColor = { 1.f, 1.f, 1.f, 1.f };
        desc.fFontScale = 1.f;
        desc.iAlign = 1;
        desc.vPosition = { 0.f, 0.f };
        desc.iRenderLayer = 2;
        desc.vSize = { 300.f, 100.f };

        hr = pGeneric->Add_Part(
            iPartProtoLevel, strProtoTag, strPartTag, &desc);

        break;
    }

    default:
        hr = E_FAIL;
        break;
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

HRESULT CLevel_Tool::Rename_UIPart(CGameObject* pContainer, const _wstring& strOldTag, const _wstring& strNewTag)
{
    auto* pGeneric = dynamic_cast<CUI_GenericContainer*>(pContainer);
    if (nullptr == pGeneric)
        return E_FAIL;

    return pGeneric->Rename_Part(strOldTag, strNewTag);
}

HRESULT CLevel_Tool::Save_UIManifest(const _wstring& strManifestPath)
{
    if (strManifestPath.empty())
        return E_FAIL;

    json jManifest;
    jManifest["UIContainers"] = json::array();

    for (const UI_CONTAINER_ENTRY& Entry : m_UIContainers)
    {
        if (nullptr == Entry.pContainer)
            continue;

        if (!Entry.bExport)
            continue;

        if (Entry.strPath.empty())
        {
            Log_Warning("Save_UIManifest: skipped unsaved container.");
            continue;
        }

        _wstring strLayerTag = Entry.strLayerTag;

        if (strLayerTag.empty())
            strLayerTag = Entry.pContainer->Get_LayerTag();

        if (strLayerTag.empty())
            strLayerTag = L"Layer_UI";

        json jEntry;
        jEntry["ObjectTag"] = WstrToStr(Entry.pContainer->Get_ObjectTag());
        jEntry["Path"] = WstrToStr(Entry.strPath);
        jEntry["LayerTag"] = WstrToStr(strLayerTag);
        jEntry["InitialActive"] = Entry.pContainer->Is_Active() != false;

        jManifest["UIContainers"].push_back(jEntry);
    }

    namespace fs = std::filesystem;
    fs::path path = strManifestPath;

    std::error_code ec;
    if (path.has_parent_path())
        fs::create_directories(path.parent_path(), ec);

    std::ofstream fout(path);
    if (!fout.is_open())
    {
        Log_Error("Save_UIManifest open fail: " + WstrToStr(path.wstring()));
        return E_FAIL;
    }

    fout << jManifest.dump(2, ' ', false, json::error_handler_t::replace);
    fout.close();

    Log_Info("Saved UI manifest: " + WstrToStr(path.wstring()));
    return S_OK;
}

HRESULT CLevel_Tool::Load_UIManifest(const _wstring& strManifestPath)
{
    if (strManifestPath.empty())
        return E_FAIL;

    if (FAILED(Client::Ready_Level_UIResources(
        m_pGameInstance_Proxy,
        m_pDevice,
        m_pContext,
        strManifestPath.c_str(),
        ETOUI(TOOL_LEVEL::EDIT))))
    {
        Log_Error("Load_UIManifest: Ready_Level_UIResources failed.");
        return E_FAIL;
    }

    std::ifstream fin(strManifestPath);
    if (!fin.is_open())
    {
        Log_Error("Load_UIManifest open fail: " + WstrToStr(strManifestPath));
        return E_FAIL;
    }

    json jManifest;
    try
    {
        fin >> jManifest;
    }
    catch (const std::exception& e)
    {
        Log_Error(std::string("Load_UIManifest parse fail: ") + e.what());
        return E_FAIL;
    }

    if (!jManifest.contains("UIContainers") || !jManifest["UIContainers"].is_array())
    {
        Log_Error("Load_UIManifest: UIContainers array missing.");
        return E_FAIL;
    }

    const _uint iContainerProtoLevel = ETOUI(TOOL_LEVEL::EDIT);
    const _uint iObjectLevel = ETOUI(TOOL_LEVEL::EDIT);
    const _uint iPartProtoLevel = ETOUI(TOOL_LEVEL::STATIC);

    for (const auto& jEntry : jManifest["UIContainers"])
    {
        const _wstring strPath =
            StrToWstr(jEntry.value("Path", std::string()));

        const _wstring strObjectTag =
            StrToWstr(jEntry.value("ObjectTag", std::string()));

        _wstring strLayerTag =
            StrToWstr(jEntry.value("LayerTag", std::string("Layer_UI")));

        const _bool bInitialActive =
            jEntry.value("InitialActive", true);

        if (strPath.empty() || strObjectTag.empty())
        {
            Log_Warning("Load_UIManifest: skipped invalid entry.");
            continue;
        }

        std::ifstream uiFin(strPath);
        if (!uiFin.is_open())
        {
            Log_Error("Load_UIManifest: UI file open fail: " + WstrToStr(strPath));
            continue;
        }

        json jUI;
        try
        {
            uiFin >> jUI;
        }
        catch (const std::exception& e)
        {
            Log_Error(std::string("Load_UIManifest: UI json parse fail: ") + e.what());
            continue;
        }

        _wstring strAuthoredTag =
            StrToWstr(jUI.value("ProtoTag", std::string()));

        if (strAuthoredTag.empty())
        {
            Log_Error("Load_UIManifest: UI ProtoTag missing: " + WstrToStr(strPath));
            continue;
        }

        _wstring strSpawnTag = strAuthoredTag;

        auto* pReg = Client::CGameObject_Factory::GetInstance()
            ->Get_Registration(strSpawnTag);

        if (pReg && pReg->strCategory != L"UI_CONTAINER")
            pReg = nullptr;

        if (!pReg)
        {
            strSpawnTag = Client::CUI_GenericContainer::PROTOTYPE_TAG;
            pReg = Client::CGameObject_Factory::GetInstance()
                ->Get_Registration(strSpawnTag);
        }

        if (!pReg)
        {
            Log_Error("Load_UIManifest: GenericContainer not registered.");
            continue;
        }

        if (!m_pGameInstance_Proxy->Has_Prototype(iContainerProtoLevel, strSpawnTag))
        {
            pReg->ResourceLoader(m_pGameInstance_Proxy, m_pDevice, m_pContext);
            if (FAILED(m_pGameInstance_Proxy->Add_Prototype(
                iContainerProtoLevel,
                strSpawnTag,
                pReg->CreatorFunc(m_pDevice, m_pContext))))
            {
                Log_Error("Load_UIManifest: Add_Prototype failed.");
                continue;
            }
        }

        CGameObject* pObj = nullptr;
        if (FAILED(m_pGameInstance_Proxy->Add_GameObject_Return(
            &pObj,
            iContainerProtoLevel,
            strSpawnTag,
            iObjectLevel,
            strLayerTag,
            strObjectTag,
            nullptr)))
        {
            Log_Error("Load_UIManifest: Add_GameObject_Return failed.");
            continue;
        }

        Set_AuthoredProtoTag(pObj, strAuthoredTag);

        if (jUI.contains("Textures") && jUI["Textures"].is_object())
        {
            for (auto& [strTag, jPath] : jUI["Textures"].items())
            {
                Register_TextureProto(StrToWstr(strTag), StrToWstr(jPath.get<std::string>()), iObjectLevel);
            }
        }

        if (jUI.contains("Container"))
        {
            json jContainer = jUI["Container"];

            if (jContainer.contains("UIPartObjects") &&
                jContainer["UIPartObjects"].is_object())
            {
                for (auto& [strPartTag, jPart] :
                    jContainer["UIPartObjects"].items())
                {
                    jPart["ProtoLevel"] = iPartProtoLevel;

                    if (jPart.contains("TextureProtoTag") &&
                        !jPart.value("TextureProtoTag", std::string()).empty())
                    {
                        jPart["TextureLevel"] = iObjectLevel;
                    }
                }
            }

            pObj->Deserialize(jContainer);
        }

        pObj->Set_Active(bInitialActive);

        _float2 vDesignSize = { 1600.f, 900.f };
        if (jUI.contains("DesignSize") &&
            jUI["DesignSize"].is_array() &&
            jUI["DesignSize"].size() == 2)
        {
            vDesignSize = {
                jUI["DesignSize"][0].get<float>(),
                jUI["DesignSize"][1].get<float>()
            };
        }

        Track_UIContainer(pObj, strPath, vDesignSize);
    }

    Log_Info("Loaded UI manifest: " + WstrToStr(strManifestPath));
    return S_OK;
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

    const _wstring strStem = std::filesystem::path(strTexturePath).stem().wstring();
    const _wstring strProtoTag = L"Proto_Tex_" + strStem;

    return Register_TextureProto(
        strProtoTag,
        strTexturePath,
        ETOUI(TOOL_LEVEL::EDIT));
}

_wstring CLevel_Tool::Register_TextureProto(const _wstring& strTextureProtoTag, const _wstring& strTexturePath, _uint iLevelIndex)
{
    if (strTextureProtoTag.empty() || strTexturePath.empty())
        return L"";

    m_TextureProtoPaths[strTextureProtoTag] = strTexturePath;

    if (m_pGameInstance_Proxy->Has_Prototype(iLevelIndex, strTextureProtoTag))
        return strTextureProtoTag;

    CTexture* pTexture = CTexture::Create(
        m_pDevice,
        m_pContext,
        strTexturePath.c_str(),
        1);

    if (nullptr == pTexture)
    {
        Log_Error("Register_TextureProto: CTexture::Create failed: " + WstrToStr(strTexturePath));
        return L"";
    }

    if (FAILED(m_pGameInstance_Proxy->Add_Prototype(
        iLevelIndex,
        strTextureProtoTag,
        pTexture)))
    {
        Safe_Release(pTexture);
        Log_Error("Register_TextureProto: Add_Prototype failed: " + WstrToStr(strTextureProtoTag));
        return L"";
    }

    Log_Info("Registered texture proto: " + WstrToStr(strTextureProtoTag));
    return strTextureProtoTag;
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