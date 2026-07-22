#include "LightShaft.h"
#include "GameInstance.h"
#include "GameContent_const.h"
#include "Shader.h"
#include "Model.h"

CLightShaft::CLightShaft(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CGameObject(pDevice, pContext)
{
}

CLightShaft::CLightShaft(const CLightShaft& Prototype)
    : CGameObject(Prototype)
{
}

HRESULT CLightShaft::Initialize_Prototype()
{
    m_eProjType = PROJ_TYPE::PERSPEC;
    return S_OK;
}

HRESULT CLightShaft::Initialize(void* pArg)
{
    if (nullptr == pArg)
        return E_FAIL;

    const LIGHTSHAFT_DESC* pDesc = static_cast<const LIGHTSHAFT_DESC*>(pArg);
    if (nullptr == pDesc->pModelProtoTag)
        return E_FAIL;

    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    m_iModelLevel = pDesc->iModelLevel;
    m_strModelTag = pDesc->pModelProtoTag;

    if (FAILED(Ready_Components()))
        return E_FAIL;

    return S_OK;
}

void CLightShaft::Configure(const LIGHTSHAFT_DESC& Desc)
{
    m_vColor = Desc.vColor;
    m_vScaleMin = Desc.vScaleMin;
    m_vScaleMax = Desc.vScaleMax;
    m_fIntensity = Desc.fIntensity;
    m_fMaxAlpha = Desc.fMaxAlpha;
    m_eEase = Desc.eEase;

    Set_Progress(0.f);
}

void CLightShaft::Set_Progress(_float t)
{
    const _float fSaturated = (t < 0.f) ? 0.f : ((t > 1.f) ? 1.f
        : t);
    const _float fEased = (EASE_SMOOTH == m_eEase)
        ? fSaturated * fSaturated * (3.f - 2.f * fSaturated)
        : fSaturated;

    m_fAlpha = fEased * m_fMaxAlpha;

    const _float3 vScale =
    {
        m_vScaleMin.x + (m_vScaleMax.x - m_vScaleMin.x) * fEased,
        m_vScaleMin.y + (m_vScaleMax.y - m_vScaleMin.y) * fEased,
        m_vScaleMin.z + (m_vScaleMax.z - m_vScaleMin.z) * fEased,
    };
    if (nullptr == m_pTransformCom)
        return;

    m_pTransformCom->Set_Scale(vScale.x, vScale.y, vScale.z);

    const _float fApexLift = 0.2f * vScale.y;
    _vector vPos = XMLoadFloat3(&m_vBaseAnchor) + XMLoadFloat3(&m_vUpDir) * fApexLift;
    m_pTransformCom->Set_State(STATE::POSITION,  XMVectorSetW(vPos, 1.f));
}

void CLightShaft::Set_Position(_fvector vPos)
{
    XMStoreFloat3(&m_vBaseAnchor, vPos);      
    if (m_pTransformCom)
        m_pTransformCom->Set_State(STATE::POSITION, XMVectorSetW(vPos, 1.f));
}

void CLightShaft::Align_Up(_fvector vUpDir)
{
    if (nullptr == m_pTransformCom)
        return;

    _vector vUp = XMVector3Normalize(vUpDir);
    XMStoreFloat3(&m_vUpDir, vUp);             

        _vector vRef = (fabsf(XMVectorGetY(vUp)) > 0.99f)
        ? XMVectorSet(1.f, 0.f, 0.f, 0.f)
        : XMVectorSet(0.f, 1.f, 0.f, 0.f);
    _vector vRight = XMVector3Normalize(XMVector3Cross(vRef, vUp));
    _vector vLook = XMVector3Normalize(XMVector3Cross(vRight,
        vUp));

    m_pTransformCom->Set_State(STATE::RIGHT, vRight);
    m_pTransformCom->Set_State(STATE::UP, vUp);
    m_pTransformCom->Set_State(STATE::LOOK, vLook);
}

HRESULT CLightShaft::Ready_Components()
{
    m_pShaderCom = Add_Component<CShader>(Shader_LightShaft.iLevelID, Shader_LightShaft.szProtoTag, TEXT("Com_Shader"));
    if (nullptr == m_pShaderCom)
        return E_FAIL;

    m_pModelCom = Add_Component<CModel>(m_iModelLevel, m_strModelTag.c_str(), TEXT("Com_Model"));
    if (nullptr == m_pModelCom)
        return E_FAIL;

    return S_OK;
}

void CLightShaft::Update(_float fTimeDelta)
{
    UNREFERENCED_PARAMETER(fTimeDelta);
}

void CLightShaft::Late_Update(_float fTimeDelta)
{
    UNREFERENCED_PARAMETER(fTimeDelta);

    if (m_fAlpha > 0.001f)
        m_pGameInstance_Proxy->Add_RenderGroup(RENDERID::BLEND, this);
}

HRESULT CLightShaft::Bind_ShaderResources()
{
    if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
        return E_FAIL;

    if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::VIEW, m_eProjType))))
        return E_FAIL;

    if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::PROJ, m_eProjType))))
        return E_FAIL;

    if (FAILED(m_pShaderCom->Bind_RawValue("g_vColor", &m_vColor, sizeof(_float3))))
        return E_FAIL;

    if (FAILED(m_pShaderCom->Bind_RawValue("g_fAlpha", &m_fAlpha, sizeof(_float))))
        return E_FAIL;

    if (FAILED(m_pShaderCom->Bind_RawValue("g_fIntensity", &m_fIntensity, sizeof(_float))))
        return E_FAIL;

    return S_OK;
}

HRESULT CLightShaft::Render()
{
    if (m_fAlpha <= 0.001f || nullptr == m_pModelCom || nullptr == m_pShaderCom)
        return S_OK;

    if (FAILED(Bind_ShaderResources()))
        return E_FAIL;

    const _uint iNumMeshes = static_cast<_uint>(m_pModelCom->Get_NumMeshes());
    for (_uint i = 0; i < iNumMeshes; ++i)
    {
        if (FAILED(m_pShaderCom->Begin(0)))
            return E_FAIL;

        if (FAILED(m_pModelCom->Render(i)))
            return E_FAIL;
    }

    return S_OK;
}

CLightShaft* CLightShaft::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CLightShaft* pInstance = new CLightShaft(pDevice, pContext);
    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created : CLightShaft");
        Safe_Release(pInstance);
    }
    return pInstance;
}

CGameObject* CLightShaft::Clone(void* pArg)
{
    CLightShaft* pInstance = new CLightShaft(*this);
    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : CLightShaft");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CLightShaft::Free()
{
    __super::Free();
}
