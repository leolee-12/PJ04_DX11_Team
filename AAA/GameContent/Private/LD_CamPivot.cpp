#include "LD_CamPivot.h"
#include "LevelDesign_Registry.h"

#include "GameInstance.h"

NS_BEGIN(Client)

CLD_CamPivot::CLD_CamPivot(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CLevelDesignObject(pDevice, pContext)
{
	XMStoreFloat4x4(&m_matPivotWorld, XMMatrixIdentity());
}

CLD_CamPivot::CLD_CamPivot(const CLD_CamPivot& Prototype)
	: CLevelDesignObject(Prototype)
	, m_matPivotWorld(Prototype.m_matPivotWorld)
{
}

HRESULT CLD_CamPivot::Initialize_Prototype()
{
	return __super::Initialize_Prototype();
}

HRESULT CLD_CamPivot::Initialize(void* pArg)
{
	if (nullptr == pArg)
		return E_FAIL;

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	Sync_PivotWorldMatrix();

#ifdef _DEBUG
	if (FAILED(Ready_DebugCollider()))
		return E_FAIL;
#endif

	return Validate_Initialized();
}

HRESULT CLD_CamPivot::Validate_Initialized()
{
	if (FAILED(__super::Validate_Initialized()))
		return E_FAIL;

	if (m_tLevelDesignDesc.strObjectName != OBJECT_NAME)
		return E_FAIL;

	if (LD_CATEGORY::META != m_tLevelDesignDesc.eCategory)
		return E_FAIL;

#ifdef _DEBUG
	if (nullptr == m_pDebugCollider)
		return E_FAIL;
#endif

	return S_OK;
}

void CLD_CamPivot::Priority_Update(_float fTimeDelta)
{
	if (!m_bShotCam)
	{
		CUTSCENE_CAMERA_DESC cam{};
		cam.eCam = ECutsceneCam::Cutscene;
		cam.szTrack = L"Ending_Cut1_camera1";
		cam.pAnchorWorld = Get_PivotWorldMatrixPtr();
		m_pGameInstance_Proxy->Publish(EventTag::Cutscene_CameraChange, &cam);
		m_bShotCam = true;
	}
}

void CLD_CamPivot::Late_Update(_float fTimeDelta)
{
	UNREFERENCED_PARAMETER(fTimeDelta);

	if (!m_bActive || Is_Dead())
		return;

#ifdef _DEBUG
	m_pDebugCollider->Update(XMLoadFloat4x4(&m_matPivotWorld));
	m_pGameInstance_Proxy->Add_DebugComponent(m_pDebugCollider);
#endif
}

void CLD_CamPivot::Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData)
{
	if (nullptr == pOutData)
		return;

	pOutData->strPrototypeTag = PROTOTYPE_TAG;
}

HRESULT CLD_CamPivot::On_EditTransformChanged()
{
	if (FAILED(__super::On_EditTransformChanged()))
		return E_FAIL;

	Sync_PivotWorldMatrix();
	return S_OK;
}

void CLD_CamPivot::Sync_PivotWorldMatrix()
{
	XMStoreFloat4x4(&m_matPivotWorld, XMMatrixRotationY(XMConvertToRadians(180.f)) * XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));
}

#ifdef _DEBUG
HRESULT CLD_CamPivot::Ready_DebugCollider()
{
	m_pDebugCollider = Add_Component<CCollider>(
		L"Com_DebugCollider",
		CCollider::Create(m_pDevice, m_pContext, COLLIDER::OBB));

	if (nullptr == m_pDebugCollider)
		return E_FAIL;

	CCollider::COLLIDER_DESC ColliderDesc{};
	ColliderDesc.pOwner = this;
	ColliderDesc.vCenter = { 0.f, 0.f, 0.f };
	ColliderDesc.vSize = { 0.5f, 0.5f, 0.5f };

	if (FAILED(m_pDebugCollider->Initialize(&ColliderDesc)))
		return E_FAIL;

	m_pDebugCollider->Set_DebugRenderColor({ 1.f, 0.5f, 0.f, 1.f });
	return S_OK;
}
#endif

void CLD_CamPivot::Register_LevelDesignSpecs()
{
	LD_SPAWN_SPEC Spec{};
	Spec.strObjectName = OBJECT_NAME;
	Spec.strPrototypeTag = PROTOTYPE_TAG;
	Spec.strLayerTag = LAYER_TAG;
	Spec.eCategory = LD_CATEGORY::META;
	Spec.pPrototypeFactory = &Create_Prototype;
	Spec.pBuildDesc = nullptr;

	CLevelDesign_Registry::Register(Spec.strObjectName, Spec);
}

CGameObject* CLD_CamPivot::Create_Prototype(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	return CLD_CamPivot::Create(pDevice, pContext);
}

CLD_CamPivot* CLD_CamPivot::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CLD_CamPivot* pInstance = new CLD_CamPivot(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CLD_CamPivot");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CLD_CamPivot::Clone(void* pArg)
{
	CLD_CamPivot* pInstance = new CLD_CamPivot(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CLD_CamPivot");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CLD_CamPivot::Free()
{
	__super::Free();
}

NS_END