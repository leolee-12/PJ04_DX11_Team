#include "KoKabu.h"
#include "GameInstance.h"
#include "GameContent_Const.h"
#include "GameContent_Events.h"
#include "Projectile_Movement.h"
#include "Collider.h"
#include "Effect_Loader.h"


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

	m_bAlive = true;                    
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
	SWALLOW_EVENT payload{ this };
	m_pGameInstance_Proxy->Publish(EventTag::Swallowed, &payload);

	m_pCaptor = nullptr;

	if (m_pHitBox)     m_pHitBox->Set_Enabled(false);
	if (m_pController) m_pController->Set_Enabled(false);
	if (m_pMovement)   m_pMovement->Stop();

	m_bAlive = false;
}

HRESULT CKokabu::Initialize(void* pArg)
{
	m_fSpeed = 6.f;
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
	if (!m_bAlive)
		return;

	switch (m_eState)
	{
	case KOKABU_STATE::CAPTURED:			Update_Captured(fTimeDelta);		break;
	case KOKABU_STATE::DISAPPEARING:		Update_State(fTimeDelta);			break;
	default:																	break;
	}

	if (m_eState == KOKABU_STATE::CAPTURED ||
		m_eState == KOKABU_STATE::SPAT ||
		m_eState == KOKABU_STATE::DISAPPEARING)
	{
		m_pAnimatorCom->Update(fTimeDelta);
		return;
	}

	// EJECTED / FALLING / LANDED / HIT
	m_fAccLife += fTimeDelta;
	if (m_fAccLife >= m_fLifeTime && KOKABU_STATE::HIT != m_eState)
	{
		Change_State(KOKABU_STATE::DISAPPEARING);
		return;
	}

	if (m_pMovement)
	{
		m_pMovement->Tick(fTimeDelta);

		if (m_pMovement->Is_HitWall()
			&& (KOKABU_STATE::EJECTED == m_eState
				|| KOKABU_STATE::FALLING == m_eState
				|| KOKABU_STATE::LANDED == m_eState))
		{
			// wall: no attacker object -> knockback opposite to LOOK
			_float3 vVel = m_pMovement->Get_Velocity();
			XMStoreFloat3(&m_vHitFrom,
				m_pTransformCom->Get_State(STATE::POSITION)	+ XMLoadFloat3(&vVel));
			Change_State(KOKABU_STATE::HIT);
		}
	}

	Update_State(fTimeDelta);
	if (!m_bAlive) return;

	if (m_eState == KOKABU_STATE::EJECTED)
			Update_Spin(fTimeDelta);

	if (m_pAnimatorCom)
		m_pAnimatorCom->Update(fTimeDelta);

	if (m_pHitBox && m_pHitBox->Is_Enabled())
		m_pHitBox->Update(
			XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));

	
}

HRESULT CKokabu::Render()
{
	if (!m_bAlive)
		return S_OK;

	if (nullptr == m_pModelCom || nullptr == m_pShaderCom)
		return S_OK;

	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	const _uint iNumMeshes = static_cast<_uint>(m_pModelCom->Get_NumMeshes());
	for (_uint i = 0; i < iNumMeshes; ++i)
	{
		if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_DiffuseTexture", i, MTEX_TYPE::DIFFUSE, 0)))
			return E_FAIL;
		if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_NormalTexture", i, MTEX_TYPE::NORMALS, 0)))
			return E_FAIL;
		if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_MRATexture", i, MTEX_TYPE::METALNESS, 0)))
			return E_FAIL;
		if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i)))
			return E_FAIL;
		if (FAILED(m_pShaderCom->Begin(1)))
			return E_FAIL;
		if (FAILED(m_pModelCom->Render(i)))
			return E_FAIL;
	}
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
	m_pAnimatorCom = Add_Component<CAnimator>(TEXT("Com_Animator"),	CAnimator::Create(m_pDevice, m_pContext));
	if (nullptr == m_pAnimatorCom || FAILED(m_pAnimatorCom->Initialize(&ad)))
		return E_FAIL;

	m_pAnimatorCom->Play("Wait", true, true);

	return S_OK;
}

void CKokabu::On_Launched()
{
	Change_State(KOKABU_STATE::EJECTED);
}

void CKokabu::On_Impact()
{
	if (KOKABU_STATE::HIT == m_eState
		|| KOKABU_STATE::DEATH == m_eState
		|| KOKABU_STATE::CAPTURED == m_eState
		|| KOKABU_STATE::SPAT == m_eState
		|| KOKABU_STATE::DISAPPEARING == m_eState)
		return;

	Change_State(KOKABU_STATE::HIT);
}

HRESULT CKokabu::Ready_HitBox()
{
	if (FAILED(__super::Ready_HitBox()))
		return E_FAIL;

	// re-wire OnEnter to capture attacker position
	if (m_pHitBox)
	{
		m_pHitBox->Set_OnEnter([this](CCollider* pOther) {
			if (!m_bAlive)
				return;
			if (ETOUI(COLLISION_LAYER::PLAYER_HURT) != pOther->Get_RegisteredGroup())
				return;

			CGameObject* pAtk = pOther->Get_Owner();
			if (auto* pVictim = dynamic_cast<IDamageable*>(pAtk))
			{
				ATTACK_INFO atk{};
				atk.fDamage = m_fDamage;
				atk.fKnockback = m_fKnockback;
				XMStoreFloat3(&atk.vAttackerPos, m_pTransformCom->Get_State(STATE::POSITION));
				atk.pAttacker = this;
				pVictim->Damaged(atk);
			}

			if (pAtk)
				XMStoreFloat3(&m_vHitFrom,
					pAtk->Get_Transform()->Get_State(STATE::POSITION));
			On_Impact();
		});
	}

	return S_OK;
}

void CKokabu::Change_State(KOKABU_STATE eNext)
{
	if (m_eState == eNext)
		return;

	Exit_State(m_eState);
	m_eState = eNext;
	Enter_State(eNext);
}

void CKokabu::Enter_State(KOKABU_STATE eState)
{
	switch (eState)
	{
	case KOKABU_STATE::EJECTED:
		if (m_pAnimatorCom)
			m_pAnimatorCom->Play("Wait", true, true);
		break;

	case KOKABU_STATE::FALLING:
		if (m_pAnimatorCom)
			m_pAnimatorCom->Play("Fall", true, true);
		break;

	case KOKABU_STATE::LANDED:
		if (m_pAnimatorCom)
			m_pAnimatorCom->Play("Landing", false, true);
		break;

	case KOKABU_STATE::HIT:
	{
		if (m_pHitBox)     m_pHitBox->Set_Enabled(false);

		_vector vSelf = m_pTransformCom->Get_State(STATE::POSITION);
		_vector vDir = vSelf - XMLoadFloat3(&m_vHitFrom);
		vDir = XMVectorSetY(vDir, 0.f);
		if (XMVectorGetX(XMVector3LengthSq(vDir)) < 1e-6f)
			vDir = XMVectorNegate(m_pTransformCom->Get_State(STATE::LOOK));
		vDir = XMVector3Normalize(vDir) * s_fHitKnockback;
		vDir = XMVectorSetY(vDir, s_fHitKnockUp);

		if (m_pMovement)    m_pMovement->Launch(vDir);
		if (m_pAnimatorCom) m_pAnimatorCom->Play("Damage", false, true);

		m_fStateTimer = s_fHitReactTime;
		break;
	}

	case KOKABU_STATE::DEATH:
	{
		if (m_pHitBox)     m_pHitBox->Set_Enabled(false);
		if (m_pController) m_pController->Set_Enabled(false);
		if (m_pMovement)   m_pMovement->Stop();

		m_pGameInstance_Proxy->Play_SFX(L"CharaBasic_Dead.wav", 0.3f);

		const _float4* pCamLook = m_pGameInstance_Proxy->Get_CamLook();
		_float3 vFaceCam{};
		XMStoreFloat3(&vFaceCam,
			XMVectorNegate(XMLoadFloat4(pCamLook)));
		_float3 vPos{};
		XMStoreFloat3(&vPos,
			m_pTransformCom->Get_State(STATE::POSITION));

		CEffect_Loader::GetInstance()->Spawn(
			L"DespawnEffect", Get_LevelIndex(),
			vPos, vFaceCam, _float3(0.f, 0.f, 0.f), nullptr);

		Despawn();
		break;
	}

	case KOKABU_STATE::CAPTURED:
		if (m_pHitBox)     m_pHitBox->Set_Enabled(false);
		if (m_pController) m_pController->Set_Enabled(false);
		if (m_pMovement)   m_pMovement->Stop();
		if (m_pAnimatorCom)
			m_pAnimatorCom->Play("Damage", true, true);
		m_fPullSpeed = 0.f;
		m_fScaleRatio = 1.f;
		break;

	case KOKABU_STATE::SPAT:
		if (m_pHitBox)     m_pHitBox->Set_Enabled(false);
		if (m_pController) m_pController->Set_Enabled(false);
		if (m_pMovement)   m_pMovement->Stop();
		if (m_pAnimatorCom)
			m_pAnimatorCom->Play("Wait", true, true);
		break;

	case KOKABU_STATE::DISAPPEARING:
		if (m_pHitBox)     m_pHitBox->Set_Enabled(false);
		if (m_pController) m_pController->Set_Enabled(false);
		if (m_pMovement)   m_pMovement->Stop();
		if (m_pAnimatorCom)
			m_pAnimatorCom->Play("Warp1", false, true);
		break;

	default:
		break;
	}
}

void CKokabu::Update_State(_float fTimeDelta)
{
	switch (m_eState)
	{
	case KOKABU_STATE::EJECTED:
		if (m_pMovement && !m_pMovement->Is_Grounded())
			Change_State(KOKABU_STATE::FALLING);
		break;

	case KOKABU_STATE::LANDED:
		if (m_pMovement && !m_pMovement->Is_Grounded())
		{
			Change_State(KOKABU_STATE::FALLING);
			break;
		}
		if (m_pAnimatorCom && m_pAnimatorCom->Is_Finished())
			Change_State(KOKABU_STATE::EJECTED);
		break;

	case KOKABU_STATE::FALLING:
		if (m_pMovement && m_pMovement->Is_Grounded())
			Change_State(KOKABU_STATE::LANDED);
		break;

	case KOKABU_STATE::HIT:
		m_fStateTimer -= fTimeDelta;
		if (m_fStateTimer <= 0.f)
			Change_State(KOKABU_STATE::DEATH);
		break;

	case KOKABU_STATE::DISAPPEARING:
		if (m_pAnimatorCom && m_pAnimatorCom->Is_Finished())
			Despawn();
		break;

	default:
		break;
	}
}

void CKokabu::Exit_State(KOKABU_STATE eState)
{
	UNREFERENCED_PARAMETER(eState);
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
	m_fSpinAngle += XMConvertToRadians(s_fSpinSpeedDeg) * fTimeDelta;
	if (m_fSpinAngle >= XM_2PI)
		m_fSpinAngle -= XM_2PI;

	m_pTransformCom->Rotation(
		XMVectorSet(0.f, 1.f, 0.f, 0.f), m_fSpinAngle);
}

void CKokabu::Update_SpatPivot_FromBone()
{
	if (nullptr == m_pModelCom)
		return;

	const _float4x4* pBone = m_pModelCom->Get_BoneMatrixPtr("CenterL");
	if (nullptr == pBone)
		return;

	m_vSpatPivot = _float3(pBone->_41, pBone->_42, pBone->_43);
}

void CKokabu::Despawn()
{
	Kill();
}

HRESULT CKokabu::Bind_ShaderResources()
{
	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::VIEW, m_eProjType))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::PROJ, m_eProjType))))
		return E_FAIL;
	return S_OK;
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
