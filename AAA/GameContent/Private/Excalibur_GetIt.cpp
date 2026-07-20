#include "Excalibur_GetIt.h"
#include "GameInstance.h"

CExcalibur_GetIt::CExcalibur_GetIt(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CMonsterPart(pDevice, pContext) {
}
CExcalibur_GetIt::CExcalibur_GetIt(const CExcalibur_GetIt& Prototype)
    : CMonsterPart(Prototype) {
}

HRESULT CExcalibur_GetIt::Initialize(void* pArg)
{
    if (FAILED(__super::Initialize(pArg)))  return E_FAIL;
    if (FAILED(Ready_Components()))         return E_FAIL;

    m_pTransformCom->Set_State(STATE::POSITION, XMVectorSet(0.f, HOVER_HEIGHT, 0.f, 1.f));

    return S_OK;
}

void CExcalibur_GetIt::Update(_float fTimeDelta)
{
    if (!Is_Active())
        return;

    __super::Update(fTimeDelta);

    m_fAccTime += fTimeDelta;

    const _float fY = HOVER_HEIGHT + sinf(m_fAccTime * BOB_SPEED) * BOB_AMPL;
    _vector vPos = m_pTransformCom->Get_State(STATE::POSITION);
    m_pTransformCom->Set_State(STATE::POSITION, XMVectorSetY(vPos, fY));
}

void CExcalibur_GetIt::Late_Update(_float fTimeDelta)
{
    if (!Is_Active())
        return;

    Compute_CombinedWorldMatrix(XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));
    m_pGameInstance_Proxy->Add_RenderGroup(RENDERID::BLEND, this);
}

HRESULT CExcalibur_GetIt::Render()
{
    if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", &m_CombinedWorldMatrix)))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::VIEW, m_eProjType))))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::PROJ, m_eProjType))))
        return E_FAIL;

    const _uint iNumMeshes = static_cast<_uint>(m_pModelCom->Get_NumMeshes());
    for (_uint i = 0; i < iNumMeshes; ++i)
    {
        if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_DiffuseTexture", i, MTEX_TYPE::UNKNOWN, 0)))
            return E_FAIL;

        if (FAILED(m_pShaderCom->Begin(8)))
            return E_FAIL;
        if (FAILED(m_pModelCom->Render(i)))
            return E_FAIL;
    }
    return S_OK;
}

HRESULT CExcalibur_GetIt::Ready_Components()
{
    PART_SETUP t{};
    t.tShader = Shader_Metaknight;
    t.szModelProtoTag = MODEL_PROTO_TAG;
    t.bAnimated = false;
    return Ready_MeshPart(t);
}

CExcalibur_GetIt* CExcalibur_GetIt::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CExcalibur_GetIt* pInstance = new CExcalibur_GetIt(pDevice, pContext);
    if (FAILED(pInstance->Initialize_Prototype())) {
        MSG_BOX("Failed to Created: CExcalibur_GetIt");
        Safe_Release(pInstance);
    }
    return pInstance;
}

CExcalibur_GetIt* CExcalibur_GetIt::Clone(void* pArg)
{
    CExcalibur_GetIt* pInstance = new CExcalibur_GetIt(*this);
    if (FAILED(pInstance->Initialize(pArg))) {
        MSG_BOX("Failed to Cloned: CExcalibur_GetIt");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CExcalibur_GetIt::Free() { __super::Free(); }