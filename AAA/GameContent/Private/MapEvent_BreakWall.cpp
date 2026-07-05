#include "MapEvent_BreakWall.h"
#include "GameContrnt_Events.h"
#include "MeshLayer_Binder.h"

#include "GameInstance.h"

namespace
{
	struct BREAK_FRAGMENT_BIND
	{
		const _char* pFragmentName = nullptr;
		_float3 vPivot = {};
	};

	static const BREAK_FRAGMENT_BIND g_GsDefault4BreakBinds[] =
	{
		  { "GsDefault_10", { 147.14450f, 27.23368f, 1306.22900f } },
		  { "GsDefault_11", { 147.42530f, 28.35020f, 1310.04500f } },
		  { "GsDefault_12", { 148.33050f, 26.39541f, 1307.77400f } },
		  { "GsDefault_13", { 148.99160f, 25.69413f, 1301.43300f } },
		  { "GsDefault_14", { 142.27910f, 26.67553f, 1302.79100f } },
		  { "GsDefault_15", { 143.78630f, 25.83631f, 1303.03300f } },
		  { "GsDefault_16", { 148.54220f, 26.77026f, 1302.02500f } },
		  { "GsDefault_17", { 149.38960f, 24.72021f, 1302.92300f } },
		  { "GsDefault_18", { 148.40590f, 28.09056f, 1303.71700f } },
		  { "GsDefault_19", { 143.31180f, 27.06821f, 1306.50100f } },
		  { "GsDefault_20", { 142.75370f, 28.40662f, 1303.28600f } },
		  { "GsDefault_21", { 144.59510f, 25.78181f, 1307.17300f } },
		  { "GsDefault_22", { 147.40860f, 26.11647f, 1308.86000f } },
		  { "GsDefault_23", { 140.99720f, 25.34992f, 1305.69600f } },
		  { "GsDefault_24", { 140.85890f, 27.90112f, 1310.78600f } },
		  { "GsDefault_25", { 141.34510f, 25.55193f, 1310.50000f } },
		  { "GsDefault_26", { 145.46370f, 26.66180f, 1310.14300f } },
	};

	string Get_FragmentNameFromMeshName(const string& strMeshName)
	{
		const size_t iSep = strMeshName.find("__");
		if (string::npos == iSep)
			return strMeshName;

		return strMeshName.substr(0, iSep);
	}
}

NS_BEGIN(Client)

CMapEvent_BreakWall::CMapEvent_BreakWall(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CMapObject{ pDevice, pContext }
{
}

CMapEvent_BreakWall::CMapEvent_BreakWall(const CMapEvent_BreakWall& Prototype)
	: CMapObject(Prototype)
{
}

HRESULT CMapEvent_BreakWall::Initialize_Prototype()
{
	return __super::Initialize_Prototype();
}

HRESULT CMapEvent_BreakWall::Initialize(void* pArg)
{
	if (nullptr == pArg)
		return E_FAIL;

	const MAP_EVENT_BREAK_WALL_DESC* pDesc = static_cast<const MAP_EVENT_BREAK_WALL_DESC*>(pArg);
	m_iModelProtoLevel = pDesc->iModelProtoLevel;
	m_bRenderable = pDesc->bRenderable;

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_Fragments()))
		return E_FAIL;

	if (FAILED(Ready_BoostTrigger()))
		return E_FAIL;

	if (FAILED(Validate_Initialized()))
		return E_FAIL;

	return S_OK;
}

HRESULT CMapEvent_BreakWall::Validate_Initialized()
{
	if (FAILED(__super::Validate_Initialized()))
		return E_FAIL;

	if (nullptr == m_pBoostTrigger)
		return E_FAIL;
	if (BREAK_STATE::INTACT != m_eBreakState)
		return E_FAIL;
	if (m_Fragments.empty())
		return E_FAIL;

	const _uint iNumMeshes = static_cast<_uint>(m_pModelCom->Get_NumMeshes());
	if (m_MeshFragmentIndices.size() != iNumMeshes)
		return E_FAIL;

	const _uint iNumFragments = static_cast<_uint>(m_Fragments.size());
	for (const _uint iFragment : m_MeshFragmentIndices)
	{
		if (INVALID_FRAGMENT_INDEX == iFragment)
			continue;
		if (iFragment >= iNumFragments)
			return E_FAIL;
	}

	for (const BREAK_FRAGMENT& Fragment : m_Fragments)
	{
		if (Fragment.strFragmentName.empty())
			return E_FAIL;
		if (Fragment.MeshIndices.empty())
			return E_FAIL;
	}

	return S_OK;
}

void CMapEvent_BreakWall::Late_Update(_float fTimeDelta)
{
	if (BREAK_STATE::INTACT == m_eBreakState)
	{
		m_pBoostTrigger->Update(XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));

#ifdef _DEBUG
		m_pGameInstance_Proxy->Add_DebugComponent(m_pBoostTrigger);
#endif
	}

	if (!m_bRenderable)
		return;

	if (BREAK_STATE::INTACT == m_eBreakState)
	{
		m_pGameInstance_Proxy->Add_RenderGroup(RENDERID::NONBLEND, this);
		return;
	}

	if (BREAK_STATE::BREAKING != m_eBreakState)
		return;

	constexpr _float fGravity = -15.f;
	_bool bAnyActive = false;

	for (BREAK_FRAGMENT& Fragment : m_Fragments)
	{
		if (!Fragment.bActive)
			continue;

		Fragment.vVelocity.y += fGravity * fTimeDelta;
		Fragment.vOffset.x += Fragment.vVelocity.x * fTimeDelta;
		Fragment.vOffset.y += Fragment.vVelocity.y * fTimeDelta;
		Fragment.vOffset.z += Fragment.vVelocity.z * fTimeDelta;

		_vector vAngularVelocity = XMLoadFloat3(&Fragment.vAngularVelocity);
		const _float fAngularSpeed = XMVectorGetX(XMVector3Length(vAngularVelocity));

		if (fAngularSpeed > 0.f)
		{
			_vector vAxis = XMVector3Normalize(vAngularVelocity);
			_vector vDeltaRotation = XMQuaternionRotationAxis(vAxis, fAngularSpeed * fTimeDelta);
			_vector vRotation = XMQuaternionNormalize(XMQuaternionMultiply(XMLoadFloat4(&Fragment.vRotation), vDeltaRotation));
			XMStoreFloat4(&Fragment.vRotation, vRotation);
		}

		const _float3 vEnginePivot = { Fragment.vPivot.x, Fragment.vPivot.y, -Fragment.vPivot.z };
		const _vector vWorldPosition = XMVector3TransformCoord(
			XMVectorSet(
				vEnginePivot.x + Fragment.vOffset.x,
				vEnginePivot.y + Fragment.vOffset.y,
				vEnginePivot.z + Fragment.vOffset.z,
				1.f),
			XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));

		const _float fWorldY = XMVectorGetY(vWorldPosition);
		if (fWorldY < 15.f)
		{
			Fragment.bActive = false;
			continue;
		}

		bAnyActive = true;
	}

	if (!bAnyActive)
	{
		m_eBreakState = BREAK_STATE::BROKEN;
		return;
	}

	m_pGameInstance_Proxy->Add_RenderGroup(RENDERID::NONBLEND, this);
}

HRESULT CMapEvent_BreakWall::Render()
{
	if (BREAK_STATE::INTACT != m_eBreakState
		&& BREAK_STATE::BREAKING != m_eBreakState)
	{
		return S_OK;
	}

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::VIEW, m_eProjType))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::PROJ, m_eProjType))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_iMaterialID", &m_iMaterialID, sizeof(_uint))))
		return E_FAIL;

	const _uint iNumMeshes = static_cast<_uint>(m_pModelCom->Get_NumMeshes());
	const _matrix BreakWallWorld = XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr());

	for (_uint i = 0; i < iNumMeshes; ++i)
	{
		const BREAK_FRAGMENT* pFragment = Find_Fragment(i);
		if (nullptr == pFragment)
			continue;
		if (BREAK_STATE::BREAKING == m_eBreakState && !pFragment->bActive)
			continue;

		const _float3 vEnginePivot = { pFragment->vPivot.x, pFragment->vPivot.y, -pFragment->vPivot.z };

		_matrix Rotation = XMMatrixRotationQuaternion(XMLoadFloat4(&pFragment->vRotation));
		_matrix Translation = XMMatrixTranslation(
			vEnginePivot.x + pFragment->vOffset.x,
			vEnginePivot.y + pFragment->vOffset.y,
			vEnginePivot.z + pFragment->vOffset.z);

		_float4x4 WorldMatrix{};
		XMStoreFloat4x4(&WorldMatrix, Rotation * Translation * BreakWallWorld);

		if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", &WorldMatrix)))
			return E_FAIL;

		const MESH_LAYER_IDX& Layer = m_pModelCom->Get_MeshLayer(i);

		MESH_LAYER_BIND_CONTEXT Ctx{};
		Ctx.pShader = m_pShaderCom;
		Ctx.pModel = m_pModelCom;
		Ctx.pGI_Proxy = m_pGameInstance_Proxy;
		Ctx.iMesh = i;
		Ctx.pLayer = &Layer;
		Ctx.eProfile = MESH_LAYER_PROFILE::MAP;
		Ctx.eKind = MESH_LAYER_RENDER_KIND::MAIN;
		Ctx.iFallbackPass = ETOI(MAP_DEFAULT_PASS);
		Ctx.bUseLayerEx = true;

		MESH_LAYER_BIND_RESULT Result{};
		if (FAILED(MeshLayerBinder::Bind(Ctx, &Result)))
			return E_FAIL;

		if (Result.bSkipMesh)
			continue;

		Bind_MeshLayers(i);

		if (FAILED(m_pShaderCom->Begin(Result.iPass)))
			return E_FAIL;
		if (FAILED(m_pModelCom->Render(i)))
			return E_FAIL;
	}

	return S_OK;
}

void CMapEvent_BreakWall::Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData)
{
	if (nullptr == pOutData)
		return;

	pOutData->strPrototypeTag = PROTOTYPE_TAG;
}

const _tchar* CMapEvent_BreakWall::Get_ModelProtoTag() const
{
	return STAGE12_MODEL_PROTO_TAG;
}

_uint CMapEvent_BreakWall::Get_ModelProtoLevel() const
{
	return m_iModelProtoLevel;
}

HRESULT CMapEvent_BreakWall::Ready_Fragments()
{
	m_Fragments.clear();
	m_MeshFragmentIndices.clear();

	if (nullptr == m_pModelCom)
		return E_FAIL;

	const _uint iNumMeshes = static_cast<_uint>(m_pModelCom->Get_NumMeshes());
	m_MeshFragmentIndices.assign(iNumMeshes, INVALID_FRAGMENT_INDEX);
	m_Fragments.reserve(_countof(g_GsDefault4BreakBinds));

	for (const BREAK_FRAGMENT_BIND& Bind : g_GsDefault4BreakBinds)
	{
		BREAK_FRAGMENT Fragment{};
		Fragment.strFragmentName = Bind.pFragmentName;
		Fragment.vPivot = Bind.vPivot;
		m_Fragments.push_back(Fragment);
	}

	for (_uint i = 0; i < iNumMeshes; ++i)
	{
		const string strFragmentName = Get_FragmentNameFromMeshName(m_pModelCom->Get_MeshName(i));
		auto iter = find_if(m_Fragments.begin(), m_Fragments.end(),
			[&](const BREAK_FRAGMENT& Fragment)->_bool
			{
				return Fragment.strFragmentName == strFragmentName;
			});

		if (m_Fragments.end() == iter)
			continue;

		const _uint iFragment = static_cast<_uint>(iter - m_Fragments.begin());
		iter->MeshIndices.push_back(i);
		m_MeshFragmentIndices[i] = iFragment;
	}

	for (const BREAK_FRAGMENT& Fragment : m_Fragments)
	{
		if (!Fragment.MeshIndices.empty())
			continue;

#ifdef _DEBUG
		OutputDebugStringA(("[MapEvent_BreakWall] Missing fragment mesh group: " + Fragment.strFragmentName + "\n").c_str());
#endif
		return E_FAIL;
	}

	return S_OK;
}

const CMapEvent_BreakWall::BREAK_FRAGMENT* CMapEvent_BreakWall::Find_Fragment(_uint iMesh) const
{
	const _uint iFragment = m_MeshFragmentIndices[iMesh];
	if (INVALID_FRAGMENT_INDEX == iFragment)
		return nullptr;

	return &m_Fragments[iFragment];
}

HRESULT CMapEvent_BreakWall::Ready_BoostTrigger()
{
	if (m_Fragments.empty())
		return E_FAIL;

	const _float3 vFirstPivot = {
			m_Fragments.front().vPivot.x,
			m_Fragments.front().vPivot.y,
			-m_Fragments.front().vPivot.z
	};

	_float3 vMin = vFirstPivot;
	_float3 vMax = vFirstPivot;

	for (const BREAK_FRAGMENT& Fragment : m_Fragments)
	{
		const _float3 vPivot = {
				Fragment.vPivot.x,
				Fragment.vPivot.y,
				-Fragment.vPivot.z
		};

		vMin.x = min(vMin.x, vPivot.x);
		vMin.y = min(vMin.y, vPivot.y);
		vMin.z = min(vMin.z, vPivot.z);

		vMax.x = max(vMax.x, vPivot.x);
		vMax.y = max(vMax.y, vPivot.y);
		vMax.z = max(vMax.z, vPivot.z);
	}

	constexpr _float fPadding = 2.f;

	CCollider::COLLIDER_DESC ColliderDesc{};
	ColliderDesc.pOwner = this;
	ColliderDesc.vCenter = {
			(vMin.x + vMax.x) * 0.5f,
			(vMin.y + vMax.y) * 0.5f,
			(vMin.z + vMax.z) * 0.5f
	};
	ColliderDesc.vSize = {
			(vMax.x - vMin.x) + fPadding * 2.f,
			(vMax.y - vMin.y) + fPadding * 2.f,
			(vMax.z - vMin.z) + fPadding * 2.f
	};

	m_pBoostTrigger = Add_Component<CCollider>(L"Com_BoostTrigger", CCollider::Create(m_pDevice, m_pContext, COLLIDER::AABB));

	if (nullptr == m_pBoostTrigger)
		return E_FAIL;

	if (FAILED(m_pBoostTrigger->Initialize(&ColliderDesc)))
		return E_FAIL;

	m_pBoostTrigger->Set_OnEnter([this](CCollider* pOther) { On_BoostTriggerEnter(pOther); });

	m_pGameInstance_Proxy->Register_Collider(m_pBoostTrigger, ETOUI(COLLISION_LAYER::ENV_TRIGGER));

	return S_OK;
}

void CMapEvent_BreakWall::On_BoostTriggerEnter(CCollider* pOther)
{
	if (nullptr == pOther)
		return;

	if (ETOUI(COLLISION_LAYER::CAR_BOOST) != pOther->Get_RegisteredGroup())
		return;

	Start_Break();
}

void CMapEvent_BreakWall::Start_Break()
{
	if (BREAK_STATE::INTACT != m_eBreakState)
		return;

	m_eBreakState = BREAK_STATE::BREAKING;
	m_bRenderable = true;

	m_pBoostTrigger->Set_Enabled(false);

	m_pGameInstance_Proxy->Publish(EventTag::Stage12_CarBreakWall, nullptr);

	CAMERA_SHAKE_DESC ShakeDesc{ 0.6f, 0.4f };
	m_pGameInstance_Proxy->Publish(EventTag::Camera_Shake, &ShakeDesc);

	for (_uint i = 0; i < static_cast<_uint>(m_Fragments.size()); ++i)
	{
		BREAK_FRAGMENT& Fragment = m_Fragments[i];

		const _float fAngle = static_cast<_float>(i) * 0.73f;
		const _float fSpeed = 8.f + static_cast<_float>(i % 5) * 2.4f;
		
		Fragment.bActive = true;
		Fragment.vOffset = {};
		Fragment.vRotation = { 0.f, 0.f, 0.f, 1.f };
		Fragment.vVelocity = { cosf(fAngle) * fSpeed, 12.f + static_cast<_float>(i % 4) * 1.8f, sinf(fAngle) * fSpeed };
		Fragment.vAngularVelocity = {
			1.15f + static_cast<_float>(i % 3) * 0.5f,
			1.60f + static_cast<_float>(i % 5) * 0.5f,
			1.45f + static_cast<_float>(i % 4) * 0.5f };
	}


#ifdef _DEBUG
	OutputDebugStringA("[MapEvent_BreakWall] Break started.\n");
#endif
}

CMapEvent_BreakWall* CMapEvent_BreakWall::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CMapEvent_BreakWall* pInstance = new CMapEvent_BreakWall(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CMapEvent_BreakWall");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CMapEvent_BreakWall::Clone(void* pArg)
{
	CMapEvent_BreakWall* pInstance = new CMapEvent_BreakWall(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CMapEvent_BreakWall");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CMapEvent_BreakWall::Free()
{
	__super::Free();
}

NS_END