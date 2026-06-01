#include "Loader_Prototype.h"
#include "GameContent_const.h"
#include "GameInstance.h"
#include "DataLoader.h"
#include "GameObject_Factory.h"
#include "GameObject.h"
#include "VIBuffer_Point_Instance.h"

NS_BEGIN(Client)

HRESULT Ready_Prototype_SharedResources(CGameInstance_Proxy* pProxy, ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    if (FAILED(pProxy->Add_Prototype(VI_Rect.iLevelID, VI_Rect.szProtoTag,
        CVIBuffer_Rect::Create(pDevice, pContext))))
        return E_FAIL;

    if (FAILED(pProxy->Add_Prototype(ETOUI(LEVEL::STATIC), TEXT("Prototype_Component_Navigation_Lumia"),
        CNavigation::Create(pDevice, pContext, L"../../Resources/Models/Maps/Navigation/Lumia_Navi.nav"))))
        return E_FAIL;

    if (FAILED(pProxy->Add_Prototype(VI_Trail.iLevelID, VI_Trail.szProtoTag,
        CVIBuffer_Trail::Create(pDevice, pContext))))
        return E_FAIL;

    if (FAILED(pProxy->Add_Prototype(VI_Point.iLevelID, VI_Point.szProtoTag,
        CVIBuffer_Point::Create(pDevice, pContext))))
        return E_FAIL;

    CVIBuffer_Point_Instance::POINT_INSTANCE_DESC instDesc{};
    instDesc.iNumInstance = 15;
    instDesc.vCenter = { 0.f, 0.f, 0.f };
    instDesc.vRange = { 5.f, 1.f, 5.f };
    instDesc.vSize = { 0.8f, 1.2f };
    instDesc.vAspect = { 4.f, 4.f };
    instDesc.vRollRange = { -XMConvertToRadians(45), XMConvertToRadians(45) };
    instDesc.vSpeed = { 2.f, 3.f };
    instDesc.vLifeTime = { 0.4f, 0.6f };
    instDesc.fEmitDuration = 1.5f;
    instDesc.isLoop = false;
    instDesc.vPivot = { 0.f, 0.f, 0.f };
    pProxy->Add_Prototype(VI_CrackInstance.iLevelID, VI_CrackInstance.szProtoTag,
        CVIBuffer_Point_Instance::Create(pDevice, pContext, &instDesc));

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
    /*if (FAILED(pProxy->Add_Font(TEXT("Font_Default"), TEXT("../../Resources/Fonts/ER_Default.spritefont"))))
        return E_FAIL;

    if (FAILED(pProxy->Add_Font(TEXT("Font_Semibold"), TEXT("../../Resources/Fonts/ER_Semibold.spritefont"))))
        return E_FAIL;*/

    return S_OK;
}

NS_END


