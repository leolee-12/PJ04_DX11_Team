#include "LD_LensFlare.h"
#include "LensFlare.h"
#include "LevelDesign_Registry.h"

#include "GameInstance.h"
#include "Geometry_Utils.h"

NS_BEGIN(Client)

CLD_LensFlare::CLD_LensFlare(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CLevelDesignObject(pDevice, pContext)
{
}

CLD_LensFlare::CLD_LensFlare(const CLD_LensFlare& Prototype)
	: CLevelDesignObject(Prototype)
{
}

HRESULT CLD_LensFlare::Initialize_Prototype()
{
	return __super::Initialize_Prototype();
}

HRESULT CLD_LensFlare::Initialize(void* pArg)
{
	if (nullptr == pArg)
		return E_FAIL;

	const LD_PARSED_OBJECT* pParsedDesc = static_cast<const LD_PARSED_OBJECT*>(pArg);

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	m_tEffectAreaDesc = pParsedDesc->EffectArea;
	m_LensFlareHandle.Clear();
	m_iActivatorCount = 0u;

	if (FAILED(Ready_Components(*pParsedDesc)))
		return E_FAIL;

	SetUp_Collider_Callback();

	if (FAILED(Validate_Initialized()))
		return E_FAIL;

	return S_OK;
}

HRESULT CLD_LensFlare::Validate_Initialized()
{
	if (FAILED(__super::Validate_Initialized()))
		return E_FAIL;

	if (m_tLevelDesignDesc.eCategory != LD_CATEGORY::GUIDE_AREA)
		return E_FAIL;

	if (m_tLevelDesignDesc.strObjectName != OBJECT_NAME)
		return E_FAIL;

	if (nullptr == m_pTrigger)
		return E_FAIL;

	if (!GeometryUtils::Has_UsableSize(m_tEffectAreaDesc.vAreaSize))
		return E_FAIL;

	if (!m_tEffectAreaDesc.bHasEffectPos)
		return E_FAIL;

	return S_OK;
}

void CLD_LensFlare::Late_Update(_float fTimeDelta)
{
	UNREFERENCED_PARAMETER(fTimeDelta);

	Update_RuntimeMatrices();
	Update_Trigger();

#ifdef _DEBUG
	m_pGameInstance_Proxy->Add_DebugComponent(m_pTrigger);
#endif
}

void CLD_LensFlare::Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData)
{
	if (nullptr == pOutData)
		return;

	pOutData->strPrototypeTag = PROTOTYPE_TAG;
}

#pragma region Editable
HRESULT CLD_LensFlare::On_EditTransformChanged()
{
	if (FAILED(__super::On_EditTransformChanged()))
		return E_FAIL;

	Update_RuntimeMatrices();
	Update_Trigger();

	return S_OK;
}

void CLD_LensFlare::Set_EditorPreviewActive(_bool bActive)
{
	if (m_bEditorPreviewActive == bActive)
		return;

	const _bool bWasRequested = Is_LensFlareRequested();

	m_bEditorPreviewActive = bActive;

	const _bool bIsRequested = Is_LensFlareRequested();
	if (bWasRequested == bIsRequested)
		return;

	if (bIsRequested)
		Start_LensFlare();
	else
		Stop_LensFlare();
}

Engine::CEffect_Container* CLD_LensFlare::Get_EditorPreviewEffect() const
{
	if (nullptr == m_pGameInstance_Proxy || !m_pGameInstance_Proxy->IsConnected())
		return nullptr;

	CEffect_Loader* pEffectLoader = CEffect_Loader::GetInstance();

	return pEffectLoader->Is_Current(m_LensFlareHandle)
		? m_LensFlareHandle.p
		: nullptr;
}
#pragma endregion

HRESULT CLD_LensFlare::Ready_Components(const LD_PARSED_OBJECT& Desc)
{
	if (!Build_LocalMatrices(Desc))
		return E_FAIL;

	Update_RuntimeMatrices();

	if (FAILED(Ready_Trigger()))
		return E_FAIL;

	return S_OK;
}

HRESULT CLD_LensFlare::Ready_Trigger()
{
	m_pTrigger = Add_Component<CCollider>(L"Com_Trigger", CCollider::Create(m_pDevice, m_pContext, COLLIDER::OBB));

	if (nullptr == m_pTrigger)
		return E_FAIL;

	CCollider::COLLIDER_DESC ColliderDesc{};
	ColliderDesc.pOwner = this;
	ColliderDesc.vCenter = { 0.f, 0.f, 0.f };
	ColliderDesc.vSize = { 1.f, 1.f, 1.f };

	if (FAILED(m_pTrigger->Initialize(&ColliderDesc)))
		return E_FAIL;

	Update_Trigger();
	m_pGameInstance_Proxy->Register_Collider(m_pTrigger, ETOUI(COLLISION_LAYER::ENV_TRIGGER));

	return S_OK;
}

_bool CLD_LensFlare::Build_LocalMatrices(const LD_PARSED_OBJECT& Desc)
{
	if (!m_tEffectAreaDesc.bHasEffectPos)
		return false;

	const _float3 vAreaSize = GeometryUtils::Make_AbsSize(m_tEffectAreaDesc.vAreaSize);
	if (!GeometryUtils::Has_UsableSize(vAreaSize))
		return false;

	const _float3& vSourceScale = Desc.vParsedScale;
	const _float4& qSourceRotation = Desc.qParsedRotation;
	const _float3& vSourcePosition = Desc.vParsedPosition;

	const _matrix SourceWorldMatrix =
		XMMatrixScaling(vSourceScale.x, vSourceScale.y, vSourceScale.z)
		* XMMatrixRotationQuaternion(XMLoadFloat4(&qSourceRotation))
		* XMMatrixTranslation(vSourcePosition.x, vSourcePosition.y, vSourcePosition.z);

	const _vector vSourceDeterminant = XMMatrixDeterminant(SourceWorldMatrix);
	if (fabsf(XMVectorGetX(vSourceDeterminant)) <= Helper::fEpsilon)
		return false;

	const _matrix InvSourceWorldMatrix = XMMatrixInverse(nullptr, SourceWorldMatrix);

	_vector qAreaRotation = XMLoadFloat4(&m_tEffectAreaDesc.qAreaRot);
	if (XMVectorGetX(XMVector4LengthSq(qAreaRotation)) <= Helper::fEpsilon)
		return false;

	qAreaRotation = XMQuaternionNormalize(qAreaRotation);

	_vector qEffectRotation = XMQuaternionIdentity();
	if (m_tEffectAreaDesc.bHasEffectRot)
	{
		qEffectRotation = XMLoadFloat4(&m_tEffectAreaDesc.qEffectRot);
		if (XMVectorGetX(XMVector4LengthSq(qEffectRotation)) <= Helper::fEpsilon)
			return false;

		qEffectRotation = XMQuaternionNormalize(qEffectRotation);
	}

	const _float3& vAreaCenter = m_tEffectAreaDesc.vAreaCenter;
	const _float3& vEffectPosition = m_tEffectAreaDesc.vEffectPos;

	const _matrix AreaWorldMatrix =
		XMMatrixScaling(vAreaSize.x, vAreaSize.y, vAreaSize.z)
		* XMMatrixRotationQuaternion(qAreaRotation)
		* XMMatrixTranslation(vAreaCenter.x, vAreaCenter.y, vAreaCenter.z);

	const _matrix EffectAnchorWorldMatrix =
		XMMatrixRotationQuaternion(qEffectRotation)
		* XMMatrixTranslation(vEffectPosition.x, vEffectPosition.y, vEffectPosition.z);

	XMStoreFloat4x4(&m_matAreaLocal, AreaWorldMatrix * InvSourceWorldMatrix);
	XMStoreFloat4x4(&m_matEffectAnchorLocal, EffectAnchorWorldMatrix * InvSourceWorldMatrix);

	return true;
}

void CLD_LensFlare::Update_RuntimeMatrices()
{
	const _matrix CurrentWorldMatrix = XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr());
	XMStoreFloat4x4(&m_matEffectAnchorWorld, XMLoadFloat4x4(&m_matEffectAnchorLocal) * CurrentWorldMatrix);
}

void CLD_LensFlare::Update_Trigger()
{
	const _matrix CurrentWorldMatrix = XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr());
	m_pTrigger->Update(XMLoadFloat4x4(&m_matAreaLocal) * CurrentWorldMatrix);
}

void CLD_LensFlare::SetUp_Collider_Callback()
{
	if (nullptr == m_pTrigger)
		return;

	m_pTrigger->Set_OnEnter([this](CCollider* pOther) { Handle_TriggerEnter(pOther); });
	m_pTrigger->Set_OnExit([this](CCollider* pOther) { Handle_TriggerExit(pOther); });
}

void CLD_LensFlare::Handle_TriggerEnter(CCollider* pOther)
{
	if (!Is_TriggerActivator(pOther))
		return;

	const _bool bWasRequested = Is_LensFlareRequested();

	++m_iActivatorCount;

	if (!bWasRequested)
		Start_LensFlare();
}

void CLD_LensFlare::Handle_TriggerExit(CCollider* pOther)
{
	if (!Is_TriggerActivator(pOther))
		return;

	if (0u == m_iActivatorCount)
		return;

	const _bool bWasRequested = Is_LensFlareRequested();

	--m_iActivatorCount;

	if (bWasRequested && !Is_LensFlareRequested())
		Stop_LensFlare();
}

_bool CLD_LensFlare::Is_TriggerActivator(const CCollider* pOther) const
{
	return nullptr != pOther
		&& ETOUI(COLLISION_LAYER::PLAYER_HURT) == pOther->Get_RegisteredGroup();
}

_bool CLD_LensFlare::Is_LensFlareRequested() const
{
	return 0u < m_iActivatorCount || m_bEditorPreviewActive;
}

void CLD_LensFlare::Start_LensFlare()
{
	CEffect_Loader* pEffectLoader = CEffect_Loader::GetInstance();

	Update_RuntimeMatrices();

	if (pEffectLoader->Is_Current(m_LensFlareHandle))
	{
		CLensFlare* pLensFlare = dynamic_cast<CLensFlare*>(m_LensFlareHandle.p);

		if (nullptr != pLensFlare && pLensFlare->Is_Playing())
		{
			pLensFlare->Reset_LensRuntimeState();
			pLensFlare->EffectContainer_Start(_float3{}, _float3{}, &m_matEffectAnchorWorld);
			return;
		}

		if (nullptr == pLensFlare)
			m_LensFlareHandle.p->EffectContainer_Stop();

		m_LensFlareHandle.Clear();
	}

	if (FAILED(pEffectLoader->Spawn(
		L"LensFlare",
		ETOUI(LEVEL::STATIC),
		_float3{},
		_float3{},
		_float3{},
		&m_matEffectAnchorWorld,
		nullptr,
		&m_LensFlareHandle)))
	{
		m_LensFlareHandle.Clear();
		return;
	}

	CLensFlare* pLensFlare = dynamic_cast<CLensFlare*>(m_LensFlareHandle.p);

	if (nullptr != pLensFlare)
		pLensFlare->Reset_LensRuntimeState();
}

void CLD_LensFlare::Stop_LensFlare()
{
	CEffect_Loader* pEffectLoader = CEffect_Loader::GetInstance();

	if (!pEffectLoader->Is_Current(m_LensFlareHandle))
	{
		m_LensFlareHandle.Clear();
		return;
	}

	m_LensFlareHandle.p->Start_FadeOut(1.5f);
}

void CLD_LensFlare::Release_LensFlare()
{
	if (nullptr == m_pGameInstance_Proxy || !m_pGameInstance_Proxy->IsConnected())
	{
		m_LensFlareHandle.Clear();
		return;
	}

	CEffect_Loader* pEffectLoader = CEffect_Loader::GetInstance();

	if (pEffectLoader->Is_Current(m_LensFlareHandle))
		m_LensFlareHandle.p->EffectContainer_Stop();

	m_LensFlareHandle.Clear();
}

void CLD_LensFlare::Register_LevelDesignSpecs()
{
	LD_SPAWN_SPEC Spec{};
	Spec.strObjectName = OBJECT_NAME;
	Spec.strPrototypeTag = PROTOTYPE_TAG;
	Spec.strLayerTag = LAYER_TAG;
	Spec.eCategory = LD_CATEGORY::GUIDE_AREA;
	Spec.pPrototypeFactory = &Create_Prototype;
	Spec.pBuildDesc = nullptr;

	CLevelDesign_Registry::Register(Spec.strObjectName, Spec);
}

CGameObject* CLD_LensFlare::Create_Prototype(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	return CLD_LensFlare::Create(pDevice, pContext);
}

CLD_LensFlare* CLD_LensFlare::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CLD_LensFlare* pInstance = new CLD_LensFlare(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CLD_LensFlare");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CLD_LensFlare::Clone(void* pArg)
{
	CLD_LensFlare* pInstance = new CLD_LensFlare(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CLD_LensFlare");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CLD_LensFlare::Free()
{
	Release_LensFlare();

	if (nullptr != m_pTrigger)
		m_pTrigger->Clear_Callbacks();

	__super::Free();
}

NS_END