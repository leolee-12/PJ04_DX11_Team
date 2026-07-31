#include "Spit_Projectile.h"
#include "GameInstance.h"
#include "Inhalable.h"
#include "GameContent_const.h"
#include "Effect_Loader.h" 

CSpit_Projectile::CSpit_Projectile(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CProjectile(pDevice, pContext)
{
}

CSpit_Projectile::CSpit_Projectile(const CSpit_Projectile& Prototype)
	: CProjectile(Prototype)
{
}

HRESULT CSpit_Projectile::Initialize(void* pArg)
{
	m_fHitRadius = 1.25f;
	m_fLifeTime = 2.0f;
	return __super::Initialize(pArg);
}

void CSpit_Projectile::Fire(IInhalable* const* ppPayloads, _uint iCount, const _float3& vPos, const _float3& vDir)
{
	m_iPayloadCount = (iCount < s_iMaxPayload) ? iCount : s_iMaxPayload;

	for (_uint i = 0; i < m_iPayloadCount; ++i)
	{
		IInhalable* p = ppPayloads[i];
		m_Payloads[i].pInhalable = p;
		if (p == nullptr)
			continue;

		p->On_SpatBegin();

		CTransform* pT = p->Get_GameObject()->Get_Transform();
		m_Payloads[i].vScale = pT->Get_Scaled();

		_matrix matW = XMLoadFloat4x4(pT->Get_WorldMatrixPtr());
		matW.r[0] = XMVector3Normalize(matW.r[0]);
		matW.r[1] = XMVector3Normalize(matW.r[1]);
		matW.r[2] = XMVector3Normalize(matW.r[2]);
		matW.r[3] = XMVectorSet(0.f, 0.f, 0.f, 1.f);
		XMStoreFloat4x4(&m_Payloads[i].matRot, matW);
	}

	if (m_iPayloadCount == 1)
		m_fDamage = 80.f;
	else if (m_iPayloadCount == 2)
		m_fDamage = 120.f;
	else
		m_fDamage = 190.f;

	XMStoreFloat3(&m_vSpinAxis, XMVector3Normalize(XMLoadFloat3(&vDir)));
	m_fSpinDeg = 0.f;

	Launch(vPos, vDir);

	if (m_pSpitFx)   
	{
		m_pSpitFx->EffectContainer_Stop();
		m_pSpitFx = nullptr;
	}

	const _float4* pCamLook =
		m_pGameInstance_Proxy->Get_CamLook();
	_float3 vFaceCam{};
	XMStoreFloat3(&vFaceCam,
		XMVectorNegate(XMLoadFloat4(pCamLook)));

	CEffect_Loader::GetInstance()->Spawn(
		L"SpitObject", Get_LevelIndex(),
		_float3(0.f, 0.f, 0.f),
		vFaceCam,
		_float3(0.f, 0.f, 0.f),
		m_pTransformCom->Get_WorldMatrixPtr(),
		&m_pSpitFx);
}

void CSpit_Projectile::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
	if (!m_bAlive)
		return;

	m_fSpinDeg += SPIN_SPEED_DEG * fTimeDelta;
	Drive_Payload();
}

void CSpit_Projectile::Late_Update(_float fTimeDelta)
{
	if (!m_bAlive)
		return;
#ifdef _DEBUG
	if (m_pHitBox && m_pHitBox->Is_Enabled())
		m_pGameInstance_Proxy->Add_DebugComponent(m_pHitBox);
#endif
}

HRESULT CSpit_Projectile::Ready_HitBox()
{
	CCollider::COLLIDER_DESC desc{};
	desc.pOwner = this;
	desc.fRadius = m_fHitRadius;

	m_pHitBox = Add_Component<CCollider>(Collider_Sphere.iLevelID, Collider_Sphere.szProtoTag,
		TEXT("Com_HitBox"), &desc);
	if (nullptr == m_pHitBox)
		return E_FAIL;

	m_pHitBox->Set_OnEnter([this](CCollider* pOther) {
		if (!m_bAlive)
			return;
		COLLISION_LAYER eLayer = static_cast<COLLISION_LAYER>(pOther->Get_RegisteredGroup());
		if (COLLISION_LAYER::MONSTER_HURT != eLayer && COLLISION_LAYER::ENV_HURT != eLayer)
			return;
		CGameObject* pVictimObj = pOther->Get_Owner();
		for (_uint i = 0; i < m_iPayloadCount; ++i)
			if (m_Payloads[i].pInhalable && m_Payloads[i].pInhalable->Get_GameObject() == pVictimObj)
				return;

		if (auto* pVictim = dynamic_cast<IDamageable*>(pVictimObj))
		{
			ATTACK_INFO atk{};
			atk.fDamage = m_fDamage;
			atk.fKnockback = m_fKnockback;
			XMStoreFloat3(&atk.vAttackerPos, m_pTransformCom->Get_State(STATE::POSITION));
			atk.pAttacker = this;
			pVictim->Damaged(atk);
		}
		On_Impact();
		});

	m_pHitBox->Set_Enabled(false);
	m_pGameInstance_Proxy->Register_Collider(m_pHitBox, ETOUI(COLLISION_LAYER::PLAYER_PROJECTILE));
	return S_OK;
}

void CSpit_Projectile::Kill()
{
	if (!m_bAlive)
		return;

	if (m_pSpitFx)
	{
		m_pSpitFx->EffectContainer_StopAfterEmission();
		m_pSpitFx = nullptr;
	}

	for (_uint i = 0; i < m_iPayloadCount; ++i)
	{
		if (m_Payloads[i].pInhalable)
		{
			m_Payloads[i].pInhalable->On_SpatEnd();
			m_Payloads[i].pInhalable = nullptr;
		}
	}
	m_iPayloadCount = 0;
	__super::Kill();
}

void CSpit_Projectile::Drive_Payload()
{
	_vector vPos = XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()).r[3];
	_matrix matTrans = XMMatrixTranslationFromVector(XMVectorSetW(vPos, 1.f));
	_matrix matSpin = XMMatrixRotationAxis(XMLoadFloat3(&m_vSpinAxis), XMConvertToRadians(m_fSpinDeg));

	for (_uint i = 0; i < m_iPayloadCount; ++i)
	{
		IInhalable* p = m_Payloads[i].pInhalable;
		if (p == nullptr)
			continue;
		CTransform* pDst = p->Get_GameObject()->Get_Transform();

		_float3 vPivot = p->Get_SpatPivotOffset();
		_matrix matPivot = XMMatrixTranslation(-vPivot.x, -vPivot.y, -vPivot.z);
		_matrix matScale = XMMatrixScaling(m_Payloads[i].vScale.x, m_Payloads[i].vScale.y, m_Payloads[i].vScale.z);
		_matrix matRot = XMLoadFloat4x4(&m_Payloads[i].matRot);

		pDst->Set_WorldMatrix(matPivot * matScale * matRot * matSpin * matTrans);
	}
}

CSpit_Projectile* CSpit_Projectile::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CSpit_Projectile* p = new CSpit_Projectile(pDevice, pContext);
	if (FAILED(p->Initialize_Prototype())) { MSG_BOX("Failed to Created : CSpit_Projectile"); Safe_Release(p); }
	return p;
}
CGameObject* CSpit_Projectile::Clone(void* pArg)
{
	CSpit_Projectile* p = new CSpit_Projectile(*this);
	if (FAILED(p->Initialize(pArg))) { MSG_BOX("Failed to Cloned : CSpit_Projectile"); Safe_Release(p); }
	return p;
}
void CSpit_Projectile::Free() { __super::Free(); }
