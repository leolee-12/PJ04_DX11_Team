#include "BubbleMovement.h"
#include "Transform.h"
#include "Controller.h"
#include "PhysX/characterkinematic/PxController.h"

CBubbleMovement::CBubbleMovement(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CComponent {pDevice, pContext}
{
}

CBubbleMovement::CBubbleMovement(const CBubbleMovement& Prototype)
	: CComponent (Prototype)
{
}

HRESULT CBubbleMovement::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CBubbleMovement::Initialize(void* pArg)
{
	return S_OK;
}

void CBubbleMovement::Set_Refs(CTransform* pTransform, CController* pController)
{
	m_pTransform = pTransform;
	m_pController = pController;
}

void CBubbleMovement::Set_Physics(_float fGravity, _float fRestitution, _float fHorizDamp)
{
	m_fGravity = fGravity;
	m_fRestitution = fRestitution;
	m_fHorizDamp = fHorizDamp;
}

void CBubbleMovement::Launch(_fvector vVelocity)
{
	XMStoreFloat3(&m_vVelocity, vVelocity);
}

_bool CBubbleMovement::Tick(_float fTimeDelta)
{
	if (!m_pController || !m_pTransform)
		return false;

	m_vVelocity.y += m_fGravity * fTimeDelta;

	_vector vDisp = XMLoadFloat3(&m_vVelocity) * fTimeDelta;
	_uint iFlags = m_pController->Move(vDisp, 0.001f, fTimeDelta);

	m_pTransform->Set_State(STATE::POSITION, m_pController->Get_FootPosition());

	m_bGrounded = (iFlags & physx::PxControllerCollisionFlag::eCOLLISION_DOWN) != 0;

	if (m_bGrounded && m_vVelocity.y < 0.f)
	{
		m_vVelocity.y = -m_vVelocity.y * m_fRestitution;
		m_vVelocity.x *= m_fHorizDamp;
		m_vVelocity.z *= m_fHorizDamp;
		return true;
	}

	return false;
}

void CBubbleMovement::Stop()
{
	m_vVelocity = { 0.f, 0.f, 0.f };
}

CBubbleMovement* CBubbleMovement::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CBubbleMovement* pInstance = new CBubbleMovement(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CBubbleMovement");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CComponent* CBubbleMovement::Clone(void* pArg)
{
	return new CBubbleMovement(*this);
}

void CBubbleMovement::Free()
{
	__super::Free();
}
