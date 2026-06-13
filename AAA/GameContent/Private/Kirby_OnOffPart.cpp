#include "Kirby_OnOffPart.h"

#include "GameInstance.h"

#include "GameContent_const.h"

#include "Animator.h"

CKirby_OnOffPart::CKirby_OnOffPart(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CPartObject(pDevice, pContext)
{
}

CKirby_OnOffPart::CKirby_OnOffPart(const CKirby_OnOffPart& Prototype)
    : CPartObject(Prototype) {
}

HRESULT CKirby_OnOffPart::Initialize_Prototype()
{
    m_eProjType = PROJ_TYPE::PERSPEC;
    return S_OK;
}

HRESULT CKirby_OnOffPart::Initialize(void* pArg)
{
    KIRBY_ONONFFPART_DESC* pDesc = static_cast<KIRBY_ONONFFPART_DESC*>(pArg);

    m_pSocketBoneMatrix = pDesc->pSocketBoneMatrix;

    if (FAILED(__super::Initialize(pDesc)))
        return E_FAIL;

    return S_OK;
}

void CKirby_OnOffPart::Priority_Update(_float fTimeDelta)
{
}

void CKirby_OnOffPart::Update(_float fTimeDelta)
{
}

void CKirby_OnOffPart::Late_Update(_float fTimeDelta)
{
    __super::Compute_CombinedWorldMatrix(XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()) *
        XMLoadFloat4x4(m_pSocketBoneMatrix));
}

HRESULT CKirby_OnOffPart::Render()
{

    return S_OK;
}

void CKirby_OnOffPart::Free()
{
    __super::Free();
}