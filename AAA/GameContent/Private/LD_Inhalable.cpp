#include "LD_Inhalable.h"
#include "GameContent_const.h"
#include "Damageable.h"

#include "GameInstance.h"

NS_BEGIN(Client)

CLD_Inhalable::CLD_Inhalable(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CLevelDesignObject(pDevice, pContext)
{
}

CLD_Inhalable::CLD_Inhalable(const CLD_Inhalable& Prototype)
	: CLevelDesignObject(Prototype)
	, m_eState(Prototype.m_eState)
	, m_fColliderRadius(Prototype.m_fColliderRadius)
	, m_fPullSpeed(Prototype.m_fPullSpeed)
	, m_vBaseScale(Prototype.m_vBaseScale)
	, m_fScaleRatio(Prototype.m_fScaleRatio)
	, m_fLifeTime(Prototype.m_fLifeTime)
{
}

HRESULT CLD_Inhalable::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_HurtBox()))
		return E_FAIL;

	return S_OK;
}

HRESULT CLD_Inhalable::Validate_Initialized()
{
	if (FAILED(__super::Validate_Initialized()))
		return E_FAIL;

	if (nullptr == m_pHurtBox)
		return E_FAIL;
	if (m_fColliderRadius <= 0.f)
		return E_FAIL;
	if (LD_STATE::IDLE != m_eState && LD_STATE::CAPTURED != m_eState)
		return E_FAIL;

	return S_OK;
}

void CLD_Inhalable::Update(_float fTimeDelta)
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
	}
}

void CLD_Inhalable::Late_Update(_float fTimeDelta)
{
	if (!m_bActive)
		return;

	if (m_pHurtBox->Is_Enabled())
	{
		m_pHurtBox->Update(XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));
#ifdef _DEBUG
		m_pGameInstance_Proxy->Add_DebugComponent(m_pHurtBox);
#endif
	}
}

HRESULT CLD_Inhalable::Ready_HurtBox()
{
	if (m_fColliderRadius <= 0.f)
		return E_FAIL;
	if (nullptr == m_pGameInstance_Proxy)
		return E_FAIL;

	CCollider::COLLIDER_DESC Desc{};
	Desc.pOwner = this;
	Desc.fRadius = m_fColliderRadius;
	Desc.vCenter = _float3(0.f, m_fColliderRadius * 0.5f, 0.f);

	m_pHurtBox = Add_Component<CCollider>(Collider_Sphere.iLevelID, Collider_Sphere.szProtoTag, TEXT("Com_HurtBox"), &Desc);
	if (nullptr == m_pHurtBox)
		return E_FAIL;

	m_pGameInstance_Proxy->Register_Collider(m_pHurtBox, ETOUI(COLLISION_LAYER::ENV_HURT));

	SetUp_Collider_Callback();

	return S_OK;
}

void CLD_Inhalable::SetUp_Collider_Callback()
{
}

void CLD_Inhalable::On_Swallowed()
{
	SWALLOW_EVENT payload{ this };
	m_pGameInstance_Proxy->Publish(EventTag::Swallowed, &payload);
	m_pCaptor = nullptr;
	Enable_Colliders(false);
	Set_Active(false);
}

void CLD_Inhalable::Enable_Colliders(_bool b)
{
	m_pHurtBox->Set_Enabled(b);
}

void CLD_Inhalable::Despawn_Spat()
{
	Enable_Colliders(false);
	Set_Active(false);
}

void CLD_Inhalable::Update_Captured(_float fTimeDelta)
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

void CLD_Inhalable::Be_Captured(CGameObject* pInhaler)
{
	m_pCaptor = pInhaler;
	m_eState = LD_STATE::CAPTURED;
	Enable_Colliders(false);

	m_fPullSpeed = s_fPullInitSpeed;
	m_vBaseScale = Get_Transform()->Get_Scaled();
	m_fScaleRatio = 1.f;
}

void CLD_Inhalable::On_SpatBegin()
{
	m_pCaptor = nullptr;
	Set_Active(true);

	m_eState = LD_STATE::IDLE;  
	Enable_Colliders(false);    
	Get_Transform()->Set_Scale(m_vBaseScale.x, m_vBaseScale.y, m_vBaseScale.z);
}

void CLD_Inhalable::On_SpatEnd()
{
	m_pCaptor = nullptr;
	Set_Active(false);
}

NS_END

