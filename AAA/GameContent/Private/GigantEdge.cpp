#include "GigantEdge.h"
#include "GameInstance.h"
#include "Movement.h"

CGigantEdge::CGigantEdge(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CCharacter(pDevice, pContext)
{

}
CGigantEdge::CGigantEdge(const CGigantEdge& Prototype)
	: CCharacter(Prototype)
{

}

HRESULT CGigantEdge::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CGigantEdge::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_PartObjects()))
		return E_FAIL;

	if (FAILED(Ready_Components()))
		return E_FAIL;

	return S_OK;
}

void CGigantEdge::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);
}

void CGigantEdge::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
}

void CGigantEdge::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
}

HRESULT CGigantEdge::Render()
{
	return S_OK;
}

void CGigantEdge::On_Deserialized()
{
}

HRESULT CGigantEdge::Ready_Components()
{
    _float3 vFootPos;
    XMStoreFloat3(&vFootPos, m_pTransformCom->Get_State(STATE::POSITION));
    m_pCCT = m_pGameInstance_Proxy->Create_CapsuleController(vFootPos, s_fCCT_Radius, s_fCCT_Height);

    m_pMovement = Add_Component<CMovement>(TEXT("Com_Movement"), CMovement::Create(m_pDevice, m_pContext));
    if (m_pMovement == nullptr)
        return E_FAIL;

    m_pMovement->Set_Refs(m_pTransformCom, m_pCCT);

	return S_OK;
}
HRESULT CGigantEdge::Ready_PartObjects()
{
	return S_OK;
}
