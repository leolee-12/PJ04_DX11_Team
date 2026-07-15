#include "DropStar.h"
#include "GameInstance.h"
#include "GameContent_const.h"
#include "GameContrnt_Events.h"
#include "Controller.h"
#include "DropStar_Body.h"
#include "DropStar_Manager.h"
#include "BubbleMovement.h"

CDropStar::CDropStar(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CContainerObject { pDevice,  pContext }
{
}

CDropStar::CDropStar(const CDropStar& Prototype)
	: CContainerObject (Prototype)
{
}

HRESULT CDropStar::Initialize_Prototype()
{
	if (FAILED(__super::Initialize_Prototype()))
		return E_FAIL;

	return S_OK;
}

HRESULT CDropStar::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_PartObjects()))
		return E_FAIL;

	if (FAILED(Ready_Collider()))
		return E_FAIL;

	if (FAILED(Ready_Movement()))
		return E_FAIL;

	m_vBaseScale = m_pTransformCom->Get_Scaled();

	return S_OK;
}

void CDropStar::Update(_float fTimeDelta)
{
	if (!m_bActive)
		return;

	if (m_eState == DROPSTAR_STATE::DELAY)
	{
		m_fDelay -= fTimeDelta;
		if (m_fDelay <= 0.f)
			Reveal();
		return;
	}

	if (m_eState == DROPSTAR_STATE::LIVE)
		Apply_Roll(fTimeDelta);

	__super::Update(fTimeDelta);

	if (m_eState == DROPSTAR_STATE::CAPTURED)
	{
		Update_Captured(fTimeDelta);
		return;
	}

	if (m_eState == DROPSTAR_STATE::SPAT)
		return;								// Spit ProjectileÀÌ Transform ±¸µ¿

	if (m_pMovement)
		m_pMovement->Tick(fTimeDelta);

	m_fTimer += fTimeDelta;

	if (m_fTimer >= s_fDeSpawnTime)
		Despawn();
}

void CDropStar::Late_Update(_float fTimeDelta)
{
	if (!m_bActive || m_eState == DROPSTAR_STATE::DELAY)
		return;

	__super::Late_Update(fTimeDelta);

	if (m_pCollider && m_pCollider->Is_Enabled())
	{
		m_pCollider->Update(XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));
#ifdef _DEBUG
	m_pGameInstance_Proxy->Add_DebugComponent(m_pCollider);
#endif
	}
}

void CDropStar::Set_Pool(CDropStar_Manager* pPool, _uint iLevel, const _wstring& strKey)
{
	m_pPool			= pPool;
	m_iPoolLevel	= iLevel;
	m_strPoolKey	= strKey;
}

void CDropStar::Activate(const _float3& vPos, const _float3& vLook, const _float3& vDir, _float fDelay)
{
	m_bActive = true;
	m_bAvailable = true;
	m_pCaptor = nullptr;
	m_eState = DROPSTAR_STATE::DELAY;

	if (m_pBody && m_pBody->Get_Animator())
		m_pBody->Get_Animator()->Play("NormalPosition", true, true);

	m_fDelay = fDelay;      
	m_fTimer = 0.f;
	m_fPullSpeed = 0.f;
	m_fScaleRatio = 1.f;
	m_fRollAngle = 0.f;

	m_pTransformCom->Set_State(STATE::POSITION,
		XMVectorSetW(XMLoadFloat3(&vPos), 1.f));

	Set_Orientation(vLook);

	m_vLaunchDir = vDir;

	m_pTransformCom->Set_Scale(m_vBaseScale.x,
		m_vBaseScale.y, m_vBaseScale.z);

	if (m_pMovement)
		m_pMovement->Stop();
	if (m_pController)
		m_pController->Set_Enabled(false);
	if (m_pCollider)
		m_pCollider->Set_Enabled(false);
}

_bool CDropStar::Can_BeInhaled(const INHALE_QUERY& q) const
{
	return m_bAvailable && m_eState == DROPSTAR_STATE::LIVE;
}

void CDropStar::Be_Captured(CGameObject* pInhaler)
{
	if (m_eState == DROPSTAR_STATE::CAPTURED)
		return;

	m_eState = DROPSTAR_STATE::CAPTURED;
	m_pCaptor = pInhaler;

	if (m_pMovement)
		m_pMovement->Stop();
	if (m_pController)
		m_pController->Set_Enabled(false);
	if (m_pCollider)
		m_pCollider->Set_Enabled(false);

	m_fPullSpeed = 0.f;
	m_fScaleRatio = 1.f;
}

void CDropStar::On_SpatBegin()
{
	m_pCaptor = nullptr;
	m_eState = DROPSTAR_STATE::SPAT;

	Set_Active(true);

	Update_SpatPivot_FromBone();

	if (m_pMovement)
		m_pMovement->Stop();
	if (m_pController)
		m_pController->Set_Enabled(false);
	if (m_pCollider)
		m_pCollider->Set_Enabled(false);

	m_pTransformCom->Set_Scale(m_vBaseScale.x, m_vBaseScale.y, m_vBaseScale.z);
}

void CDropStar::On_SpatEnd()
{
	m_pCaptor = nullptr;
	m_bAvailable = false;

	if (m_pMovement)
		m_pMovement->Stop();
	if (m_pController)
		m_pController->Set_Enabled(false);
	if (m_pCollider)
		m_pCollider->Set_Enabled(false);

	Set_Active(false);
	Return_ToPool();
}

HRESULT CDropStar::Ready_Collider()
{
	CCollider::COLLIDER_DESC ColliderDesc{};
	ColliderDesc.pOwner = this;
	ColliderDesc.vCenter = { 0.f, 0.85f, 0.f };
	ColliderDesc.fRadius = 0.85f;

	m_pCollider = Add_Component<CCollider>(Collider_Sphere.iLevelID, Collider_Sphere.szProtoTag, TEXT("Com_DropStarCollider"), &ColliderDesc);
	if (nullptr == m_pCollider)
		return E_FAIL;

	m_pGameInstance_Proxy->Register_Collider(m_pCollider, ETOUI(m_eCollLayer));

	return S_OK;
}

HRESULT CDropStar::Ready_Movement()
{
	CController::CONTROLLER_DESC cd{};
	cd.pOwner = this;
	cd.fRadius = 0.85f;
	cd.fHeight = 0.01f;
	cd.vFootPos = { 0.f, 0.f, 0.f };

	m_pController = Add_Component<CController>(TEXT("Com_DropStarController"), CController::Create(m_pDevice, m_pContext));
	if (nullptr == m_pController)
		return E_FAIL;
	if (FAILED(m_pController->Initialize(&cd)))
		return E_FAIL;
	m_pController->Set_Enabled(false);

	m_pMovement = Add_Component<CBubbleMovement>(TEXT("Com_Movement"), CBubbleMovement::Create(m_pDevice, m_pContext));
	if (nullptr == m_pMovement)
		return E_FAIL;

	m_pMovement->Set_Refs(m_pTransformCom, m_pController);
	m_pMovement->Set_Physics(s_fGravity, s_fRestitution, s_fHorizDamp);

	return S_OK;
}

HRESULT CDropStar::Ready_PartObjects()
{
	CDropStar_Body::DROPSTAR_BODY_DESC desc{};
	desc.pParentMatrix = m_pTransformCom->Get_WorldMatrixPtr();

	if (FAILED(Add_PartObject(m_iPrototypeLevel, CDropStar_Body::PROTOTYPE_TAG, BODY_PART_TAG, &desc)))
		return E_FAIL;

	CDropStar_Body* pBody = dynamic_cast<CDropStar_Body*>(m_PartObjects[BODY_PART_TAG]);
	if (nullptr == pBody)
		return E_FAIL;

	m_pBody = pBody;

	return S_OK;
}

void CDropStar::Reveal()
{
	m_eState = DROPSTAR_STATE::LIVE;
	m_fTimer = 0.f;
	if (m_pBody && m_pBody->Get_Animator())
		m_pBody->Get_Animator()->Play("BoundlPosition", true, true);


	if (m_pController)
	{
		m_pController->Set_Enabled(true);
		m_pController->Set_Solid(false);
		m_pController->Set_FootPosition(m_pTransformCom->Get_State(STATE::POSITION));
	}

	Launch();    

	if (m_pCollider)
		m_pCollider->Set_Enabled(true);
}

void CDropStar::Update_Captured(_float fTimeDelta)
{
	if (nullptr == m_pCaptor)
		return;

	CTransform* pCapT = m_pCaptor->Get_Transform();
	_vector vMouth = pCapT->Get_State(STATE::POSITION)
		+ pCapT->Get_State(STATE::LOOK) * 0.6f
		+ pCapT->Get_State(STATE::UP) * 0.6f;

	_vector vSelf = m_pTransformCom->Get_State(STATE::POSITION);
	_vector vDir = vMouth - vSelf;
	_float  fDist = XMVectorGetX(XMVector3Length(vDir));

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

void CDropStar::Update_SpatPivot_FromBone()
{
	if (nullptr == m_pBody)
		return;

	const _float4x4* pBone = m_pBody->Get_BoneMatrixPtr("CenterL");
	if (nullptr == pBone)
		return;

	m_vSpatPivot = _float3(pBone->_41, pBone->_42, pBone->_43);
}

void CDropStar::On_Swallowed()
{
	SWALLOW_EVENT payload{ this };
	m_pGameInstance_Proxy->Publish(EventTag::Swallowed, &payload);

	m_pCaptor = nullptr;

	if (m_pCollider)
		m_pCollider->Set_Enabled(false);

	Set_Active(false);
}

void CDropStar::Despawn()
{
	if (!m_bActive)
		return;

	m_bAvailable = false;

	Set_Active(false);

	if (m_pMovement)
		m_pMovement->Stop();
	if (m_pController)
		m_pController->Set_Enabled(false);
	if (m_pCollider)
		m_pCollider->Set_Enabled(false);

	Return_ToPool();
}

void CDropStar::Return_ToPool()
{
	if (m_pPool)
		m_pPool->Return(m_iPoolLevel, m_strPoolKey, this);
}

void CDropStar::Set_Orientation(const _float3& vLook)
{
	_vector vL = XMLoadFloat3(&vLook);

	if (XMVectorGetX(XMVector3LengthSq(vL)) < 1e-4f)
	{
		m_pTransformCom->Set_State(STATE::RIGHT,
			XMVectorSet(1.f, 0.f, 0.f, 0.f));
		m_pTransformCom->Set_State(STATE::UP,
			XMVectorSet(0.f, 1.f, 0.f, 0.f));
		m_pTransformCom->Set_State(STATE::LOOK,
			XMVectorSet(0.f, 0.f, 1.f, 0.f));
		return;
	}

	vL = XMVector3Normalize(vL);

	_vector vUp = XMVectorSet(0.f, 1.f, 0.f, 0.f);
	_vector vR = XMVector3Cross(vUp, vL);
	if (XMVectorGetX(XMVector3LengthSq(vR)) < 1e-4f)   
		vR = XMVectorSet(1.f, 0.f, 0.f, 0.f);
	vR = XMVector3Normalize(vR);

	_vector vU = XMVector3Cross(vL, vR);

	m_pTransformCom->Set_State(STATE::RIGHT, vR);
	m_pTransformCom->Set_State(STATE::UP, vU);
	m_pTransformCom->Set_State(STATE::LOOK, vL);
}

void CDropStar::Launch()
{
	if (m_pMovement == nullptr)
		return;

	_vector v = XMLoadFloat3(&m_vLaunchDir);
	v = XMVectorSetY(v, s_fPopUpSpeed);
	m_pMovement->Launch(v);
}

void CDropStar::Apply_Roll(_float fTimeDelta)
{
	if (m_pBody == nullptr)
		return;

	CAnimator* pAnim = m_pBody->Get_Animator();
	if (pAnim == nullptr)
		return;

	m_fRollAngle = fmodf(m_fRollAngle + s_fRollSpeed * fTimeDelta, 360.f);
	pAnim->SetBoneRotation("CenterL", m_fRollAngle, XMVectorSet(0.f, 0.f, 1.f, 0.f));
}

CDropStar* CDropStar::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CDropStar* pInstance = new CDropStar(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CDropStar");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CDropStar::Clone(void* pArg)
{
	CDropStar* pInstance = new CDropStar(*this);
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Created : CDropStar");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CDropStar::Free()
{
	__super::Free();
}
