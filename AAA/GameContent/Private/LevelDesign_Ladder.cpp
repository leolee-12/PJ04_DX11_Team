#include "LevelDesign_Ladder.h"
#include "Shader_PassMeta.h"
#include "Parsing_Utils.h"

#include "GameInstance.h"

NS_BEGIN(Client)

CLevelDesign_Ladder::CLevelDesign_Ladder(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CLevelDesignObject(pDevice, pContext)
{
}

CLevelDesign_Ladder::CLevelDesign_Ladder(const CLevelDesign_Ladder& Prototype)
	: CLevelDesignObject(Prototype)
	, m_tLadderDesc(Prototype.m_tLadderDesc)
	, m_iModelProtoLevel(Prototype.m_iModelProtoLevel)
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

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	m_tLadderDesc = *static_cast<const LD_LADDER_DESC*>(pArg);

	if (FAILED(Validate_Desc()))
		return E_FAIL;

	if (FAILED(Ready_Components()))
		return E_FAIL;

	if (FAILED(Resolve_SegmentStepY()))
		return E_FAIL;

	return S_OK;
}

void CLevelDesign_Ladder::Late_Update(_float fTimeDelta)
{
	UNREFERENCED_PARAMETER(fTimeDelta);

	m_pGameInstance_Proxy->Add_RenderGroup(RENDERID::NONBLEND, this);
}

HRESULT CLevelDesign_Ladder::Render()
{
	if (nullptr == m_pShaderCom)
		return S_OK;

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

HRESULT CLevelDesign_Ladder::Validate_Desc()
{
	if (0u == m_tLadderDesc.iLength)
		return E_FAIL;

	return S_OK;
}

HRESULT CLevelDesign_Ladder::Ready_Components()
{
	m_pShaderCom = Add_Component<CShader>(Shader_NonAnimMesh_PBR.iLevelID, Shader_NonAnimMesh_PBR.szProtoTag, TEXT("Com_Shader"));
	if (nullptr == m_pShaderCom)
		return E_FAIL;

	m_ModelComs[SEGMENT::BOT] = Add_Component<CModel>(m_iModelProtoLevel, BOT_MODEL_PROTO_TAG, TEXT("Com_Model_Bottom"));
	if (nullptr == m_ModelComs[SEGMENT::BOT])
		return E_FAIL;

	m_ModelComs[SEGMENT::MID] = Add_Component<CModel>(m_iModelProtoLevel, MID_MODEL_PROTO_TAG, TEXT("Com_Model_Middle"));
	if (nullptr == m_ModelComs[SEGMENT::MID])
		return E_FAIL;

	m_ModelComs[SEGMENT::TOP] = Add_Component<CModel>(m_iModelProtoLevel, TOP_MODEL_PROTO_TAG, TEXT("Com_Model_Top"));
	if (nullptr == m_ModelComs[SEGMENT::TOP])
		return E_FAIL;

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