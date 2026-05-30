#include "Level_Tool.h"
#include "GameInstance.h"
#include "EditCamera.h"
#include "Edit_Grid.h"

using namespace AnimUITool;

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
    desc.fRotationPerSec = XMConvertToRadians(540.f);
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