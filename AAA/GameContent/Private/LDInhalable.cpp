#include "LDInhalable.h"
#include "GameContent_const.h"
#include "Damageable.h"

#include "GameInstance.h"

CLDInhalable::CLDInhalable(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CLevelDesignObject(pDevice, pContext)
{
}

CLDInhalable::CLDInhalable(const CLevelDesignObject& Prototype)
	: CLevelDesignObject(Prototype)
{
}

HRESULT CLDInhalable::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_Collider()))
		return E_FAIL;

	return S_OK;
}

void CLDInhalable::Update(_float fTimeDelta)
{
	if (!m_bActive)
		return;

	switch (m_eState)
	{
		case LD_STATE::IDLE:
			break;
		case LD_STATE::CAPTURED:
			Update_Captured(fTimeDelta);
			break;
		case LD_STATE::SPAT:
			Update_Spat(fTimeDelta);
			break;
	}
}

void CLDInhalable::Late_Update(_float fTimeDelta)
{
	if (!m_bActive)
		return;

	if (m_pHurtBox && m_pHurtBox->Is_Enabled())
	{
		m_pHurtBox->Update(XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));
#ifdef _DEBUG
		m_pGameInstance_Proxy->Add_DebugComponent(m_pHurtBox);
#endif
	}

	if (m_pProjectileBox && m_pProjectileBox->Is_Enabled())
	{
		m_pProjectileBox->Update(XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));
#ifdef _DEBUG
		m_pGameInstance_Proxy->Add_DebugComponent(m_pProjectileBox);
#endif
	}
}

HRESULT	CLDInhalable::Ready_Collider()
{
	if (m_fColliderRadius <= 0.f)
		return S_OK;

	CCollider::COLLIDER_DESC Desc{};
	Desc.pOwner = this;
	Desc.fRadius = m_fColliderRadius;
	Desc.vCenter = _float3(0.f, m_fColliderRadius * 0.5f, 0.f);

	m_pHurtBox = Add_Component<CCollider>(Collider_Sphere.iLevelID, Collider_Sphere.szProtoTag, TEXT("Com_HurtBox"), &Desc);
	if (nullptr == m_pHurtBox)
		return E_FAIL;

	m_pGameInstance_Proxy->Register_Collider(m_pHurtBox, ETOUI(COLLISION_LAYER::ENV_HURT));


	m_pProjectileBox = Add_Component<CCollider>(Collider_Sphere.iLevelID, Collider_Sphere.szProtoTag, TEXT("Com_ProjectileBox"), &Desc);
	if (nullptr == m_pProjectileBox)
		return E_FAIL;

	m_pGameInstance_Proxy->Register_Collider(m_pProjectileBox, ETOUI(COLLISION_LAYER::PLAYER_PROJECTILE));

	SetUp_Collider_CallBack();

	return S_OK;
}

void CLDInhalable::SetUp_Collider_CallBack()
{
	if (m_pProjectileBox)
	{
		m_pProjectileBox->Set_OnEnter([this](CCollider* pOther) {
			if (ETOUI(COLLISION_LAYER::MONSTER_HURT) != pOther->Get_RegisteredGroup())
				return;
			if (pOther->Get_Owner() == this)
				return;

			IDamageable* pVictim = dynamic_cast<IDamageable*>(pOther->Get_Owner());
			if (nullptr == pVictim)
				return;

			ATTACK_INFO atk{};
			atk.fDamage = s_fSpatDamage;
			atk.fKnockback = s_fSpatKnockback;
			XMStoreFloat3(&atk.vAttackerPos, m_pTransformCom->Get_State(STATE::POSITION));
			atk.pAttacker = this;
			pVictim->Damaged(atk);

			Despawn_Spat();
			});
	}
}

void CLDInhalable::On_Swallowed()
{
	SWALLOW_EVENT payload{ this };
	m_pGameInstance_Proxy->Publish(EVT_SWALLOWED, &payload);
	m_pCaptor = nullptr;
	Set_Active(false);
}

void   CLDInhalable::Enable_Colliders(_bool b)
{
	if (m_pHurtBox)
		m_pHurtBox->Set_Enabled(b);
}

void   CLDInhalable::Despawn_Spat()
{
	if(m_pProjectileBox)
		m_pProjectileBox->Set_Enabled(false);
	m_vSpatVelocity = {};
	Set_Active(false);
}

void CLDInhalable::Update_Captured(_float fTimeDelta)
{
	if (!m_pCaptor)
		return;

	CTransform* pCapT = m_pCaptor->Get_Transform();

	_vector vMouth = pCapT->Get_State(STATE::POSITION)
		+ pCapT->Get_State(STATE::LOOK) * 0.6f
		+ pCapT->Get_State(STATE::UP) * 0.6f;

	CTransform* pT = Get_Transform();
	_vector vSelf = pT->Get_State(STATE::POSITION);
	_vector vDir = vMouth - vSelf;
	_float fDist = XMVectorGetX(XMVector3Length(vDir));

	if (fDist <= 0.5f)
	{
		On_Swallowed();
		return;
	}

	m_fPullSpeed += s_fPullAccel * fTimeDelta;
	_float fMove = min(m_fPullSpeed * fTimeDelta, fDist);
	pT->Set_State(STATE::POSITION, vSelf + XMVector3Normalize(vDir) * fMove);

	m_fScaleRatio += (s_fMinScaleRatio - m_fScaleRatio) * min(s_fShrinkLerp * fTimeDelta, 1.f);

	pT->Set_Scale(m_vBaseScale.x * m_fScaleRatio,
		m_vBaseScale.y * m_fScaleRatio,
		m_vBaseScale.z * m_fScaleRatio);
}

void CLDInhalable::Update_Spat(_float fTimeDelta)
{
	CTransform* pT = Get_Transform();
	_vector vVel = XMLoadFloat3(&m_vSpatVelocity);
	pT->Set_State(STATE::POSITION,
		pT->Get_State(STATE::POSITION) + vVel * fTimeDelta);
	_vector vUp = pT->Get_State(STATE::UP);

	m_fSpinAngle += s_fSpinSpeedDeg * fTimeDelta;

	pT->Rotate(XMQuaternionRotationAxis(vUp,
		XMConvertToRadians(s_fSpinSpeedDeg) * fTimeDelta));

	m_fLifeTime -= fTimeDelta;
	if (m_fLifeTime <= 0.f)
		Despawn_Spat();
}

void CLDInhalable::Be_Captured(CGameObject* pInhaler)
{
	m_pCaptor = pInhaler;
	m_eState = LD_STATE::CAPTURED;
	Enable_Colliders(false);

	m_fPullSpeed = s_fPullInitSpeed;
	m_vBaseScale = Get_Transform()->Get_Scaled();
	m_fScaleRatio = 1.f;
}

void CLDInhalable::Be_Spat(_fvector vPos, _fvector vDir, _float fSpeed)
{
	m_pCaptor = nullptr;
	Set_Active(true);

	CTransform* pT = m_pTransformCom;
	pT->Set_State(STATE::POSITION, vPos);
	pT->LookAt(XMVectorAdd(vPos, vDir));
	pT->Rotate(XMQuaternionRotationAxis(pT->Get_State(STATE::RIGHT), XMConvertToRadians(90.f)));

	XMStoreFloat3(&m_vSpatVelocity, XMVector3Normalize(vDir) * fSpeed);

	Get_Transform()->Set_Scale(m_vBaseScale.x, m_vBaseScale.y, m_vBaseScale.z);

	m_eState = LD_STATE::SPAT;

	m_fSpinAngle = 0.f;

	Enable_Colliders(false);
	if (m_pProjectileBox)
		m_pProjectileBox->Set_Enabled(true);

	m_fLifeTime = s_fMaxLifeTime;
}


