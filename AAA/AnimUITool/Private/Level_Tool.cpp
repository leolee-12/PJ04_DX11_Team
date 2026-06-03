#include "Level_Tool.h"
#include "GameInstance.h"
#include "EditCamera.h"
#include "Edit_Grid.h"

#include "Shader.h"
#include "Model.h"
#include "Preview_Actor.h"
#include "GameContent_const.h"
#include "GameObject_Factory.h"
#include "UI_Image.h"
#include "UI_TestImageContainer.h"

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

    if (FAILED(Ready_TestUI()))
        return E_FAIL;

    return S_OK;
}

HRESULT CLevel_Tool::Ready_Lights()
{
    LIGHT_DESC LightDesc{};
    LightDesc.eType = LIGHT::DIRECTIONAL;
    LightDesc.vDiffuse = _float4(1.f, 1.f, 1.f, 1.f);   
    LightDesc.vAmbient = _float4(1.f, 1.f, 1.f, 1.f);
    LightDesc.vSpecular = _float4(1.f, 1.f, 1.f, 1.f);
    LightDesc.vDirection = _float4(1.f, -1.f, 1.f, 0.f);
    return m_pGameInstance_Proxy->Add_Light(LightDesc);
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
        ->Get_Registration(Client::CUI_TestImageContainer::PROTOTYPE_TAG);
    if (!pReg)
        return E_FAIL;

    const _uint L = ETOUI(TOOL_LEVEL::STATIC);

    if (!m_pGameInstance_Proxy->Has_Prototype(L, Client::CUI_TestImageContainer::PROTOTYPE_TAG))
    {
        pReg->ResourceLoader(m_pGameInstance_Proxy, m_pDevice, m_pContext);

        m_pGameInstance_Proxy->Add_Prototype(
            L,
            Client::CUI_TestImageContainer::PROTOTYPE_TAG,
            pReg->CreatorFunc(m_pDevice, m_pContext));
    }

    Client::CUI_TestImageContainer::UI_TESTIMAGE_CONTAINER_DESC desc{};
    desc.vPosition = { -200.f, 0.f, 0.f, 1.f };
    desc.bCreateImagePart = true;
    desc.szPartTag = L"Image";

    desc.ImageDesc.iTextureLevel = ETOUI(LEVEL::STATIC);
    desc.ImageDesc.szTextureProtoTag = L"Proto_Tex_TestUI";
    desc.ImageDesc.vSize = { 300.f, 300.f };
    desc.ImageDesc.vPosition = { 0.f, 0.f };
    desc.ImageDesc.iRenderLayer = 1;

    CGameObject* pSource = nullptr;
    if (FAILED(m_pGameInstance_Proxy->Add_GameObject_Return(&pSource, L, Client::CUI_TestImageContainer::PROTOTYPE_TAG, ETOUI(TOOL_LEVEL::EDIT), L"Layer_UI", L"Test_UI_Source",  &desc)))
        return E_FAIL;

    json jUI = pSource->Serialize();
    jUI["Transform"]["vPosition"][0] = 200.f;

    CGameObject* pLoaded = nullptr;
    if (FAILED(m_pGameInstance_Proxy->Add_GameObject_Return(&pLoaded, L, Client::CUI_TestImageContainer::PROTOTYPE_TAG, ETOUI(TOOL_LEVEL::EDIT), L"Layer_UI", L"Test_UI_Loaded",  nullptr)))
        return E_FAIL;
    
    pLoaded->Deserialize(jUI);

    return S_OK;
}

void CLevel_Tool::Update(_float fTimeDelta) 
{
}

HRESULT CLevel_Tool::Render()
{
    if (m_pGrid)
    {
        const _float4x4* pView = m_pGameInstance_Proxy->Get_Matrix(D3DTS::VIEW, PROJ_TYPE::PERSPEC);
        const _float4x4* pProj = m_pGameInstance_Proxy->Get_Matrix(D3DTS::PROJ, PROJ_TYPE::PERSPEC);
        m_pGrid->Render(pView, pProj);
    }
    return S_OK;
}

void CLevel_Tool::Set_CameraActive(_bool bActive)
{
    if (m_pCamera) m_pCamera->Set_Active(bActive);
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
    Safe_Release(m_pGrid);   
}