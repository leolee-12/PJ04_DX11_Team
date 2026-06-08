#include "Loader_Prototype.h"
#include "GameContent_const.h"
#include "GameInstance.h"
#include "DataLoader.h"
#include "GameObject_Factory.h"
#include "GameObject.h"
#include "VIBuffer_Point_Instance.h"
#include "UI_Fonts.h"
#include "UI_Image.h"
#include "UI_Text.h"
#include "UI_SpriteAnim.h"
#include "Texture.h"

NS_BEGIN(Client)

HRESULT Ready_Prototype_SharedResources(CGameInstance_Proxy* pProxy, ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    if (FAILED(pProxy->Add_Prototype(VI_Rect.iLevelID, VI_Rect.szProtoTag,
        CVIBuffer_Rect::Create(pDevice, pContext))))
        return E_FAIL;

    if (FAILED(pProxy->Add_Prototype(VI_Trail.iLevelID, VI_Trail.szProtoTag,
        CVIBuffer_Trail::Create(pDevice, pContext))))
        return E_FAIL;

    if (FAILED(pProxy->Add_Prototype(VI_Point.iLevelID, VI_Point.szProtoTag,
        CVIBuffer_Point::Create(pDevice, pContext))))
        return E_FAIL;

    static const ENV_ENTRY g_EnvTable[] = {
      { TEXT("Field"), TEXT("../../Resources/Env/Field_Diffuse.dds"), TEXT("../../Resources/Env/Field_Specular.dds"), 0.5f },
        // 맵 추가 = 행 추가
    };

    for (auto& e : g_EnvTable)
        pProxy->Register_Environment(e.tag, e.diff, e.spec, e.intensity);


    //sky Sphere

    if (FAILED(pProxy->Add_Prototype(Model_SkyTest.iLevelID, Model_SkyTest.szProtoTag,
        CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/YSH/Sky/Test/Model.ysh"))))
        return E_FAIL;
    
    if (FAILED(Ready_Prototype_UIPartObjects(pProxy, pDevice, pContext)))
        return E_FAIL;

    return S_OK;
}

HRESULT Ready_Prototype_Shaders(CGameInstance_Proxy* pProxy, ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{     
    if (FAILED(pProxy->Add_Prototype(Shader_VtxTex.iLevelID, Shader_VtxTex.szProtoTag,
        CShader::Create(pDevice, pContext, Shader_VtxTex.szFileTag, VTXTEX::Elements, VTXTEX::iNumElements))))
        return E_FAIL;

    if (FAILED(pProxy->Add_Prototype(Shader_NonAnimMesh_PBR.iLevelID, Shader_NonAnimMesh_PBR.szProtoTag,
        CShader::Create(pDevice, pContext, Shader_NonAnimMesh_PBR.szFileTag, VTXMESH::Elements, VTXMESH::iNumElements))))
        return E_FAIL;

    if (FAILED(pProxy->Add_Prototype(Shader_AnimMesh_PBR.iLevelID, Shader_AnimMesh_PBR.szProtoTag,
        CShader::Create(pDevice, pContext, Shader_AnimMesh_PBR.szFileTag, VTXANIMMESH::Elements, VTXANIMMESH::iNumElements))))
        return E_FAIL;

    if (FAILED(pProxy->Add_Prototype(Shader_Map.iLevelID, Shader_Map.szProtoTag,
        CShader::Create(pDevice, pContext, Shader_Map.szFileTag, VTXMAPMESH::Elements, VTXMAPMESH::iNumElements))))
        return E_FAIL;

    if (FAILED(pProxy->Add_Prototype(Shader_MtrlTest.iLevelID, Shader_MtrlTest.szProtoTag,
        CShader::Create(pDevice, pContext, Shader_MtrlTest.szFileTag, VTXMESH::Elements, VTXMESH::iNumElements))))
        return E_FAIL;

    if (FAILED(pProxy->Add_Prototype(Shader_Kirby.iLevelID, Shader_Kirby.szProtoTag,
        CShader::Create(pDevice, pContext, Shader_Kirby.szFileTag, VTXANIMMESH::Elements, VTXANIMMESH::iNumElements))))
        return E_FAIL;

    if (FAILED(pProxy->Add_Prototype(Shader_SkySphere.iLevelID, Shader_SkySphere.szProtoTag,
        CShader::Create(pDevice, pContext, Shader_SkySphere.szFileTag, VTXMESH::Elements, VTXMESH::iNumElements))))
        return E_FAIL;

    if (FAILED(pProxy->Add_Prototype(Shader_UI.iLevelID, Shader_UI.szProtoTag,
        CShader::Create(pDevice, pContext, Shader_UI.szFileTag, VTXTEX::Elements, VTXTEX::iNumElements))))
        return E_FAIL;

    return S_OK;
}

HRESULT CLIENT_DLL Load_Level(CGameInstance_Proxy* pProxy, ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const _tchar* strFilePath, _uint iLevelIndex)
{
    string strContent = {};
    if (FAILED(CDataLoader::Read_Json(strFilePath, &strContent)))
        return E_FAIL;

    json jLevel = json::parse(strContent);

    for (auto& jObj : jLevel["Objects"])
    {
        wstring wProto = StrToWstr(jObj["Prototype_Tag"].get<string>());
        wstring wLayer = StrToWstr(jObj["Layer_Tag"].get<string>());
        wstring wObjectName = StrToWstr(jObj["Object_Tag"].get<string>());

        auto* pReg = CGameObject_Factory::GetInstance()->Get_Registration(wProto);
        if (!pReg) continue;

        if (!pProxy->Has_Prototype(iLevelIndex, wProto))
        {
            pReg->ResourceLoader(pProxy, pDevice, pContext);
            pProxy->Add_Prototype(iLevelIndex, wProto.c_str(),
                pReg->CreatorFunc(pDevice, pContext));
        }

        CGameObject* pObj = nullptr;
        pProxy->Add_GameObject_Return(
            &pObj,
            iLevelIndex, wProto.c_str(),
            iLevelIndex, wLayer.c_str(), wObjectName, nullptr);

        if (pObj)
            pObj->Deserialize(jObj);
    }
    return S_OK;
}

HRESULT CLIENT_DLL Load_Fonts(CGameInstance_Proxy* pProxy)
{
    for (auto& f : g_UIFonts)
        pProxy->Add_Font(f.szTag, f.szPath);

    return S_OK;
}

HRESULT CLIENT_DLL Ready_Prototype_UIPartObjects(CGameInstance_Proxy* pProxy, ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    if (FAILED(pProxy->Add_Prototype(ETOUI(LEVEL::STATIC), CUI_Image::PROTOTYPE_TAG,
        CUI_Image::Create(pDevice, pContext))))
        return E_FAIL;

    if (FAILED(pProxy->Add_Prototype(ETOUI(LEVEL::STATIC), CUI_SpriteAnim::PROTOTYPE_TAG,
        CUI_SpriteAnim::Create(pDevice, pContext))))
        return E_FAIL;

    if (FAILED(pProxy->Add_Prototype(ETOUI(LEVEL::STATIC), CUI_Text::PROTOTYPE_TAG,
        CUI_Text::Create(pDevice, pContext))))
        return E_FAIL;

    return S_OK;
}

HRESULT CLIENT_DLL Ready_Level_UIResources(CGameInstance_Proxy* pProxy, ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const _tchar* strFilePath, _uint iLevelIndex)
{
    string strFileContent;
    if (FAILED(CDataLoader::Read_Json(strFilePath, &strFileContent)))
        return E_FAIL;

    json jFile = json::parse(strFileContent);

    if (!jFile.contains("UIContainers") || !jFile["UIContainers"].is_array())
        return E_FAIL;

    for (const auto& jEntry : jFile["UIContainers"])
    {
        const string strPathA = jEntry.value("Path", string());
        if (strPathA.empty())
            return E_FAIL;

        string strUIContent;
        if (FAILED(CDataLoader::Read_Json(StrToWstr(strPathA).c_str(), &strUIContent)))
            return E_FAIL;

        json jUI = json::parse(strUIContent);

        const _wstring strContainerProtoTag = StrToWstr(jUI.value("ProtoTag", string()));

        if (strContainerProtoTag.empty())
            return E_FAIL;

        auto* pReg = CGameObject_Factory::GetInstance()
            ->Get_Registration(strContainerProtoTag);

        if (!pReg || pReg->strCategory != L"UI_CONTAINER")
            return E_FAIL;

        if (!pProxy->Has_Prototype(iLevelIndex, strContainerProtoTag))
        {
            pReg->ResourceLoader(pProxy, pDevice, pContext);

            if (FAILED(pProxy->Add_Prototype(iLevelIndex, strContainerProtoTag, pReg->CreatorFunc(pDevice, pContext))))
                return E_FAIL;
        }

        if (jUI.contains("Textures") && jUI["Textures"].is_object())
        {
            for (const auto& [strTextureProtoTag, jTexturePath] : jUI["Textures"].items())
            {
                const _wstring wTextureProtoTag = StrToWstr(strTextureProtoTag);
                const _wstring wTexturePath = StrToWstr(jTexturePath.get<string>());

                if (!pProxy->Has_Prototype(iLevelIndex, wTextureProtoTag))
                {
                    CTexture* pTexture = CTexture::Create(pDevice, pContext, wTexturePath.c_str(), 1);
                    if (nullptr == pTexture)
                        return E_FAIL;

                    if (FAILED(pProxy->Add_Prototype(iLevelIndex, wTextureProtoTag, pTexture)))
                    {
                        Safe_Release(pTexture);
                        return E_FAIL;
                    }
                }
            }
        }
    }

    return S_OK;
}

HRESULT CLIENT_DLL Load_Level_UI(CGameInstance_Proxy* pProxy, ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const _tchar* strFilePath, _uint iLevelIndex)
{
    const _uint iContProtoLevel = iLevelIndex;      // Container의 Proto도 해당 레벨에 등록했기 때문에 여기서 찾아야함
    const _uint iObjLevel    = iLevelIndex;

    string strFileContent;
    if (FAILED(CDataLoader::Read_Json(strFilePath, &strFileContent)))
        return E_FAIL;

    json jFile;
    try
    {
        jFile = json::parse(strFileContent);
    }
    catch (const std::exception&)
    {
        return E_FAIL;
    }

    if (!jFile.contains("UIContainers") ||
        !jFile["UIContainers"].is_array())
    {
        return E_FAIL;
    }

    for (const auto& jEntry : jFile["UIContainers"])
    {
        const string strObjectTagA =
            jEntry.value("ObjectTag", string());

        const string strPathA =
            jEntry.value("Path", string());

        const string strLayerTagA =
            jEntry.value("LayerTag", string("Layer_UI"));

        const _bool bInitialActive =
            jEntry.value("InitialActive", true);

        if (strObjectTagA.empty() || strPathA.empty())
            return E_FAIL;

        const _wstring strObjectTag = StrToWstr(strObjectTagA);
        const _wstring strPath = StrToWstr(strPathA);
        const _wstring strLayerTag = StrToWstr(strLayerTagA);

        string strUIContent;
        if (FAILED(CDataLoader::Read_Json(strPath.c_str(), &strUIContent)))
            return E_FAIL;

        json jUI;
        try
        {
            jUI = json::parse(strUIContent);
        }
        catch (const std::exception&)
        {
            return E_FAIL;
        }

        // 아직 Loader를 거치지 않고 바로 GamePlay 들어가기 때문에 
        // 임시 fallback 등록 나중에 Loader에서 호출하면 제거 가능
        /*if (jUI.contains("Textures") && jUI["Textures"].is_object())
        {
            for (const auto& [strTextureProtoTag, jTexturePath] : jUI["Textures"].items())
            {
                if (strTextureProtoTag.empty() || !jTexturePath.is_string())
                    return E_FAIL;

                const _wstring wTextureProtoTag = StrToWstr(strTextureProtoTag);
                const _wstring wTexturePath = StrToWstr(jTexturePath.get<string>());

                if (wTextureProtoTag.empty() || wTexturePath.empty())
                    return E_FAIL;

                if (!pProxy->Has_Prototype(iLevelIndex, wTextureProtoTag))
                {
                    CTexture* pTexture = CTexture::Create(
                        pDevice,
                        pContext,
                        wTexturePath.c_str(),
                        1);

                    if (nullptr == pTexture)
                        return E_FAIL;

                    if (FAILED(pProxy->Add_Prototype(
                        iLevelIndex,
                        wTextureProtoTag,
                        pTexture)))
                    {
                        Safe_Release(pTexture);
                        return E_FAIL;
                    }
                }
            }
        }*/


        const _wstring strProtoTag =
            StrToWstr(jUI.value("ProtoTag", string()));

        if (strProtoTag.empty())
            return E_FAIL;

        auto* pReg = CGameObject_Factory::GetInstance()
            ->Get_Registration(strProtoTag);

        if (nullptr == pReg)
            return E_FAIL;

        if (pReg->strCategory != L"UI_CONTAINER")
            return E_FAIL;

        // Loading까지 들어가면 주석해제 (최종 사용본)
        if (!pProxy->Has_Prototype(iContProtoLevel, strProtoTag))
            return E_FAIL;


        // Loader 거치지 않는 fallback 등록 버전 - 나중에 제거해야함
        /*if (!pProxy->Has_Prototype(iContProtoLevel, strProtoTag))
        {
            pReg->ResourceLoader(pProxy, pDevice, pContext);

            if (FAILED(pProxy->Add_Prototype(
                iContProtoLevel,
                strProtoTag.c_str(),
                pReg->CreatorFunc(pDevice, pContext))))
            {
                return E_FAIL;
            }
        }*/

        CGameObject* pObj = nullptr;

        if (FAILED(pProxy->Add_GameObject_Return(&pObj, iContProtoLevel, strProtoTag.c_str(), iObjLevel, strLayerTag.c_str(), strObjectTag, nullptr)))
            return E_FAIL;

        if (nullptr == pObj)
            return E_FAIL;

        if (jUI.contains("Container"))
        {
            json jContainer = jUI["Container"];
            
            if (jContainer.contains("UIPartObjects") && jContainer["UIPartObjects"].is_object())
            {
                for (auto& [strPartTag, jPart] : jContainer["UIPartObjects"].items())
                {
                    // Part prototype은 항상 STATIC에 있음
                    jPart["ProtoLevel"] = ETOUI(LEVEL::STATIC);

                    // TextureProtoTag가 있는 Part만 현재 Texture prototype을 사용 ( Game Level과 AnimUITool의 LEVEL 인덱스가 다른 부분 보정 )
                    if (jPart.contains("TextureProtoTag") && !jPart["TextureProtoTag"].get<string>().empty())
                    {
                        jPart["TextureLevel"] = iLevelIndex;
                    }
                }
            }

            pObj->Deserialize(jContainer);
        }

        pObj->Set_Active(bInitialActive);
    }

    return S_OK;
}

NS_END


