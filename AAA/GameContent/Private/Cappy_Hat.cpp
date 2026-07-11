#include "Cappy_Hat.h"
#include "GameInstance.h"
#include "GameContrnt_Events.h"
#include "Collider.h"
#include "Transform.h"
#include "Effect_Loader.h"

CCappy_Hat::CCappy_Hat(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CMonsterPart{ pDevice, pContext }
{
}

CCappy_Hat::CCappy_Hat(const CCappy_Hat& Prototype)
	: CMonsterPart(Prototype)
{
}

HRESULT CCappy_Hat::Initialize_Prototype()
{
	m_eProjType = PROJ_TYPE::PERSPEC;
	return S_OK;
}

HRESULT CCappy_Hat::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_Components()))
		return E_FAIL;
	if (FAILED(Ready_HurtBox()))
		return E_FAIL;

	XMStoreFloat4x4(&m_matDetachParent, XMMatrixIdentity());

	m_pAnimatorCom->Play("Wait", true, true);

	return S_OK;
}

void CCappy_Hat::Update(_float fTimeDelta)
{
	if (!m_bActive)
		return;

	__super::Update(fTimeDelta);

	if (m_bCaptured)
	{
		Update_CapturePull(fTimeDelta);
		return;
	}
}

void CCappy_Hat::Late_Update(_float fTimeDelta)
{
	if (!m_bActive)
		return;

	__super::Late_Update(fTimeDelta);

	if (m_pHurtBox)
	{
		m_pHurtBox->Update(XMLoadFloat4x4(&m_CombinedWorldMatrix));  // 무조건 갱신
#ifdef _DEBUG
		if (m_pHurtBox->Is_Enabled())
			m_pGameInstance_Proxy->Add_DebugComponent(m_pHurtBox);   // 디버그만 가드
#endif
	}
}

_bool CCappy_Hat::Can_BeInhaled(const INHALE_QUERY& q) const
{
	return m_bActive && !m_bDetached;
}

void CCappy_Hat::Be_Captured(CGameObject* pInhaler)
{
	if (m_bDetached)
		return;

	m_pCaptor = pInhaler;
	m_bDetached = true;
	m_bCaptured = true;
	m_fPullSpeed = 0.f;

	Detach();
}

COPY_ABILITY_TYPE CCappy_Hat::Get_CopyAbility() const
{
	return COPY_ABILITY_TYPE::NONE;
}

CGameObject* CCappy_Hat::Get_GameObject()
{
	return this;
}

void CCappy_Hat::On_SpatBegin()
{
	Set_Active(true);           
	m_bDetached = true;
	m_bCaptured = false;
}

void CCappy_Hat::On_SpatEnd()
{
	m_bGone = true;

	_vector vPosV = m_pTransformCom->Get_State(STATE::POSITION);
	_float3 vPos{};
	XMStoreFloat3(&vPos, vPosV);

	const _float4* pCamLook = m_pGameInstance_Proxy->Get_CamLook();
	_float3 vFaceCam{};
	XMStoreFloat3(&vFaceCam,
		XMVectorNegate(XMLoadFloat4(pCamLook)));

	m_pGameInstance_Proxy->Play_SFX3D(L"CharaBasic_Dead.wav", vPosV, 0.3f);

	CEffect_Loader::GetInstance()->Spawn(
		L"DespawnEffect", Get_LevelIndex(),
		vPos, vFaceCam, _float3(0.f, 0.f, 0.f), nullptr);

	Set_Active(false);
}

void CCappy_Hat::Enable_HurtBox(_bool b)
{
	if (m_pHurtBox)
		m_pHurtBox->Set_Enabled(b);
}

HRESULT CCappy_Hat::Ready_Components()
{
	PART_SETUP t{};
	t.tShader = Shader_Monster;
	t.szModelProtoTag = TEXT("Prototype_Component_Model_Cappy_Hat");

	if (FAILED(Ready_MeshPart(t)))
		return E_FAIL;

	return S_OK;
}

HRESULT CCappy_Hat::Ready_HurtBox()
{
	CCollider::COLLIDER_DESC desc{};
	desc.pOwner = this;          
	desc.fRadius = 0.65f;

	m_pHurtBox = Add_Component<CCollider>(
		Collider_Sphere.iLevelID, Collider_Sphere.szProtoTag,
		TEXT("Com_HurtBox"), &desc);
	if (nullptr == m_pHurtBox)
		return E_FAIL;

	m_pGameInstance_Proxy->Register_Collider(m_pHurtBox, ETOUI(COLLISION_LAYER::MONSTER_HURT));
	m_pHurtBox->Set_Enabled(true);
	return S_OK;
}

void CCappy_Hat::Update_CapturePull(_float fTimeDelta)
{
	if (nullptr == m_pCaptor)
		return;

	CTransform* pT = m_pCaptor->Get_Transform();
	_vector vMouth = pT->Get_State(STATE::POSITION)
		+ pT->Get_State(STATE::LOOK) * 0.6f
		+ pT->Get_State(STATE::UP) * 0.6f;

	_vector vSelf = m_pTransformCom->Get_State(STATE::POSITION);
	_vector vDir = vMouth - vSelf;
	_float fDist = XMVectorGetX(XMVector3Length(vDir));

	if (fDist <= 0.5f)
	{
		SWALLOW_EVENT payload{ this };   
		m_pGameInstance_Proxy->Publish(EventTag::Swallowed, &payload);
		m_bCaptured = false;
		Set_Active(false);               
		return;
	}

	m_fPullSpeed += s_fPullAccel * fTimeDelta;
	_float fMove = min(m_fPullSpeed * fTimeDelta, fDist);
	m_pTransformCom->Set_State(STATE::POSITION,
		vSelf + XMVector3Normalize(vDir) * fMove);
}

void CCappy_Hat::Detach()
{
	// 현재 소켓 월드 포즈 스냅샷 → 이후 combined = transformWorld
	m_pTransformCom->Set_WorldMatrix(
		XMLoadFloat4x4(&m_CombinedWorldMatrix));

	m_pSocketBoneMatrix = nullptr;              // 소켓 추종 해제
	m_pParentMatrix = &m_matDetachParent;   // 부모 = identity
	m_pHitFlash = nullptr;
	m_pHitFlashColor = nullptr;

	if (m_pHurtBox)
		m_pHurtBox->Set_Enabled(false);         // 재획득 방지

	if (m_pAnimatorCom)
		m_pAnimatorCom->Play("Fly", true, true);  // 비행 애니
}

CCappy_Hat* CCappy_Hat::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CCappy_Hat* pInstance = new CCappy_Hat(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype())) 
	{
		MSG_BOX("Failed to Created: CCappy_Hat");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CCappy_Hat::Clone(void* pArg)
{
	CCappy_Hat* pInstance = new CCappy_Hat(*this);
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned: CCappy_Hat");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CCappy_Hat::Free()
{
	__super::Free();
}
