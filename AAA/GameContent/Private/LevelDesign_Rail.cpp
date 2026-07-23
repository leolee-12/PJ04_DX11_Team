#include "LevelDesign_Rail.h"
#include "GameContent_const.h"
#include "RailTrack.h"
#include "MeshLayer_Binder.h"

#include "DebugDraw.h"
#include "GameInstance_Proxy.h"
#include "Geometry_Utils.h"

namespace
{
	_vector Get_BezierNodeDirection(const LD_RAIL_DESC& RailDesc, _uint iNodeIndex)
	{
		const _uint iNodeCount = static_cast<_uint>(RailDesc.Nodes.size());
		if (iNodeCount < 2 || iNodeIndex >= iNodeCount)
			return XMVectorZero();

		_vector vDirection;

		if (RailDesc.bClose && iNodeCount > 2)
		{
			const _uint iPrevIndex = (iNodeIndex + iNodeCount - 1) % iNodeCount;
			const _uint iNextIndex = (iNodeIndex + 1) % iNodeCount;
			vDirection = XMLoadFloat3(&RailDesc.Nodes[iNextIndex].vPosition) - XMLoadFloat3(&RailDesc.Nodes[iPrevIndex].vPosition);
		}
		else if (0 == iNodeIndex)
		{
			vDirection = XMLoadFloat3(&RailDesc.Nodes[1].vPosition) - XMLoadFloat3(&RailDesc.Nodes[0].vPosition);
		}
		else if (iNodeIndex == iNodeCount - 1)
		{
			vDirection = XMLoadFloat3(&RailDesc.Nodes[iNodeIndex].vPosition) - XMLoadFloat3(&RailDesc.Nodes[iNodeIndex - 1].vPosition);
		}
		else
		{
			vDirection = XMLoadFloat3(&RailDesc.Nodes[iNodeIndex + 1].vPosition) - XMLoadFloat3(&RailDesc.Nodes[iNodeIndex - 1].vPosition);
		}

		return XMVectorGetX(XMVector3LengthSq(vDirection)) > 1e-8f ? XMVector3Normalize(vDirection) : XMVectorZero();
	}
}

NS_BEGIN(Client)

CLevelDesign_Rail::CLevelDesign_Rail(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CLevelDesignObject(pDevice, pContext)
{
}

CLevelDesign_Rail::CLevelDesign_Rail(const CLevelDesign_Rail& Prototype)
	: CLevelDesignObject(Prototype)
	, m_tRailDesc{ Prototype.m_tRailDesc }
	, m_pRailTrack{ nullptr }
{
}

HRESULT CLevelDesign_Rail::Initialize_Prototype()
{
	return __super::Initialize_Prototype();
}

HRESULT CLevelDesign_Rail::Initialize(void* pArg)
{
	if (nullptr == pArg)
		return E_FAIL;

	const LD_PARSED_OBJECT* pParsedDesc = static_cast<const LD_PARSED_OBJECT*>(pArg);

	m_tRailDesc = pParsedDesc->Rail;

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	m_tLevelDesignDesc.eCategory = LD_CATEGORY::RAIL;

	m_pRailTrack = CRailTrack::Create();
	if (nullptr == m_pRailTrack)
		return E_FAIL;

	m_pRailTrack->Build(m_tRailDesc);

#ifdef _DEBUG
	if (FAILED(Ready_DebugResources()))
		return E_FAIL;
#endif

	if (FAILED(Validate_Initialized()))
		return E_FAIL;

	return S_OK;
}

HRESULT CLevelDesign_Rail::Validate_Initialized()
{
	if (FAILED(__super::Validate_Initialized()))
		return E_FAIL;

	if (nullptr == m_pGameInstance_Proxy || nullptr == m_pTransformCom || nullptr == m_pRailTrack)
		return E_FAIL;
	if (LD_CATEGORY::RAIL != m_tLevelDesignDesc.eCategory)
		return E_FAIL;
	if (m_tLevelDesignDesc.strObjectName.empty())
		return E_FAIL;

	if (m_tRailDesc.fRadius < 0.f || m_tRailDesc.fBezierControlLength < 0.f)
		return E_FAIL;
	if (0 == Get_SegmentCount(m_tRailDesc))
		return E_FAIL;
	if (!m_pRailTrack->Is_Valid())
		return E_FAIL;

	for (const LD_RAIL_NODE_DESC& Node : m_tRailDesc.Nodes)
	{
		if (Node.fBezierControlLength < 0.f)
			return E_FAIL;
	}

#ifdef _DEBUG
	if (nullptr == m_pBatch || nullptr == m_pEffect || nullptr == m_pInputLayout)
		return E_FAIL;
#endif

	return S_OK;
}

void CLevelDesign_Rail::Late_Update(_float fTimeDelta)
{
	UNREFERENCED_PARAMETER(fTimeDelta);

	if (m_bVisualReady)
	{
		Check_Visible();
		Submit_RenderGroups();
		return;
	}

#ifdef _DEBUG
	if (m_tRailDesc.Nodes.size() >= 2 && m_pGameInstance_Proxy->IsOn_DebugRender())
		m_pGameInstance_Proxy->Add_RenderGroup(RENDERID::NONLIGHT, this);
#endif
}

HRESULT CLevelDesign_Rail::Render()
{
	if (m_bVisualReady)
	{
		if (FAILED(Bind_ShaderResources()))
			return E_FAIL;

		return Render_Model();
	}

#ifdef _DEBUG
	return Render_Rail();
#else
	return S_OK;
#endif
}

HRESULT CLevelDesign_Rail::Render_Shadow()
{
	if (!m_bVisualReady)
		return S_OK;

	if (nullptr == m_pShaderCom || nullptr == m_pModelCom || nullptr == m_pInstanceBuffer || nullptr == m_pGameInstance_Proxy || m_Instances.empty())
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance_Proxy->Get_Shadow_Transform(D3DTS::VIEW))))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance_Proxy->Get_Shadow_Transform(D3DTS::PROJ))))
		return E_FAIL;

	const _uint iInstanceCount = static_cast<_uint>(m_Instances.size());
	const _uint iNumMeshes = static_cast<_uint>(m_pModelCom->Get_NumMeshes());

	for (_uint i = 0; i < iNumMeshes; ++i)
	{
		const MESH_LAYER_IDX& Layer = m_pModelCom->Get_MeshLayer(i);

		MESH_LAYER_BIND_CONTEXT Ctx{};
		Ctx.Set_Renderer(m_pShaderCom, m_pModelCom, m_pGameInstance_Proxy);
		Ctx.iMesh = i;
		Ctx.pLayer = &Layer;
		Ctx.eProfile = MESH_LAYER_PROFILE::WORLD_INSTANCE;
		Ctx.eKind = MESH_LAYER_RENDER_KIND::SHADOW;
		Ctx.iFallbackPass = ETOUI(WORLD_PASS::SHADOW);

		MESH_LAYER_BIND_RESULT Result{};
		if (FAILED(MeshLayerBinder::Bind(Ctx, &Result)))
			return E_FAIL;

		if (Result.bSkipMesh)
			continue;

		if (FAILED(m_pShaderCom->Begin(Result.iPass)))
			return E_FAIL;

		if (FAILED(m_pModelCom->Render_Instanced(i, m_pInstanceBuffer, sizeof(COASTER_RAIL_INSTANCE_DATA), iInstanceCount)))
			return E_FAIL;
	}

	return S_OK;
}

void CLevelDesign_Rail::Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData)
{
	if (nullptr == pOutData)
		return;

	pOutData->strPrototypeTag = PROTOTYPE_TAG;
}

HRESULT CLevelDesign_Rail::Ready_RenderComponents(_uint iModelPrototypeLevel)
{
	if (nullptr == m_pShaderCom)
	{
		m_pShaderCom = Add_Component<CShader>(Shader_World_Instance.iLevelID, Shader_World_Instance.szProtoTag, TEXT("Com_Shader"));
		if (nullptr == m_pShaderCom)
			return E_FAIL;
	}

	if (nullptr == m_pModelCom)
	{
		m_pModelCom = Add_Component<CModel>(iModelPrototypeLevel, COASTER_RAIL_MODEL_PROTO_TAG, TEXT("Com_Model"));
		if (nullptr == m_pModelCom)
			return E_FAIL;
	}

	return S_OK;
}

HRESULT CLevelDesign_Rail::Ready_InstanceData()
{
	if (!m_Instances.empty())
		return S_OK;

	if (nullptr == m_pRailTrack || !m_pRailTrack->Is_Valid() || nullptr == m_pModelCom)
		return E_FAIL;

	_float3 vMin{};
	_float3 vMax{};
	m_pModelCom->Get_ModelAABB(&vMin, &vMax);

	const _float fPieceLength = vMax.z - vMin.z;
	const _float fRailLength = m_pRailTrack->Get_Length();

	if (fPieceLength <= 0.001f || fRailLength <= 0.001f)
		return E_FAIL;

	constexpr _float fOverlap = 0.05f;
	const _float fStep = max(fPieceLength - fOverlap, 0.01f);

	m_Instances.reserve(static_cast<size_t>(ceilf(fRailLength / fStep)));

	for (_float fDistance = 0.f; fDistance < fRailLength; fDistance += fStep)
	{
		const _float fNextDistance = min(fDistance + fPieceLength, fRailLength);

		if (fNextDistance - fDistance <= 0.0001f)
			break;

		_float3 vStart{};
		_float3 vMiddle{};
		_float3 vEnd{};
		_float3 vTangent{};

		if (!m_pRailTrack->Sample(fDistance, &vStart)
			|| !m_pRailTrack->Sample((fDistance + fNextDistance) * 0.5f, &vMiddle, &vTangent)
			|| !m_pRailTrack->Sample(fNextDistance, &vEnd))
		{
			m_Instances.clear();
			return E_FAIL;
		}

		const _vector vSegment = XMLoadFloat3(&vEnd) - XMLoadFloat3(&vStart);
		const _float fActualLength = XMVectorGetX(XMVector3Length(vSegment));
		const _vector vTangentVector = XMLoadFloat3(&vTangent);

		if (fActualLength <= 0.0001f || XMVectorGetX(XMVector3LengthSq(vTangentVector)) <= 1e-8f)
		{
			m_Instances.clear();
			return E_FAIL;
		}

		const _float fScaleZ = fActualLength / fPieceLength;
		const _vector vLook = XMVector3Normalize(vTangentVector);
		_vector vReferenceRight = XMVectorSet(0.f, 0.f, 1.f, 0.f);

		if (fabsf(XMVectorGetX(XMVector3Dot(vLook, vReferenceRight))) > 0.999f)
			vReferenceRight = XMVectorSet(1.f, 0.f, 0.f, 0.f);

		const _vector vUp = XMVector3Normalize(XMVector3Cross(vLook, vReferenceRight));
		const _vector vRight = XMVector3Normalize(XMVector3Cross(vUp, vLook));
		const _matrix World = XMMATRIX(vRight, vUp, vLook * fScaleZ, XMVectorSet(vMiddle.x, vMiddle.y, vMiddle.z, 1.f));

		COASTER_RAIL_INSTANCE_DATA InstanceData{};
		XMStoreFloat4x4(&InstanceData.matWorld, World);
		m_Instances.emplace_back(InstanceData);
	}

	return m_Instances.empty() ? E_FAIL : S_OK;
}

HRESULT CLevelDesign_Rail::Ready_InstanceBuffer()
{
	if (nullptr != m_pInstanceBuffer)
		return S_OK;

	if (m_Instances.empty())
		return E_FAIL;

	D3D11_BUFFER_DESC tDesc{};
	tDesc.ByteWidth = static_cast<_uint>(sizeof(COASTER_RAIL_INSTANCE_DATA) * m_Instances.size());
	tDesc.Usage = D3D11_USAGE_IMMUTABLE;
	tDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	tDesc.StructureByteStride = sizeof(COASTER_RAIL_INSTANCE_DATA);

	D3D11_SUBRESOURCE_DATA InitialData{};
	InitialData.pSysMem = m_Instances.data();

	if (FAILED(m_pDevice->CreateBuffer(&tDesc, &InitialData, &m_pInstanceBuffer)))
		return E_FAIL;

	return S_OK;
}

HRESULT CLevelDesign_Rail::Ready_VisualCullBounds()
{
	if (nullptr == m_pModelCom || nullptr == m_pTransformCom || m_Instances.empty())
		return E_FAIL;

	_float3 vMin{};
	_float3 vMax{};
	m_pModelCom->Get_ModelAABB(&vMin, &vMax);

	if (!GeometryUtils::Is_ValidAABB(vMin, vMax))
		return E_FAIL;

	const BoundingBox ModelBounds = GeometryUtils::Make_AABB_FromMinMax(vMin, vMax);
	const _matrix RailWorldMatrix = XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr());
	const _float fDeterminant = XMVectorGetX(XMMatrixDeterminant(RailWorldMatrix));

	if (fabsf(fDeterminant) <= 1e-8f)
		return E_FAIL;

	const _matrix InverseRailWorldMatrix = XMMatrixInverse(nullptr, RailWorldMatrix);

	BoundingBox CullBounds{};
	_bool bHasBounds = false;

	for (const COASTER_RAIL_INSTANCE_DATA& InstanceData : m_Instances)
	{
		const _matrix InstanceLocalMatrix = XMLoadFloat4x4(&InstanceData.matWorld) * InverseRailWorldMatrix;

		BoundingBox InstanceBounds{};
		if (!GeometryUtils::Transform_AABB(ModelBounds, InstanceLocalMatrix, &InstanceBounds))
			return E_FAIL;

		if (!GeometryUtils::Is_ValidAABB(InstanceBounds))
			return E_FAIL;

		CullBounds = bHasBounds ? GeometryUtils::Merge_AABB(CullBounds, InstanceBounds) : InstanceBounds;
		bHasBounds = true;
	}

	if (!bHasBounds)
		return E_FAIL;

	return Ready_CullingState(CullBounds);
}

HRESULT CLevelDesign_Rail::Bind_ShaderResources()
{
	if (nullptr == m_pShaderCom || nullptr == m_pGameInstance_Proxy)
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::VIEW, m_eProjType))))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::PROJ, m_eProjType))))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_RawValue("g_iMaterialID", &m_iMaterialID, sizeof(_uint))))
		return E_FAIL;

	return S_OK;
}

HRESULT CLevelDesign_Rail::Render_Model()
{
	if (nullptr == m_pShaderCom || nullptr == m_pModelCom || nullptr == m_pInstanceBuffer || m_Instances.empty())
		return E_FAIL;

	const _uint iInstanceCount = static_cast<_uint>(m_Instances.size());
	const _uint iNumMeshes = static_cast<_uint>(m_pModelCom->Get_NumMeshes());

	for (_uint i = 0; i < iNumMeshes; ++i)
	{
		const MESH_LAYER_IDX& Layer = m_pModelCom->Get_MeshLayer(i);
		const _bool bUseColorPass = (0u == m_pModelCom->Get_MeshTextureCount(i, MTEX_TYPE::DIFFUSE));

		MESH_LAYER_BIND_CONTEXT Ctx{};
		Ctx.Set_Renderer(m_pShaderCom, m_pModelCom, m_pGameInstance_Proxy, m_pCullingState);
		Ctx.iMesh = i;
		Ctx.pLayer = &Layer;
		Ctx.eProfile = MESH_LAYER_PROFILE::WORLD_INSTANCE;
		Ctx.eKind = MESH_LAYER_RENDER_KIND::MAIN;
		Ctx.iFallbackPass = bUseColorPass ? ETOUI(WORLD_PASS::COLOR_CONST_MRA) : ETOUI(WORLD_PASS::DMN);

		MESH_LAYER_BIND_RESULT Result{};
		if (FAILED(MeshLayerBinder::Bind(Ctx, &Result)))
			return E_FAIL;

		if (Result.bSkipMesh)
			continue;

		if (FAILED(m_pShaderCom->Begin(Result.iPass)))
			return E_FAIL;

		if (FAILED(m_pModelCom->Render_Instanced(i, m_pInstanceBuffer, sizeof(COASTER_RAIL_INSTANCE_DATA), iInstanceCount)))
			return E_FAIL;
	}

	return S_OK;
}

HRESULT CLevelDesign_Rail::Enable_Visual(RAIL_VISUAL_TYPE eType, _uint iModelPrototypeLevel)
{
	if (RAIL_VISUAL_TYPE::NONE == eType)
		return S_OK;

	if (RAIL_VISUAL_TYPE::COASTER != eType)
		return E_FAIL;

	if (m_bVisualReady)
		return m_eVisualType == eType ? S_OK : E_FAIL;

	if (FAILED(Ready_RenderComponents(iModelPrototypeLevel)))
		return E_FAIL;

	if (FAILED(Ready_InstanceData()))
		return E_FAIL;

	if (FAILED(Ready_VisualCullBounds()))
		return E_FAIL;

	if (FAILED(Ready_InstanceBuffer()))
		return E_FAIL;

	m_eVisualType = eType;
	m_bUseShadow = true;
	m_bVisualReady = true;
	return S_OK;
}

CLevelDesign_Rail* CLevelDesign_Rail::Find_ByUid(CGameInstance_Proxy* pProxy, _uint iLevelIndex, _uint iRailUid)
{
	if (nullptr == pProxy || 0 == iRailUid)
		return nullptr;

	const _wstring strObjectTag = _wstring(OBJECT_NAME) + L"_" + to_wstring(iRailUid);
	return pProxy->Find_GameObject<CLevelDesign_Rail>(iLevelIndex, LAYER_TAG, strObjectTag);
}

_uint CLevelDesign_Rail::Get_SegmentCount(const LD_RAIL_DESC& RailDesc)
{
	switch (RailDesc.eType)
	{
	case LD_RAIL_TYPE::LINE:
	case LD_RAIL_TYPE::BEZIER:
		if (RailDesc.Nodes.size() < 2)
			return 0;

		return static_cast<_uint>(RailDesc.Nodes.size() - 1 + (RailDesc.bClose && RailDesc.Nodes.size() > 2 ? 1 : 0));

	case LD_RAIL_TYPE::CIRCLE:
		return RailDesc.fRadius > 0.001f ? 1u : 0u;

	default:
		return 0;
	}
}

_bool CLevelDesign_Rail::Evaluate_Segment(const LD_RAIL_DESC& RailDesc, _uint iSegmentIndex, _float fT, _float3* pOutPosition, _float3* pOutTangent)
{
	if (nullptr == pOutPosition)
		return false;

	const _uint iSegmentCount = Get_SegmentCount(RailDesc);
	if (0 == iSegmentCount || iSegmentIndex >= iSegmentCount)
		return false;

	const _float fClampedT = std::clamp(fT, 0.f, 1.f);

	if (LD_RAIL_TYPE::LINE == RailDesc.eType)
	{
		const _uint iStartIndex = iSegmentIndex;
		const _uint iEndIndex = iSegmentIndex + 1 < RailDesc.Nodes.size() ? iSegmentIndex + 1 : 0;
		const _vector vStart = XMLoadFloat3(&RailDesc.Nodes[iStartIndex].vPosition);
		const _vector vEnd = XMLoadFloat3(&RailDesc.Nodes[iEndIndex].vPosition);
		const _vector vDelta = vEnd - vStart;

		XMStoreFloat3(pOutPosition, XMVectorLerp(vStart, vEnd, fClampedT));

		if (nullptr != pOutTangent)
		{
			if (XMVectorGetX(XMVector3LengthSq(vDelta)) > 1e-8f)
				XMStoreFloat3(pOutTangent, XMVector3Normalize(vDelta));
			else
				*pOutTangent = {};
		}

		return true;
	}

	if (LD_RAIL_TYPE::BEZIER == RailDesc.eType)
	{
		const _uint iStartIndex = iSegmentIndex;
		const _uint iEndIndex = iSegmentIndex + 1 < RailDesc.Nodes.size() ? iSegmentIndex + 1 : 0;
		const LD_RAIL_NODE_DESC& StartNode = RailDesc.Nodes[iStartIndex];
		const LD_RAIL_NODE_DESC& EndNode = RailDesc.Nodes[iEndIndex];

		const _vector vStart = XMLoadFloat3(&StartNode.vPosition);
		const _vector vEnd = XMLoadFloat3(&EndNode.vPosition);
		const _vector vStartDirection = Get_BezierNodeDirection(RailDesc, iStartIndex);
		const _vector vEndDirection = Get_BezierNodeDirection(RailDesc, iEndIndex);
		const _vector vControlStart = vStart + vStartDirection * max(StartNode.fBezierControlLength, 0.f);
		const _vector vControlEnd = vEnd - vEndDirection * max(EndNode.fBezierControlLength, 0.f);
		const _float fInverseT = 1.f - fClampedT;
		const _float fInverseT2 = fInverseT * fInverseT;
		const _float fT2 = fClampedT * fClampedT;

		const _vector vPosition = vStart * (fInverseT2 * fInverseT)
			+ vControlStart * (3.f * fInverseT2 * fClampedT)
			+ vControlEnd * (3.f * fInverseT * fT2)
			+ vEnd * (fT2 * fClampedT);

		XMStoreFloat3(pOutPosition, vPosition);

		if (nullptr != pOutTangent)
		{
			_vector vTangent = (vControlStart - vStart) * (3.f * fInverseT2)
				+ (vControlEnd - vControlStart) * (6.f * fInverseT * fClampedT)
				+ (vEnd - vControlEnd) * (3.f * fT2);

			if (XMVectorGetX(XMVector3LengthSq(vTangent)) <= 1e-8f)
				vTangent = vEnd - vStart;

			XMStoreFloat3(pOutTangent, XMVectorGetX(XMVector3LengthSq(vTangent)) > 1e-8f ? XMVector3Normalize(vTangent) : XMVectorZero());
		}

		return true;
	}

	if (LD_RAIL_TYPE::CIRCLE == RailDesc.eType)
	{
		_float fStartAngle = 0.f;

		if (!RailDesc.Nodes.empty())
		{
			const _float3& vStart = RailDesc.Nodes.front().vPosition;
			fStartAngle = atan2f(vStart.z - RailDesc.vCenterPos.z, vStart.x - RailDesc.vCenterPos.x);
		}

		const _float fAngleDelta = RailDesc.bClockwise ? -XM_2PI : XM_2PI;
		const _float fAngle = fStartAngle + fAngleDelta * fClampedT;
		const _float fSin = sinf(fAngle);
		const _float fCos = cosf(fAngle);

		pOutPosition->x = RailDesc.vCenterPos.x + fCos * RailDesc.fRadius;
		pOutPosition->y = RailDesc.vCenterPos.y;
		pOutPosition->z = RailDesc.vCenterPos.z + fSin * RailDesc.fRadius;

		if (nullptr != pOutTangent)
		{
			const _vector vTangent = XMVectorSet(-fSin * fAngleDelta, 0.f, fCos * fAngleDelta, 0.f);
			XMStoreFloat3(pOutTangent, XMVector3Normalize(vTangent));
		}

		return true;
	}

	return false;
}

#ifdef _DEBUG
HRESULT CLevelDesign_Rail::Ready_DebugResources()
{
	m_pBatch =
		new PrimitiveBatch<VertexPositionColor>(m_pContext);

	m_pEffect = new BasicEffect(m_pDevice);
	m_pEffect->SetVertexColorEnabled(true);

	const void* pShaderByteCode = nullptr;
	size_t iShaderByteCodeLength = 0;

	m_pEffect->GetVertexShaderBytecode(
		&pShaderByteCode,
		&iShaderByteCodeLength);

	if (FAILED(m_pDevice->CreateInputLayout(
		VertexPositionColor::InputElements,
		VertexPositionColor::InputElementCount,
		pShaderByteCode,
		iShaderByteCodeLength,
		&m_pInputLayout)))
	{
		return E_FAIL;
	}

	return S_OK;
}

HRESULT CLevelDesign_Rail::Render_Rail()
{
	if (m_tRailDesc.Nodes.size() < 2)
		return S_OK;

	const _float4x4* pView = m_pGameInstance_Proxy->Get_Matrix(D3DTS::VIEW, PROJ_TYPE::PERSPEC);

	const _float4x4* pProj = m_pGameInstance_Proxy->Get_Matrix(D3DTS::PROJ, PROJ_TYPE::PERSPEC);

	if (nullptr == pView || nullptr == pProj)
		return E_FAIL;

	// Rail 노드는 월드 좌표이므로 오브젝트 Transform을 적용하지 않는다.
	m_pEffect->SetWorld(XMMatrixIdentity());
	m_pEffect->SetView(XMLoadFloat4x4(pView));
	m_pEffect->SetProjection(XMLoadFloat4x4(pProj));

	m_pContext->OMSetBlendState(nullptr, nullptr, 0xffffffff);
	m_pContext->OMSetDepthStencilState(nullptr, 0);
	m_pContext->RSSetState(nullptr);
	m_pContext->IASetInputLayout(m_pInputLayout);

	m_pEffect->Apply(m_pContext);

	const _float4 vRailColor = { 1.f, 0.85f, 0.15f, 1.f };
	const _float4 vNodeColor = { 1.f, 0.85f, 0.15f, 1.f };
	const _bool bCircle = LD_RAIL_TYPE::CIRCLE == m_tRailDesc.eType;

	m_pBatch->Begin();

	if (bCircle)
	{
		const _vector vCenter = XMLoadFloat3(&m_tRailDesc.vCenterPos);
		const _vector vMajorAxis = XMVectorSet(m_tRailDesc.fRadius, 0.f, 0.f, 0.f);
		const _vector vMinorAxis = XMVectorSet(0.f, 0.f, m_tRailDesc.fRadius, 0.f);

		DX::DrawRing(m_pBatch, vCenter, vMajorAxis, vMinorAxis, XMLoadFloat4(&vRailColor));
	}
	else
	{
		vector<VertexPositionColor> Vertices;

		if (LD_RAIL_TYPE::BEZIER == m_tRailDesc.eType)
		{
			constexpr _uint iSamplesPerSegment = 16;
			const _uint iSegmentCount = Get_SegmentCount(m_tRailDesc);
			Vertices.reserve(iSegmentCount * iSamplesPerSegment + 1);

			for (_uint iSegment = 0; iSegment < iSegmentCount; ++iSegment)
			{
				for (_uint iSample = 0; iSample <= iSamplesPerSegment; ++iSample)
				{
					if (iSegment > 0 && 0 == iSample)
						continue;

					_float3 vPosition{};
					const _float fT = static_cast<_float>(iSample) / static_cast<_float>(iSamplesPerSegment);

					if (Evaluate_Segment(m_tRailDesc, iSegment, fT, &vPosition))
						Vertices.emplace_back(vPosition, vRailColor);
				}
			}
		}
		else
		{
			Vertices.reserve(m_tRailDesc.Nodes.size() + (m_tRailDesc.bClose ? 1u : 0u));

			for (const LD_RAIL_NODE_DESC& Node : m_tRailDesc.Nodes)
				Vertices.emplace_back(Node.vPosition, vRailColor);

			if (m_tRailDesc.bClose && m_tRailDesc.Nodes.size() > 2)
				Vertices.emplace_back(m_tRailDesc.Nodes.front().vPosition, vRailColor);
		}

		if (Vertices.size() >= 2)
			m_pBatch->Draw(D3D_PRIMITIVE_TOPOLOGY_LINESTRIP, Vertices.data(), Vertices.size());
	}

	constexpr _float fMarkerHalfSize = 0.25f;

	for (const LD_RAIL_NODE_DESC& Node : m_tRailDesc.Nodes)
	{
		const _float3& vNode = Node.vPosition;

		m_pBatch->DrawLine(
			VertexPositionColor( { vNode.x - fMarkerHalfSize, vNode.y, vNode.z }, vNodeColor),
			VertexPositionColor( { vNode.x + fMarkerHalfSize, vNode.y, vNode.z }, vNodeColor));

		m_pBatch->DrawLine(
			VertexPositionColor( { vNode.x, vNode.y - fMarkerHalfSize, vNode.z }, vNodeColor),
			VertexPositionColor( { vNode.x, vNode.y + fMarkerHalfSize, vNode.z }, vNodeColor));

		m_pBatch->DrawLine(
			VertexPositionColor( { vNode.x, vNode.y, vNode.z - fMarkerHalfSize }, vNodeColor),
			VertexPositionColor( { vNode.x, vNode.y, vNode.z + fMarkerHalfSize }, vNodeColor));
	}

	m_pBatch->End();

	return S_OK;
}
#endif

CLevelDesign_Rail* CLevelDesign_Rail::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CLevelDesign_Rail* pInstance = new CLevelDesign_Rail(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CLevelDesign_Rail");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CLevelDesign_Rail::Clone(void* pArg)
{
	CLevelDesign_Rail* pInstance = new CLevelDesign_Rail(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CLevelDesign_Rail");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CLevelDesign_Rail::Free()
{
	Safe_Release(m_pInstanceBuffer);
	Safe_Release(m_pRailTrack);

#ifdef _DEBUG
	Safe_Release(m_pInputLayout);
	Safe_Delete(m_pEffect);
	Safe_Delete(m_pBatch);
#endif

	__super::Free();
}

NS_END