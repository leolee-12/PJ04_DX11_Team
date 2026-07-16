#include "LevelDesign_Boundary.h"
#include "LevelDesign_Registry.h"

#include "GameInstance.h"
#include "PhysX_Manager.h"

namespace
{
	constexpr _float kMinBoundaryExtent = 0.001f;

	_float AbsAxis(_float fValue)
	{
		return fValue < 0.f ? -fValue : fValue;
	}

	_bool Has_UsableTrianglePoints(const LD_PARSED_OBJECT& Desc)
	{
		return Desc.Volume.Points.size() >= 3
			&& 0u == (static_cast<_uint>(Desc.Volume.Points.size()) % 3u);
	}

	_bool Has_UsableBoxSize(const _float3& vSize)
	{
		return AbsAxis(vSize.x) > kMinBoundaryExtent
			&& AbsAxis(vSize.y) > kMinBoundaryExtent
			&& AbsAxis(vSize.z) > kMinBoundaryExtent;
	}

	_float3 Make_HalfExtents(const _float3& vSize)
	{
		return {
				AbsAxis(vSize.x) * 0.5f,
				AbsAxis(vSize.y) * 0.5f,
				AbsAxis(vSize.z) * 0.5f
		};
	}
}

NS_BEGIN(Client)

CLevelDesign_Boundary::CLevelDesign_Boundary(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CLevelDesignObject(pDevice, pContext)
{
}

CLevelDesign_Boundary::CLevelDesign_Boundary(const CLevelDesign_Boundary& Prototype)
	: CLevelDesignObject(Prototype)
{
}

HRESULT CLevelDesign_Boundary::Initialize_Prototype()
{
	return __super::Initialize_Prototype();
}

HRESULT CLevelDesign_Boundary::Initialize(void* pArg)
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

HRESULT CLevelDesign_Boundary::Validate_Initialized()
{
	if (FAILED(__super::Validate_Initialized()))
		return E_FAIL;

	if (m_tLevelDesignDesc.eCategory != LD_CATEGORY::VOLUME)
		return E_FAIL;

	const _wstring& strObjectName = m_tLevelDesignDesc.strObjectName;
	if (strObjectName != L"InvisibleCollision"
		&& strObjectName != L"InvisibleCollisionBox")
	{
		return E_FAIL;
	}

	if (nullptr == m_pRigidStatic)
		return E_FAIL;

	return S_OK;
}

void CLevelDesign_Boundary::Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData)
{
	if (nullptr == pOutData)
		return;

	pOutData->strPrototypeTag = PROTOTYPE_TAG;
}

HRESULT CLevelDesign_Boundary::On_EditTransformChanged()
{
	if (FAILED(__super::On_EditTransformChanged()))
		return E_FAIL;

	if (!Is_Active() || nullptr == m_pRigidStatic)
		return S_OK;

	if (nullptr == m_pGameInstance_Proxy || nullptr == m_pTransformCom)
		return E_FAIL;

	const _matrix WorldMatrix = XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr());

	physx::PxRigidStatic* pNewRigidStatic = nullptr;

	if (nullptr != m_pCollisionMesh)
	{
		pNewRigidStatic = m_pGameInstance_Proxy->Create_StaticActor(m_pCollisionMesh, WorldMatrix);
	}
	else
	{
		const _float3 vLocalCenter = { 0.f, 0.f, 0.f };
		const _float3 vLocalHalfExtents = { 0.5f, 0.5f, 0.5f };

		pNewRigidStatic = m_pGameInstance_Proxy->Create_StaticBox(
			vLocalCenter,
			vLocalHalfExtents,
			WorldMatrix);
	}

	if (nullptr == pNewRigidStatic)
		return E_FAIL;

	m_pGameInstance_Proxy->Remove_StaticActor(m_pRigidStatic);
	m_pRigidStatic = pNewRigidStatic;

	return S_OK;
}

void CLevelDesign_Boundary::On_LDEventReceived(const _wstring& strEventTag)
{
	UNREFERENCED_PARAMETER(strEventTag);

	if (!Is_Active())
		return;

	Release_RigidStatic();
	Set_Active(false);
}

HRESULT CLevelDesign_Boundary::Ready_Components(const LD_PARSED_OBJECT& Desc)
{
	if (FAILED(Ready_RigidStatic(Desc)))
		return E_FAIL;

	return S_OK;
}

HRESULT CLevelDesign_Boundary::Ready_RigidStatic(const LD_PARSED_OBJECT& Desc)
{
	Release_RigidStatic();

	if (Has_UsableTrianglePoints(Desc))
		return Ready_RigidStatic_FromPoints(Desc);

	if (Has_UsableBoxSize(Desc.Volume.vAreaSize))
		return Ready_RigidStatic_FromBox();

	return E_FAIL;
}

HRESULT CLevelDesign_Boundary::Ready_RigidStatic_FromPoints(const LD_PARSED_OBJECT& Desc)
{
	if (nullptr == m_pGameInstance_Proxy || nullptr == m_pTransformCom)
		return E_FAIL;

	const _float3& vSourceScale = Desc.vParsedScale;
	const _float4& qSourceRotation = Desc.qParsedRotation;
	const _float3& vSourcePosition = Desc.vParsedPosition;

	const _matrix SourceWorldMatrix =
		XMMatrixScaling(vSourceScale.x, vSourceScale.y, vSourceScale.z)
		* XMMatrixRotationQuaternion(XMLoadFloat4(&qSourceRotation))
		* XMMatrixTranslation(vSourcePosition.x, vSourcePosition.y, vSourcePosition.z);
	const _matrix InvSourceWorldMatrix = XMMatrixInverse(nullptr, SourceWorldMatrix);

	vector<_float3> LocalPositions;
	LocalPositions.reserve(Desc.Volume.Points.size());

	for (const _float3& vPointWorld : Desc.Volume.Points)
	{
		_float3 vPointLocal{};
		XMStoreFloat3(
			&vPointLocal,
			XMVector3TransformCoord(XMLoadFloat3(&vPointWorld), InvSourceWorldMatrix));
		LocalPositions.push_back(vPointLocal);
	}

	vector<_uint> Indices;
	Indices.reserve(LocalPositions.size());

	for (_uint i = 0; i < static_cast<_uint>(LocalPositions.size()); ++i)
		Indices.push_back(i);

	m_pCollisionMesh = m_pGameInstance_Proxy->Cook_TriangleMesh(
		LocalPositions.data(),
		static_cast<_uint>(LocalPositions.size()),
		Indices.data(),
		static_cast<_uint>(Indices.size()),
		true);

	if (nullptr == m_pCollisionMesh)
		return E_FAIL;

	const _matrix CurrentWorldMatrix = XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr());

	m_pRigidStatic = m_pGameInstance_Proxy->Create_StaticActor(m_pCollisionMesh, CurrentWorldMatrix);
	if (nullptr == m_pRigidStatic)
	{
		m_pCollisionMesh->release();
		m_pCollisionMesh = nullptr;
		return E_FAIL;
	}

	return S_OK;
}

HRESULT CLevelDesign_Boundary::Ready_RigidStatic_FromBox()
{
	if (nullptr == m_pGameInstance_Proxy || nullptr == m_pTransformCom)
		return E_FAIL;

	const _float3 vLocalCenter = { 0.f, 0.f, 0.f };
	const _float3 vLocalHalfExtents = { 0.5f, 0.5f, 0.5f };
	const _matrix WorldMatrix = XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr());

	m_pRigidStatic = m_pGameInstance_Proxy->Create_StaticBox(
		vLocalCenter,
		vLocalHalfExtents,
		WorldMatrix);

	return (nullptr != m_pRigidStatic) ? S_OK : E_FAIL;
}

void CLevelDesign_Boundary::Release_RigidStatic()
{
	if (nullptr != m_pRigidStatic && nullptr != m_pGameInstance_Proxy)
		m_pGameInstance_Proxy->Remove_StaticActor(m_pRigidStatic);

	m_pRigidStatic = nullptr;

	if (nullptr != m_pCollisionMesh)
	{
		m_pCollisionMesh->release();
		m_pCollisionMesh = nullptr;
	}
}

void CLevelDesign_Boundary::Register_LevelDesignSpecs()
{
	const _wstring ObjectNames[] =
	{
			L"InvisibleCollision",
			L"InvisibleCollisionBox"
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

CGameObject* CLevelDesign_Boundary::Create_Prototype(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	return CLevelDesign_Boundary::Create(pDevice, pContext);
}

CLevelDesign_Boundary* CLevelDesign_Boundary::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CLevelDesign_Boundary* pInstance = new CLevelDesign_Boundary(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CLevelDesign_Boundary");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CLevelDesign_Boundary::Clone(void* pArg)
{
	CLevelDesign_Boundary* pInstance = new CLevelDesign_Boundary(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CLevelDesign_Boundary");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CLevelDesign_Boundary::Free()
{
	Release_RigidStatic();

	__super::Free();
}

NS_END
