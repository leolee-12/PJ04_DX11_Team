#include "TestMap.h"
#include "GameInstance.h"
#include "GameContent_const.h"

CTestMap::CTestMap(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const _wstring& strPrototypeTag, const _wstring& strModelTag)
	: CGameObject{ pDevice, pContext }
	, m_wstrPrototypeTag{ strPrototypeTag }
	, m_wstrModelTag{ strModelTag }
{
}

CTestMap::CTestMap(const CTestMap& Prototype)
	: CGameObject(Prototype)
	, m_wstrPrototypeTag{ Prototype.m_wstrPrototypeTag }
	, m_wstrModelTag{ Prototype.m_wstrModelTag }
{
}

HRESULT CTestMap::Initialize_Prototype()
{
	m_eProjType = PROJ_TYPE::PERSPEC;
	return S_OK;
}

HRESULT CTestMap::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_Components()))
		return E_FAIL;

	return S_OK;
}

void CTestMap::Priority_Update(_float fTimeDelta)
{

}

void CTestMap::Update(_float fTimeDelta)
{
}

void CTestMap::Late_Update(_float fTimeDelta)
{
	m_pGameInstance_Proxy->Add_RenderGroup(RENDERID::NONBLEND, this);
}

HRESULT CTestMap::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	size_t      iNumMeshes = m_pModelCom->Get_NumMeshes();

	for (size_t i = 0; i < iNumMeshes; i++)
	{
		/*Texture2D g_DiffuseTexture;
		Texture2D g_NormalTexture;
		Texture2D g_UnknownTexture;
		Texture2D g_MRATexture;*/

		_uint iPassIdx = 0;
		if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_DiffuseTexture", (_uint)i, MTEX_TYPE::DIFFUSE, 0)))
			continue;

		if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_NormalTexture", (_uint)i, MTEX_TYPE::NORMALS, 0)))
			continue;

		if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_MRATexture", (_uint)i, MTEX_TYPE::METALNESS, 0)))
			continue;

		if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_UnknownTexture", (_uint)i, MTEX_TYPE::UNKNOWN, 0)))
			continue;

		if (FAILED(m_pShaderCom->Begin(1)))
			return E_FAIL;

		if (FAILED(m_pModelCom->Render((_uint)i)))
			return E_FAIL;
	}

	return S_OK;
}

HRESULT CTestMap::Ready_Components()
{
	m_pShaderCom = Add_Component<CShader>(Shader_NonAnimMesh_PBR.iLevelID, Shader_NonAnimMesh_PBR.szProtoTag, TEXT("Com_Shader"));
	if (nullptr == m_pShaderCom)
		return E_FAIL;

	m_pModelCom = Add_Component<CModel>(m_iPrototypeLevel, m_wstrModelTag, TEXT("Com_Model"));
	if (nullptr == m_pModelCom)
		return E_FAIL;

	return S_OK;
}

HRESULT CTestMap::Bind_ShaderResources()
{

	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::VIEW, m_eProjType))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::PROJ, m_eProjType))))
		return E_FAIL;

	return S_OK;
}


CTestMap* CTestMap::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const _wstring& strPrototypeTag, const _wstring& strModelTag)
{
	CTestMap* pInstance = new CTestMap(pDevice, pContext, strPrototypeTag, strModelTag);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CTestMap");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CTestMap::Clone(void* pArg)
{
	CTestMap* pInstance = new CTestMap(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CTestMap");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CTestMap::Free()
{
	__super::Free();
}
