#include "MapObject.h"
#include "Shader_PassMeta.h"

#include "GameInstance.h"

CMapObject::CMapObject(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{
}

CMapObject::CMapObject(const CMapObject& Prototype)
	: CGameObject(Prototype)
{
}

HRESULT CMapObject::Initialize_Prototype()
{
	m_eProjType = PROJ_TYPE::PERSPEC;
	return S_OK;
}

HRESULT CMapObject::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_MapComponents()))
		return E_FAIL;

	return S_OK;
}

void CMapObject::Priority_Update(_float fTimeDelta) {}
void CMapObject::Update(_float fTimeDelta) {}

void CMapObject::Late_Update(_float fTimeDelta)
{
	m_pGameInstance_Proxy->Add_RenderGroup(RENDERID::NONBLEND, this);
	m_pGameInstance_Proxy->Add_RenderGroup(RENDERID::SHADOW, this);
}

HRESULT CMapObject::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	_uint n = static_cast<_uint>(m_pModelCom->Get_NumMeshes());

	for (_uint i = 0; i < n; ++i)
	{
		if (!Should_RenderMesh(i))
			continue;

		const MESH_LAYER_IDX& Layer = m_pModelCom->Get_MeshLayer(i);

		if (FAILED(Bind_MapMeshParams(i, Layer)))
			return E_FAIL;

		if (FAILED(Bind_MapMeshTextures(i, Layer)))
			return E_FAIL;

		_int iPass = (Layer.iPass >= 0)
			? Layer.iPass
			: ETOI(MAP_DEFAULT_PASS);

		if (!Is_ValidMapPassValue(iPass))
			iPass = ETOI(MAP_DEFAULT_PASS);

		Bind_MeshLayers(i);

		if (FAILED(m_pShaderCom->Begin(iPass)))
			return E_FAIL;
		if (FAILED(m_pModelCom->Render(i)))
			return E_FAIL;
	}

	return S_OK;
}

HRESULT CMapObject::Bind_WorldMatrix()
{
	return m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix");
}

HRESULT CMapObject::Ready_MapComponents()
{
	m_pShaderCom = Add_Component<CShader>(Shader_MapEx.iLevelID, Shader_MapEx.szProtoTag, TEXT("Com_Shader"));
	if (nullptr == m_pShaderCom)
		return E_FAIL;

	m_pModelCom = Add_Component<CModel>(Get_ModelProtoLevel(), Get_ModelProtoTag(), TEXT("Com_Model"));
	if (nullptr == m_pModelCom)
		return E_FAIL;

	return S_OK;
}

HRESULT CMapObject::Bind_ShaderResources()
{
	if (FAILED(Bind_WorldMatrix()))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::VIEW, m_eProjType))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::PROJ, m_eProjType))))
		return E_FAIL;

	return S_OK;
}

HRESULT CMapObject::Bind_MapMeshParams(_uint iMesh, const MESH_LAYER_IDX& Layer)
{
	UNREFERENCED_PARAMETER(iMesh);

	const _uint iBase_UVIndex = (Layer.iUVIndex <= 3u) ? Layer.iUVIndex : 0u;
	const _uint iUnknown_UVIndex = (Layer.iUnknownUVIndex <= 3u) ? Layer.iUnknownUVIndex : 0u;
	const _uint iExtraR_UVIndex = (Layer.iExtraUVIndex[0] <= 3u) ? Layer.iExtraUVIndex[0] : 0u;
	const _uint iExtraG_UVIndex = (Layer.iExtraUVIndex[1] <= 3u) ? Layer.iExtraUVIndex[1] : 0u;
	const _uint iExtraB_UVIndex = (Layer.iExtraUVIndex[2] <= 3u) ? Layer.iExtraUVIndex[2] : 0u;
	const _uint iExtraA_UVIndex = (Layer.iExtraUVIndex[3] <= 3u) ? Layer.iExtraUVIndex[3] : 0u;

	_float4 vUVTransform;
	_float4 vUVTransformNormal;
	_float4 vUVTransformMaterial;

	if (Layer.bUseUVTransform)
	{
		vUVTransform = _float4{ Layer.vUVScale.x, Layer.vUVScale.y, Layer.vUVOffset.x, Layer.vUVOffset.y };
		vUVTransformNormal = _float4{ Layer.vUVScaleNormal.x, Layer.vUVScaleNormal.y, Layer.vUVOffset.x, Layer.vUVOffset.y };
		vUVTransformMaterial = _float4{ Layer.vUVScaleMaterial.x, Layer.vUVScaleMaterial.y, Layer.vUVOffset.x, Layer.vUVOffset.y };
	}
	else
	{
		vUVTransform = vUVTransformNormal = vUVTransformMaterial = _float4{ 1.f, 1.f, 0.f, 0.f };
	}

	//const _float4 vUVTransform =
	//	Layer.bUseUVTransform
	//	? _float4{ Layer.vUVScale.x, Layer.vUVScale.y, Layer.vUVOffset.x, Layer.vUVOffset.y }
	//: _float4{ 1.f, 1.f, 0.f, 0.f };
	//
	//const _float4 vUVTransformNormal = 
	//	Layer.bUseUVTransform 
	//	? _float4{ Layer.vUVScaleNormal.x, Layer.vUVScaleNormal.y, Layer.vUVOffset.x, Layer.vUVOffset.y }
	//: _float4{ 1.f, 1.f, 0.f, 0.f };
	//
	//const _float4 vUVTransformMaterial = 
	//	Layer.bUseUVTransform
	//	? _float4{ Layer.vUVScaleMaterial.x, Layer.vUVScaleMaterial.y, Layer.vUVOffset.x, Layer.vUVOffset.y }
	//: _float4{ 1.f, 1.f, 0.f, 0.f };



	const _float fUVRotate = Layer.bUseUVTransform ? Layer.fUVRotate : 0.f;
	const _float fNormalStrength = Layer.fNormalStrength;
	const _float fMaskStrength = Layer.fMaskStrength;

	const _float4 vExtraEnable =
	{
		Layer.iExtraBind[0] >= 0 ? 1.f : 0.f,
		Layer.iExtraBind[1] >= 0 ? 1.f : 0.f,
		Layer.iExtraBind[2] >= 0 ? 1.f : 0.f,
		Layer.iExtraBind[3] >= 0 ? 1.f : 0.f
	};

	if (FAILED(m_pShaderCom->Bind_RawValue("g_iBase_UVIndex", &iBase_UVIndex, sizeof(_uint))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_iUnknown_UVIndex", &iUnknown_UVIndex, sizeof(_uint))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_iExtraR_UVIndex", &iExtraR_UVIndex, sizeof(_uint))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_iExtraG_UVIndex", &iExtraG_UVIndex, sizeof(_uint))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_iExtraB_UVIndex", &iExtraB_UVIndex, sizeof(_uint))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_iExtraA_UVIndex", &iExtraA_UVIndex, sizeof(_uint))))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_RawValue("g_vUVTransform", &vUVTransform, sizeof(_float4))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_vUVTransformNormal", &vUVTransformNormal, sizeof(_float4))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_vUVTransformMaterial", &vUVTransformMaterial, sizeof(_float4))))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_RawValue("g_fUVRotate", &fUVRotate, sizeof(_float))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_NormalStrength", &fNormalStrength, sizeof(_float))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_MaskStrength", &fMaskStrength, sizeof(_float))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_vExtraEnable", &vExtraEnable, sizeof(_float4))))
		return E_FAIL;

	return S_OK;
}

HRESULT CMapObject::Bind_MapMeshTextures(_uint iMesh, const MESH_LAYER_IDX& Layer)
{
	if (FAILED(Bind_MapTextureSafe(iMesh, "g_DiffuseTexture", MTEX_TYPE::DIFFUSE, Layer.idx[ETOUI(MTEX_TYPE::DIFFUSE)], DEFAULT_TEXTURE::MAGENTA)))
		return E_FAIL;
	if (FAILED(Bind_MapTextureSafe(iMesh, "g_NormalTexture", MTEX_TYPE::NORMALS, Layer.idx[ETOUI(MTEX_TYPE::NORMALS)], DEFAULT_TEXTURE::FLAT_NORMAL)))
		return E_FAIL;
	if (FAILED(Bind_MapTextureSafe(iMesh, "g_MRATexture", MTEX_TYPE::METALNESS, Layer.idx[ETOUI(MTEX_TYPE::METALNESS)], DEFAULT_TEXTURE::MRA)))
		return E_FAIL;
	if (FAILED(Bind_MapTextureSafe(iMesh, "g_UnknownTexture", MTEX_TYPE::UNKNOWN, Layer.idx[ETOUI(MTEX_TYPE::UNKNOWN)], DEFAULT_TEXTURE::BLACK)))
		return E_FAIL;

	if (FAILED(Bind_MapExtraSlotSafe(iMesh, "g_ExtraRTexture", Layer.iExtraBind[0], (MTEX_TYPE)Layer.iExtraTexType[0],
DEFAULT_TEXTURE::BLACK)))
		return E_FAIL;
	if (FAILED(Bind_MapExtraSlotSafe(iMesh, "g_ExtraGTexture", Layer.iExtraBind[1], (MTEX_TYPE)Layer.iExtraTexType[1],
		DEFAULT_TEXTURE::BLACK)))
		return E_FAIL;
	if (FAILED(Bind_MapExtraSlotSafe(iMesh, "g_ExtraBTexture", Layer.iExtraBind[2], (MTEX_TYPE)Layer.iExtraTexType[2],
		DEFAULT_TEXTURE::BLACK)))
		return E_FAIL;
	if (FAILED(Bind_MapExtraSlotSafe(iMesh, "g_ExtraATexture", Layer.iExtraBind[3], (MTEX_TYPE)Layer.iExtraTexType[3],
		DEFAULT_TEXTURE::BLACK)))
		return E_FAIL;

	return S_OK;
}

HRESULT CMapObject::Bind_MapTextureSafe(_uint iMesh, const _char* pName, MTEX_TYPE eType, _uint iSlot, DEFAULT_TEXTURE eDefault)
{
	const _uint iTextureCount = m_pModelCom->Get_MeshTextureCount(iMesh, eType);

	if (iTextureCount > 0u)
	{
		const _uint iSafeSlot = (iSlot < iTextureCount) ? iSlot : (iTextureCount - 1u);

		if (SUCCEEDED(m_pModelCom->Bind_Material(m_pShaderCom, pName, iMesh, eType, iSafeSlot)))
			return S_OK;
	}

	return m_pGameInstance_Proxy->Bind_DefaultTextureFromHub(m_pShaderCom, pName, eDefault);
}

HRESULT CMapObject::Bind_MapExtraSlotSafe(_uint iMesh, const _char* pName, int iSlot, MTEX_TYPE eType, DEFAULT_TEXTURE eDefault)
{
	if (iSlot < 0)
		return m_pGameInstance_Proxy->Bind_DefaultTextureFromHub(m_pShaderCom, pName, eDefault);

	const _uint iTextureCount = m_pModelCom->Get_MeshTextureCount(iMesh, eType);

	if (iTextureCount > 0u)
	{
		const _uint iBindSlot = static_cast<_uint>(iSlot);
		const _uint iSafeSlot = (iBindSlot < iTextureCount) ? iBindSlot : (iTextureCount - 1u);

		if (SUCCEEDED(m_pModelCom->Bind_Material(m_pShaderCom, pName, iMesh, eType, iSafeSlot)))
			return S_OK;
	}

	return m_pGameInstance_Proxy->Bind_DefaultTextureFromHub(m_pShaderCom, pName, eDefault);
}

void CMapObject::Free()
{
	__super::Free();
}
