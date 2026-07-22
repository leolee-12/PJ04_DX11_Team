#include "EnvObject_Static.h"
#include "Env_InstanceController.h"
#include "World_BlendCollector.h"

#include "GameInstance_Proxy.h"

NS_BEGIN(Client)

CEnvObject_Static::CEnvObject_Static(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEnvObject(pDevice, pContext)
{
	m_strProtoTag = PROTOTYPE_TAG;
}

CEnvObject_Static::CEnvObject_Static(const CEnvObject_Static& Prototype)
	: CEnvObject(Prototype)
{
	m_strProtoTag = PROTOTYPE_TAG;
}

HRESULT CEnvObject_Static::Initialize_Prototype()
{
	return __super::Initialize_Prototype();
}

HRESULT CEnvObject_Static::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_RenderComponents(m_tDesc.iModelProtoLevel, m_tDesc.wstrModelProtoTag)))
		return E_FAIL;

	Cache_BlendMeshIndices();

	m_pBlendCollector = CWorld_BlendCollector::Find(m_pGameInstance_Proxy);
	if (nullptr == m_pBlendCollector)
		return E_FAIL;

	if (FAILED(Rebuild_PhysicsActor()))
		return E_FAIL;

	return S_OK;
}

void CEnvObject_Static::Late_Update(_float fTimeDelta)
{
	Refresh_WorldBounds();

	__super::Late_Update(fTimeDelta);

	Check_Visible();
	Submit_RenderGroups();
	Submit_BlendMeshes();
}

HRESULT CEnvObject_Static::Render_BlendMesh(_uint iMeshIndex)
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	return Render_Mesh(iMeshIndex, MESH_LAYER_RENDER_KIND::MAIN_BLEND);
}

void CEnvObject_Static::Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData)
{
	__super::Copy_PrototypeName(pOutData);
}

#pragma region Editable
HRESULT CEnvObject_Static::Apply_EditMeshLayer(_uint iModelSlot, _uint iMesh, const MESH_LAYER_IDX& Layer)
{
	if (FAILED(__super::Apply_EditMeshLayer(iModelSlot, iMesh, Layer)))
		return E_FAIL;

	Cache_BlendMeshIndices();

	if (nullptr == m_pInstanceController)
		return S_OK;

	return m_pInstanceController->Apply_ModelMeshLayer(m_tDesc.iModelProtoLevel, m_tDesc.wstrModelProtoTag, iMesh, Layer);
}
#pragma endregion

void CEnvObject_Static::Set_InstanceController(CEnv_InstanceController* pCtrl)
{
	Safe_Release(m_pInstanceController);
	m_pInstanceController = pCtrl;
	Safe_AddRef(m_pInstanceController);

	m_InstanceBatchHandle = {};

	if (nullptr != m_pInstanceController && Can_RenderInstance())
	{
		m_InstanceBatchHandle = m_pInstanceController->Register_BatchesForDesc(m_tDesc);
	}
}

_bool CEnvObject_Static::Can_RenderInstance() const
{
	if (!m_bRenderable)
		return false;

	if (!Has_RenderModel())
		return false;

	if (m_tDesc.wstrModelProtoTag.empty())
		return false;

	return true;
}

void CEnvObject_Static::Submit_RenderGroups()
{
	if (m_bVisible)
	{
		if (m_bIsDecal)
		{
			_bool bSubmitted = false;
			if (Can_RenderInstance() && nullptr != m_pInstanceController)
			{
				bSubmitted = m_pInstanceController->Submit_Decal(m_InstanceBatchHandle.iDecalBatchIndex, this);
			}
			
			if (!bSubmitted)
				m_pGameInstance_Proxy->Add_RenderGroup(RENDERID::DECAL, this);
		}
		else
		{
			_bool bSubmitted = false;
			const _bool bBypassMainInstance = Should_BypassMainInstance();
			if (!bBypassMainInstance && Can_RenderInstance() && nullptr != m_pInstanceController)
			{
				bSubmitted = m_pInstanceController->Submit_Main(m_InstanceBatchHandle.iMainBatchIndex, this);
			}

			if (!bSubmitted)
				m_pGameInstance_Proxy->Add_RenderGroup(RENDERID::NONBLEND, this);
		}
	}

	if (!m_bIsDecal && m_bVisibleShadow)
	{
		_bool bSubmittedShadow = false;

		if (Can_RenderInstance() && nullptr != m_pInstanceController)
		{
			if (INVALID_INDEX == m_InstanceBatchHandle.iShadowBatchIndex)
			{
				m_InstanceBatchHandle.iShadowBatchIndex = m_pInstanceController->Register_ShadowBatch(m_tDesc);
			}

			bSubmittedShadow = m_pInstanceController->Submit_Shadow(m_InstanceBatchHandle.iShadowBatchIndex, this);
		}

		if (!bSubmittedShadow)
			m_pGameInstance_Proxy->Add_RenderGroup(RENDERID::SHADOW, this);
	}
}

_bool CEnvObject_Static::Should_BypassMainInstance() const
{
	return m_bEditorForceMainPassNonInstanced;
}

void CEnvObject_Static::Cache_BlendMeshIndices()
{
	m_BlendMeshIndices.clear();

	const _uint iNumMeshes = static_cast<_uint>(m_pModelCom->Get_NumMeshes());

	for (_uint i = 0; i < iNumMeshes; ++i)
	{
		if (Is_WorldBlendPass(m_pModelCom->Get_MeshLayer(i).iPass))
			m_BlendMeshIndices.push_back(i);
	}
}

void CEnvObject_Static::Submit_BlendMeshes()
{
	if (!m_bVisible || m_bIsDecal || m_BlendMeshIndices.empty())
		return;

	if (m_bUseCameraDither && m_fDissolve >= 0.999f)
		return;

	const _float4x4* pWorld = m_pTransformCom->Get_WorldMatrixPtr();

	for (_uint iMeshIndex : m_BlendMeshIndices)
		m_pBlendCollector->Submit(this, this, m_pModelCom, pWorld, iMeshIndex);
}

CEnvObject_Static* CEnvObject_Static::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CEnvObject_Static* pInstance = new CEnvObject_Static(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CEnvObject_Static");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CEnvObject_Static::Clone(void* pArg)
{
	CEnvObject_Static* pInstance = new CEnvObject_Static(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CEnvObject_Static");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CEnvObject_Static::Free()
{
	Safe_Release(m_pInstanceController);

	__super::Free();
}

NS_END
