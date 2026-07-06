#include "LevelDesign_Ladder.h"
#include "LevelDesign_Registry.h"
#include "Shader_PassMeta.h"
#include "Parsing_Utils.h"

#include "GameInstance.h"

namespace
{
	struct LD_LADDER_CATALOG
	{
		const _tchar* pObjectName;
		const _char* pBotModelPath;
		const _char* pMidModelPath;
		const _char* pTopModelPath;
	};

	static const LD_LADDER_CATALOG g_LadderCatalog =
	{
		L"Ladder",
		"../../Resources/Map/Gimmick/NonAnim/Ladder/Ladder_Bottom.ysh",
		"../../Resources/Map/Gimmick/NonAnim/Ladder/Ladder_Middle.ysh",
		"../../Resources/Map/Gimmick/NonAnim/Ladder/Ladder_Top.ysh"
	};
}

NS_BEGIN(Client)

CLevelDesign_Ladder::CLevelDesign_Ladder(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CLevelDesignObject(pDevice, pContext)
{
}

CLevelDesign_Ladder::CLevelDesign_Ladder(const CLevelDesign_Ladder& Prototype)
	: CLevelDesignObject(Prototype)
	, m_tLadderDesc(Prototype.m_tLadderDesc)
	, m_fSegmentStepY(Prototype.m_fSegmentStepY)
{
}

HRESULT CLevelDesign_Ladder::Initialize_Prototype()
{
	return __super::Initialize_Prototype();
}

HRESULT CLevelDesign_Ladder::Initialize(void* pArg)
{
	if (nullptr == pArg)
		return E_FAIL;

	m_tLadderDesc = *static_cast<const LD_LADDER_DESC*>(pArg);

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_Components()))
		return E_FAIL;

	if (FAILED(Validate_Initialized()))
		return E_FAIL;

	return S_OK;
}

HRESULT CLevelDesign_Ladder::Validate_Initialized()
{
	if (FAILED(__super::Validate_Initialized()))
		return E_FAIL;

	if (m_tLadderDesc.eCategory != LD_CATEGORY::GIMMICK)
		return E_FAIL;
	if (0u == m_tLadderDesc.iLength)
		return E_FAIL;
	if (m_fSegmentStepY <= 0.f)
		return E_FAIL;

	if (nullptr == m_pShaderCom || nullptr == m_pCollider)
		return E_FAIL;

	for (_uint i = 0; i < SEGMENT::_COUNT; ++i)
	{
		if (nullptr == m_ModelComs[i])
			return E_FAIL;
	}

	return S_OK;
}

void CLevelDesign_Ladder::Late_Update(_float fTimeDelta)
{
	UNREFERENCED_PARAMETER(fTimeDelta);

	if (m_pCollider->Is_Enabled())
	{
		m_pCollider->Update(XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));
#ifdef _DEBUG
		m_pGameInstance_Proxy->Add_DebugComponent(m_pCollider);
#endif // _DEBUG

	}

	m_pGameInstance_Proxy->Add_RenderGroup(RENDERID::NONBLEND, this);
}

HRESULT CLevelDesign_Ladder::Render()
{
	const _matrix matBaseWorld = XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr());
	const _uint iLength = m_tLadderDesc.iLength;

	for (_uint i = 0; i < iLength; ++i)
	{
		SEGMENT eSegment = SEGMENT::MID;

		if (0 == i)					eSegment = SEGMENT::BOT;
		else if (iLength - 1 == i)	eSegment = SEGMENT::TOP;

		CModel* pModel = m_ModelComs[eSegment];

		_float4x4 WorldMatrix = {};
		XMStoreFloat4x4(&WorldMatrix, XMMatrixTranslation(0.f, m_fSegmentStepY * static_cast<_float>(i), 0.f) * matBaseWorld);

		if (FAILED(Bind_ShaderResources(WorldMatrix)))
			return E_FAIL;

		if (FAILED(Render_Model(pModel)))
			return E_FAIL;
	}

	return S_OK;
}

void CLevelDesign_Ladder::Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData)
{
	if (nullptr == pOutData)
		return;

	pOutData->strPrototypeTag = PROTOTYPE_TAG;
}

void CLevelDesign_Ladder::Register_LevelDesignSpecs()
{
	LD_SPAWN_SPEC Spec{};
	Spec.strObjectName = g_LadderCatalog.pObjectName;
	Spec.strPrototypeTag = PROTOTYPE_TAG;
	Spec.strLayerTag = L"Layer_LevelDesign_Gimmick";
	Spec.eCategory = LD_CATEGORY::GIMMICK;
	Spec.pPrototypeFactory = &Create_Prototype;
	Spec.pBuildDesc = &Build_Desc;
	Spec.ModelRequirements =
	{
			{ BOT_MODEL_PROTO_TAG, g_LadderCatalog.pBotModelPath },
			{ MID_MODEL_PROTO_TAG, g_LadderCatalog.pMidModelPath },
			{ TOP_MODEL_PROTO_TAG, g_LadderCatalog.pTopModelPath }
	};

	CLevelDesign_Registry::Register(Spec.strObjectName, Spec);
}

_bool CLevelDesign_Ladder::Build_Desc(const LD_OBJECT_DESC& CommonDesc, const json& jEntry, const LD_SPAWN_SPEC& Spec, LD_OBJECT_ENTRY* pOutEntry)
{
	UNREFERENCED_PARAMETER(Spec);

	if (nullptr == pOutEntry)
		return false;

	LD_LADDER_DESC Desc{};
	static_cast<LD_OBJECT_DESC&>(Desc) = CommonDesc;
	Desc.eCategory = LD_CATEGORY::GIMMICK;
	JsonUtils::Try_ReadUInt(jEntry, "Length", &Desc.iLength);

	*pOutEntry = Desc;
	return true;
}

CGameObject* CLevelDesign_Ladder::Create_Prototype(ID3D11Device* pDevice, ID3D11DeviceContext*
	pContext)
{
	return CLevelDesign_Ladder::Create(pDevice, pContext);
}

_int CLevelDesign_Ladder::Get_NearestCellIndex(_fvector vWorldPosition) const
{
	if (m_iTopCellIndex < m_iBottomCellIndex)
		return -1;

	// Convert the player's world position to ladder-local space.
	const _matrix matWorld = XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr());
	const _matrix matInverse = XMMatrixInverse(nullptr, matWorld);
	const _vector vLocalPosition = XMVector3TransformCoord(vWorldPosition, matInverse);

	// Find the nearest cell index relative to the first cell.
	const _float fLocalY = XMVectorGetY(vLocalPosition);
	_int iCellIndex = static_cast<_int>(roundf((fLocalY - m_fBottomCellLocalY) / m_fCellSpacing));
	Helper::IntClamp(iCellIndex, m_iBottomCellIndex, m_iTopCellIndex);

	return iCellIndex;
}

_bool CLevelDesign_Ladder::Try_GetCellWorld(_int iCellIndex, _vector& vOutWorldPosition) const
{
	if (iCellIndex < m_iBottomCellIndex || iCellIndex > m_iTopCellIndex)
		return false;

	// Convert the cell index to a ladder-local position.
	const _float fCellLocalY = m_fBottomCellLocalY + static_cast<_float>(iCellIndex) * m_fCellSpacing;
	const _vector vLocalCellPosition = XMVectorSet(0.f, fCellLocalY, m_fClimbOffsetZ, 1.f);

	// Transform the local cell position by the ladder world matrix.
	const _matrix matWorld = XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr());
	vOutWorldPosition = XMVector3TransformCoord(vLocalCellPosition, matWorld);

	return true;
}

_bool CLevelDesign_Ladder::Is_TopCell(_int iCellIndex) const
{
	return m_iTopCellIndex >= m_iBottomCellIndex && iCellIndex == m_iTopCellIndex;
}

_bool CLevelDesign_Ladder::Is_BottomCell(_int iCellIndex) const
{
	return m_iTopCellIndex >= m_iBottomCellIndex && iCellIndex == m_iBottomCellIndex;
}

HRESULT CLevelDesign_Ladder::Ready_Components()
{
	if (FAILED(Ready_RenderComponents()))
		return E_FAIL;

	if (FAILED(Resolve_SegmentStepY()))
		return E_FAIL;

	// Kirby 상호작용
	if (FAILED(Calculate_TopBottomCellIndices()))
		return E_FAIL;

	if (FAILED(Ready_LadderCollider()))
		return E_FAIL;

	return S_OK;
}

HRESULT CLevelDesign_Ladder::Ready_RenderComponents()
{
	m_pShaderCom = Add_Component<CShader>(Shader_NonAnimMesh_PBR.iLevelID, Shader_NonAnimMesh_PBR.szProtoTag, TEXT("Com_Shader"));
	if (nullptr == m_pShaderCom)
		return E_FAIL;

	m_ModelComs[SEGMENT::BOT] = Add_Component<CModel>(m_tLadderDesc.iModelProtoLevel, BOT_MODEL_PROTO_TAG, TEXT("Com_Model_Bottom"));
	if (nullptr == m_ModelComs[SEGMENT::BOT])
		return E_FAIL;

	m_ModelComs[SEGMENT::MID] = Add_Component<CModel>(m_tLadderDesc.iModelProtoLevel, MID_MODEL_PROTO_TAG, TEXT("Com_Model_Middle"));
	if (nullptr == m_ModelComs[SEGMENT::MID])
		return E_FAIL;

	m_ModelComs[SEGMENT::TOP] = Add_Component<CModel>(m_tLadderDesc.iModelProtoLevel, TOP_MODEL_PROTO_TAG, TEXT("Com_Model_Top"));
	if (nullptr == m_ModelComs[SEGMENT::TOP])
		return E_FAIL;

	return S_OK;
}

HRESULT CLevelDesign_Ladder::Ready_LadderCollider()
{
	CCollider::COLLIDER_DESC Desc{};
	Desc.pOwner = this;
	Desc.fHeight = (_float)m_tLadderDesc.iLength * m_fSegmentStepY;
	Desc.fRadius = 0.75f;
	Desc.vCenter = _float3(0.f, -0.75f, -1.f);

	m_pCollider = Add_Component<CCollider>(Collider_Capsule.iLevelID, Collider_Capsule.szProtoTag, TEXT("Com_Collider"), &Desc);
	if (nullptr == m_pCollider)
		return E_FAIL;

	m_pGameInstance_Proxy->Register_Collider(m_pCollider, ETOUI(COLLISION_LAYER::ENV_LADDER));

	return S_OK;
}

HRESULT CLevelDesign_Ladder::Resolve_SegmentStepY()
{
	if (nullptr == m_ModelComs[SEGMENT::MID])
		return E_FAIL;

	_float3 vMin = {};
	_float3 vMax = {};
	m_ModelComs[SEGMENT::MID]->Get_ModelAABB(&vMin, &vMax);

	if (vMin.y > vMax.y)
		return E_FAIL;

	m_fSegmentStepY = vMax.y - vMin.y;

	return S_OK;
}

HRESULT CLevelDesign_Ladder::Bind_ShaderResources(const _float4x4& WorldMatrix)
{
	if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", &WorldMatrix)))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::VIEW, m_eProjType))))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::PROJ, m_eProjType))))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_RawValue("g_iMaterialID", &m_iMaterialID, sizeof(_uint))))
		return E_FAIL;

	const _float4 vEmissiveColor = { 0.f, 0.f, 0.f, 0.f };
	if (FAILED(m_pShaderCom->Bind_RawValue("g_vEmissiveColor", &vEmissiveColor, sizeof(_float4))))
		return E_FAIL;


	return S_OK;
}

HRESULT CLevelDesign_Ladder::Render_Model(CModel* pModel)
{
	const _uint iNumMeshes = static_cast<_uint>(pModel->Get_NumMeshes());

	for (_uint i = 0; i < iNumMeshes; ++i)
	{
		const MESH_LAYER_IDX& Layer = pModel->Get_MeshLayer(i);

		auto BindMaterial = [&](const _char* pConstantName, MTEX_TYPE eType, DEFAULT_TEXTURE eDefaultKind) -> HRESULT
			{
				const _uint iLayerIndex = Layer.idx[ETOUI(eType)];
				const _uint iTextureCount = pModel->Get_MeshTextureCount(i, eType);

				if (0u < iTextureCount)
				{
					const _uint iSafeIndex = (iLayerIndex < iTextureCount) ? iLayerIndex : (iTextureCount - 1u);

					if (SUCCEEDED(pModel->Bind_Material(m_pShaderCom, pConstantName, i, eType, iSafeIndex)))
						return S_OK;
				}

				return m_pGameInstance_Proxy->Bind_DefaultTextureFromHub(m_pShaderCom, pConstantName, eDefaultKind);
			};

		if (FAILED(BindMaterial("g_DiffuseTexture", MTEX_TYPE::DIFFUSE, DEFAULT_TEXTURE::MAGENTA)))             return E_FAIL;
		if (FAILED(BindMaterial("g_NormalTexture", MTEX_TYPE::NORMALS, DEFAULT_TEXTURE::FLAT_NORMAL)))			return E_FAIL;
		if (FAILED(BindMaterial("g_MRATexture", MTEX_TYPE::METALNESS, DEFAULT_TEXTURE::MRA)))                   return E_FAIL;
		if (FAILED(BindMaterial("g_UnknownTexture", MTEX_TYPE::UNKNOWN, DEFAULT_TEXTURE::BLACK)))               return E_FAIL;

		const _uint iUVIndex = (Layer.iUVIndex <= 3u) ? Layer.iUVIndex : 0u;
		_uint iFlags = Layer.iFlags;
		_float fDissolve = 0.f;

		if (FAILED(m_pShaderCom->Bind_RawValue("g_iUVIndex", &iUVIndex, sizeof(_uint))))                        return E_FAIL;
		if (FAILED(m_pShaderCom->Bind_RawValue("g_iEnvInstanceFlags", &iFlags, sizeof(_uint))))					return E_FAIL;
		if (FAILED(m_pShaderCom->Bind_RawValue("g_fDissolve", &fDissolve, sizeof(_float))))                     return E_FAIL;

		if (FAILED(m_pShaderCom->Begin(ShaderPass::NonAnimPBR::DMN)))
			return E_FAIL;
		if (FAILED(pModel->Render(i)))
			return E_FAIL;
	}

	return S_OK;
}

HRESULT CLevelDesign_Ladder::Calculate_TopBottomCellIndices()
{
	if (m_tLadderDesc.iLength == 0 || m_fSegmentStepY <= 0.f)
		return E_FAIL;

	// 모델 세그먼트 하나를 이동 셀 한 칸으로 사용한다.
	m_fCellSpacing = m_fSegmentStepY;

	m_iBottomCellIndex = 0;
	m_iTopCellIndex = static_cast<_int>(m_tLadderDesc.iLength) - 1;

	return S_OK;
}

CLevelDesign_Ladder* CLevelDesign_Ladder::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CLevelDesign_Ladder* pInstance = new CLevelDesign_Ladder(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CLevelDesign_Ladder");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CLevelDesign_Ladder::Clone(void* pArg)
{
	CLevelDesign_Ladder* pInstance = new CLevelDesign_Ladder(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CLevelDesign_Ladder");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CLevelDesign_Ladder::Free()
{
	__super::Free();
}

NS_END
