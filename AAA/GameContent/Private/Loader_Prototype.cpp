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
#include "Movement.h"
#include "UI_Effect.h"
#include "UI_GaugeFill.h"
#include "Effect_Loader.h"
#include "UI_SpriteAnimCurtain.h"
#include "UI_Curtain.h"
#include "UI_Eraser.h"
#include "UI_CurtainTexture.h"
#include "Env_InstanceController.h"
#include "World_BlendCollector.h"
#include "Collider.h"
#include "Bubble_Manager.h"
#include "Map_Loader.h"
#include "LD_DeformObject.h"
#include "DropStar_Manager.h"
#include "UI_CurtainStatic.h"
#include "UI_CurtainStamp.h"
#include "UI_CurtainFadeOut.h"

NS_BEGIN(Client)

HRESULT Ready_Prototype_SharedResources(CGameInstance_Proxy* pProxy, ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    // 나중에 이펙트 전체 STATIC으로 로드하면 STATIC으로 바꿀것
    pProxy->Set_EffectPrototypeLevel(ETOUI(LEVEL::STATIC));

    if (FAILED(CMap_Loader::Ready_TexHub(pProxy)))
        return E_FAIL;

    // 셰어드 리소스 준비 직후 1회
    if (FAILED(CEffect_Loader::GetInstance()->Ready(pProxy, pDevice, pContext, ETOUI(LEVEL::STATIC))))
        return E_FAIL;

    if (FAILED(CBubble_Manager::GetInstance()->Initialize(pDevice, pContext)))
        return E_FAIL;

    if (FAILED(CDropStar_Manager::GetInstance()->Initialize(pDevice, pContext)))
        return E_FAIL;

    if (FAILED(CLD_DeformObject::Register_StaticPrototype(pProxy, pDevice, pContext)))
        return E_FAIL;

    // 게임플레이에서 호출 커비 등이 자기 레벨로 스폰
    //CEffect_Loader::GetInstance()->Spawn(L"InhaleContainer", Get_LevelIndex(), vMouthPos, vLook, pParent);

    if (!pProxy->Has_Prototype(VI_Rect.iLevelID, VI_Rect.szProtoTag))
    {
        if (FAILED(pProxy->Add_Prototype(VI_Rect.iLevelID, VI_Rect.szProtoTag,
            CVIBuffer_Rect::Create(pDevice, pContext))))
            return E_FAIL;
    }

    if (!pProxy->Has_Prototype(VI_Trail.iLevelID, VI_Trail.szProtoTag))
    {
        if (FAILED(pProxy->Add_Prototype(VI_Trail.iLevelID, VI_Trail.szProtoTag,
            CVIBuffer_Trail::Create(pDevice, pContext))))
            return E_FAIL;
    }

    if (!pProxy->Has_Prototype(VI_Point.iLevelID, VI_Point.szProtoTag))
    {
        if (FAILED(pProxy->Add_Prototype(VI_Point.iLevelID, VI_Point.szProtoTag,
            CVIBuffer_Point::Create(pDevice, pContext))))
            return E_FAIL;
    }

    if (FAILED(pProxy->Add_Prototype(Collider_Sphere.iLevelID, Collider_Sphere.szProtoTag,
        CCollider::Create(pDevice, pContext, COLLIDER::SPHERE))))
        return E_FAIL;

    if (FAILED(pProxy->Add_Prototype(Collider_AABB.iLevelID, Collider_AABB.szProtoTag,
        CCollider::Create(pDevice, pContext, COLLIDER::AABB))))
        return E_FAIL;

    if (FAILED(pProxy->Add_Prototype(Collider_OBB.iLevelID, Collider_OBB.szProtoTag,
        CCollider::Create(pDevice, pContext, COLLIDER::OBB))))
        return E_FAIL;

    if (FAILED(pProxy->Add_Prototype(Collider_Capsule.iLevelID, Collider_Capsule.szProtoTag,
        CCollider::Create(pDevice, pContext, COLLIDER::CAPSULE))))
        return E_FAIL;

    if (FAILED(pProxy->Add_Prototype(Collider_Torus.iLevelID, Collider_Torus.szProtoTag,
        CCollider::Create(pDevice, pContext, COLLIDER::TORUS))))
        return E_FAIL;

    static const ENV_ENTRY g_EnvTable[] = {
      { 
        TEXT("Default"), 
        TEXT("../../Resources/YSH/Env/IBL/Stage0_Step1/Diffuse.dds"), 
        TEXT("../../Resources/YSH/Env/IBL/Stage0_Step1/Specular.dds"),
        TEXT("../../Resources/YSH/Env/LUT/Grass01.dds"),
        1.f 
      },
      {
        TEXT("Volcano"),
        TEXT("../../Resources/YSH/Env/IBL/Stage1/Diffuse.dds"),
        TEXT("../../Resources/YSH/Env/IBL/Stage1/Specular.dds"),
        TEXT("../../Resources/YSH/Env/LUT/Volcano01.dds"),
        1.f
      },
        {
        TEXT("Arena"),
        TEXT("../../Resources/YSH/Env/IBL/Arena/Diffuse.dds"),
        TEXT("../../Resources/YSH/Env/IBL/Arena/Specular.dds"),
        TEXT("../../Resources/YSH/Env/LUT/Volcano01.dds"),
        1.f
      },
        {
        TEXT("Ending"),
        TEXT("../../Resources/YSH/Env/IBL/Ending/Diffuse.dds"),
        TEXT("../../Resources/YSH/Env/IBL/Ending/Specular.dds"),
        TEXT("../../Resources/YSH/Env/LUT/Credit01_01.dds"),
        1.f
      },
        // 맵 추가 = 행 추가
    };

    for (auto& e : g_EnvTable)
        pProxy->Register_Environment(e.tag, e.diff, e.spec, e.lut, e.intensity);


    //sky Sphere
    if (FAILED(pProxy->Add_Prototype(Model_SkyDefault.iLevelID, Model_SkyDefault.szProtoTag,
        CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/YSH/Sky/Default/Model.ysh"))))
        return E_FAIL;
    if (FAILED(pProxy->Add_Prototype(Model_SkyVolcano.iLevelID, Model_SkyVolcano.szProtoTag,
        CModel::Create(pDevice, pContext, MODEL::NONANIM, "../../Resources/YSH/Sky/Stage1/Model.ysh"))))
        return E_FAIL;

    if (FAILED(pProxy->Add_Prototype(ETOUI(LEVEL::STATIC),
        CEnv_InstanceController::PROTOTYPE_TAG, CEnv_InstanceController::Create(pDevice, pContext))))
        return E_FAIL;

    if (FAILED(pProxy->Add_Prototype(ETOUI(LEVEL::STATIC),
        CWorld_BlendCollector::PROTOTYPE_TAG, CWorld_BlendCollector::Create(pDevice, pContext))))
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

    if (FAILED(pProxy->Add_Prototype(Shader_MapEx.iLevelID, Shader_MapEx.szProtoTag,
        CShader::Create(pDevice, pContext, Shader_MapEx.szFileTag, VTXMAPMESH::Elements, VTXMAPMESH::iNumElements))))
        return E_FAIL;

    if (FAILED(pProxy->Add_Prototype(Shader_World_NonAnim.iLevelID, Shader_World_NonAnim.szProtoTag,
        CShader::Create(pDevice, pContext, Shader_World_NonAnim.szFileTag, VTXMESH::Elements, VTXMESH::iNumElements))))
        return E_FAIL;

    if (FAILED(pProxy->Add_Prototype(Shader_World_Anim.iLevelID, Shader_World_Anim.szProtoTag,
        CShader::Create(pDevice, pContext, Shader_World_Anim.szFileTag, VTXANIMMESH::Elements, VTXANIMMESH::iNumElements))))
        return E_FAIL;

    if (FAILED(pProxy->Add_Prototype(Shader_World_Instance.iLevelID, Shader_World_Instance.szProtoTag,
        CShader::Create(pDevice, pContext, Shader_World_Instance.szFileTag, VTXMESH_INSTANCED::Elements, VTXMESH_INSTANCED::iNumElements))))
        return E_FAIL;

    if (FAILED(pProxy->Add_Prototype(Shader_Fluid.iLevelID, Shader_Fluid.szProtoTag,
        CShader::Create(pDevice, pContext, Shader_Fluid.szFileTag, VTXMESH::Elements, VTXMESH::iNumElements))))
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

    if (!pProxy->Has_Prototype(Shader_UI.iLevelID, Shader_UI.szProtoTag))
    {
        if (FAILED(pProxy->Add_Prototype(Shader_UI.iLevelID, Shader_UI.szProtoTag,
            CShader::Create(pDevice, pContext, Shader_UI.szFileTag, VTXTEX::Elements, VTXTEX::iNumElements))))
            return E_FAIL;
    }

    if (FAILED(pProxy->Add_Prototype(Shader_Gorilla.iLevelID, Shader_Gorilla.szProtoTag,
        CShader::Create(pDevice, pContext, Shader_Gorilla.szFileTag, VTXANIMMESH::Elements, VTXANIMMESH::iNumElements))))
        return E_FAIL;
    if (FAILED(pProxy->Add_Prototype(Shader_Armadillo.iLevelID, Shader_Armadillo.szProtoTag,
        CShader::Create(pDevice, pContext, Shader_Armadillo.szFileTag, VTXANIMMESH::Elements, VTXANIMMESH::iNumElements))))
        return E_FAIL;
    if (FAILED(pProxy->Add_Prototype(Shader_Leopard.iLevelID, Shader_Leopard.szProtoTag,
        CShader::Create(pDevice, pContext, Shader_Leopard.szFileTag, VTXANIMMESH::Elements, VTXANIMMESH::iNumElements))))
        return E_FAIL;

    if (FAILED(pProxy->Add_Prototype(Shader_Bomb.iLevelID, Shader_Bomb.szProtoTag,
        CShader::Create(pDevice, pContext, Shader_Bomb.szFileTag, VTXANIMMESH::Elements, VTXANIMMESH::iNumElements))))
        return E_FAIL;

    if (FAILED(pProxy->Add_Prototype(Shader_EffectRock.iLevelID, Shader_EffectRock.szProtoTag,
        CShader::Create(pDevice, pContext, Shader_EffectRock.szFileTag, VTXEFFECTMESH::Elements, VTXEFFECTMESH::iNumElements))))
        return E_FAIL;
    if (FAILED(pProxy->Add_Prototype(Shader_Ring.iLevelID, Shader_Ring.szProtoTag,
        CShader::Create(pDevice, pContext, Shader_Ring.szFileTag, VTXEFFECTMESH::Elements, VTXEFFECTMESH::iNumElements))))
        return E_FAIL;
    if (FAILED(pProxy->Add_Prototype(Shader_Distortion.iLevelID, Shader_Distortion.szProtoTag,
        CShader::Create(pDevice, pContext, Shader_Distortion.szFileTag, VTXEFFECTMESH::Elements, VTXEFFECTMESH::iNumElements))))
        return E_FAIL;

    if (FAILED(pProxy->Add_Prototype(Shader_Monster.iLevelID, Shader_Monster.szProtoTag,
        CShader::Create(pDevice, pContext, Shader_Monster.szFileTag, VTXANIMMESH::Elements, VTXANIMMESH::iNumElements))))
        return E_FAIL;

    if (FAILED(pProxy->Add_Prototype(Shader_WaddleDee.iLevelID, Shader_WaddleDee.szProtoTag,
        CShader::Create(pDevice, pContext, Shader_WaddleDee.szFileTag, VTXANIMMESH::Elements, VTXANIMMESH::iNumElements))))
        return E_FAIL;

    if (FAILED(pProxy->Add_Prototype(Shader_AbillityModel.iLevelID, Shader_AbillityModel.szProtoTag,
        CShader::Create(pDevice, pContext, Shader_AbillityModel.szFileTag, VTXANIMMESH::Elements, VTXANIMMESH::iNumElements))))
        return E_FAIL;

    if (FAILED(pProxy->Add_Prototype(Shader_Metaknight.iLevelID, Shader_Metaknight.szProtoTag,
        CShader::Create(pDevice, pContext, Shader_Metaknight.szFileTag, VTXANIMMESH::Elements, VTXANIMMESH::iNumElements))))
        return E_FAIL;

    if (FAILED(pProxy->Add_Prototype(Shader_AttackDecal.iLevelID, Shader_AttackDecal.szProtoTag,
        CShader::Create(pDevice, pContext, Shader_AttackDecal.szFileTag, VTXMESH::Elements, VTXMESH::iNumElements))))
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
            pReg->ResourceLoader(pProxy, pDevice, pContext, iLevelIndex);
            pProxy->Add_Prototype(iLevelIndex, wProto.c_str(),
                pReg->CreatorFunc(pDevice, pContext));
        }

        CGameObject* pObj = nullptr;
        pProxy->Add_GameObject_Return(
            &pObj,
            iLevelIndex, wProto.c_str(),
            iLevelIndex, wLayer.c_str(), wObjectName, nullptr);

        if (pObj)
        {
            pObj->Deserialize(jObj);
        }
    }
    return S_OK;
}

HRESULT CLIENT_DLL Load_LevelManifest(const _tchar* strManifestPath, LEVEL_MANIFEST* pOut)
{
    if (nullptr == pOut)
        return E_FAIL;

    string strContent = {};
    if (FAILED(CDataLoader::Read_Json(strManifestPath, &strContent)))
        return E_FAIL;

    try
    {
        json jManifest = json::parse(strContent);

        if (jManifest.contains("Map_Manifest"))
            pOut->strMapManifest = StrToWstr(jManifest["Map_Manifest"].get<string>());

        if (jManifest.contains("Objects"))
            pOut->strObjectsFile = StrToWstr(jManifest["Objects"].get<string>());

        if (jManifest.contains("UI"))
            pOut->strUIFile = StrToWstr(jManifest["UI"].get<string>());

        if (jManifest.contains("RenderGlobals"))
            pOut->strRenderGlobalsFile = StrToWstr(jManifest["RenderGlobals"].get<string>());
    }
    catch (json::exception&)
    {
        return E_FAIL;
    }

    return S_OK;
}

HRESULT CLIENT_DLL Load_Level_FromManifest(const LEVEL_LOAD_CONTEXT& ctx, const _tchar* strManifestPath, _uint iLevelIndex, MAP_LOAD_RESULT* pOutReport, CMapStage** ppOutMapStage)
{
    LEVEL_MANIFEST Manifest{};
    if (FAILED(Load_LevelManifest(strManifestPath, &Manifest)))
    {
        MSG_BOX("LevelManifest Load Failed");
        return E_FAIL;
    }

    MAP_LOAD_RESULT report{};
    CMapStage* pMapStage = nullptr;
    if (FAILED(CMap_Loader::Spawn_Map(ctx.pDevice, ctx.pContext,
        Manifest.strMapManifest, Manifest.strObjectsFile,
        iLevelIndex, &report, &pMapStage)))
    {
        MSG_BOX("MapLoad Failed");
        return E_FAIL;
    }

    if (FAILED(Load_Level(ctx.pProxy, ctx.pDevice, ctx.pContext,
        Manifest.strObjectsFile.c_str(), iLevelIndex)))
    {
        MSG_BOX("LevelObject Load Failed");
        return E_FAIL;
    }

    if (!Manifest.strUIFile.empty())
    {
        if (FAILED(Load_Level_UI(ctx.pProxy, ctx.pDevice, ctx.pContext,
            Manifest.strUIFile.c_str(), iLevelIndex)))
        {
            MSG_BOX("UI Load Failed");
            return E_FAIL;
        }
    }

    if (!Manifest.strRenderGlobalsFile.empty())
        Apply_RenderGlobals_FromFile(ctx.pProxy, Manifest.strRenderGlobalsFile.c_str());

#ifdef _DEBUG
    const _wstring msg =
        L"[MapLoad][LevelDesign] json=" + to_wstring(report.iLevelDesignJsonLoadedCount) +
        L", parsed=" + to_wstring(report.iLevelDesignParsedObjectCount) +
        L", created=" + to_wstring(report.iLevelDesignCreatedCount) +
        L", fallback=" + to_wstring(report.iLevelDesignFallbackSpecCount) +
        L", failed=" + to_wstring(report.iLevelDesignSkippedCreateFailedCount) + L"\n";
    OutputDebugStringW(msg.c_str());
#endif

    if (pOutReport)    *pOutReport = report;
    if (ppOutMapStage) *ppOutMapStage = pMapStage;
    return S_OK;
}

HRESULT CLIENT_DLL Apply_RenderGlobals_FromFile(CGameInstance_Proxy* pProxy, const _tchar* strPath)
{
    if (nullptr == pProxy || nullptr == strPath)
        return E_FAIL;

    string strContent = {};
    if (FAILED(CDataLoader::Read_Json(strPath, &strContent)))
        return E_FAIL;

    try
    {
        json jGlobals = json::parse(strContent);
        for (auto it = jGlobals.begin(); it != jGlobals.end(); ++it)
        {
            const auto& a = it.value();               // [x, y, z, w]
            if (!a.is_array() || a.size() < 4)
                continue;

            pProxy->Set_ShaderGlobal(it.key(),
                _float4(a[0].get<float>(), a[1].get<float>(),
                    a[2].get<float>(), a[3].get<float>()));
        }
    }
    catch (json::exception&)
    {
        return E_FAIL;
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
    const _uint iLevel = ETOUI(LEVEL::STATIC);

    if (!pProxy->Has_Prototype(iLevel, CUI_Image::PROTOTYPE_TAG))
    {
        if (FAILED(pProxy->Add_Prototype(iLevel, CUI_Image::PROTOTYPE_TAG, CUI_Image::Create(pDevice, pContext))))
            return E_FAIL;
    }

    if (!pProxy->Has_Prototype(iLevel, CUI_SpriteAnim::PROTOTYPE_TAG))
    {
        if (FAILED(pProxy->Add_Prototype(iLevel, CUI_SpriteAnim::PROTOTYPE_TAG, CUI_SpriteAnim::Create(pDevice, pContext))))
            return E_FAIL;
    }

    if (!pProxy->Has_Prototype(iLevel, CUI_SpriteAnimCurtain::PROTOTYPE_TAG))
    {
        if (FAILED(pProxy->Add_Prototype(iLevel, CUI_SpriteAnimCurtain::PROTOTYPE_TAG,
            CUI_SpriteAnimCurtain::Create(pDevice, pContext))))
            return E_FAIL;
    }

    if (!pProxy->Has_Prototype(iLevel, CUI_Curtain::PROTOTYPE_TAG))
    {
        if (FAILED(pProxy->Add_Prototype(iLevel, CUI_Curtain::PROTOTYPE_TAG, CUI_Curtain::Create(pDevice, pContext))))
            return E_FAIL;
    }

    if (!pProxy->Has_Prototype(iLevel, CUI_Eraser::PROTOTYPE_TAG))
    {
        if (FAILED(pProxy->Add_Prototype(iLevel, CUI_Eraser::PROTOTYPE_TAG, CUI_Eraser::Create(pDevice, pContext))))
            return E_FAIL;
    }

    if (!pProxy->Has_Prototype(iLevel, CUI_CurtainStatic::PROTOTYPE_TAG))
    {
        if (FAILED(pProxy->Add_Prototype(iLevel, CUI_CurtainStatic::PROTOTYPE_TAG, CUI_CurtainStatic::Create(pDevice, pContext))))
            return E_FAIL;
    }

    if (!pProxy->Has_Prototype(iLevel, CUI_CurtainTexture::PROTOTYPE_TAG))
    {
        if (FAILED(pProxy->Add_Prototype(iLevel, CUI_CurtainTexture::PROTOTYPE_TAG, CUI_CurtainTexture::Create(pDevice, pContext))))
            return E_FAIL;
    }

    if (!pProxy->Has_Prototype(iLevel, CUI_CurtainStamp::PROTOTYPE_TAG))
    {
        if (FAILED(pProxy->Add_Prototype(iLevel, CUI_CurtainStamp::PROTOTYPE_TAG, CUI_CurtainStamp::Create(pDevice, pContext))))
            return E_FAIL;
    }

    if (!pProxy->Has_Prototype(iLevel, CUI_CurtainFadeOut::PROTOTYPE_TAG))
    {
        if (FAILED(pProxy->Add_Prototype(iLevel, CUI_CurtainFadeOut::PROTOTYPE_TAG, CUI_CurtainFadeOut::Create(pDevice, pContext))))
            return E_FAIL;
    }

    if (!pProxy->Has_Prototype(iLevel, CUI_Text::PROTOTYPE_TAG))
    {
        if (FAILED(pProxy->Add_Prototype(iLevel, CUI_Text::PROTOTYPE_TAG, CUI_Text::Create(pDevice, pContext))))
            return E_FAIL;
    }

    if (!pProxy->Has_Prototype(iLevel, CUI_Effect::PROTOTYPE_TAG))
    {
        if (FAILED(pProxy->Add_Prototype(iLevel, CUI_Effect::PROTOTYPE_TAG, CUI_Effect::Create(pDevice, pContext))))
            return E_FAIL;
    }

    if (!pProxy->Has_Prototype(iLevel, CUI_GaugeFill::PROTOTYPE_TAG))
    {
        if (FAILED(pProxy->Add_Prototype(iLevel, CUI_GaugeFill::PROTOTYPE_TAG, CUI_GaugeFill::Create(pDevice, pContext))))
            return E_FAIL;
    }

    return S_OK;
}

static HRESULT Ready_UIResources_FromBundle(CGameInstance_Proxy* pProxy, ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const _tchar* strFilePath, _uint iLevelIndex)
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
            pReg->ResourceLoader(pProxy, pDevice, pContext, iLevelIndex);

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

static HRESULT Spawn_UIContainers_FromBundle(CGameInstance_Proxy* pProxy, ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const _tchar* strFilePath, _uint iLevelIndex)
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

        if (!pProxy->Has_Prototype(iContProtoLevel, strProtoTag))
            return E_FAIL;

        CGameObject* pObj = nullptr;

        if (FAILED(pProxy->Add_GameObject_Return(&pObj, iContProtoLevel, strProtoTag.c_str(), iObjLevel, strLayerTag.c_str(), strObjectTag, nullptr)))
            return E_FAIL;

        if (nullptr == pObj)
            return E_FAIL;

        if (jUI.contains("Container"))
        {
            json jContainer = jUI["Container"];
            Prepare_UIContainerJson(pProxy, pDevice, pContext, jContainer,
                iContProtoLevel, ETOUI(LEVEL::STATIC), iLevelIndex);
            pObj->Deserialize(jContainer);
        }

        pObj->Set_Active(bInitialActive);
    }

    return S_OK;
}

HRESULT CLIENT_DLL Ready_Level_UIResources(CGameInstance_Proxy* pProxy, ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const _tchar* strFilePath, _uint iLevelIndex)
{
    string strContent;
    if (FAILED(CDataLoader::Read_Json(strFilePath, &strContent)))
        return E_FAIL;

    json jFile;
    try { jFile = json::parse(strContent); }
    catch (const std::exception&) { return E_FAIL; }

    // 신규: 레벨 UI 매니페스트(레이어 번들 목록)
    if (jFile.contains("UILayers") && jFile["UILayers"].is_array())
    {
        for (const auto& jLayer : jFile["UILayers"])
        {
            const string strLayerPath = jLayer.value("Path", string());
            if (strLayerPath.empty()) return E_FAIL;
            if (FAILED(Ready_UIResources_FromBundle(pProxy, pDevice, pContext,
                StrToWstr(strLayerPath).c_str(), iLevelIndex)))
                return E_FAIL;
        }
        return S_OK;
    }

    // 하위호환: 예전 단일 번들({"UIContainers":[...]})
    if (jFile.contains("UIContainers") && jFile["UIContainers"].is_array())
        return Ready_UIResources_FromBundle(pProxy, pDevice, pContext, strFilePath, iLevelIndex);

    return E_FAIL;
}

HRESULT CLIENT_DLL Load_Level_UI(CGameInstance_Proxy* pProxy, ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const _tchar* strFilePath, _uint iLevelIndex)
{
    string strContent;
    if (FAILED(CDataLoader::Read_Json(strFilePath, &strContent)))
        return E_FAIL;

    json jFile;
    try { jFile = json::parse(strContent); }
    catch (const std::exception&) { return E_FAIL; }

    if (jFile.contains("UILayers") && jFile["UILayers"].is_array())
    {
        for (const auto& jLayer : jFile["UILayers"])
        {
            const string strLayerPath = jLayer.value("Path", string());
            if (strLayerPath.empty()) return E_FAIL;
            if (FAILED(Spawn_UIContainers_FromBundle(pProxy, pDevice, pContext,
                StrToWstr(strLayerPath).c_str(), iLevelIndex)))
                return E_FAIL;
        }
        return S_OK;
    }

    if (jFile.contains("UIContainers") && jFile["UIContainers"].is_array())
        return Spawn_UIContainers_FromBundle(pProxy, pDevice, pContext, strFilePath, iLevelIndex);

    return E_FAIL;
}

void Prepare_UIContainerJson(
    CGameInstance_Proxy* pProxy, ID3D11Device* pDevice, ID3D11DeviceContext* pContext,
    json& jContainer, _uint iContainerProtoLevel, _uint iPartProtoLevel, _uint iTextureLevel)
{
    if (jContainer.contains("UIPartObjects") && jContainer["UIPartObjects"].is_object())
    {
        for (auto& [strPartTag, jPart] : jContainer["UIPartObjects"].items())
        {
            jPart["ProtoLevel"] = iPartProtoLevel;
            if (jPart.contains("TextureProtoTag") && !jPart.value("TextureProtoTag", string()).empty())
                jPart["TextureLevel"] = iTextureLevel;
            if (jPart.contains("MaskTextureProtoTag") && !jPart.value("MaskTextureProtoTag", string()).empty())
                jPart["MaskTextureLevel"] = iTextureLevel;
        }
    }

    if (jContainer.contains("Children") && jContainer["Children"].is_object())
    {
        for (auto& [strChildTag, jChild] : jContainer["Children"].items())
        {
            jChild["ProtoLevel"] = iContainerProtoLevel;
            if (jChild.contains("ProtoTag"))
            {
                _wstring strChildProto = StrToWstr(jChild["ProtoTag"].get<string>());
                auto* pChildReg = CGameObject_Factory::GetInstance()->Get_Registration(strChildProto);
                if (pChildReg && !pProxy->Has_Prototype(iContainerProtoLevel, strChildProto))
                {
                    pChildReg->ResourceLoader(pProxy, pDevice, pContext, iContainerProtoLevel);
                    pProxy->Add_Prototype(iContainerProtoLevel, strChildProto,
                        pChildReg->CreatorFunc(pDevice, pContext));
                }
            }
            Client::Prepare_UIContainerJson(pProxy, pDevice, pContext, jChild,
                iContainerProtoLevel, iPartProtoLevel, iTextureLevel);
        }
    }
}

NS_END


