#include "KoKabu.h"
#include "GameInstance.h"
#include "GameContent_Const.h"
#include "GameContrnt_Events.h"
#include "Projectile_Movement.h"


CKokabu::CKokabu(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CPhysicsProjectile{ pDevice, pContext }
{
}

CKokabu::CKokabu(const CKokabu& Prototype)
	: CPhysicsProjectile (Prototype)
{
}

_bool CKokabu::Can_BeInhaled(const INHALE_QUERY& q) const
{
	return m_bAlive 
		&&		(KOKABU_STATE::EJECTED == m_eState
		||		KOKABU_STATE::FALLING == m_eState
		||		KOKABU_STATE::LANDED == m_eState);
}

void CKokabu::Be_Captured(CGameObject* pInhaler)
{
	if (KOKABU_STATE::CAPTURED == m_eState)
		return;

	m_pCaptor = pInhaler;
	Change_State(KOKABU_STATE::CAPTURED);
}

void CKokabu::On_SpatBegin()
{
	m_pCaptor = nullptr;
	Change_State(KOKABU_STATE::SPAT);

	Update_SpatPivot_FromBone();

	m_pTransformCom->Set_Scale(
		m_vBaseScale.x, m_vBaseScale.y, m_vBaseScale.z);
}

void CKokabu::On_SpatEnd()
{
	m_pCaptor = nullptr;
	Despawn();
}

void CKokabu::On_Swallowed()
{
	m_pTransformCom->Set_Scale(
		m_vBaseScale.x, m_vBaseScale.y, m_vBaseScale.z);

	SWALLOW_EVENT payload{ this };
	m_pGameInstance_Proxy->Publish(EventTag::Swallowed, &payload);

	m_pCaptor = nullptr;
	Despawn();
}

HRESULT CKokabu::Initialize(void* pArg)
{
	m_fSpeed = 20.f;
	m_fDamage = 10.f;
	m_fKnockback = 4.f;
	m_fHitRadius = 0.6f;		// º¸°í Æ©´×
	m_fLifeTime = 3.0f;

	m_eProjType = PROJ_TYPE::PERSPEC;

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	m_vBaseScale = m_pTransformCom->Get_Scaled();

	// ground slide: no bounce, no horizontal damp
	if (m_pMovement)
		m_pMovement->Set_Physics(-45.f, 0.f, 1.0f);

	return S_OK;
}

void CKokabu::Update(_float fTimeDelta)
{
}

HRESULT CKokabu::Render()
{
	return S_OK;
}

HRESULT CKokabu::Ready_Visual()
{
	m_pShaderCom = Add_Component<CShader>(Shader_Monster.iLevelID, Shader_Monster.szProtoTag, TEXT("Com_Shader"));

	if (nullptr == m_pShaderCom)
		return E_FAIL;

	m_pModelCom = Add_Component<CModel>(
		m_iPrototypeLevel, MODEL_PROTO_TAG, TEXT("Com_Model"));
	if (nullptr == m_pModelCom)
		return E_FAIL;

	CAnimator::ANIMATOR_DESC ad{};
	ad.pModel = m_pModelCom;
	m_pAnimatorCom = Add_Component<CAnimator>(
		TEXT("Com_Animator"),
		CAnimator::Create(m_pDevice, m_pContext));
	if (nullptr == m_pAnimatorCom
		|| FAILED(m_pAnimatorCom->Initialize(&ad)))
		return E_FAIL;

	m_pAnimatorCom->Play("Wait", true, true);

	return S_OK;
}

void CKokabu::On_Launched()
{
}

void CKokabu::On_Impact()
{

}

void CKokabu::Change_State(KOKABU_STATE eNext)
{
}

void CKokabu::Enter_State(KOKABU_STATE eState)
{
}

void CKokabu::Update_State(_float fTimeDelta)
{
}

void CKokabu::Exit_State(KOKABU_STATE eState)
{
}

void CKokabu::Update_Captured(_float fTimeDelta)
{
	if (nullptr == m_pCaptor)
		return;

	CTransform* pCapT = m_pCaptor->Get_Transform();
	_vector vMouth = pCapT->Get_State(STATE::POSITION)
		+ pCapT->Get_State(STATE::LOOK) * 0.6f
		+ pCapT->Get_State(STATE::UP) * 0.6f;

	_vector vSelf = m_pTransformCom->Get_State(STATE::POSITION);
	_vector vDir = vMouth - vSelf;
	_float fDist = XMVectorGetX(XMVector3Length(vDir));

	if (fDist <= 0.5f)
	{
		On_Swallowed();
		return;
	}

	m_fPullSpeed += s_fPullAccel * fTimeDelta;
	_float fMove = min(m_fPullSpeed * fTimeDelta, fDist);
	m_pTransformCom->Set_State(STATE::POSITION,
		vSelf + XMVector3Normalize(vDir) * fMove);

	m_fScaleRatio += (s_fMinScaleRatio - m_fScaleRatio)
		* min(s_fShrinkLerp * fTimeDelta, 1.f);
	m_pTransformCom->Set_Scale(m_vBaseScale.x * m_fScaleRatio,
		m_vBaseScale.y * m_fScaleRatio,
		m_vBaseScale.z * m_fScaleRatio);
}

void CKokabu::Update_Spin(_float fTimeDelta)
{

}

void CKokabu::Update_SpatPivot_FromBone()
{

}

void CKokabu::Despawn()
{
}

HRESULT CKokabu::Bind_ShaderResources()
{
	return E_NOTIMPL;
}

CKokabu* CKokabu::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CKokabu* pInstance = new CKokabu(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CKokabu");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CKokabu* CKokabu::Clone(void* pArg)
{
	CKokabu* pInstance = new CKokabu(*this);
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CKokabu");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CKokabu::Free()
{
	__super::Free();
}
