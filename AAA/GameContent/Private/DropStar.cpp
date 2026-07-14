#include "DropStar.h"
#include "GameInstance.h"
#include "GameContent_const.h"
#include "GameContrnt_Events.h"
#include "DropStar_Body.h"
#include "DropStar_Manager.h"

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

	m_vBaseScale = m_pTransformCom->Get_Scaled();

	return S_OK;
}

void CDropStar::Update(_float fTimeDelta)
{
	if (!m_bActive)
		return;

	__super::Update(fTimeDelta);

	if (m_bCaptured)
	{
		Update_Captured(fTimeDelta);
		return;
	}

	// 물리는 보류 
	m_fTimer += fTimeDelta;
	if (m_fTimer >= s_fDeSpawnTime)
		Despawn();
}

void CDropStar::Late_Update(_float fTimeDelta)
{
	if (!m_bActive)
		return;

	__super::Late_Update(fTimeDelta);

	if (m_pCollider && m_pCollider->Is_Enabled())
	{
		m_pCollider->Update(XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));
	}
#ifdef _DEBUG
	m_pGameInstance_Proxy->Add_DebugComponent(m_pCollider);
#endif
}

void CDropStar::Set_Pool(CDropStar_Manager* pPool, _uint iLevel, const _wstring& strKey)
{
	m_pPool = pPool;
	m_iPoolLevel = iLevel;
	m_strPoolKey = strKey;
}

void CDropStar::Activate(const _float3& vPos, _float fDelay)
{
	m_bActive = true;
	m_bAvailable = true;
	m_bCaptured = false;
	m_pCaptor = nullptr;

	m_fDelay = fDelay;      
	m_fTimer = 0.f;
	m_fPullSpeed = 0.f;
	m_fScaleRatio = 1.f;

	m_pTransformCom->Set_State(STATE::POSITION,
		XMVectorSetW(XMLoadFloat3(&vPos), 1.f));
	m_pTransformCom->Set_Scale(m_vBaseScale.x,
		m_vBaseScale.y, m_vBaseScale.z);

	if (m_pCollider)
		m_pCollider->Set_Enabled(true);
}

void CDropStar::Launch(const _float3& vDir)
{
	
}

_bool CDropStar::Can_BeInhaled(const INHALE_QUERY& q) const
{
	return m_bAvailable && !m_bCaptured;
}

void CDropStar::Be_Captured(CGameObject* pInhaler)
{
	if (m_bCaptured)
		return;

	m_bCaptured = true;
	m_pCaptor = pInhaler;

	if (m_pCollider)
		m_pCollider->Set_Enabled(false);

	m_fPullSpeed = 0.f;
	m_fScaleRatio = 1.f;
}

void CDropStar::On_SpatBegin()
{
	m_pCaptor = nullptr;
	m_bCaptured = false;

	Set_Active(true);

	if (m_pCollider)
		m_pCollider->Set_Enabled(false);

	m_pTransformCom->Set_Scale(m_vBaseScale.x,
		m_vBaseScale.y, m_vBaseScale.z);
}

void CDropStar::On_SpatEnd()
{
	m_pCaptor = nullptr;
	m_bAvailable = false;

	Set_Active(false);
	Return_ToPool();
}

HRESULT CDropStar::Ready_Collider()
{
	CCollider::COLLIDER_DESC ColliderDesc{};
	ColliderDesc.pOwner = this;
	ColliderDesc.vCenter = { 0.f, 0.5f, 0.f };
	ColliderDesc.fRadius = 1.0f;

	m_pCollider = Add_Component<CCollider>(Collider_Sphere.iLevelID, Collider_Sphere.szProtoTag, TEXT("Com_DropStarCollider"), &ColliderDesc);
	if (nullptr == m_pCollider)
		return E_FAIL;

	m_pGameInstance_Proxy->Register_Collider(m_pCollider, ETOUI(m_eCollLayer));

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

void CDropStar::On_Swallowed()
{
	SWALLOW_EVENT payload{ this };
	m_pGameInstance_Proxy->Publish(
		EventTag::Swallowed, &payload);

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

	if (m_pCollider)
		m_pCollider->Set_Enabled(false);

	Return_ToPool();
}

void CDropStar::Return_ToPool()
{
	if (m_pPool)
		m_pPool->Return(m_iPoolLevel, m_strPoolKey, this);
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
