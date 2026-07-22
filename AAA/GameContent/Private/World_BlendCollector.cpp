#include "World_BlendCollector.h"

#include "GameInstance_Proxy.h"
#include "Model.h"

NS_BEGIN(Client)

CWorld_BlendCollector::CWorld_BlendCollector(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject(pDevice, pContext)
{
}

CWorld_BlendCollector::CWorld_BlendCollector(const CWorld_BlendCollector& Prototype)
	: CGameObject(Prototype)
{
}

void CWorld_BlendCollector::Submit(IBlendRenderable* pOwner, CGameObject* pRefOwner, CModel* pModel, const _float4x4* pWorld, _uint iMesh)
{
	if (nullptr == pOwner || nullptr == pRefOwner || nullptr == pModel || nullptr == pWorld || nullptr == m_pGameInstance_Proxy)
		return;

	if (iMesh >= pModel->Get_NumMeshes())
		return;

	BeginFrame_IfNeeded(m_pGameInstance_Proxy->Get_FrameIndex());

	_float3 vMin = {};
	_float3 vMax = {};
	pModel->Get_MeshAABB(iMesh, &vMin, &vMax);

	BLEND_SUBMIT_DATA Data = {};
	Data.pOwner = pOwner;
	Data.pRefOwner = pRefOwner;
	Data.iMesh = iMesh;

	const _float4* pCamPosition = m_pGameInstance_Proxy->Get_CamPosition();
	const _float4* pCamLook = m_pGameInstance_Proxy->Get_CamLook();

	if (nullptr != pCamPosition && nullptr != pCamLook)
	{
		const _vector vMeshCenter = XMVectorSet(
			(vMin.x + vMax.x) * 0.5f,
			(vMin.y + vMax.y) * 0.5f,
			(vMin.z + vMax.z) * 0.5f,
			1.f);
		const _vector vWorldCenter = XMVector3TransformCoord(vMeshCenter, XMLoadFloat4x4(pWorld));
		const _vector vToMesh = vWorldCenter - XMLoadFloat4(pCamPosition);

		Data.fViewDepth = XMVectorGetX(XMVector3Dot(vToMesh, XMVector3Normalize(XMLoadFloat4(pCamLook))));
	}

	Safe_AddRef(Data.pRefOwner);
	m_Submitted.push_back(Data);

	if (!m_bRegisteredThisFrame)
	{
		m_bRegisteredThisFrame = true;
		m_pGameInstance_Proxy->Add_RenderGroup(RENDERID::BLEND_HDR, this);
	}
}

HRESULT CWorld_BlendCollector::Render()
{
	sort(m_Submitted.begin(), m_Submitted.end(),
		[](const BLEND_SUBMIT_DATA& Left, const BLEND_SUBMIT_DATA& Right)
		{
			return Left.fViewDepth > Right.fViewDepth;
		});

	HRESULT hr = S_OK;

	for (const BLEND_SUBMIT_DATA& Data : m_Submitted)
	{
		if (nullptr == Data.pOwner)
		{
			hr = E_FAIL;
			continue;
		}

		if (FAILED(Data.pOwner->Render_BlendMesh(Data.iMesh)))
			hr = E_FAIL;
	}

	Clear_Submissions();
	m_bRegisteredThisFrame = false;

	return hr;
}

void CWorld_BlendCollector::Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData)
{
	if (nullptr == pOutData)
		return;

	pOutData->strPrototypeTag = PROTOTYPE_TAG;
}

CWorld_BlendCollector* CWorld_BlendCollector::Find(CGameInstance_Proxy* pProxy)
{
	if (nullptr == pProxy)
		return nullptr;

	return pProxy->Find_GameObject<CWorld_BlendCollector>(ETOUI(LEVEL::STATIC), LAYER_TAG, OBJECT_TAG);
}

void CWorld_BlendCollector::BeginFrame_IfNeeded(_int64 iCurrentFrame)
{
	if (m_iLastSubmitFrame == iCurrentFrame)
		return;

	Clear_Submissions();
	m_bRegisteredThisFrame = false;
	m_iLastSubmitFrame = iCurrentFrame;
}

void CWorld_BlendCollector::Clear_Submissions()
{
	for (BLEND_SUBMIT_DATA& Data : m_Submitted)
		Safe_Release(Data.pRefOwner);

	m_Submitted.clear();
}

CWorld_BlendCollector* CWorld_BlendCollector::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CWorld_BlendCollector* pInstance = new CWorld_BlendCollector(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CWorld_BlendCollector");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CWorld_BlendCollector::Clone(void* pArg)
{
	CWorld_BlendCollector* pInstance = new CWorld_BlendCollector(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CWorld_BlendCollector");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CWorld_BlendCollector::Free()
{
	Clear_Submissions();

	__super::Free();
}

NS_END