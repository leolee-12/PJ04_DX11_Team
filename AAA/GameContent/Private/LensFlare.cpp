#include "LensFlare.h"
#include "GameContent_const.h"
#include "GameContent_Log.h"
#include "RectEmitterCommon.h"
#include "MeshEmitterCommon.h"

#include "Math_Utils.h"

namespace
{
	constexpr _float LENS_AUTHOR_AXIS_ORIGIN = 40.f;
	constexpr _float LENS_AUTHOR_AXIS_UNIT = 35.f;
	constexpr _float LENS_FADE_IN_DURATION = 1.5f;
}

CLensFlare::CLensFlare(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEffect_Container(pDevice, pContext)
	, m_bUseScreenAxis{ true }
	, m_fAxisExtent{ 1.f }
	, m_fGhostViewDepth{ 60.f }
	, m_fScreenShowMargin{ 1.15f }
	, m_fScreenHideMargin{ 1.25f }
	, m_bScreenVisible{ false }
	, m_fAxisRotationDegree{ 0.f }
{
}

CLensFlare::CLensFlare(const CLensFlare& Prototype)
	: CEffect_Container(Prototype)
	, m_bUseScreenAxis{ Prototype.m_bUseScreenAxis }
	, m_fAxisExtent{ Prototype.m_fAxisExtent }
	, m_fGhostViewDepth{ Prototype.m_fGhostViewDepth }
	, m_fScreenShowMargin{ Prototype.m_fScreenShowMargin }
	, m_fScreenHideMargin{ Prototype.m_fScreenHideMargin }
	, m_bScreenVisible{ false }
	, m_fAxisRotationDegree{ Prototype.m_fAxisRotationDegree }
{
}

HRESULT CLensFlare::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CLensFlare::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_EffectPartObjects()))
		return E_FAIL;

	return S_OK;
}

void CLensFlare::On_Deserialized()
{
	__super::On_Deserialized();
	Cache_LensElements();
}

void CLensFlare::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);
}

void CLensFlare::Update(_float fTimeDelta)
{
	if (m_bLensElementCacheReady == false)
		Cache_LensElements();

	Update_FadeIn(fTimeDelta);
	Apply_FadeInAlpha();

	__super::Update(fTimeDelta);

	Restore_EmitterAlpha();
}

void CLensFlare::Late_Update(_float fTimeDelta)
{
	if (m_bIsPlay == false)
		return;

	Compute_CombinedWorldMatrix();

	if (m_bLensElementCacheReady == false)
		Cache_LensElements();

	if (m_bLensElementCacheReady == false)
		return;

	if (Update_LensFlarePlacement() == false)
	{
		if (m_bUseScreenAxis == true && m_bFadeOutRequested == false)
			Queue_FadeIn();

		return;
	}

	__super::Late_Update(fTimeDelta);
}

HRESULT CLensFlare::Render()
{
	return __super::Render();
}

json CLensFlare::Serialize() const
{
	json j = __super::Serialize();

	if (m_bLensElementCacheReady == false || m_bAuthorPlacementRestored == true || j.contains("EffectPartObjects") == false)
		return j;

	for (const auto& [strTag, Element] : m_LensElements)
	{
		const string strKey = WstrToStr(strTag);

		if (j["EffectPartObjects"].contains(strKey) == false)
			continue;

		const _float3& vAuthorPosition = Element.vAuthorLocalPosition;
		j["EffectPartObjects"][strKey]["Transform"]["vPosition"] = { vAuthorPosition.x, vAuthorPosition.y, vAuthorPosition.z, 1.f };
	}

	return j;
}

HRESULT CLensFlare::Ready_EffectPartObjects()
{
	CRectEmitterCommon::RECT_EMITTER_COMMON_DESC tRectDesc{};
	tRectDesc.iVIBufferLevel = VI_Rect.iLevelID;
	tRectDesc.wstrVIBufferTag = VI_Rect.szProtoTag;
	tRectDesc.bUseTextureCom = true;
	tRectDesc.iTextureLevel = m_iPrototypeLevel;
	tRectDesc.bUseMaskCom = false;
	tRectDesc.bCustomShader = false;

	// circleglow2 는 알파가 전 픽셀 0 → 마스크 슬롯에서 Red 채널을 알파원으로 사용
	tRectDesc.bUseMaskCom = true;
	tRectDesc.iMaskLevel = m_iPrototypeLevel;
	tRectDesc.wstrMaskTag = TEXT("Prototype_Component_Texture_LensFlare_CircleGlow2");
	tRectDesc.wstrTextureTag = TEXT("Prototype_Component_Texture_LensFlare_CircleGlow2");
	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CRectEmitterCommon::PROTOTYPE_TAG, TEXT("Line"), &tRectDesc)))
		return E_FAIL;

	tRectDesc.bUseMaskCom = false;
	tRectDesc.wstrMaskTag = L"";
	tRectDesc.wstrTextureTag = TEXT("Prototype_Component_Texture_LensFlare_Common_Circle06");
	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CRectEmitterCommon::PROTOTYPE_TAG, TEXT("Circle1"), &tRectDesc)))
		return E_FAIL;

	CMeshEmitterCommon::MESH_EMITTER_COMMON_DESC tMeshDesc{};
	tMeshDesc.iModelLevel = m_iPrototypeLevel;
	tMeshDesc.bUseDiffuseTexture = false;
	tMeshDesc.bUseUnknownTexture = true;
	tMeshDesc.bUseNormalTexture = false;
	tMeshDesc.bUseMRATexture = false;
	tMeshDesc.bUseTextureCom = true;
	tMeshDesc.iTextureLevel = m_iPrototypeLevel;
	tMeshDesc.bUseMaskCom = true;
	tMeshDesc.iMaskLevel = m_iPrototypeLevel;
	tMeshDesc.bCustomShader = false;
	tMeshDesc.iShaderLevel = Shader_SpecialEffect.iLevelID;
	tMeshDesc.wstrShaderTag = Shader_SpecialEffect.szProtoTag;

	tMeshDesc.wstrModelTag = TEXT("Prototype_Component_Model_LensFlare_Common_Circle01");
	// circlegradation 은 알파가 전 픽셀 0 → 마스크 슬롯으로 옮겨 Red 채널을 알파원으로 사용
	tMeshDesc.wstrTextureTag = TEXT("Prototype_Component_Texture_LensFlare_ThunderRoot2");
	tMeshDesc.wstrMaskTag = TEXT("Prototype_Component_Texture_LensFlare_CircleGradation");
	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshEmitterCommon::PROTOTYPE_TAG, TEXT("Hexagon1"), &tMeshDesc)))
		return E_FAIL;

	tMeshDesc.wstrModelTag = TEXT("Prototype_Component_Model_LensFlare_Common_Ring01");
	tMeshDesc.wstrTextureTag = TEXT("Prototype_Component_Texture_LensFlare_Common_Ring08");
	tMeshDesc.wstrMaskTag = TEXT("Prototype_Component_Texture_LensFlare_Common_Circle11");
	tMeshDesc.bCustomShader = true;
	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshEmitterCommon::PROTOTYPE_TAG, TEXT("Ring1"), &tMeshDesc)))
		return E_FAIL;

	tMeshDesc.bCustomShader = false;
	tMeshDesc.wstrModelTag = TEXT("Prototype_Component_Model_LensFlare_Common_Circle01");
	tMeshDesc.wstrTextureTag = TEXT("Prototype_Component_Texture_LensFlare_Common_Circle01");
	tMeshDesc.bUseMaskCom = false;
	tMeshDesc.wstrMaskTag = L"";
	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshEmitterCommon::PROTOTYPE_TAG, TEXT("Circle2"), &tMeshDesc)))
		return E_FAIL;

	tMeshDesc.wstrTextureTag = TEXT("Prototype_Component_Texture_LensFlare_Common_Circle02");
	tMeshDesc.bUseMaskCom = true;
	tMeshDesc.wstrMaskTag = TEXT("Prototype_Component_Texture_LensFlare_ThunderRoot2");
	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshEmitterCommon::PROTOTYPE_TAG, TEXT("Hexagon2"), &tMeshDesc)))
		return E_FAIL;

	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshEmitterCommon::PROTOTYPE_TAG, TEXT("Hexagon3"), &tMeshDesc)))
		return E_FAIL;

	tMeshDesc.wstrModelTag = TEXT("Prototype_Component_Model_LensFlare_Common_Ring01");
	tMeshDesc.wstrTextureTag = TEXT("Prototype_Component_Texture_LensFlare_Common_Ring08");
	tMeshDesc.wstrMaskTag = TEXT("Prototype_Component_Texture_LensFlare_Common_Circle11");
	tMeshDesc.bCustomShader = true;
	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshEmitterCommon::PROTOTYPE_TAG, TEXT("Ring2"), &tMeshDesc)))
		return E_FAIL;

	tMeshDesc.bCustomShader = false;
	tMeshDesc.wstrModelTag = TEXT("Prototype_Component_Model_LensFlare_Common_Circle01");
	tMeshDesc.wstrTextureTag = TEXT("Prototype_Component_Texture_LensFlare_Common_Circle04");
	tMeshDesc.wstrMaskTag = TEXT("Prototype_Component_Texture_LensFlare_CircleGradation");
	if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CMeshEmitterCommon::PROTOTYPE_TAG, TEXT("Circle3"), &tMeshDesc)))
		return E_FAIL;

	return S_OK;
}

void CLensFlare::Cache_LensElements()
{
	m_LensElements.clear();
	m_bScreenVisible = false;
	m_bAuthorPlacementRestored = false;

	for (auto& [strTag, pPart] : m_EffestParts)
	{
		CTransform* pTransform = pPart == nullptr ? nullptr : pPart->Get_Transform();

		if (pTransform == nullptr)
		{
			if (m_bLensElementCacheWarningLogged == false)
			{
				const char* pReason = pPart == nullptr ? "part is null" : "transform is null";
				Client::Log_GameContentWarning("[LensFlare] Lens element cache failed: tag=" + WstrToStr(strTag) + ", reason=" + pReason);
				m_bLensElementCacheWarningLogged = true;
			}

			continue;
		}

		LENS_ELEMENT Element{};
		Element.pPart = pPart;
		Element.pEmitterAlpha = Find_EmitterAlpha(pPart);

		if (Try_GetAuthorLocalPosition(pPart, &Element.vAuthorLocalPosition) == false)
		{
			if (m_bLensElementCacheWarningLogged == false)
			{
				Client::Log_GameContentWarning("[LensFlare] Lens element cache failed: tag=" + WstrToStr(strTag) + ", reason=Local Pos is missing or invalid");
				m_bLensElementCacheWarningLogged = true;
			}

			continue;
		}

		Element.fAxisPosition = (Element.vAuthorLocalPosition.z - LENS_AUTHOR_AXIS_ORIGIN) / LENS_AUTHOR_AXIS_UNIT;
		Element.vScreenOffset = {
				Element.vAuthorLocalPosition.x / LENS_AUTHOR_AXIS_UNIT,
				Element.vAuthorLocalPosition.y / LENS_AUTHOR_AXIS_UNIT
		};

		m_LensElements.emplace(strTag, Element);
	}

	m_bLensElementCacheReady = m_LensElements.size() == m_EffestParts.size();

	if (m_bLensElementCacheReady == true)
		m_bLensElementCacheWarningLogged = false;
}

_bool CLensFlare::Try_GetAuthorLocalPosition(const CEffect_Part* pPart, _float3* pOutLocalPosition) const
{
	if (nullptr == pPart || nullptr == pOutLocalPosition)
		return false;

	for (const Engine::FPROPERTY& Property : pPart->Get_Properties())
	{
		if (Engine::PROP_TYPE::FLOAT3 != Property.eType || Property.strName != L"Local Pos" || Property.strCategory != L"Effect")
			continue;

		const _float3* pLocalPosition = static_cast<const _float3*>(pPart->Get_PropertyPtr(Property.uOffset));

		if (nullptr == pLocalPosition || !MathUtils::Is_ValidFloat3(*pLocalPosition))
			return false;

		*pOutLocalPosition = *pLocalPosition;
		return true;
	}

	return false;
}

_float* CLensFlare::Find_EmitterAlpha(CEffect_Part* pPart) const
{
	if (nullptr == pPart)
		return nullptr;

	for (const Engine::FPROPERTY& Property : pPart->Get_Properties())
	{
		if (Engine::PROP_TYPE::FLOAT != Property.eType || Property.strName != L"Alpha_E")
			continue;

		return static_cast<_float*>(pPart->Get_PropertyPtr(Property.uOffset));
	}

	return nullptr;
}

_bool CLensFlare::Validate_LensProperties()
{
	if (!MathUtils::Is_FiniteFloat(m_fAxisExtent))
		return false;

	if (!MathUtils::Is_FiniteFloat(m_fAxisRotationDegree))
		return false;

	if (!MathUtils::Is_FiniteFloat(m_fGhostViewDepth) || m_fGhostViewDepth <= Helper::fEpsilon)
		return false;

	if (!MathUtils::Is_FiniteFloat(m_fScreenShowMargin) || m_fScreenShowMargin <= 0.f)
		return false;

	if (!MathUtils::Is_FiniteFloat(m_fScreenHideMargin))
		return false;

	if (m_fScreenHideMargin < m_fScreenShowMargin)
		m_fScreenHideMargin = m_fScreenShowMargin;

	return true;
}

_bool CLensFlare::Project_SourceToNDC(_float2* pOutSourceNDC) const
{
	if (pOutSourceNDC == nullptr)
		return false;

	const _matrix matView = XMLoadFloat4x4(
		m_pGameInstance_Proxy->Get_Matrix(D3DTS::VIEW, PROJ_TYPE::PERSPEC));

	const _matrix matProj = XMLoadFloat4x4(
		m_pGameInstance_Proxy->Get_Matrix(D3DTS::PROJ, PROJ_TYPE::PERSPEC));

	const _matrix matLensWorld = XMLoadFloat4x4(&m_CombinedWorldMatrix);
	const _vector vSourceWorld = XMVectorSetW(matLensWorld.r[3], 1.f);
	const _vector vSourceClip = XMVector4Transform(vSourceWorld, matView * matProj);
	const _float fClipW = XMVectorGetW(vSourceClip);

	if (!MathUtils::Is_FiniteFloat(fClipW) || fClipW <= Helper::fEpsilon)
		return false;

	const _float fInvW = 1.f / fClipW;

	pOutSourceNDC->x = XMVectorGetX(vSourceClip) * fInvW;
	pOutSourceNDC->y = XMVectorGetY(vSourceClip) * fInvW;

	return MathUtils::Is_FiniteFloat(pOutSourceNDC->x)
		&& MathUtils::Is_FiniteFloat(pOutSourceNDC->y);
}

_float2 CLensFlare::Calculate_GhostNDC(const _float2& vSourceNDC, _float fAxisPosition, const _float2& vScreenOffset) const
{
	const _float fFinalAxis = fAxisPosition * m_fAxisExtent;
	_float2 vOffset{ -vSourceNDC.x * fFinalAxis, -vSourceNDC.y * fFinalAxis };

	const _float4x4* pProj = m_pGameInstance_Proxy->Get_Matrix(D3DTS::PROJ, PROJ_TYPE::PERSPEC);
	const _bool bValidProjection = nullptr != pProj
		&& MathUtils::Is_FiniteFloat(pProj->_11)
		&& MathUtils::Is_FiniteFloat(pProj->_22)
		&& fabsf(pProj->_11) > Helper::fEpsilon
		&& fabsf(pProj->_22) > Helper::fEpsilon;
	const _float fAspect = bValidProjection ? pProj->_22 / pProj->_11 : 1.f;

	if (fabsf(m_fAxisRotationDegree) > Helper::fEpsilon)
	{
		const _float fRadian = XMConvertToRadians(m_fAxisRotationDegree);
		const _float fCos = cosf(fRadian), fSin = sinf(fRadian);
		const _float fX = vOffset.x * fAspect, fY = vOffset.y;

		vOffset.x = (fX * fCos - fY * fSin) / fAspect;
		vOffset.y = fX * fSin + fY * fCos;
	}

	vOffset.x += vScreenOffset.x / fAspect;
	vOffset.y += vScreenOffset.y;

	return { vSourceNDC.x + vOffset.x, vSourceNDC.y + vOffset.y };
}

_bool CLensFlare::Unproject_AtViewDepth(const _float2& vNDC, _float fViewDepth, _float3* pOutWorldPosition) const
{
	if (pOutWorldPosition == nullptr || !MathUtils::Is_FiniteFloat(fViewDepth) || fViewDepth <= Helper::fEpsilon)
		return false;

	const _matrix matInvProj = XMLoadFloat4x4(
		m_pGameInstance_Proxy->Get_InverseMatrix_Prespec(D3DTS::PROJ));

	const _matrix matInvView = XMLoadFloat4x4(
		m_pGameInstance_Proxy->Get_InverseMatrix_Prespec(D3DTS::VIEW));

	_vector vViewRay = XMVector3TransformCoord(
		XMVectorSet(vNDC.x, vNDC.y, 1.f, 1.f), matInvProj);

	const _float fRayZ = XMVectorGetZ(vViewRay);

	if (!MathUtils::Is_FiniteFloat(fRayZ) || fabsf(fRayZ) <= Helper::fEpsilon)
		return false;

	vViewRay *= fViewDepth / fRayZ;
	vViewRay = XMVectorSetW(vViewRay, 1.f);

	const _vector vWorldPosition = XMVector3TransformCoord(vViewRay, matInvView);
	XMStoreFloat3(pOutWorldPosition, vWorldPosition);

	return MathUtils::Is_ValidFloat3(*pOutWorldPosition);
}

_bool CLensFlare::Update_ScreenVisibility(const _float2& vSourceNDC)
{
	if (!MathUtils::Is_FiniteFloat(vSourceNDC.x) || !MathUtils::Is_FiniteFloat(vSourceNDC.y))
	{
		m_bScreenVisible = false;
		return false;
	}

	const _float fLimit = m_bScreenVisible ? m_fScreenHideMargin : m_fScreenShowMargin;
	m_bScreenVisible = fabsf(vSourceNDC.x) <= fLimit && fabsf(vSourceNDC.y) <= fLimit;

	return m_bScreenVisible;
}

_bool CLensFlare::Update_LensFlarePlacement()
{
	if (m_bUseScreenAxis == false)
	{
		m_bScreenVisible = false;
		Restore_AuthorPlacement();
		return true;
	}

	m_bAuthorPlacementRestored = false;

	if (Validate_LensProperties() == false)
	{
		m_bScreenVisible = false;
		return false;
	}

	_float2 vSourceNDC{};

	if (Project_SourceToNDC(&vSourceNDC) == false)
	{
		m_bScreenVisible = false;
		return false;
	}

	if (Update_ScreenVisibility(vSourceNDC) == false)
		return false;

	const _matrix matContainer = XMLoadFloat4x4(&m_CombinedWorldMatrix);
	const _float fDeterminant = XMVectorGetX(XMMatrixDeterminant(matContainer));

	if (!MathUtils::Is_FiniteFloat(fDeterminant) || fabsf(fDeterminant) <= Helper::fEpsilon)
		return false;

	const _matrix matInvContainer = XMMatrixInverse(nullptr, matContainer);

	vector<PENDING_LENS_POSITION> PendingPositions;
	PendingPositions.reserve(m_LensElements.size());

	for (const auto& [strTag, Element] : m_LensElements)
	{
		if (Element.pPart == nullptr)
			return false;

		CTransform* pPartTransform = Element.pPart->Get_Transform();

		if (pPartTransform == nullptr)
			return false;

		const _float2 vGhostNDC = Calculate_GhostNDC(vSourceNDC, Element.fAxisPosition, Element.vScreenOffset);

		_float3 vGhostWorld{};

		if (Unproject_AtViewDepth(vGhostNDC, m_fGhostViewDepth, &vGhostWorld) == false)
			return false;

		const _vector vGhostLocal = XMVector3TransformCoord(XMLoadFloat3(&vGhostWorld), matInvContainer);

		PENDING_LENS_POSITION Pending{};
		Pending.pTransform = pPartTransform;
		XMStoreFloat3(&Pending.vLocalPosition, vGhostLocal);

		if (!MathUtils::Is_ValidFloat3(Pending.vLocalPosition))
			return false;

		PendingPositions.push_back(Pending);
	}

	for (const PENDING_LENS_POSITION& Pending : PendingPositions)
	{
		Pending.pTransform->Set_State(
			STATE::POSITION,
			XMVectorSetW(XMLoadFloat3(&Pending.vLocalPosition), 1.f));
	}

	return true;
}

void CLensFlare::Queue_FadeIn()
{
	m_bFadeInPending = true;
	m_fFadeInAccTime = 0.f;
	m_fFadeInAlpha = 0.f;
}

void CLensFlare::Update_FadeIn(_float fTimeDelta)
{
	if (m_bIsPlay == false)
		return;

	if (m_bFadeOutRequested == true)
	{
		m_bFadeInPending = false;
		return;
	}

	if (m_bFadeInPending == true)
	{
		if (m_bUseScreenAxis == true && m_bScreenVisible == false)
			return;

		m_bFadeInPending = false;
	}

	if (m_fFadeInAlpha >= 1.f)
		return;

	m_fFadeInAccTime += fTimeDelta;

	_float fRatio = m_fFadeInAccTime / LENS_FADE_IN_DURATION;
	Helper::FloatClamp(fRatio, 0.f, 1.f);

	m_fFadeInAlpha = Helper::FloatSmoothStep(0.f, 1.f, fRatio);
}

void CLensFlare::Apply_FadeInAlpha()
{
	for (auto& [strTag, Element] : m_LensElements)
	{
		if (nullptr == Element.pEmitterAlpha)
			continue;

		Element.fRuntimeEmitterAlpha = *Element.pEmitterAlpha;
		*Element.pEmitterAlpha *= m_fFadeInAlpha;
	}
}

void CLensFlare::Restore_EmitterAlpha()
{
	for (auto& [strTag, Element] : m_LensElements)
	{
		if (nullptr != Element.pEmitterAlpha)
			*Element.pEmitterAlpha = Element.fRuntimeEmitterAlpha;
	}
}

void CLensFlare::Reset_LensRuntimeState()
{
	m_bScreenVisible = false;
	m_bAuthorPlacementRestored = false;
	Queue_FadeIn();

	if (m_bLensElementCacheReady == true)
		Restore_AuthorPlacement();

	m_bAuthorPlacementRestored = false;
}

void CLensFlare::Restore_AuthorPlacement()
{
	if (m_bAuthorPlacementRestored == true)
		return;

	if (m_bLensElementCacheReady == false)
		return;

	for (auto& [strTag, Element] : m_LensElements)
	{
		if (Element.pPart == nullptr)
			continue;

		CTransform* pPartTransform = Element.pPart->Get_Transform();

		if (pPartTransform == nullptr)
			continue;

		pPartTransform->Set_State(STATE::POSITION, XMVectorSetW(XMLoadFloat3(&Element.vAuthorLocalPosition), 1.f));
	}

	m_bAuthorPlacementRestored = true;
}

CLensFlare* CLensFlare::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CLensFlare* pInstance = new CLensFlare(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created: CLensFlare");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CLensFlare::Clone(void* pArg)
{
	CLensFlare* pInstance = new CLensFlare(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned: CLensFlare");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CLensFlare::Free()
{
	__super::Free();
}