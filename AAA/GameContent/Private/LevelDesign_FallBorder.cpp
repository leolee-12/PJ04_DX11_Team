#include "LevelDesign_FallBorder.h"
#include "LevelDesign_Registry.h"

#include "GameInstance.h"
#include "PhysX_Manager.h"

namespace
{
	constexpr _float kMinFallBorderExtent = 0.001f;

	_float AbsAxis(_float fValue)
	{
		return fValue < 0.f ? -fValue : fValue;
	}

	_bool Has_UsableBoxSize(const _float3& vSize)
	{
		return AbsAxis(vSize.x) > kMinFallBorderExtent
			&& AbsAxis(vSize.y) > kMinFallBorderExtent
			&& AbsAxis(vSize.z) > kMinFallBorderExtent;
	}

	_float3 Make_ColliderSize(const _float3& vSize)
	{
		return { AbsAxis(vSize.x), AbsAxis(vSize.y), AbsAxis(vSize.z) };
	}
}

NS_BEGIN(Client)

CLevelDesign_FallBorder::CLevelDesign_FallBorder(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CLevelDesignObject(pDevice, pContext)
{
}

CLevelDesign_FallBorder::CLevelDesign_FallBorder(const CLevelDesign_FallBorder& Prototype)
	: CLevelDesignObject(Prototype)
{
}

HRESULT CLevelDesign_FallBorder::Initialize_Prototype()
{
	return __super::Initialize_Prototype();
}

HRESULT CLevelDesign_FallBorder::Initialize(void* pArg)
{
	if (nullptr == pArg)
		return E_FAIL;

	const LD_PARSED_OBJECT* pParsedDesc = static_cast<const LD_PARSED_OBJECT*>(pArg);

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_Components(*pParsedDesc)))
		return E_FAIL;

	if (FAILED(Validate_Initialized()))
		return E_FAIL;

	return S_OK;
}

HRESULT CLevelDesign_FallBorder::Validate_Initialized()
{
	if (FAILED(__super::Validate_Initialized()))
		return E_FAIL;

	if (m_tLevelDesignDesc.eCategory != LD_CATEGORY::VOLUME)
		return E_FAIL;

	const _wstring& strObjectName = m_tLevelDesignDesc.strObjectName;
	if (strObjectName != L"FallBorder")
		return E_FAIL;

	if (nullptr == m_pColliderCom)
		return E_FAIL;

	return S_OK;
}

void CLevelDesign_FallBorder::Late_Update(_float fTimeDelta)
{
	UNREFERENCED_PARAMETER(fTimeDelta);

	m_pColliderCom->Update(XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));

#ifdef _DEBUG
	m_pGameInstance_Proxy->Add_DebugComponent(m_pColliderCom);
#endif
}

void CLevelDesign_FallBorder::Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData)
{
	if (nullptr == pOutData)
		return;

	pOutData->strPrototypeTag = PROTOTYPE_TAG;
}

HRESULT CLevelDesign_FallBorder::Ready_Components(const LD_PARSED_OBJECT& Desc)
{
	if (!Has_UsableBoxSize(Desc.Volume.vAreaSize))
		return E_FAIL;

	m_pColliderCom = Add_Component<CCollider>(L"Com_Collider", CCollider::Create(m_pDevice, m_pContext, COLLIDER::OBB));

	if (nullptr == m_pColliderCom)
		return E_FAIL;

	CCollider::COLLIDER_DESC ColliderDesc{};
	ColliderDesc.pOwner = this;
	ColliderDesc.vCenter = { 0.f, 0.f, 0.f };
	ColliderDesc.vSize = Make_ColliderSize(Desc.Volume.vAreaSize);

	if (FAILED(m_pColliderCom->Initialize(&ColliderDesc)))
		return E_FAIL;

	m_pGameInstance_Proxy->Register_Collider(m_pColliderCom, ETOUI(COLLISION_LAYER::ENV_TRIGGER));

	SetUp_Collider_Callback();

	return S_OK;
}

void CLevelDesign_FallBorder::SetUp_Collider_Callback()
{
	if (nullptr == m_pColliderCom)
		return;

	m_pColliderCom->Set_OnEnter([this](CCollider* pOther)
		{
			if (nullptr == pOther)
				return;

			if (ETOUI(COLLISION_LAYER::PLAYER_HURT) != pOther->Get_RegisteredGroup())
				return;

			Handle_Fall(pOther->Get_Owner());
		});
}

void CLevelDesign_FallBorder::Handle_Fall(CGameObject* pPlayer)
{
	if (nullptr == pPlayer)
		return;

#ifdef _DEBUG
	const _wstring strMessage = L"[LevelDesign_FallBorder] player entered: " + pPlayer->Get_ObjectTag() + L" / border=" +
		Make_LevelDesignObjectKey() + L"\n";
	OutputDebugStringW(strMessage.c_str());
#endif
}

void CLevelDesign_FallBorder::Register_LevelDesignSpecs()
{
	const _wstring ObjectNames[] =
	{
			L"FallBorder",
	};

	for (const _wstring& strObjectName : ObjectNames)
	{
		LD_SPAWN_SPEC Spec{};
		Spec.strObjectName = strObjectName;
		Spec.strPrototypeTag = PROTOTYPE_TAG;
		Spec.strLayerTag = LAYER_TAG;
		Spec.eCategory = LD_CATEGORY::VOLUME;
		Spec.pPrototypeFactory = &Create_Prototype;
		Spec.pBuildDesc = nullptr;

		CLevelDesign_Registry::Register(Spec.strObjectName, Spec);
	}
}

CGameObject* CLevelDesign_FallBorder::Create_Prototype(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	return CLevelDesign_FallBorder::Create(pDevice, pContext);
}

CLevelDesign_FallBorder* CLevelDesign_FallBorder::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CLevelDesign_FallBorder* pInstance = new CLevelDesign_FallBorder(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CLevelDesign_FallBorder");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CLevelDesign_FallBorder::Clone(void* pArg)
{
	CLevelDesign_FallBorder* pInstance = new CLevelDesign_FallBorder(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CLevelDesign_FallBorder");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CLevelDesign_FallBorder::Free()
{
	__super::Free();
}

NS_END