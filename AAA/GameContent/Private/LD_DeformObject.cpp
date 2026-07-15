#include "LD_DeformObject.h"
#include "LevelDesign_Registry.h"
#include "Parsing_Utils.h"
#include "GameContent_const.h"
#include "GameObject_Factory.h"
#include "Effect_Loader.h"

#include "Geometry_Utils.h"

namespace
{
	// 회전 & 빨려들어가는 물리 관련
	static constexpr _float s_fFixedAcquireDuration = { 1.f };
	static constexpr _float s_fPullInitSpeed = { 4.f };
	static constexpr _float s_fPullAccel = { 18.f };
	static constexpr _float s_fAcquireDistance = { 1.2f };
	static constexpr _float s_fAlignRotSpeedDegree = { 72.f };
	static constexpr _float s_fPullTargetFwd = { 1.8f };
	static constexpr _float s_fReleaseSpeed = { 20.f };
	static constexpr _float s_fReleaseFwd = { -5.f };

	static constexpr _float s_fGravity = { -45.f };
	static constexpr _float s_fMaxFallSpeed = { -15.f };
	static constexpr _float s_fGroundProbeDistance = { 0.02f };
	static constexpr _float s_fMinSweepHalfExtent = { 0.05f };

	static constexpr _float s_fAnimSpeed = { 1.f };

	enum class DEFORM_ANIM_SLOT : _uint
	{
		IDLE,
		CAPTURED,
		FALLING,
		LANDING,
		END
	};

	static_assert(static_cast<_uint>(DEFORM_ANIM_SLOT::END) == Client::LD_ANIM_SLOT_COUNT);

	struct LD_DEFORMOBJECT_CATALOG
	{
		const _tchar* pObjectName;
		const _tchar* pModelProtoTag;
		const _char* pModelPath;
		MODEL eModelType;
		DEFORM_TYPE eDeformType;
		DEFORM_OBJECT_KIND eKind;
		_float fInteractionRadius;
		const _tchar* pSpitDestroyEffectId;
		const _char* pAnimNames[Client::LD_ANIM_SLOT_COUNT];
	};

	static const LD_DEFORMOBJECT_CATALOG g_DeformObjectCatalog[] =
	{
		  { L"DeformCar", L"Proto_Component_Model_DeformCar", "../../Resources/Map/Gimmick/Anim/DeformCar/DeformCar.ysh",
		  MODEL::ANIM, DEFORM_TYPE::CAR, DEFORM_OBJECT_KIND::MOBILE, 6.5f, nullptr, { "Wait", "Shake", "Fall", "Landing" } },
		  { L"DeformCylinder", L"Proto_Component_Model_DeformCylinder", "../../Resources/Map/Gimmick/Anim/DeformCylinder/DeformCylinder.ysh",
		  MODEL::ANIM, DEFORM_TYPE::CYLINDER, DEFORM_OBJECT_KIND::MOBILE, 3.f, L"Split_Cylinder", { "Wait", "Wait", "Fall", "Landing" } },
		  { L"DeformCoaster", L"Proto_Component_Model_DeformCoaster", "../../Resources/Map/Gimmick/Anim/DeformCoaster/DeformCoaster.ysh",
		  MODEL::ANIM, DEFORM_TYPE::COASTER, DEFORM_OBJECT_KIND::FIXED, 3.f, L"Split_Coaster", { "Wait", "Wait", "", "" } },
	};

	void Apply_DeformObjectCatalog(const LD_DEFORMOBJECT_CATALOG& Entry, Client::LD_DEFORMOBJECT_DESC* pDesc)
	{
		if (nullptr == pDesc)
			return;

		pDesc->eModelType = Entry.eModelType;
		pDesc->wstrModelProtoTag = Entry.pModelProtoTag;
		pDesc->eDeformType = Entry.eDeformType;
		pDesc->fInteractionRadius = Entry.fInteractionRadius;

		for (_uint i = 0; i < Client::LD_ANIM_SLOT_COUNT; ++i)
		{
			const _char* pAnimName = Entry.pAnimNames[i];
			pDesc->strAnimNames[i] = nullptr != pAnimName ? pAnimName : "";
		}
	}

	_bool Is_SupportedCatalog(const LD_DEFORMOBJECT_CATALOG& Entry)
	{
		const _char* pIdleAnim = Entry.pAnimNames[ETOUI(DEFORM_ANIM_SLOT::IDLE)];

		return nullptr != Entry.pObjectName
			&& nullptr != Entry.pModelProtoTag
			&& nullptr != Entry.pModelPath
			&& MODEL::END != Entry.eModelType
			&& DEFORM_TYPE::NONE != Entry.eDeformType
			&& nullptr != pIdleAnim
			&& '\0' != pIdleAnim[0];
	}

	const LD_DEFORMOBJECT_CATALOG* Find_DeformObjectCatalog(const _wstring& strObjectName)
	{
		for (const LD_DEFORMOBJECT_CATALOG& Entry : g_DeformObjectCatalog)
		{
			if (JsonUtils::Equals_NoCase(Entry.pObjectName, strObjectName.c_str()))
				return &Entry;
		}

		return nullptr;
	}

	const LD_DEFORMOBJECT_CATALOG* Find_DeformObjectCatalog_ByType(DEFORM_TYPE eType)
	{
		for (const LD_DEFORMOBJECT_CATALOG& Entry : g_DeformObjectCatalog)
		{
			if (Entry.eDeformType == eType && Is_SupportedCatalog(Entry))
				return &Entry;
		}
		return nullptr;
	}
}

NS_BEGIN(Client)

CLD_DeformObject::CLD_DeformObject(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CLD_EventObject(pDevice, pContext)
{
}

CLD_DeformObject::CLD_DeformObject(const CLD_DeformObject& Prototype)
	: CLD_EventObject(Prototype)
	, m_tDeformObjectDesc(Prototype.m_tDeformObjectDesc)
	, m_bAvailable(Prototype.m_bAvailable)
	, m_eState(Prototype.m_eState)
	, m_eKind(Prototype.m_eKind)
{
}

HRESULT CLD_DeformObject::Initialize(void* pArg)
{
	if (nullptr == pArg)
		return E_FAIL;

	m_tDeformObjectDesc = *static_cast<const LD_DEFORMOBJECT_DESC*>(pArg);

	const LD_DEFORMOBJECT_CATALOG* pCatalog = Find_DeformObjectCatalog(m_tDeformObjectDesc.strObjectName);
	if (nullptr == pCatalog)
		return E_FAIL;

	m_eKind = pCatalog->eKind;

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	LD_ANIM_PLAY_DESC AnimDescs[Client::LD_ANIM_SLOT_COUNT] = {};
	_uint iAnimDescCount = 0;

	const auto Add_AnimDesc = [&](DEFORM_ANIM_SLOT eSlot, _bool bLoop) -> HRESULT
		{
			const _string& strAnimName = m_tDeformObjectDesc.strAnimNames[ETOUI(eSlot)];
			if (strAnimName.empty())
				return S_OK;

			for (_uint i = 0; i < iAnimDescCount; ++i)
			{
				if (AnimDescs[i].strAnimName != strAnimName)
					continue;

				return AnimDescs[i].bLoop == bLoop ? S_OK : E_FAIL;
			}

			AnimDescs[iAnimDescCount++] = { strAnimName, bLoop, s_fAnimSpeed };
			return S_OK;
		};

	if (FAILED(Add_AnimDesc(DEFORM_ANIM_SLOT::IDLE, true))
		|| FAILED(Add_AnimDesc(DEFORM_ANIM_SLOT::CAPTURED, true))
		|| FAILED(Add_AnimDesc(DEFORM_ANIM_SLOT::FALLING, true))
		|| FAILED(Add_AnimDesc(DEFORM_ANIM_SLOT::LANDING, false)))
		return E_FAIL;

	if (FAILED(Ready_AnimPlayDescs(AnimDescs, iAnimDescCount)))
		return E_FAIL;

	Play_Anim(m_tDeformObjectDesc.strAnimNames[ETOUI(DEFORM_ANIM_SLOT::IDLE)]);

	m_bUseShadow = true;

	if (!m_tDeformObjectDesc.strReceiveEventTag.empty())
	{
		Set_TriggerEnabled(false);
		Release_RigidStatic();
		Set_Active(false);
	}

	return S_OK;
}

HRESULT CLD_DeformObject::Validate_Initialized()
{
	if (FAILED(__super::Validate_Initialized()))
		return E_FAIL;

	const LD_DEFORMOBJECT_CATALOG* pCatalog = Find_DeformObjectCatalog(m_tDeformObjectDesc.strObjectName);
	if (nullptr == pCatalog || !Is_SupportedCatalog(*pCatalog))
		return E_FAIL;

	if (m_tDeformObjectDesc.wstrModelProtoTag != pCatalog->pModelProtoTag
		|| m_tDeformObjectDesc.eModelType != pCatalog->eModelType
		|| m_tDeformObjectDesc.eDeformType != pCatalog->eDeformType)
		return E_FAIL;

	for (_uint i = 0; i < Client::LD_ANIM_SLOT_COUNT; ++i)
	{
		const _char* pAnimName = pCatalog->pAnimNames[i];
		const _string strCatalogAnimName = nullptr != pAnimName ? pAnimName : "";

		if (m_tDeformObjectDesc.strAnimNames[i] != strCatalogAnimName)
			return E_FAIL;
	}

	if (m_tDeformObjectDesc.fInteractionRadius < 0.f)
		return E_FAIL;

	if (nullptr == m_pTrigger)
		return E_FAIL;

	if (!m_tDeformObjectDesc.bUseCollMesh || nullptr == m_pRigidStatic)
		return E_FAIL;

	return S_OK;
}

void CLD_DeformObject::Update(_float fTimeDelta)
{
#ifdef _DEBUG
	if (!m_pGameInstance_Proxy->Is_EditMode()
		&& m_pGameInstance_Proxy->Key_Down(DIK_F3))
	{
		if (Is_Active())
		{
			if (DEFORM_OBJECT_STATE::IDLE == m_eState && m_bAvailable)
			{
				Change_State(DEFORM_OBJECT_STATE::ACQUIRED);
				On_DeformAcquired();
				return;
			}
		}
		else
		{
			Spawn_Released(
				m_pGameInstance_Proxy,
				Get_DeformType(),
				Get_LevelIndex(),
				m_pTransformCom->Get_WorldMatrixPtr());
			return;
		}
	}
#endif

	switch (m_eState)
	{
	case DEFORM_OBJECT_STATE::IDLE:
	{
		if (!m_bAvailable)
			return;

		__super::Update(fTimeDelta);
		break;
	}
	case DEFORM_OBJECT_STATE::CAPTURED:
	{
		switch (m_eKind)
		{
		case DEFORM_OBJECT_KIND::MOBILE:
			Update_Captured(fTimeDelta);
			break;

		case DEFORM_OBJECT_KIND::FIXED:
			Update_CapturedFixed(fTimeDelta);
			break;
		}

		if (DEFORM_OBJECT_STATE::CAPTURED == m_eState)
			__super::Update(fTimeDelta);

		break;
	}
	case DEFORM_OBJECT_STATE::ACQUIRED:
		break;

	case DEFORM_OBJECT_STATE::FALLING:
	{
		Update_Falling(fTimeDelta);

		if (DEFORM_OBJECT_STATE::FALLING == m_eState)
			__super::Update(fTimeDelta);

		break;
	}
	case DEFORM_OBJECT_STATE::LANDING:
	{
		__super::Update(fTimeDelta);

		if (m_bAnimationActive)
			break;

		m_bAvailable = true;
		Set_TriggerEnabled(true);
		m_pTrigger->Update(XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));
		Change_State(DEFORM_OBJECT_STATE::IDLE);
		break;
	}
	}
}

void CLD_DeformObject::Late_Update(_float fTimeDelta)
{
	if (m_bAvailable && m_pTrigger->Is_Enabled())
	{
		m_pTrigger->Update(XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));

#ifdef _DEBUG
		m_pGameInstance_Proxy->Add_DebugComponent(m_pTrigger);
#endif
	}

	__super::Late_Update(fTimeDelta);
}

void CLD_DeformObject::Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData)
{
	if (nullptr == pOutData)
		return;

	pOutData->strPrototypeTag = PROTOTYPE_TAG;
}

void CLD_DeformObject::Register_LevelDesignSpecs()
{
	for (const LD_DEFORMOBJECT_CATALOG& Entry : g_DeformObjectCatalog)
	{
		if (!Is_SupportedCatalog(Entry))
			continue;

		LD_SPAWN_SPEC Spec{};
		Spec.strObjectName = Entry.pObjectName;
		Spec.strPrototypeTag = PROTOTYPE_TAG;
		Spec.strLayerTag = LAYER_TAG;
		Spec.eCategory = LD_CATEGORY::GIMMICK;
		Spec.wstrModelProtoTag = Entry.pModelProtoTag;
		Spec.eModelType = Entry.eModelType;
		Spec.pPrototypeFactory = &Create_Prototype;
		Spec.pBuildDesc = &Build_Desc;

		_float4x4 PreTransformMatrix{};
		XMStoreFloat4x4(&PreTransformMatrix, XMMatrixRotationY(XMConvertToRadians(180.f)));

		Spec.ModelRequirements = { { Entry.pModelProtoTag, Entry.pModelPath, Entry.eModelType, true, PreTransformMatrix }, };

		CLevelDesign_Registry::Register(Spec.strObjectName, Spec);
	}
}

_bool CLD_DeformObject::Build_Desc(const LD_OBJECT_DESC& CommonDesc, const json& jEntry, const LD_SPAWN_SPEC& Spec, LD_OBJECT_ENTRY* pOutEntry)
{
	UNREFERENCED_PARAMETER(jEntry);

	if (nullptr == pOutEntry)
		return false;

	const LD_DEFORMOBJECT_CATALOG* pCatalog = Find_DeformObjectCatalog(CommonDesc.strObjectName);
	if (nullptr == pCatalog || !Is_SupportedCatalog(*pCatalog))
		return false;

	if (Spec.strObjectName != pCatalog->pObjectName
		|| Spec.strPrototypeTag != PROTOTYPE_TAG
		|| Spec.strLayerTag != LAYER_TAG
		|| Spec.eCategory != LD_CATEGORY::GIMMICK
		|| Spec.wstrModelProtoTag != pCatalog->pModelProtoTag
		|| Spec.eModelType != pCatalog->eModelType)
		return false;

	LD_DEFORMOBJECT_DESC Desc{};
	static_cast<LD_OBJECT_DESC&>(Desc) = CommonDesc;
	Desc.eCategory = Spec.eCategory;
	Desc.bUseCollMesh = true;
	Desc.strAnimEventFile.clear();

	Apply_DeformObjectCatalog(*pCatalog, &Desc);

	*pOutEntry = Desc;
	return true;
}

CGameObject* CLD_DeformObject::Create_Prototype(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	return CLD_DeformObject::Create(pDevice, pContext);
}

HRESULT CLD_DeformObject::On_DeformAcquired()
{
	if (!m_bAvailable)
		return S_FALSE;

	m_bAvailable = false;
	Set_TriggerEnabled(false);
	Release_RigidStatic();
	Set_Active(false);

	return S_OK;
}

HRESULT CLD_DeformObject::On_DeformReleased(const _float3& vWorldPosition, const _float3& vTargetPosition)
{
	if (m_bAvailable)
		return S_FALSE;

	m_pTransformCom->Set_State(STATE::POSITION, XMVectorSetW(XMLoadFloat3(&vWorldPosition), 1.f));
	m_vReleaseTargetPosition = vTargetPosition;
	m_fVerticalVelocity = 0.f;

	Set_Active(true);
	Set_TriggerEnabled(false);
	Change_State(DEFORM_OBJECT_STATE::FALLING);

	return S_OK;
}

void CLD_DeformObject::Begin_Released(const _float4x4* AnchorWorld)
{
	if (nullptr == AnchorWorld)
		return;

	m_bAvailable = false;      // On_DeformReleased 가드(!m_bAvailable) 통과
	Set_TriggerEnabled(false);
	Release_RigidStatic();     // Initialize에서 생성된 RigidStatic 제거(착지 시 재생성)

	_matrix Anchor = XMLoadFloat4x4(AnchorWorld);

	_vector vLook = XMVectorSetY(Anchor.r[2], 0.f);
	if (XMVectorGetX(XMVector3LengthSq(vLook)) <= FLT_EPSILON)
		vLook = XMVectorSet(0.f, 0.f, 1.f, 0.f);
	vLook = XMVector3Normalize(vLook);

	const _vector vReleaseStartPosition = XMVectorSetW(Anchor.r[3], 1.f);
	const _vector vReleaseTargetPosition = XMVectorSetW(vReleaseStartPosition + vLook *
		s_fReleaseFwd, 1.f);

	_float3 vStartPosition{};
	_float3 vTargetPosition{};
	XMStoreFloat3(&vStartPosition, vReleaseStartPosition);
	XMStoreFloat3(&vTargetPosition, vReleaseTargetPosition);

	if (S_OK != On_DeformReleased(vStartPosition, vTargetPosition))
		return;

	m_pTransformCom->LookAt(vReleaseStartPosition + vLook);
}

HRESULT CLD_DeformObject::Register_StaticPrototype(CGameInstance_Proxy* pProxy, ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	if (nullptr == pProxy)
		return E_FAIL;

	const _uint iStatic = ETOUI(LEVEL::STATIC);
	if (pProxy->Has_Prototype(iStatic, PROTOTYPE_TAG))
		return S_OK;

	auto* pReg = CGameObject_Factory::GetInstance()->Get_Registration(PROTOTYPE_TAG);
	if (nullptr == pReg)
		return E_FAIL;

	// 팩토리 LOADER 재사용 → 모델(Proto_Component_Model_DeformCar)을 STATIC에 등록
	CGameObject_Factory::GetInstance()->LoadResource(PROTOTYPE_TAG, pProxy, pDevice, pContext,
		iStatic);

	// 객체 프로토타입을 STATIC에 등록
	return pProxy->Add_Prototype(iStatic, PROTOTYPE_TAG, pReg->CreatorFunc(pDevice, pContext));
}

HRESULT CLD_DeformObject::Spawn_Released(CGameInstance_Proxy* pProxy, DEFORM_TYPE eType, _uint iTargetLevel, const _float4x4* AnchorWorld)
{
	if (nullptr == pProxy || nullptr == AnchorWorld)
		return E_FAIL;

	const LD_DEFORMOBJECT_CATALOG* pCatalog = Find_DeformObjectCatalog_ByType(eType);
	if (nullptr == pCatalog)
		return E_FAIL;

	if (nullptr != pCatalog->pSpitDestroyEffectId)
	{
		const _matrix Anchor = XMLoadFloat4x4(AnchorWorld);
		_float3 vEffectPosition{};
		XMStoreFloat3(&vEffectPosition, Anchor.r[3]);

		return CEffect_Loader::GetInstance()->Spawn(
			pCatalog->pSpitDestroyEffectId, iTargetLevel, vEffectPosition);
	}

	LD_DEFORMOBJECT_DESC Desc{};   // Clone(pArg==nullptr) 합성 경로와 동일
	Desc.strObjectName = pCatalog->pObjectName;
	Desc.strKind = pCatalog->pObjectName;
	Desc.eCategory = LD_CATEGORY::GIMMICK;
	Desc.iModelProtoLevel = ETOUI(LEVEL::STATIC);      // STATIC 모델 참조
	Desc.bUseCollMesh = true;
	Desc.strAnimEventFile.clear();

	Apply_DeformObjectCatalog(*pCatalog, &Desc);

	static _uint s_iSpawnCounter = 0;
	const _wstring strObjectTag = _wstring(pCatalog->pObjectName) + L"_Spit_" + std::to_wstring(s_iSpawnCounter++);

	CGameObject* pObject = nullptr;
	if (FAILED(pProxy->Add_GameObject_Return(&pObject, ETOUI(LEVEL::STATIC), PROTOTYPE_TAG,
		iTargetLevel, LAYER_TAG, strObjectTag, &Desc)) || nullptr == pObject)
		return E_FAIL;

	static_cast<CLD_DeformObject*>(pObject)->Begin_Released(AnchorWorld);
	return S_OK;
}

#pragma region Deformable
_bool CLD_DeformObject::Request_Deform(const _float4x4* AnchorWorld)
{
	if (nullptr == AnchorWorld || !m_bAvailable || DEFORM_OBJECT_STATE::IDLE != m_eState)
		return false;

	m_PreDeformWorld = *m_pTransformCom->Get_WorldMatrixPtr();
	m_AnchorWorld = *AnchorWorld;
	m_fCapturedTime = 0.f;

	Change_State(DEFORM_OBJECT_STATE::CAPTURED);
	m_bAlignDone = false;

	Set_TriggerEnabled(false);
	Release_RigidStatic();

	if (DEFORM_OBJECT_KIND::MOBILE == m_eKind)
	{
		m_fPullSpeed = s_fPullInitSpeed;
		m_pTransformCom->Set_RotationPerSec(s_fAlignRotSpeedDegree);
	}

	return true;
}
#pragma endregion

void CLD_DeformObject::Update_Captured(_float fTimeDelta)
{
	_matrix AnchorWorld = XMLoadFloat4x4(&m_AnchorWorld);
	_vector vAnchorPos = AnchorWorld.r[3];
	_vector vSelf = m_pTransformCom->Get_State(STATE::POSITION);

	if (!m_bAlignDone)
	{
		const _vector vAlignTarget = vSelf - (vAnchorPos - vSelf);
		m_bAlignDone = m_pTransformCom->LookAt_Smooth(vAlignTarget, fTimeDelta);
		return;
	}

	_vector vLook = XMVectorSetY(AnchorWorld.r[2], 0.f);
	if (XMVectorGetX(XMVector3LengthSq(vLook)) <= FLT_EPSILON)
		vLook = XMVectorSet(0.f, 0.f, 1.f, 0.f);
	vLook = XMVector3Normalize(vLook);

	_vector vTarget = vAnchorPos + vLook * s_fPullTargetFwd;
	vTarget = XMVectorSetY(vTarget, XMVectorGetY(vSelf));
	vTarget = XMVectorSetW(vTarget, 1.f);

	_vector vDir = vTarget - vSelf;
	_float fDist = XMVectorGetX(XMVector3Length(vDir));

	if (fDist <= s_fAcquireDistance)
	{
		Complete_DeformAcquired();
		return;
	}

	m_fPullSpeed += s_fPullAccel * fTimeDelta;
	_float fMove = min(m_fPullSpeed * fTimeDelta, fDist);
	m_pTransformCom->Set_State(STATE::POSITION, vSelf + XMVector3Normalize(vDir) * fMove);
}

void CLD_DeformObject::Update_CapturedFixed(_float fTimeDelta)
{
	m_fCapturedTime += fTimeDelta;
	if (m_fCapturedTime < s_fFixedAcquireDuration)
		return;

	Complete_DeformAcquired();
}

void CLD_DeformObject::Update_Falling(_float fTimeDelta)
{
	m_fVerticalVelocity = max(m_fVerticalVelocity + s_fGravity * fTimeDelta, s_fMaxFallSpeed);

	const _float fFallDistance = -m_fVerticalVelocity * fTimeDelta;
	if (fFallDistance <= 0.f)
		return;

	_vector vPosition = m_pTransformCom->Get_State(STATE::POSITION);
	_vector vHorizontalTarget = XMLoadFloat3(&m_vReleaseTargetPosition);
	vHorizontalTarget = XMVectorSetY(vHorizontalTarget, XMVectorGetY(vPosition));
	vHorizontalTarget = XMVectorSetW(vHorizontalTarget, 1.f);

	const _vector vHorizontalDirection = vHorizontalTarget - vPosition;
	const _float fHorizontalDistance = XMVectorGetX(XMVector3Length(vHorizontalDirection));

	if (fHorizontalDistance > FLT_EPSILON)
	{
		const _float fHorizontalMoveDistance = min(s_fReleaseSpeed * fTimeDelta, fHorizontalDistance);
		vPosition += XMVector3Normalize(vHorizontalDirection) * fHorizontalMoveDistance;
		m_pTransformCom->Set_State(STATE::POSITION, vPosition);
	}

	const _float3 vScale = m_pTransformCom->Get_Scaled();
	const _float3 vHalfExtents =
	{
			max(m_LocalCollisionBounds.Extents.x * vScale.x, s_fMinSweepHalfExtent),
			max(m_LocalCollisionBounds.Extents.y * vScale.y, s_fMinSweepHalfExtent),
			max(m_LocalCollisionBounds.Extents.z * vScale.z, s_fMinSweepHalfExtent)
	};

	const _matrix WorldMatrix = XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr());

	_float3 vWorldCenter{};
	XMStoreFloat3(&vWorldCenter, XMVector3TransformCoord(XMLoadFloat3(&m_LocalCollisionBounds.Center), WorldMatrix));

	_float4 vRotation{};
	XMStoreFloat4(&vRotation, XMQuaternionRotationMatrix(m_pTransformCom->Get_RotationMatrix()));
	const _float3 vDown = { 0.f, -1.f, 0.f };

	_float fHitDistance = 0.f;

	const _bool bHit = m_pGameInstance_Proxy->Sweep_Box(
		vWorldCenter,
		vHalfExtents,
		vRotation,
		vDown,
		fFallDistance + s_fGroundProbeDistance,
		nullptr,
		&fHitDistance,
		true,
		false);

	_float fMoveDistance = fFallDistance;
	if (bHit)
		fMoveDistance = max(0.f, fHitDistance);

	vPosition = XMVectorSetY(vPosition, XMVectorGetY(vPosition) - fMoveDistance);
	m_pTransformCom->Set_State(STATE::POSITION, vPosition);

	if (!bHit)
		return;

	if (FAILED(Ready_RigidStatic()))
		return;

	m_fVerticalVelocity = 0.f;
	Change_State(DEFORM_OBJECT_STATE::LANDING);
}

HRESULT CLD_DeformObject::Ready_Components()
{
	if (FAILED(__super::Ready_Components()))
		return E_FAIL;

	return Ready_Trigger();
}

void CLD_DeformObject::On_Deserialized()
{
	__super::On_Deserialized();

	if (!m_bAvailable)
		return;

	if (FAILED(Ready_RigidStatic()))
		return;

	m_pTrigger->Update(XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));
}

void CLD_DeformObject::On_LDEventReceived(const _wstring& strEventTag)
{
	UNREFERENCED_PARAMETER(strEventTag);

	if (!m_bAvailable || Is_Dead() || Is_Active())
		return;

	if (nullptr == m_pTrigger || FAILED(Ready_RigidStatic()))
		return;

	m_pTrigger->Update(XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));
	Set_TriggerEnabled(true);
	Set_Active(true);
}

HRESULT CLD_DeformObject::Ready_Trigger()
{
	_float3 vMin{};
	_float3 vMax{};
	if (!m_pModelCom->Get_CollisionAABB(&vMin, &vMax))
		return E_FAIL;

	if (!GeometryUtils::Is_ValidAABB(vMin, vMax))
		return E_FAIL;

	m_LocalCollisionBounds = GeometryUtils::Make_AABB_FromMinMax(vMin, vMax);

	const _float fBoundsRadius = XMVectorGetX(XMVector3Length(XMLoadFloat3(&m_LocalCollisionBounds.Extents)));
	const _float fInteractionRadius = 0.f < m_tDeformObjectDesc.fInteractionRadius ? m_tDeformObjectDesc.fInteractionRadius : fBoundsRadius;

	if (fInteractionRadius <= 0.f)
		return E_FAIL;

	CCollider::COLLIDER_DESC ColliderDesc{};
	ColliderDesc.pOwner = this;
	ColliderDesc.vCenter = m_LocalCollisionBounds.Center;
	ColliderDesc.fRadius = fInteractionRadius;

	m_pTrigger = Add_Component<CCollider>(Collider_Sphere.iLevelID, Collider_Sphere.szProtoTag,
		TEXT("Com_Trigger"), &ColliderDesc);
	if (nullptr == m_pTrigger)
		return E_FAIL;

	m_pTrigger->Set_OnEnter(
		[this](CCollider* pOther)
		{
			if (pOther->Get_RegisteredGroup() == ETOUI(COLLISION_LAYER::PLAYER_HURT))
				m_bKirbyInTrigger = true;
		});

	m_pTrigger->Set_OnExit(
		[this](CCollider* pOther)
		{
			if (pOther->Get_RegisteredGroup() == ETOUI(COLLISION_LAYER::PLAYER_HURT))
				m_bKirbyInTrigger = false;
		});

	m_pGameInstance_Proxy->Register_Collider(m_pTrigger, ETOUI(COLLISION_LAYER::DEFORM_OBJECT));

	return S_OK;
}

void CLD_DeformObject::Set_TriggerEnabled(_bool bEnabled)
{
	if (!bEnabled)
		m_bKirbyInTrigger = false;

	if (nullptr == m_pTrigger)
		return;

	m_pTrigger->Set_Enabled(bEnabled);
}

void CLD_DeformObject::Change_State(DEFORM_OBJECT_STATE eNextState)
{
	if (m_eState == eNextState)
		return;

	const auto Play_StateAnim = [this](DEFORM_ANIM_SLOT eSlot)
		{
			const _string& strAnimName = m_tDeformObjectDesc.strAnimNames[ETOUI(eSlot)];
			if (strAnimName.empty())
			{
				m_bAnimationActive = false;
				return;
			}

			Play_Anim(strAnimName);
		};

	m_eState = eNextState;

	switch (m_eState)
	{
	case DEFORM_OBJECT_STATE::IDLE:
		Play_StateAnim(DEFORM_ANIM_SLOT::IDLE);
		break;

	case DEFORM_OBJECT_STATE::CAPTURED:
		Play_StateAnim(DEFORM_ANIM_SLOT::CAPTURED);
		break;

	case DEFORM_OBJECT_STATE::FALLING:
		Play_StateAnim(DEFORM_ANIM_SLOT::FALLING);
		break;

	case DEFORM_OBJECT_STATE::LANDING:
		Play_StateAnim(DEFORM_ANIM_SLOT::LANDING);
		break;

	case DEFORM_OBJECT_STATE::ACQUIRED:
		m_bAnimationActive = false;
		break;

	default:
		break;
	}
}

void CLD_DeformObject::Complete_DeformAcquired()
{
	Change_State(DEFORM_OBJECT_STATE::ACQUIRED);
	On_DeformAcquired();

	DEFORM_ACQUIRED_EVENT Payload{};
	Payload.eType = Get_DeformType();
	Payload.pSource = this;
	Payload.PreDeformWorld = m_PreDeformWorld;
	m_pGameInstance_Proxy->Publish(EventTag::Deform_Acquired, &Payload);
}

CLD_DeformObject* CLD_DeformObject::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CLD_DeformObject* pInstance = new CLD_DeformObject(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CLD_DeformObject");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CLD_DeformObject* CLD_DeformObject::Create_ByType(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, DEFORM_TYPE eType)
{
	const LD_DEFORMOBJECT_CATALOG* pCatalog = Find_DeformObjectCatalog_ByType(eType);
	if (nullptr == pCatalog || !Is_SupportedCatalog(*pCatalog))
		return nullptr;

	CLD_DeformObject* pInstance = Create(pDevice, pContext);
	if (nullptr == pInstance)
		return nullptr;

	pInstance->m_tDeformObjectDesc.eDeformType = pCatalog->eDeformType;
	return pInstance;
}

CGameObject* CLD_DeformObject::Clone(void* pArg)
{
	CLD_DeformObject* pInstance = new CLD_DeformObject(*this);

	LD_DEFORMOBJECT_DESC TempDesc{};
	if (nullptr == pArg)
	{
		const LD_DEFORMOBJECT_CATALOG* pCatalog =
			Find_DeformObjectCatalog_ByType(m_tDeformObjectDesc.eDeformType);

		if (nullptr == pCatalog && DEFORM_TYPE::NONE == m_tDeformObjectDesc.eDeformType)
		{
			for (const LD_DEFORMOBJECT_CATALOG& Entry : g_DeformObjectCatalog)
			{
				if (Is_SupportedCatalog(Entry))
				{
					pCatalog = &Entry;
					break;
				}
			}
		}

		if (nullptr == pCatalog)
		{
			MSG_BOX("Failed to Cloned : CLD_DeformObject");
			Safe_Release(pInstance);
			return nullptr;
		}

		TempDesc.strObjectName = pCatalog->pObjectName;
		TempDesc.strKind = pCatalog->pObjectName;
		TempDesc.eCategory = LD_CATEGORY::GIMMICK;
		TempDesc.iModelProtoLevel = m_iPrototypeLevel;
		TempDesc.bUseCollMesh = true;
		TempDesc.strAnimEventFile.clear();

		Apply_DeformObjectCatalog(*pCatalog, &TempDesc);

		pArg = &TempDesc;
	}

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CLD_DeformObject");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CLD_DeformObject::Free()
{
	__super::Free();
}

NS_END
