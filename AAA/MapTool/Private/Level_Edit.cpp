#include "Level_Edit.h"
#include "Loader.h"
#include "EditCamera.h"
#include "Edit_Grid.h"

#include "GameObject_Factory.h"
#include "GameContent_const.h"
#include "GameContent_Log.h"
#include "Map_EditFile.h"
#include "Map_EditSession.h"
#include "Map_Loader.h"
#include "MapStage.h"
#include "MapSection.h"
#include "LevelDesignObject.h"
#include "EnvObject_Static.h"
#include "EnvTrigger_RenderGlobals.h"
#include "EnvVolume_Effect.h"
#include "EnvVolume_Culling.h"
#include "EnvVolume_Light.h"
#include "Env_SpotLight.h"
#include "Env_InstanceController.h"
#include "LevelDesign_Registry.h"
#include "LD_LensFlare.h"

#include "GameInstance.h"

#include <cmath>

namespace
{
	void Forward_GameContentLog(Client::GAMECONTENT_LOG_LEVEL eLevel, const char* pMessage)
	{
		const string strMessage = (nullptr != pMessage) ? pMessage : "";

		switch (eLevel)
		{
		case Client::GAMECONTENT_LOG_LEVEL::INFO:
			MapTool::Log_Info(strMessage);
			break;
		case Client::GAMECONTENT_LOG_LEVEL::WARNING:
			MapTool::Log_Warning(strMessage);
			break;
		case Client::GAMECONTENT_LOG_LEVEL::ERROR_:
			MapTool::Log_Error(strMessage);
			break;
		default:
			MapTool::Log_Info(strMessage);
			break;
		}
	}

	using namespace std::filesystem;

	constexpr _tchar kMapModelRoot[] = L"../../Resources/Map";

	ENV_OBJECT_DESC Make_DefaultEnvRuntimeDesc(const wstring& strPrototypeTag, const wstring& strObjectName, const _float3& vPosition)
	{
		ENV_OBJECT_DESC Desc{};
		Engine::CGameObject::GAMEOBJECT_DESC& BaseDesc =
			static_cast<Engine::CGameObject::GAMEOBJECT_DESC&>(Desc);

		Desc.eKind = ENV_OBJECT_KIND::EFFECT;
		Desc.wstrObjectName = strObjectName;
		Desc.wstrComponentName = strObjectName;

		Desc.vPosition = vPosition;
		Desc.vRotation = { 0.f, 0.f, 0.f, 1.f };
		Desc.vScale = { 1.f, 1.f, 1.f };

		BaseDesc.vPosition = _float4(vPosition.x, vPosition.y, vPosition.z, 1.f);

		Desc.tCollision.bInvisibleCollision = true;
		Desc.tRender.bHasShadow = false;
		Desc.tRender.bUseCullDistance = false;
		Desc.tRender.bUseCullFrustum = false;

		Desc.tEffect.vAreaCenter = { 0.f, 0.f, 0.f };
		Desc.tEffect.vAreaSize = { 1.f, 1.f, 1.f };
		Desc.tEffect.vAreaRot = { 0.f, 0.f, 0.f, 1.f };

		if (strPrototypeTag == CEnvTrigger_RenderGlobals::PROTOTYPE_TAG)
			Desc.tEffect.eEffectType = ENV_EFFECT_TYPE::TONE_MAPPING_AREA;
		else if (strPrototypeTag == CEnvVolume_Effect::PROTOTYPE_TAG)
			Desc.tEffect.eEffectType = ENV_EFFECT_TYPE::FIELD_EFFECT;
		else if (strPrototypeTag == CEnvVolume_Culling::PROTOTYPE_TAG)
			Desc.tEffect.eEffectType = ENV_EFFECT_TYPE::DECOR_PARTS_CULLING_AREA;
		else if (strPrototypeTag == CEnvVolume_Light::PROTOTYPE_TAG)
			Desc.tEffect.eEffectType = ENV_EFFECT_TYPE::LOCAL_AREA_LIGHT;
		else if (strPrototypeTag == CEnv_SpotLight::PROTOTYPE_TAG)
			Desc.tEffect.eEffectType = ENV_EFFECT_TYPE::SPOT_LIGHT;

		return Desc;
	}

	_bool Equals_NoCase(const wstring& strLeft, const wstring& strRight)
	{
		return 0 == _wcsicmp(strLeft.c_str(), strRight.c_str());
	}

	_bool IsNearlyEqualFloat4x4(const _float4x4& A, const _float4x4& B, _float fEpsilon = 0.0001f)
	{
		for (_uint iRow = 0; iRow < 4; ++iRow)
		{
			for (_uint iCol = 0; iCol < 4; ++iCol)
			{
				if (fabsf(A.m[iRow][iCol] - B.m[iRow][iCol]) > fEpsilon)
					return false;
			}
		}

		return true;
	}

	_matrix Build_EnvBaseWorldMatrix(const ENV_OBJECT_DESC& Desc)
	{
		if (Desc.bHasWorldMatrix)
			return XMLoadFloat4x4(&Desc.matWorld);

		const _vector vScale = XMLoadFloat3(&Desc.vScale);
		const _vector vRotation = XMLoadFloat4(&Desc.vRotation);
		const _vector vPosition = XMVectorSet(Desc.vPosition.x, Desc.vPosition.y, Desc.vPosition.z, 1.f);

		return XMMatrixScalingFromVector(vScale)
			* XMMatrixRotationQuaternion(vRotation)
			* XMMatrixTranslationFromVector(vPosition);
	}

	_matrix Build_SectionBaseWorldMatrix(const MAP_SECTION_DESC& Desc)
	{
		_float4x4 Mat{};
		Mat.m[0][0] = Desc.vRight.x;
		Mat.m[0][1] = Desc.vRight.y;
		Mat.m[0][2] = Desc.vRight.z;
		Mat.m[0][3] = Desc.vRight.w;
		Mat.m[1][0] = Desc.vUp.x;
		Mat.m[1][1] = Desc.vUp.y;
		Mat.m[1][2] = Desc.vUp.z;
		Mat.m[1][3] = Desc.vUp.w;
		Mat.m[2][0] = Desc.vLook.x;
		Mat.m[2][1] = Desc.vLook.y;
		Mat.m[2][2] = Desc.vLook.z;
		Mat.m[2][3] = Desc.vLook.w;
		Mat.m[3][0] = Desc.vPosition.x;
		Mat.m[3][1] = Desc.vPosition.y;
		Mat.m[3][2] = Desc.vPosition.z;
		Mat.m[3][3] = Desc.vPosition.w;
		return XMLoadFloat4x4(&Mat);
	}

	_matrix Build_LevelDesignBaseWorldMatrix(const LD_OBJECT_DESC& Desc)
	{
		const CGameObject::GAMEOBJECT_DESC& BaseDesc = static_cast<const CGameObject::GAMEOBJECT_DESC&>(Desc);

		_float4x4 Mat{};
		Mat.m[0][0] = BaseDesc.vRight.x;
		Mat.m[0][1] = BaseDesc.vRight.y;
		Mat.m[0][2] = BaseDesc.vRight.z;
		Mat.m[0][3] = BaseDesc.vRight.w;
		Mat.m[1][0] = BaseDesc.vUp.x;
		Mat.m[1][1] = BaseDesc.vUp.y;
		Mat.m[1][2] = BaseDesc.vUp.z;
		Mat.m[1][3] = BaseDesc.vUp.w;
		Mat.m[2][0] = BaseDesc.vLook.x;
		Mat.m[2][1] = BaseDesc.vLook.y;
		Mat.m[2][2] = BaseDesc.vLook.z;
		Mat.m[2][3] = BaseDesc.vLook.w;
		Mat.m[3][0] = BaseDesc.vPosition.x;
		Mat.m[3][1] = BaseDesc.vPosition.y;
		Mat.m[3][2] = BaseDesc.vPosition.z;
		Mat.m[3][3] = BaseDesc.vPosition.w;
		return XMLoadFloat4x4(&Mat);
	}

	void Update_EnvPolicyEditFromCurrent(CEnvObject* pEnvObject, EDIT_OBJECT_OVERRIDE_DESC* pInOutEdit)
	{
		if (nullptr == pEnvObject || nullptr == pInOutEdit)
			return;

		EDITABLE_DESC EditDesc{};
		if (!pEnvObject->Get_EditDesc(&EditDesc))
			return;

		const ENV_OBJECT_DESC& Desc = pEnvObject->Get_Desc();
		const EDIT_OBJECT_POLICY& Policy = EditDesc.Policy;
		EDIT_OBJECT_COMMON_OVERRIDE& Common = pInOutEdit->Common;
		pInOutEdit->eKind = EDITABLE_OBJECT_KIND::ENV_OBJECT;

		const _bool bBaseRenderable = !Desc.tCollision.bInvisibleCollision;
		if ((Common.iPolicyMask & EDIT_CAP_RENDERABLE) || Policy.bRenderable != bBaseRenderable)
			Common.iPolicyMask |= EDIT_CAP_RENDERABLE;
		else
			Common.iPolicyMask &= ~EDIT_CAP_RENDERABLE;
		Common.Policy.bRenderable = Policy.bRenderable;

		if ((Common.iPolicyMask & EDIT_CAP_CULL_DISTANCE) || Policy.bUseCullDistance != Desc.tRender.bUseCullDistance)
			Common.iPolicyMask |= EDIT_CAP_CULL_DISTANCE;
		else
			Common.iPolicyMask &= ~EDIT_CAP_CULL_DISTANCE;
		Common.Policy.bUseCullDistance = Policy.bUseCullDistance;

		if ((Common.iPolicyMask & EDIT_CAP_CULL_FRUSTUM) || Policy.bUseCullFrustum != Desc.tRender.bUseCullFrustum)
			Common.iPolicyMask |= EDIT_CAP_CULL_FRUSTUM;
		else
			Common.iPolicyMask &= ~EDIT_CAP_CULL_FRUSTUM;
		Common.Policy.bUseCullFrustum = Policy.bUseCullFrustum;

		if (Desc.tRender.bHasShadow && ((Common.iPolicyMask & EDIT_CAP_SHADOW) || Policy.bUseShadow != Desc.tRender.bUseShadow))
			Common.iPolicyMask |= EDIT_CAP_SHADOW;
		else
			Common.iPolicyMask &= ~EDIT_CAP_SHADOW;
		Common.Policy.bUseShadow = Policy.bUseShadow;

		if (Desc.tCollision.bHasCollMesh && ((Common.iPolicyMask & EDIT_CAP_COLLISION_MESH) || Policy.bUseCollMesh != Desc.tCollision.bUseCollMesh))
			Common.iPolicyMask |= EDIT_CAP_COLLISION_MESH;
		else
			Common.iPolicyMask &= ~EDIT_CAP_COLLISION_MESH;
		Common.Policy.bUseCollMesh = Policy.bUseCollMesh;
	}

	void Update_SectionPolicyEditFromCurrent(CMapSection* pSection, EDIT_OBJECT_OVERRIDE_DESC* pInOutEdit)
	{
		if (nullptr == pSection || nullptr == pInOutEdit)
			return;

		EDITABLE_DESC EditDesc{};
		if (!pSection->Get_EditDesc(&EditDesc))
			return;

		const MAP_SECTION_DESC& Desc = pSection->Get_Desc();
		const EDIT_OBJECT_POLICY& Policy = EditDesc.Policy;
		EDIT_OBJECT_COMMON_OVERRIDE& Common = pInOutEdit->Common;
		pInOutEdit->eKind = EDITABLE_OBJECT_KIND::MAP_SECTION;

		if ((Common.iPolicyMask & EDIT_CAP_RENDERABLE) || Policy.bRenderable != Desc.bRenderable)
			Common.iPolicyMask |= EDIT_CAP_RENDERABLE;
		else
			Common.iPolicyMask &= ~EDIT_CAP_RENDERABLE;
		Common.Policy.bRenderable = Policy.bRenderable;

		if ((Common.iPolicyMask & EDIT_CAP_CULL_FRUSTUM) || Policy.bUseCullFrustum != Desc.bEnableCulling)
			Common.iPolicyMask |= EDIT_CAP_CULL_FRUSTUM;
		else
			Common.iPolicyMask &= ~EDIT_CAP_CULL_FRUSTUM;
		Common.Policy.bUseCullFrustum = Policy.bUseCullFrustum;

		if (pSection->Has_CollMesh() && ((Common.iPolicyMask & EDIT_CAP_COLLISION_MESH) || Policy.bUseCollMesh != Desc.bUseCollMesh))
			Common.iPolicyMask |= EDIT_CAP_COLLISION_MESH;
		else
			Common.iPolicyMask &= ~EDIT_CAP_COLLISION_MESH;
		Common.Policy.bUseCollMesh = Policy.bUseCollMesh;
	}

	void Update_WorldMatrixEdit(const _float4x4& CurrentWorld, _matrix BaseWorldMatrix, EDIT_OBJECT_OVERRIDE_DESC* pInOutEdit)
	{
		if (nullptr == pInOutEdit)
			return;

		_float4x4 BaseWorld{};
		XMStoreFloat4x4(&BaseWorld, BaseWorldMatrix);

		if (pInOutEdit->Common.bHasWorldMatrix || !IsNearlyEqualFloat4x4(CurrentWorld, BaseWorld))
		{
			pInOutEdit->Common.bHasWorldMatrix = true;
			pInOutEdit->Common.matWorld = CurrentWorld;
		}
		else
		{
			pInOutEdit->Common.bHasWorldMatrix = false;
			pInOutEdit->Common.matWorld = {};
		}
	}

	void Add_UniqueCandidate(const wstring& strCandidate, vector<wstring>* pOutCandidates)
	{
		if (nullptr == pOutCandidates || strCandidate.empty())
			return;

		auto Iter = find_if(
			pOutCandidates->begin(),
			pOutCandidates->end(),
			[&](const wstring& strExisting) { return Equals_NoCase(strExisting, strCandidate); });
		if (Iter == pOutCandidates->end())
			pOutCandidates->push_back(strCandidate);
	}

	void Build_MapSectionModelCandidates(const wstring& strSectionName, vector<wstring>* pOutCandidates)
	{
		if (nullptr == pOutCandidates)
			return;

		pOutCandidates->clear();
		Add_UniqueCandidate(strSectionName + L".ysh", pOutCandidates);

		if (0 == strSectionName.rfind(L"Land_", 0))
			Add_UniqueCandidate(strSectionName.substr(5) + L".ysh", pOutCandidates);
	}

	_bool Try_ResolveMapSectionModelPath(const wstring& strSectionName, wstring* pOutPath)
	{
		if (nullptr == pOutPath)
			return false;

		error_code ErrorCode;
		const path Root = weakly_canonical(path(kMapModelRoot), ErrorCode);
		if (ErrorCode || !exists(Root))
			return false;

		vector<wstring> Candidates;
		Build_MapSectionModelCandidates(strSectionName, &Candidates);
		if (Candidates.empty())
			return false;

		for (recursive_directory_iterator Iter(Root, directory_options::skip_permission_denied, ErrorCode), End;
			Iter != End;
			Iter.increment(ErrorCode))
		{
			if (ErrorCode)
				break;

			if (!Iter->is_regular_file())
				continue;

			const wstring strFilename = Iter->path().filename().wstring();
			auto CandidateIter = find_if(
				Candidates.begin(),
				Candidates.end(),
				[&](const wstring& strCandidate) { return Equals_NoCase(strFilename, strCandidate); });
			if (CandidateIter == Candidates.end())
				continue;

			*pOutPath = Iter->path().wstring();
			return true;
		}

		return false;
	}

	_wstring Make_MapSectionModelProtoTag(const wstring& strSectionName)
	{
		return L"Prototype_Component_Model_MapSection_" + strSectionName;
	}

	_bool Try_GetLegacyMapSectionModelPath(const wstring& strSectionName, wstring* pOutPath)
	{
		if (nullptr == pOutPath)
			return false;

		const wstring strLegacyPath = L"../../ResourcesTest/Stage1-0/" + strSectionName + L".ysh";
		error_code ErrorCode;
		if (!exists(path(strLegacyPath), ErrorCode) || ErrorCode)
			return false;

		*pOutPath = strLegacyPath;
		return true;
	}

	wstring Build_EnvPreviewCountText(const MAP_LOAD_RESULT& Report, _bool bUseCreatedLabel = false)
	{
		wstring strStatus = bUseCreatedLabel ? L"created=" : L"env=";
		strStatus += to_wstring(Report.iEnvCreatedCount);
		strStatus += L" / desc=" + to_wstring(Report.iEnvDescriptorCount);
		strStatus += L" / json=" + to_wstring(Report.iEnvJsonLoadedCount);
		strStatus += L" / missingModel=" + to_wstring(Report.iEnvSkippedMissingModel);
		strStatus += L" / createFailed=" + to_wstring(Report.iEnvSkippedCreateFailed);
		return strStatus;
	}
}

CLevel_Edit::CLevel_Edit(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CLevel{ pDevice, pContext }
{
}

HRESULT CLevel_Edit::Initialize()
{
	if (FAILED(__super::Initialize()))
		return E_FAIL;

	if (FAILED(Ready_EditLights()))
		return E_FAIL;

	if (FAILED(Ready_EditCamera()))
		return E_FAIL;

	if (FAILED(Ready_EditGrid()))
		return E_FAIL;

	m_pMapPreviewSession = CMap_EditSession::Create();
	if (nullptr == m_pMapPreviewSession)
		return E_FAIL;

	return S_OK;
}

void CLevel_Edit::Update(_float fTimeDelta)
{
#ifdef _DEBUG
	if (m_pGameInstance_Proxy->Key_Down(DIK_F2))
		m_pGameInstance_Proxy->Toggle_DebugRender();
#endif //  _DEBUG
}

HRESULT CLevel_Edit::Render()
{
	if (m_pGrid)
	{
		const _float4x4* pView = m_pGameInstance_Proxy->Get_Matrix(D3DTS::VIEW, PROJ_TYPE::PERSPEC);
		const _float4x4* pProj = m_pGameInstance_Proxy->Get_Matrix(D3DTS::PROJ, PROJ_TYPE::PERSPEC);
		m_pGrid->Render(pView, pProj);
	}

	return S_OK;
}

CGameObject* CLevel_Edit::Spawn_Object(const _wstring& strProtoTag, const _wstring& strLayerTag, const _wstring& strName, void* pArg)
{
	auto* pReg = CGameObject_Factory::GetInstance()->Get_Registration(strProtoTag);
	if (!pReg) return nullptr;

	if (!m_pGameInstance_Proxy->Has_Prototype(ETOUI(TOOL_LEVEL::EDIT), strProtoTag))
	{
		pReg->ResourceLoader(m_pGameInstance_Proxy, m_pDevice, m_pContext, ETOUI(TOOL_LEVEL::EDIT));

		m_pGameInstance_Proxy->Add_Prototype(ETOUI(TOOL_LEVEL::EDIT), strProtoTag.c_str(),
			pReg->CreatorFunc(m_pDevice, m_pContext));
	}

	_wstring strFinalLayer = strLayerTag;
	if (pReg->strCategory == L"CAMERA_OBJECT")
		strFinalLayer = L"Layer_Camera";

	CGameObject* pObj = nullptr;
	const HRESULT hr = m_pGameInstance_Proxy->
		Add_GameObject_Return(
			&pObj,
			ETOUI(TOOL_LEVEL::EDIT),
			strProtoTag.c_str(),
			ETOUI(TOOL_LEVEL::EDIT),
			strFinalLayer.c_str(),
			strName,
			pArg);

	if (FAILED(hr) || nullptr == pObj)
	{
		if (nullptr != pObj)
			m_pGameInstance_Proxy->Destroy_GameObject(pObj);

		return nullptr;
	}

	Add_Layer(strFinalLayer);
	m_Layers[strFinalLayer].push_back({ strProtoTag, strName, pObj });

	Mark_HierarchyDirty();

	return pObj;
}

void CLevel_Edit::Add_Layer(const wstring& strLayerTag)
{
	if (m_Layers.find(strLayerTag) == m_Layers.end())
	{
		m_Layers[strLayerTag] = {};
		Mark_HierarchyDirty();
	}
}

HRESULT CLevel_Edit::Remove_Layer(const wstring& strLayerTag)
{
	auto iter = m_Layers.find(strLayerTag);
	if (iter == m_Layers.end()) return E_FAIL;

	if (!iter->second.empty()) {
		MSG_BOX("The layer contains objects");
		return E_FAIL;
	}

	m_Layers.erase(iter);
	Mark_HierarchyDirty();
	return S_OK;
}

void CLevel_Edit::Change_ObjectLayer(CGameObject* pObject, const wstring& strNewLayer)
{
	if (!pObject)
		return;

	if (m_MapPreviewObjects.find(pObject) != m_MapPreviewObjects.end())
		return;

	if (!m_MapPreviewObjects.empty() && Client::CMap_Loader::Is_MapLayer(strNewLayer))
		return;

	Add_Layer(strNewLayer);

	for (auto& [LayerTag, Objects] : m_Layers)
	{
		for (auto iter = Objects.begin(); iter != Objects.end(); ++iter)
		{
			if (iter->pObject == pObject)
			{
				if (LayerTag == strNewLayer)
					return;

				EDITOR_OBJECT_HANDLE Handle = *iter;
				Objects.erase(iter);
				m_Layers[strNewLayer].push_back(Handle);
				Mark_HierarchyDirty();
				return;
			}
		}
	}
}

void CLevel_Edit::Delete_Object(CGameObject* pObject)
{
	if (!pObject)
		return;

	if (Is_MapPreviewObject(pObject) && !Handle_MapSpecificDeletion(pObject))
		return;

	if (m_pSelected == pObject)
		Set_Selected(nullptr);

	for (auto& [LayerTag, Objects] : m_Layers)
	{
		for (auto iter = Objects.begin(); iter != Objects.end(); ++iter)
		{
			if (iter->pObject == pObject)
			{
				Objects.erase(iter);
				Mark_HierarchyDirty();
				m_pGameInstance_Proxy->Destroy_GameObject(pObject);
				return;
			}
		}
	}
}

void CLevel_Edit::Pick_And_Place(_fvector vOrigin, _fvector vDir)
{
	_float3 fHitPos = {};
	if (!m_pGameInstance_Proxy->Pick_RayToPlane(vOrigin, vDir, &fHitPos))
		return;

	if (!Is_PlaceMode()) return;
	fHitPos.x = roundf(fHitPos.x);
	fHitPos.z = roundf(fHitPos.z);
	fHitPos.y = 0.f;
	Place_Object_At(fHitPos);
}

void CLevel_Edit::Pick_And_SelectEnvObject(_fvector vOrigin, _fvector vDir)
{
	CGameObject* pPicked = nullptr;
	_float3 vHit = {};
	_float fDist = FLT_MAX;

	if (Pick_EnvObjectByRay(vOrigin, vDir, &pPicked, &vHit, &fDist))
	{
		Set_Selected(pPicked);
	}
	else
	{
		Set_Selected(nullptr);
	}
}

void CLevel_Edit::Place_Object_At(const _float3& vPos)
{
	wstring strName = m_strPendingProto + L"_" + to_wstring(m_iPlaceCount++);

	ENV_OBJECT_DESC EnvRuntimeDesc{};
	void* pArg = nullptr;

	auto* pRegistration = CGameObject_Factory::GetInstance()->Get_Registration(m_strPendingProto);
	if (nullptr != pRegistration && pRegistration->strCategory == L"ENV_TRIGGER")
	{
		EnvRuntimeDesc = Make_DefaultEnvRuntimeDesc(m_strPendingProto, strName, vPos);
		pArg = &EnvRuntimeDesc;
	}

	CGameObject* pObj = Spawn_Object(m_strPendingProto, m_strPendingLayer, strName, pArg);
	if (nullptr == pObj)
	{
		MapTool::Log_Error("Failed to place object: " + WstrToStr(m_strPendingProto));
		return;
	}

	pObj->Get_Transform()->Set_State(
		STATE::POSITION,
		XMVectorSet(vPos.x, vPos.y, vPos.z, 1.f)
	);
	pObj->Initialize_NaviPlacement();

	Try_RegisterAddedMapOverridePlacement(pObj, strName);
}

void CLevel_Edit::Begin_PlaceMode(const wstring& strProtoTag, const wstring& strLayerTag)
{
	m_ePlaceMode = PLACE_MODE::PENDING;
	m_strPendingLayer = strLayerTag;
	m_strPendingProto = strProtoTag;
}

void CLevel_Edit::End_PlaceMode()
{
	m_ePlaceMode = PLACE_MODE::NONE;
	m_strPendingLayer = {};
	m_strPendingProto = {};
}

_bool CLevel_Edit::Track_EditedMapPreviewEnvObject(CGameObject* pObject, const EDIT_OBJECT_OVERRIDE_DESC& Edit)
{
	if (nullptr == m_pMapPreviewSession)
		return false;

	return m_pMapPreviewSession->Track_EditedPreviewObject(pObject, Edit);
}

_bool CLevel_Edit::Clear_EditedMapPreviewEnvObject(CGameObject* pObject)
{
	if (nullptr == m_pMapPreviewSession)
		return false;

	return m_pMapPreviewSession->Clear_EditedPreviewObject(pObject);
}

_bool CLevel_Edit::Try_GetMapPreviewEnvEdit(CGameObject* pObject, EDIT_OBJECT_OVERRIDE_DESC* pOutEdit) const
{
	if (nullptr == m_pMapPreviewSession || nullptr == pObject || nullptr == pOutEdit)
		return false;

	CEnvObject* pEnvObject = dynamic_cast<CEnvObject*>(pObject);
	if (nullptr == pEnvObject)
		return false;

	const _wstring strStableKey =
		CMap_EditFile::Make_EnvKey(pEnvObject->Get_Desc());

	return m_pMapPreviewSession->Try_GetEditedEnvObject(strStableKey, pOutEdit);
}

_bool CLevel_Edit::Track_EditedMapPreviewSection(const _wstring& strSectionKey, const EDIT_OBJECT_OVERRIDE_DESC& Edit)
{
	if (nullptr == m_pMapPreviewSession)
		return false;

	return m_pMapPreviewSession->Track_EditedMapSection(strSectionKey, Edit);
}

_bool CLevel_Edit::Clear_EditedMapPreviewSection(const _wstring& strSectionKey)
{
	if (nullptr == m_pMapPreviewSession)
		return false;

	return m_pMapPreviewSession->Clear_EditedMapSection(strSectionKey);
}

_bool CLevel_Edit::Try_GetMapPreviewSectionEdit(const _wstring& strSectionKey, EDIT_OBJECT_OVERRIDE_DESC* pOutEdit) const
{
	if (nullptr == m_pMapPreviewSession)
		return false;

	return m_pMapPreviewSession->Try_GetEditedMapSection(strSectionKey, pOutEdit);
}

_bool CLevel_Edit::Track_EditedMapPreviewLevelDesignObject(CGameObject* pObject, const EDIT_OBJECT_OVERRIDE_DESC& Edit)
{
	if (nullptr == m_pMapPreviewSession)
		return false;

	return m_pMapPreviewSession->Track_EditedLevelDesignObject(pObject, Edit);
}

_bool CLevel_Edit::Clear_EditedMapPreviewLevelDesignObject(CGameObject* pObject)
{
	if (nullptr == m_pMapPreviewSession)
		return false;

	return m_pMapPreviewSession->Clear_EditedLevelDesignObject(pObject);
}

_bool CLevel_Edit::Try_GetMapPreviewLevelDesignEdit(CGameObject* pObject, EDIT_OBJECT_OVERRIDE_DESC* pOutEdit) const
{
	if (nullptr == m_pMapPreviewSession || nullptr == pObject || nullptr == pOutEdit)
		return false;

	return m_pMapPreviewSession->Try_GetEditedLevelDesignObject(pObject, pOutEdit);
}

HRESULT CLevel_Edit::Clear_MapPreviewEnvObjectOverride(CGameObject* pObject)
{
	if (nullptr == pObject || nullptr == m_pMapPreviewSession)
		return E_FAIL;

	const _int iPresetIndex = m_pMapPreviewSession->Get_EditData().iPresetIndex;
	if (0 > iPresetIndex)
		return E_FAIL;

	if (m_pMapPreviewSession->Is_AddedObject(pObject))
		Delete_Object(pObject);
	else
		m_pMapPreviewSession->Clear_EditedPreviewObject(pObject);

	return Load_MapPreviewEnv(static_cast<_uint>(iPresetIndex));
}

_bool CLevel_Edit::Commit_MapEditObjectFromCurrentState(CGameObject* pObject)
{
	if (nullptr == pObject || nullptr == m_pMapPreviewSession)
		return false;

	if (CEnvObject* pEnvObject = dynamic_cast<CEnvObject*>(pObject))
	{
		if (FAILED(pEnvObject->On_EditTransformChanged()))
			return false;
		if (m_pMapPreviewSession->Is_AddedObject(pObject))
			return true;
		if (!m_pMapPreviewSession->Can_DeleteAsEnvOverride(pObject))
			return false;
		if (ENV_OBJECT_KIND::EFFECT == pEnvObject->Get_Desc().eKind)
			return Promote_MapPreviewEnvObjectToAdded(pObject);

		EDIT_OBJECT_OVERRIDE_DESC Edit{};
		Edit.eKind = EDITABLE_OBJECT_KIND::ENV_OBJECT;
		Try_GetMapPreviewEnvEdit(pObject, &Edit);
		Update_EnvPolicyEditFromCurrent(pEnvObject, &Edit);

		const _float4x4& CurrentWorld = *pEnvObject->Get_Transform()->Get_WorldMatrixPtr();
		Update_WorldMatrixEdit(CurrentWorld, Build_EnvBaseWorldMatrix(pEnvObject->Get_Desc()), &Edit);
		return Track_EditedMapPreviewEnvObject(pObject, Edit);
	}

	if (CMapSection* pSection = dynamic_cast<CMapSection*>(pObject))
	{
		const _wstring strSectionKey = Make_MapPreviewSectionKey(pSection);
		if (strSectionKey.empty())
			return false;

		EDIT_OBJECT_OVERRIDE_DESC Edit{};
		Edit.eKind = EDITABLE_OBJECT_KIND::MAP_SECTION;
		Try_GetMapPreviewSectionEdit(strSectionKey, &Edit);
		Update_SectionPolicyEditFromCurrent(pSection, &Edit);

		const _float4x4& CurrentWorld = *pSection->Get_Transform()->Get_WorldMatrixPtr();
		Update_WorldMatrixEdit(CurrentWorld, Build_SectionBaseWorldMatrix(pSection->Get_Desc()), &Edit);
		return Track_EditedMapPreviewSection(strSectionKey, Edit);
	}

	if (CLevelDesignObject* pLDObject = dynamic_cast<CLevelDesignObject*>(pObject))
	{
		if (m_pMapPreviewSession->Is_AddedObject(pObject))
			return true;

		EDIT_OBJECT_OVERRIDE_DESC Edit{};
		Edit.eKind = EDITABLE_OBJECT_KIND::LEVEL_DESIGN_OBJECT;
		Try_GetMapPreviewLevelDesignEdit(pObject, &Edit);

		const _float4x4& CurrentWorld = *pLDObject->Get_Transform()->Get_WorldMatrixPtr();
		Update_WorldMatrixEdit(CurrentWorld, Build_LevelDesignBaseWorldMatrix(pLDObject->Get_LevelDesignDesc()), &Edit);
		return Track_EditedMapPreviewLevelDesignObject(pObject, Edit);
	}

	return false;
}

HRESULT CLevel_Edit::Restore_DeletedMapPreviewEnv(const _wstring& strStableKey)
{
	if (nullptr == m_pMapPreviewSession)
		return E_FAIL;

	const MAP_EDIT_DATA MapContentDesc =
		m_pMapPreviewSession->Get_EditData();

	if (0 > MapContentDesc.iPresetIndex)
		return E_FAIL;

	if (!m_pMapPreviewSession->Restore_DeletedEnvItem(strStableKey))
		return E_FAIL;

	return Load_MapPreviewEnv(static_cast<_uint>(MapContentDesc.iPresetIndex));
}

HRESULT CLevel_Edit::Restore_AllDeletedMapPreviewEnv()
{
	if (nullptr == m_pMapPreviewSession)
		return E_FAIL;

	const MAP_EDIT_DATA MapContentDesc =
		m_pMapPreviewSession->Get_EditData();

	if (0 > MapContentDesc.iPresetIndex)
		return E_FAIL;

	m_pMapPreviewSession->Restore_AllDeletedEnvItems();
	return Load_MapPreviewEnv(static_cast<_uint>(MapContentDesc.iPresetIndex));
}

HRESULT CLevel_Edit::Save_PlaceEdit()
{
	if (nullptr == m_pMapPreviewSession)
		return E_FAIL;

	const MAP_EDIT_DATA MapContentDesc = m_pMapPreviewSession->Build_EditDataSnapShot();

	if (!MapContentDesc.bHasMapContent || 0 > MapContentDesc.iPresetIndex)
		return E_FAIL;

	return CMap_Loader::Save_PresetEditFile(
		static_cast<_uint>(MapContentDesc.iPresetIndex),
		MapContentDesc,
		m_pMapStage);
}

HRESULT CLevel_Edit::Save_MapOverride()
{
	return Save_PlaceEdit();
}

HRESULT CLevel_Edit::Load_MapPreview(_uint iPresetIndex)
{
	const _bool bPresetChanged =
		nullptr == m_pMapPreviewSession
		|| !m_pMapPreviewSession->Get_EditData().bHasMapContent
		|| m_pMapPreviewSession->Get_EditData().iPresetIndex != static_cast<_int>(iPresetIndex);

	if (bPresetChanged && nullptr != m_pMapPreviewSession)
		m_pMapPreviewSession->Reset();

	Clear_MapPreview();

	if (FAILED(Prepare_MapContentForPreviewLoad(iPresetIndex, bPresetChanged, false)))
	{
		Set_MapPreviewStatus(L"Map edit file parse failed.");
		Sync_MapPreviewRuntimeStateToSession();
		return E_FAIL;
	}

	MAP_EDIT_DATA MapContentDesc = Build_MapPreviewContentDescSnapshot();
	MapContentDesc.bLoadStage = true;
	MapContentDesc.bLoadEnv = true;
	Apply_MapPreviewContentDesc(MapContentDesc, false);

	vector<Client::ENV_OBJECT_DESC> DeletedEnvDescs;
	MAP_LOAD_RESULT EnvReport{};

	if (FAILED(Ready_MapStage()))
	{
		Set_MapPreviewStatus(L"Map stage preview load failed.");
		Sync_MapPreviewRuntimeStateToSession();
		return E_FAIL;
	}

	if (FAILED(Load_LDPreview(iPresetIndex)))
	{
		Set_MapPreviewStatus(
			L"Map preset LevelDesign load failed.");
		Sync_MapPreviewRuntimeStateToSession();
		return E_FAIL;
	}

	if (FAILED(Ready_EnvObjects(&DeletedEnvDescs, &EnvReport)))
	{
		const wstring strStageName = Get_MapPreviewLoadedStageNameRef().empty()
			? StrToWstr(CMap_Loader::Get_MapName(iPresetIndex))
			: Get_MapPreviewLoadedStageNameRef();

		Set_MapPreviewStatus(
			L"Environment preview load failed: stage=" + strStageName
			+ L" / " + Build_EnvPreviewCountText(EnvReport, true));
		Sync_MapPreviewRuntimeStateToSession();
		return E_FAIL;
	}

	const wstring strStageName = Get_MapPreviewLoadedStageNameRef().empty()
		? StrToWstr(CMap_Loader::Get_MapName(iPresetIndex))
		: Get_MapPreviewLoadedStageNameRef();

	Set_MapPreviewStatus(
		L"Map preset loaded: " + strStageName
		+ L" / " + Build_EnvPreviewCountText(EnvReport)
		+ L" / levelDesign=loaded");

	if (nullptr != m_pMapPreviewSession)
		m_pMapPreviewSession->Rebuild_DeletedEnvItems(DeletedEnvDescs);

	Sync_MapPreviewRuntimeStateToSession();
	return S_OK;
}

HRESULT CLevel_Edit::Load_MapPreviewStage(_uint iPresetIndex)
{
	_bool bPresetChanged = false;
	_bool bPreviousLoadEnv = false;

	if (nullptr != m_pMapPreviewSession)
	{
		const auto& MapContentDesc = m_pMapPreviewSession->Get_EditData();
		bPreviousLoadEnv = MapContentDesc.bLoadEnv;
		bPresetChanged = !MapContentDesc.bHasMapContent
			|| MapContentDesc.iPresetIndex != static_cast<_int>(iPresetIndex);

		if (bPresetChanged)
			m_pMapPreviewSession->Reset();
	}
	else
	{
		const MAP_EDIT_DATA MapContentDesc =
			Build_MapPreviewContentDescSnapshot();

		bPresetChanged = MapContentDesc.iPresetIndex != static_cast<_int>(iPresetIndex);
		bPreviousLoadEnv = Is_MapEnvLoaded();
	}

	if (bPresetChanged)
		Clear_MapPreview();
	else
		Clear_MapPreviewStage();

	if (FAILED(Prepare_MapContentForPreviewLoad(
		iPresetIndex,
		bPresetChanged,
		!bPresetChanged)))
	{
		Set_MapPreviewStatus(L"Map edit file parse failed.");
		Sync_MapPreviewRuntimeStateToSession();
		return E_FAIL;
	}

	MAP_EDIT_DATA MapContentDesc = Build_MapPreviewContentDescSnapshot();
	MapContentDesc.bLoadStage = true;
	MapContentDesc.bLoadEnv = bPresetChanged ? false : bPreviousLoadEnv;
	Apply_MapPreviewContentDesc(MapContentDesc, !bPresetChanged);

	if (FAILED(Ready_MapStage()))
	{
		if (0 != Get_MapPreviewEnvCreatedCountInternal())
		{
			Set_MapPreviewStatus(
				L"Map stage preview load failed. / env="
				+ to_wstring(Get_MapPreviewEnvCreatedCountInternal()));
		}
		else
		{
			Set_MapPreviewStatus(L"Map stage preview load failed.");
		}

		Sync_MapPreviewRuntimeStateToSession();
		return E_FAIL;
	}

	const wstring strStageName = Get_MapPreviewLoadedStageNameRef().empty()
		? StrToWstr(CMap_Loader::Get_MapName(iPresetIndex))
		: Get_MapPreviewLoadedStageNameRef();

	Set_MapPreviewStatus(
		L"Map stage preview loaded: " + strStageName
		+ L" / env=" + to_wstring(Get_MapPreviewEnvCreatedCountInternal()));

	Sync_MapPreviewRuntimeStateToSession();
	return S_OK;
}

HRESULT CLevel_Edit::Load_MapPreviewEnv(_uint iPresetIndex)
{
	_bool bPresetChanged = false;
	if (nullptr != m_pMapPreviewSession)
	{
		const auto& MapContentDesc = m_pMapPreviewSession->Get_EditData();
		bPresetChanged = !MapContentDesc.bHasMapContent
			|| MapContentDesc.iPresetIndex != static_cast<_int>(iPresetIndex);

		if (bPresetChanged)
			m_pMapPreviewSession->Reset();
	}
	else
	{
		const MAP_EDIT_DATA MapContentDesc =
			Build_MapPreviewContentDescSnapshot();

		bPresetChanged = MapContentDesc.iPresetIndex != static_cast<_int>(iPresetIndex);
	}

	if (bPresetChanged)
		Clear_MapPreview();
	else
		Clear_MapPreviewEnv();

	if (FAILED(Prepare_MapContentForPreviewLoad(iPresetIndex, bPresetChanged, false)))
	{
		Set_MapPreviewStatus(L"Map edit file parse failed.");
		Sync_MapPreviewRuntimeStateToSession();
		return E_FAIL;
	}

	MAP_EDIT_DATA MapContentDesc = Build_MapPreviewContentDescSnapshot();
	MapContentDesc.bLoadStage = !bPresetChanged && Is_MapStageLoaded();
	MapContentDesc.bLoadEnv = true;
	Apply_MapPreviewContentDesc(MapContentDesc, false);

	vector<Client::ENV_OBJECT_DESC> DeletedEnvDescs;
	MAP_LOAD_RESULT EnvReport{};

	if (FAILED(Ready_EnvObjects(&DeletedEnvDescs, &EnvReport)))
	{
		if (nullptr != m_pMapStage)
		{
			const wstring strStageName = Get_MapPreviewLoadedStageNameRef().empty()
				? L"(loaded stage)"
				: Get_MapPreviewLoadedStageNameRef();

			Set_MapPreviewStatus(
				L"Environment preview load failed: stage=" + strStageName
				+ L" / " + Build_EnvPreviewCountText(EnvReport, true));
		}
		else
		{
			Set_MapPreviewStatus(
				L"Environment preview load failed: "
				+ Build_EnvPreviewCountText(EnvReport, true));
		}

		Sync_MapPreviewRuntimeStateToSession();
		return E_FAIL;
	}

	wstring strStatus = L"Environment preview loaded: "
		+ Build_EnvPreviewCountText(EnvReport);

	if (nullptr != m_pMapStage)
	{
		const wstring strStageName = Get_MapPreviewLoadedStageNameRef().empty()
			? L"(loaded stage)"
			: Get_MapPreviewLoadedStageNameRef();

		strStatus += L" / stage=" + strStageName;
	}

	Set_MapPreviewStatus(strStatus);

	if (nullptr != m_pMapPreviewSession)
		m_pMapPreviewSession->Rebuild_DeletedEnvItems(DeletedEnvDescs);

	Sync_MapPreviewRuntimeStateToSession();
	return S_OK;
}

HRESULT CLevel_Edit::Load_LDPreview(_uint iPresetIndex)
{
	_wstring strManifestPath;
	if (FAILED(CMap_Loader::Get_MapManifestPath(
		iPresetIndex,
		&strManifestPath)))
	{
		Set_MapPreviewStatus(
			L"LevelDesign manifest resolve failed.");
		return E_FAIL;
	}

	_bool bPresetChanged = false;

	if (nullptr != m_pMapPreviewSession)
	{
		const MAP_EDIT_DATA& MapContentDesc = m_pMapPreviewSession->Get_EditData();
		bPresetChanged = !MapContentDesc.bHasMapContent
			|| MapContentDesc.iPresetIndex != static_cast<_int>(iPresetIndex);

		if (bPresetChanged)
			m_pMapPreviewSession->Reset();
	}
	else
	{
		const MAP_EDIT_DATA MapContentDesc = Build_MapPreviewContentDescSnapshot();
		bPresetChanged = MapContentDesc.iPresetIndex != static_cast<_int>(iPresetIndex);
	}

	if (bPresetChanged)
		Clear_MapPreview();
	else
		Clear_LDPreview();

	if (FAILED(Prepare_MapContentForPreviewLoad(
		iPresetIndex,
		bPresetChanged,
		!bPresetChanged)))
	{
		Set_MapPreviewStatus(L"Map edit file parse failed.");
		Sync_MapPreviewRuntimeStateToSession();
		return E_FAIL;
	}

	MAP_EDIT_DATA MapContentDesc = Build_MapPreviewContentDescSnapshot();

	MAP_RUNTIME_LOAD_CONTEXT Context{};
	Context.pDevice = m_pDevice;
	Context.pContext = m_pContext;
	Context.iPlaceLevel = ETOUI(TOOL_LEVEL::EDIT);
	Context.iModelLevel = ETOUI(LEVEL::STATIC);
	Context.pCreatedCallback =
		&On_MapPreviewObjectCreated;
	Context.pCallbackContext = this;

	MAP_LOAD_RESULT Report{};
	if (FAILED(CMap_Loader::Load_LevelDesign_Runtime(
		Context,
		strManifestPath,
		&Report,
		&MapContentDesc.OverrideDesc)))
	{
		Set_MapPreviewStatus(
			L"LevelDesign preview load failed.");
		return E_FAIL;
	}

	Set_MapPreviewStatus(
		L"LevelDesign preview loaded: created="
		+ to_wstring(Report.iLevelDesignCreatedCount)
		+ L" / fallback="
		+ to_wstring(Report.iLevelDesignFallbackSpecCount)
		+ L" / failed="
		+ to_wstring(
			Report.iLevelDesignSkippedCreateFailedCount));

	return S_OK;
}

void CLevel_Edit::Clear_MapPreview()
{
	MAP_EDIT_DATA MapContentDesc = Build_MapPreviewContentDescSnapshot();

	vector<wstring> MapLayers;
	MapLayers.reserve(m_Layers.size());

	for (const auto& Pair : m_Layers)
	{
		if (CMap_Loader::Is_MapLayer(Pair.first)
			|| Client::CLevelDesign_Registry::Is_LevelDesignLayer(Pair.first))
		{
			MapLayers.push_back(Pair.first);
		}
	}

	for (const auto& strLayerTag : MapLayers)
		Clear_MapPreviewLayer(strLayerTag);

	Clear_MapPreviewEnvInstanceController();

	m_pMapStage = nullptr;
	Set_MapPreviewStageRuntime(false, L"");
	Set_MapPreviewEnvRuntime(false, 0);
	Set_MapPreviewStatus(L"Map preset not loaded.");
	m_MapPreviewObjects.clear();

	MapContentDesc.bLoadStage = false;
	MapContentDesc.bLoadEnv = false;
	Apply_MapPreviewContentDesc(MapContentDesc, false);

	Sync_MapPreviewRuntimeStateToSession();
}

void CLevel_Edit::Clear_MapPreviewStage()
{
	MAP_EDIT_DATA MapContentDesc = Build_MapPreviewContentDescSnapshot();

	vector<wstring> MapStageLayers;
	MapStageLayers.reserve(m_Layers.size());

	for (const auto& Pair : m_Layers)
	{
		if (CMap_Loader::Is_MapStageLayer(Pair.first))
			MapStageLayers.push_back(Pair.first);
	}

	for (const auto& strLayerTag : MapStageLayers)
		Clear_MapPreviewLayer(strLayerTag);

	m_pMapStage = nullptr;
	Set_MapPreviewStageRuntime(false, L"");
	MapContentDesc.bLoadStage = false;

	if (0 < Get_MapPreviewEnvCreatedCountInternal())
	{
		Set_MapPreviewStatus(
			L"Environment preview loaded only. / env="
			+ to_wstring(Get_MapPreviewEnvCreatedCountInternal()));
	}
	else
	{
		Set_MapPreviewStatus(L"Map preset not loaded.");
	}

	Apply_MapPreviewContentDesc(MapContentDesc, true);
	Sync_MapPreviewRuntimeStateToSession();
}

void CLevel_Edit::Clear_MapPreviewEnv()
{
	MAP_EDIT_DATA MapContentDesc = Build_MapPreviewContentDescSnapshot();

	vector<wstring> MapLayers;
	MapLayers.reserve(m_Layers.size());

	for (const auto& Pair : m_Layers)
	{
		if (CMap_Loader::Is_MapEnvLayer(Pair.first))
			MapLayers.push_back(Pair.first);
	}

	for (const auto& strLayerTag : MapLayers)
		Clear_MapPreviewLayer(strLayerTag);

	Clear_MapPreviewEnvInstanceController();

	Set_MapPreviewEnvRuntime(false, 0);
	MapContentDesc.bLoadEnv = false;

	if (nullptr != m_pMapStage)
	{
		const wstring strStageName = Get_MapPreviewLoadedStageNameRef().empty()
			? L"(loaded stage)"
			: Get_MapPreviewLoadedStageNameRef();

		Set_MapPreviewStatus(L"Map stage preview loaded: " + strStageName + L" / env=0");
	}
	else
	{
		Set_MapPreviewStatus(L"Map preset not loaded.");
	}

	Apply_MapPreviewContentDesc(MapContentDesc, false);
	Sync_MapPreviewRuntimeStateToSession();
}

void CLevel_Edit::Clear_LDPreview()
{
	vector<wstring> LevelDesignLayers;
	LevelDesignLayers.reserve(m_Layers.size());

	for (const auto& Pair : m_Layers)
	{
		if (CLevelDesign_Registry::Is_LevelDesignLayer(Pair.first))
			LevelDesignLayers.push_back(Pair.first);
	}

	for (const wstring& strLayerTag : LevelDesignLayers)
		Clear_MapPreviewLayer(strLayerTag);

	if (Is_MapStageLoaded())
	{
		const wstring strStageName = Get_MapPreviewLoadedStageNameRef().empty()
			? L"(loaded stage)"
			: Get_MapPreviewLoadedStageNameRef();

		Set_MapPreviewStatus(L"Map preview loaded without LevelDesign: stage=" + strStageName + L" / env=" +
			to_wstring(Get_MapPreviewEnvCreatedCountInternal()));
	}
	else if (Is_MapEnvLoaded())
	{
		Set_MapPreviewStatus(L"Environment preview loaded only. / env=" + to_wstring(Get_MapPreviewEnvCreatedCountInternal()));
	}
	else
	{
		Set_MapPreviewStatus(L"LevelDesign preview cleared.");
	}

	Sync_MapPreviewRuntimeStateToSession();
}

const _wstring& CLevel_Edit::Get_MapPreviewStatus() const
{
	static const _wstring s_strDefault = L"Map preset not loaded.";
	return nullptr != m_pMapPreviewSession
		? m_pMapPreviewSession->Get_PreviewStatus()
		: s_strDefault;
}

_bool CLevel_Edit::Is_MapStageLoaded() const
{
	return nullptr != m_pMapPreviewSession
		&& m_pMapPreviewSession->Is_StageLoaded();
}

_bool CLevel_Edit::Is_MapEnvLoaded() const
{
	return nullptr != m_pMapPreviewSession
		&& m_pMapPreviewSession->Is_EnvLoaded();
}

void CLevel_Edit::Clear_MapPreviewLayer(const wstring& strLayerTag)
{
	auto layerIter = m_Layers.find(strLayerTag);
	if (layerIter == m_Layers.end())
		return;

	_bool bLayerChanged = false;
	auto& Objects = layerIter->second;

	for (auto iter = Objects.begin(); iter != Objects.end();)
	{
		CGameObject* pObject = iter->pObject;
		if (nullptr == pObject || m_MapPreviewObjects.find(pObject) == m_MapPreviewObjects.end())
		{
			++iter;
			continue;
		}

		if (m_pSelected == pObject)
			Set_Selected(nullptr);

		if (pObject == m_pMapStage)
			m_pMapStage = nullptr;

		if (nullptr != m_pMapPreviewSession)
			m_pMapPreviewSession->Unregister_PreviewObject(pObject);

		m_MapPreviewObjects.erase(pObject);
		m_pGameInstance_Proxy->Destroy_GameObject(pObject);
		iter = Objects.erase(iter);
		bLayerChanged = true;
	}

	if (Objects.empty())
	{
		m_Layers.erase(layerIter);
		bLayerChanged = true;
	}

	if (bLayerChanged)
		Mark_HierarchyDirty();
}

void CLevel_Edit::Clear_MapPreviewEnvInstanceController()
{
	if (nullptr == m_pMapPreviewEnvInstanceController)
		return;

	if (m_pSelected == m_pMapPreviewEnvInstanceController)
		Set_Selected(nullptr);

	m_MapPreviewObjects.erase(m_pMapPreviewEnvInstanceController);
	m_pGameInstance_Proxy->Destroy_GameObject(m_pMapPreviewEnvInstanceController);
	m_pMapPreviewEnvInstanceController = nullptr;
}

void CLevel_Edit::Add_MapPreviewObjectHandle(
	const wstring& strPrototypeTag,
	const wstring& strLayerTag,
	const wstring& strObjectTag,
	CGameObject* pObject)
{
	if (nullptr == pObject)
		return;

	Add_Layer(strLayerTag);

	auto& Objects = m_Layers[strLayerTag];
	const auto iter = find_if(
		Objects.begin(),
		Objects.end(),
		[&](const EDITOR_OBJECT_HANDLE& Handle)
		{
			return Handle.pObject == pObject;
		});

	if (iter == Objects.end())
	{
		Objects.push_back({strPrototypeTag, strObjectTag, pObject});
		Mark_HierarchyDirty();
	}

	m_MapPreviewObjects.insert(pObject);

	if (nullptr != m_pMapPreviewSession)
	{
		m_pMapPreviewSession->Register_PreviewObject(strLayerTag, strObjectTag, pObject);
		Try_RegisterLoadedAddedMapObject(pObject, strPrototypeTag, strLayerTag, strObjectTag);
	}
}

void CLevel_Edit::On_MapPreviewObjectCreated(
	void* pContext,
	CGameObject* pObject,
	const wstring& strPrototypeTag,
	const wstring& strLayerTag,
	const wstring& strObjectTag)
{
	CLevel_Edit* pLevel = static_cast<CLevel_Edit*>(pContext);
	if (nullptr == pLevel || nullptr == pObject)
		return;

	pLevel->Add_MapPreviewObjectHandle(
		strPrototypeTag,
		strLayerTag,
		strObjectTag,
		pObject);

	if (0 == _wcsicmp(strPrototypeTag.c_str(), Client::CMapStage::PROTOTYPE_TAG))
		pLevel->m_pMapStage = dynamic_cast<Client::CMapStage*>(pObject);
}

void CLevel_Edit::Set_Selected(CGameObject* pSelected)
{
	if (m_pSelected == pSelected)
		return;

	if (auto* pPrevStatic = dynamic_cast<CEnvObject_Static*>(m_pSelected))
		pPrevStatic->Set_EditorForceMainPassNonInstanced(false);

	if (auto* pPrevLensFlare = dynamic_cast<Client::CLD_LensFlare*>(m_pSelected))
		pPrevLensFlare->Set_EditorPreviewActive(false);

	m_pSelected = pSelected;

	if (auto* pNewStatic = dynamic_cast<CEnvObject_Static*>(m_pSelected))
		pNewStatic->Set_EditorForceMainPassNonInstanced(true);

	if (auto* pLensFlare = dynamic_cast<Client::CLD_LensFlare*>(m_pSelected))
		pLensFlare->Set_EditorPreviewActive(true);

	if (auto* pRenderGlobals = dynamic_cast<CEnvTrigger_RenderGlobals*>(m_pSelected))
		pRenderGlobals->Apply_RenderGlobals();

	++m_iSelectionRevision;
}

void CLevel_Edit::Set_CameraActive(_bool b)
{
	if (m_pCamera)
		m_pCamera->Set_Active(b);
}

void CLevel_Edit::Preview_Camera(CGameObject* pCam)
{
	if (!pCam) return;

	Set_CameraActive(false);

	// 카메라 레이어 전부 Off
	auto it = m_Layers.find(L"Layer_Camera");
	if (it != m_Layers.end())
	{
		for (auto& handle : it->second)
		{
			if (handle.pObject)
				handle.pObject->Set_Active(false);
		}
	}

	pCam->Set_Active(true);
}

void CLevel_Edit::Back_To_Edit()
{
	auto it = m_Layers.find(L"Layer_Camera");
	if (it != m_Layers.end())
	{
		for (auto& handle : it->second)
		{
			if (handle.pObject && handle.pObject != m_pCamera)
				handle.pObject->Set_Active(false);
		}
	}

	Set_CameraActive(true);
}

void CLevel_Edit::Reset_EditCameraRotation()
{
	if (nullptr == m_pCamera)
		return;

	Back_To_Edit();
	m_pCamera->Reset_Rotation();
}

void CLevel_Edit::Jump_EditCamera(_float fForwardDistance, _float fRightDistance)
{
	if (nullptr == m_pCamera)
		return;

	Back_To_Edit();
	m_pCamera->Jump_Local(fForwardDistance, fRightDistance);
}

void CLevel_Edit::Teleport_EditCamera(const _float3& vPosition)
{
	if (nullptr == m_pCamera)
		return;

	Back_To_Edit();
	m_pCamera->Get_Transform()->Set_State(STATE::POSITION, XMVectorSet(vPosition.x, vPosition.y, vPosition.z, 1.f));
}

const vector<CLevel_Edit::EDITOR_OBJECT_HANDLE>* CLevel_Edit::Get_CameraLayer() const
{
	auto it = m_Layers.find(L"Layer_Camera");
	return (it != m_Layers.end()) ? &it->second : nullptr;
}

HRESULT CLevel_Edit::Ready_EditLights()
{
	LIGHT_DESC LightDesc{};

	LightDesc.eType = LIGHT::DIRECTIONAL;
	LightDesc.vDiffuse = _float4(5.0f, 5.5f, 6.0f, 1.f);
	LightDesc.vAmbient = _float4(0.f, 0.f, 0.f, 1.f);
	LightDesc.vSpecular = _float4(1.f, 1.f, 1.f, 1.f);
	LightDesc.vDirection = _float4(0.557f, -0.766f, 0.321f, 0.f);

	if (FAILED(m_pGameInstance_Proxy->Add_Light(LightDesc)))
		return E_FAIL;

	return S_OK;
}

HRESULT CLevel_Edit::Ready_EditCamera()
{
	if (!m_pGameInstance_Proxy->Has_Prototype(ETOUI(TOOL_LEVEL::STATIC), CEditCamera::PROTOTYPE_TAG))
	{
		m_pGameInstance_Proxy->Add_Prototype(
			ETOUI(TOOL_LEVEL::STATIC),
			CEditCamera::PROTOTYPE_TAG,
			CEditCamera::Create(m_pDevice, m_pContext));
	}

	// 오브젝트 매니저에 추가 (레이어는 카메라 전용)
	CGameObject* pCam = nullptr;
	CEditCamera::EDIT_CAMERA_FREE_DESC desc{};
	desc.vEye = { 0.f, 5.f, -10.f };
	desc.vAt = { 0.f, 0.f,   0.f };
	desc.fFovy = XMConvertToRadians(60.f);
	desc.fNear = 0.1f; desc.fFar = 1000.f;
	desc.fSpeedPerSec = 20.f;
	desc.fRotationPerSec = XMConvertToRadians(180.f);
	desc.fMouseSensor = 0.05f;

	m_pGameInstance_Proxy->Add_GameObject_Return(
		&pCam,
		ETOUI(TOOL_LEVEL::STATIC), CEditCamera::PROTOTYPE_TAG,
		ETOUI(TOOL_LEVEL::STATIC), L"Layer_Camera", L"Edit_Camera", &desc);

	m_pCamera = static_cast<CEditCamera*>(pCam);

	return S_OK;
}

HRESULT CLevel_Edit::Ready_EditGrid()
{
	m_pGrid = CEdit_Grid::Create(m_pDevice, m_pContext, 100u, 100.f);
	return (m_pGrid == nullptr) ? E_FAIL : S_OK;
}

HRESULT CLevel_Edit::Ready_MapStage()
{
	MAP_EDIT_DATA MapContentDesc = Build_MapPreviewContentDescSnapshot();

	if (0 > MapContentDesc.iPresetIndex)
		return E_FAIL;

	if (MapContentDesc.strManifestPath.empty())
	{
		if (FAILED(CMap_Loader::Get_MapManifestPath(
			static_cast<_uint>(MapContentDesc.iPresetIndex),
			&MapContentDesc.strManifestPath)))
		{
			return E_FAIL;
		}

		Apply_MapPreviewContentDesc(MapContentDesc, true);
	}

	MAP_RUNTIME_LOAD_CONTEXT Context{};
	Context.pDevice = m_pDevice;
	Context.pContext = m_pContext;
	Context.iPlaceLevel = ETOUI(TOOL_LEVEL::EDIT);
	Context.iModelLevel = ETOUI(LEVEL::STATIC);
	Context.pCreatedCallback = &On_MapPreviewObjectCreated;
	Context.pCallbackContext = this;

	CMapStage* pLoadedStage = nullptr;
	if (FAILED(CMap_Loader::Load_MapStage_Runtime(
		Context,
		MapContentDesc.strManifestPath,
		&pLoadedStage)))
	{
		return E_FAIL;
	}

	m_pMapStage = pLoadedStage;

	wstring strLoadedStageName = (nullptr != m_pMapStage) ? m_pMapStage->Get_StageName() : L"";
	if (strLoadedStageName.empty())
	{
		strLoadedStageName = StrToWstr(CMap_Loader::Get_MapName(
			static_cast<_uint>(MapContentDesc.iPresetIndex)));
	}

	Set_MapPreviewStageRuntime(true, strLoadedStageName);

	return S_OK;
}

HRESULT CLevel_Edit::Ready_EnvObjects(vector<ENV_OBJECT_DESC>* pOutDeletedEnvDescs, MAP_LOAD_RESULT* pOutReport)
{
	if (nullptr != pOutReport)
		*pOutReport = {};

	MAP_EDIT_DATA MapContentDesc = Build_MapPreviewContentDescSnapshot();

	if (0 > MapContentDesc.iPresetIndex)
		return E_FAIL;

	if (MapContentDesc.strManifestPath.empty())
	{
		if (FAILED(CMap_Loader::Get_MapManifestPath(
			static_cast<_uint>(MapContentDesc.iPresetIndex),
			&MapContentDesc.strManifestPath)))
		{
			return E_FAIL;
		}

		Apply_MapPreviewContentDesc(MapContentDesc, true);
	}

	MAP_RUNTIME_LOAD_CONTEXT Context{};
	Context.pDevice = m_pDevice;
	Context.pContext = m_pContext;
	Context.iPlaceLevel = ETOUI(TOOL_LEVEL::EDIT);
	Context.iModelLevel = ETOUI(LEVEL::STATIC);
	Context.pCreatedCallback = &On_MapPreviewObjectCreated;
	Context.pCallbackContext = this;
	Context.ppOutEnvInstanceController = &m_pMapPreviewEnvInstanceController;

	MAP_LOAD_RESULT Report{};
	const HRESULT hr = CMap_Loader::Load_Env_Runtime(
		Context,
		MapContentDesc.strManifestPath,
		&MapContentDesc.OverrideDesc,
		&Report,
		pOutDeletedEnvDescs,
		true);

	if (nullptr != pOutReport)
		*pOutReport = Report;

	if (FAILED(hr))
	{
		Log_Warning("Environment preview load failed: " + WstrToStr(Build_EnvPreviewCountText(Report, true)));
		return E_FAIL;
	}

	Set_MapPreviewEnvRuntime(true, Report.iEnvCreatedCount);
	Log_Info("Environment preview loaded: " + WstrToStr(Build_EnvPreviewCountText(Report)));
	return S_OK;
}

_bool XM_CALLCONV CLevel_Edit::Pick_EnvObjectByRay(_fvector vOrigin, _fvector vDir, CGameObject** ppOutObject, _float3* pOutHit, _float* pOutDistance)
{
	struct ENV_PICK_RESULT
	{
		_bool bHit = false;
		_float fDistance = FLT_MAX;
		_float3 vHit = {};
		CGameObject* pObject = nullptr;
	};

	if (nullptr != ppOutObject)
		*ppOutObject = nullptr;

	ENV_PICK_RESULT NormalHit{};
	ENV_PICK_RESULT DecalHit{};

	const auto IsEnvLayer = [](const wstring& strLayerTag) -> _bool
		{
			return strLayerTag == L"Layer_EnvStatic"
				|| strLayerTag == L"Layer_EnvInteract"
				|| strLayerTag == L"Layer_EnvEffect";
		};

	for (auto& [strLayerTag, Objects] : m_Layers)
	{
		if (!IsEnvLayer(strLayerTag))
			continue;

		for (const EDITOR_OBJECT_HANDLE& Handle : Objects)
		{
			CGameObject* pObject = Handle.pObject;
			if (nullptr == pObject || pObject->Is_Dead() || !pObject->Is_Active())
				continue;

			CEnvObject* pEnvObject = dynamic_cast<CEnvObject*>(pObject);
			if (nullptr == pEnvObject)
				continue;

			_float3 vHit = {};
			_float fDistance = FLT_MAX;

			if (!pEnvObject->Pick_Marb1e(vOrigin, vDir, &vHit, &fDistance))
				continue;

			ENV_PICK_RESULT& BestHit = pEnvObject->Is_Decal() ? DecalHit : NormalHit;
			if (fDistance < BestHit.fDistance)
			{
				BestHit.bHit = true;
				BestHit.fDistance = fDistance;
				BestHit.vHit = vHit;
				BestHit.pObject = pObject;
			}
		}
	}

	const ENV_PICK_RESULT& BestHit = NormalHit.bHit ? NormalHit : DecalHit;
	if (!BestHit.bHit)
		return false;

	if (nullptr != ppOutObject)
		*ppOutObject = BestHit.pObject;

	if (nullptr != pOutHit)
		*pOutHit = BestHit.vHit;

	if (nullptr != pOutDistance)
		*pOutDistance = BestHit.fDistance;

	return true;
}

HRESULT CLevel_Edit::Prepare_MapContentForPreviewLoad(_uint iPresetIndex, _bool bPresetChanged, _bool bPreserveEnvRuntimeState)
{
	MAP_EDIT_DATA WorkingDesc{};

	if (!bPresetChanged && nullptr != m_pMapPreviewSession)
		WorkingDesc = m_pMapPreviewSession->Build_EditDataSnapShot();

	WorkingDesc.bHasMapContent = true;
	WorkingDesc.iPresetIndex = static_cast<_int>(iPresetIndex);

	if (WorkingDesc.strManifestPath.empty())
	{
		if (FAILED(CMap_Loader::Get_MapManifestPath(iPresetIndex, &WorkingDesc.strManifestPath)))
		{
			return E_FAIL;
		}
	}

	if (bPresetChanged || nullptr == m_pMapPreviewSession
		|| !m_pMapPreviewSession->Get_EditData().bHasMapContent)
	{
		MAP_EDIT_DATA SeedDesc = WorkingDesc;
		const HRESULT hrAsset = CMap_Loader::Load_PresetEditFile(iPresetIndex, WorkingDesc.strManifestPath, &SeedDesc);

		if (FAILED(hrAsset))
			return E_FAIL;

		WorkingDesc = SeedDesc;
	}

	Apply_MapPreviewContentDesc(WorkingDesc, bPreserveEnvRuntimeState);
	return S_OK;
}

MAP_EDIT_DATA CLevel_Edit::Build_MapPreviewContentDescSnapshot() const
{
	if (nullptr != m_pMapPreviewSession)
		return m_pMapPreviewSession->Build_EditDataSnapShot();

	return {};
}

void CLevel_Edit::Apply_MapPreviewContentDesc(const MAP_EDIT_DATA& Desc, _bool bPreserveRuntimeState)
{
	if (nullptr == m_pMapPreviewSession)
		return;

	if (bPreserveRuntimeState)
		m_pMapPreviewSession->Set_EditMeta(Desc);
	else
		m_pMapPreviewSession->Set_EditData(Desc);
}

_wstring CLevel_Edit::Make_MapPreviewSectionKey(const CMapSection* pSection) const
{
	if (nullptr == pSection)
		return L"";

	const _wstring strStageName = nullptr != m_pMapStage
		? m_pMapStage->Get_StageName()
		: Get_MapPreviewLoadedStageNameRef();

	return CMap_EditFile::Make_SectionKey(
		strStageName,
		pSection->Get_SectionName());
}

const _wstring& CLevel_Edit::Get_MapPreviewLoadedStageNameRef() const
{
	static const _wstring s_strEmpty = L"";

	return nullptr != m_pMapPreviewSession
		? m_pMapPreviewSession->Get_LoadedStageName()
		: s_strEmpty;
}

_uint CLevel_Edit::Get_MapPreviewEnvCreatedCountInternal() const
{
	return nullptr != m_pMapPreviewSession
		? m_pMapPreviewSession->Get_EnvCreatedCount()
		: 0;
}

void CLevel_Edit::Set_MapPreviewStageRuntime(_bool bLoaded, const wstring& strStageName)
{
	if (nullptr == m_pMapPreviewSession)
		return;

	if (bLoaded)
		m_pMapPreviewSession->Set_LoadedStageName(strStageName);
	else
		m_pMapPreviewSession->Clear_LoadedStage();
}

void CLevel_Edit::Set_MapPreviewEnvRuntime(_bool bLoaded, _uint iCreatedCount)
{
	if (nullptr == m_pMapPreviewSession)
		return;

	m_pMapPreviewSession->Set_EnvLoaded(bLoaded);
	m_pMapPreviewSession->Set_EnvCreatedCount(iCreatedCount);
}

void CLevel_Edit::Set_MapPreviewStatus(const wstring& strStatus)
{
	if (nullptr != m_pMapPreviewSession)
		m_pMapPreviewSession->Set_PreviewStatus(strStatus);
}

void CLevel_Edit::Sync_MapPreviewRuntimeStateToSession()
{
	if (nullptr == m_pMapPreviewSession)
		return;

	const MAP_EDIT_CHANGE WorkingDelta =
		m_pMapPreviewSession->Build_ChangeSnapShot();

	MAP_EDIT_DATA MapContentDesc = Build_MapPreviewContentDescSnapshot();

	_bool bSessionLoadStage = m_pMapPreviewSession->Is_StageLoaded();
	if (!bSessionLoadStage)
		bSessionLoadStage = !WorkingDelta.EditedMapSections.empty();

	_bool bSessionLoadEnv = m_pMapPreviewSession->Is_EnvLoaded();
	if (!bSessionLoadEnv)
	{
		bSessionLoadEnv = !WorkingDelta.DeletedEnvObjectKeys.empty()
			|| !WorkingDelta.EditedEnvObjects.empty()
			|| !WorkingDelta.AddedMapObjects.empty();
	}

	MapContentDesc.bHasMapContent = MapContentDesc.bHasMapContent
		|| 0 <= MapContentDesc.iPresetIndex
		|| !MapContentDesc.strManifestPath.empty();
	MapContentDesc.bLoadStage = bSessionLoadStage;
	MapContentDesc.bLoadEnv = bSessionLoadEnv;
	MapContentDesc.OverrideDesc = WorkingDelta;

	Apply_MapPreviewContentDesc(MapContentDesc, true);
	m_pMapPreviewSession->Set_LoadStageEnabled(bSessionLoadStage);
	m_pMapPreviewSession->Set_LoadEnvEnabled(bSessionLoadEnv);
}

_bool CLevel_Edit::Handle_MapSpecificDeletion(CGameObject* pObject)
{
	const _bool bWasMapPreviewObject =
		m_MapPreviewObjects.find(pObject) != m_MapPreviewObjects.end();

	const _bool bWasAddedMapObject =
		(nullptr != m_pMapPreviewSession)
		&& m_pMapPreviewSession->Is_AddedObject(pObject);

	if (bWasAddedMapObject)
	{
		if (nullptr != m_pMapPreviewSession)
		{
			m_pMapPreviewSession->Unregister_AddedObject(pObject);
			m_pMapPreviewSession->Unregister_PreviewObject(pObject);
		}

		m_MapPreviewObjects.erase(pObject);

		wstring strStatus = L"Added map override object removed.";
		if (nullptr != m_pMapPreviewSession)
		{
			strStatus += L" / added="
				+ to_wstring(m_pMapPreviewSession->Get_AddedObjectCount());
		}

		Set_MapPreviewStatus(strStatus);
		Sync_MapPreviewRuntimeStateToSession();
		return true;
	}

	if (!bWasMapPreviewObject)
		return false;
	if (nullptr == m_pMapPreviewSession)
		return false;

	const _bool bCanDeleteEnvOverride =
		m_pMapPreviewSession->Can_DeleteAsEnvOverride(pObject);
	const _bool bCanDeleteLevelDesignOverride =
		m_pMapPreviewSession->Can_DeleteAsLevelDesignOverride(pObject);

	if (!bCanDeleteEnvOverride && !bCanDeleteLevelDesignOverride)
		return false;

	const _bool bTrackedDeletedPreview =
		m_pMapPreviewSession->Track_DeletedPreviewObject(pObject);
	m_pMapPreviewSession->Unregister_AddedObject(pObject);

	m_MapPreviewObjects.erase(pObject);

	if (pObject == m_pMapStage)
	{
		m_pMapStage = nullptr;
		Set_MapPreviewStageRuntime(false, L"");
	}
	else if (bCanDeleteEnvOverride && 0 < Get_MapPreviewEnvCreatedCountInternal())
	{
		const _uint iNextEnvCount = Get_MapPreviewEnvCreatedCountInternal() - 1;
		Set_MapPreviewEnvRuntime(0 < iNextEnvCount, iNextEnvCount);
	}
	else if (bCanDeleteEnvOverride)
	{
		Set_MapPreviewEnvRuntime(false, 0);
	}

#ifdef _DEBUG
	if (bCanDeleteEnvOverride && bTrackedDeletedPreview)
	{
		for (const auto& [strCurrentLayer, CurrentObjects] : m_Layers)
		{
			if (strCurrentLayer != L"Layer_EnvStatic"
				&& strCurrentLayer != L"Layer_EnvInteract")
			{
				continue;
			}

			for (const auto& Handle : CurrentObjects)
			{
				if (m_MapPreviewObjects.find(Handle.pObject) == m_MapPreviewObjects.end())
					continue;

				Client::CEnvObject* pEnvObject =
					dynamic_cast<Client::CEnvObject*>(Handle.pObject);
			}
		}
	}
#endif

	if (bTrackedDeletedPreview && bCanDeleteEnvOverride)
	{
		Set_MapPreviewStatus(
			L"Map preview env override deleted. / deleted="
			+ to_wstring(m_pMapPreviewSession->Get_DeletedEnvCount()));
	}
	else if (bTrackedDeletedPreview && bCanDeleteLevelDesignOverride)
	{
		Set_MapPreviewStatus(
			L"Map preview LevelDesign override deleted. / deleted="
			+ to_wstring(m_pMapPreviewSession->Get_DeletedLevelDesignCount()));
	}
	else if (!Is_MapStageLoaded() && !Is_MapEnvLoaded())
	{
		Set_MapPreviewStatus(L"Map preset not loaded.");
	}
	else if (!Is_MapStageLoaded())
	{
		Set_MapPreviewStatus(
			L"Environment preview loaded only. / env="
			+ to_wstring(Get_MapPreviewEnvCreatedCountInternal()));
	}
	else if (!Is_MapEnvLoaded())
	{
		const wstring strStageName = Get_MapPreviewLoadedStageNameRef().empty()
			? L"(loaded stage)"
			: Get_MapPreviewLoadedStageNameRef();

		Set_MapPreviewStatus(
			L"Map stage preview loaded: " + strStageName + L" / env=0");
	}
	else
	{
		const wstring strStageName = Get_MapPreviewLoadedStageNameRef().empty()
			? L"(loaded stage)"
			: Get_MapPreviewLoadedStageNameRef();

		Set_MapPreviewStatus(
			L"Map preset loaded: " + strStageName
			+ L" / env=" + to_wstring(Get_MapPreviewEnvCreatedCountInternal()));
	}

	Sync_MapPreviewRuntimeStateToSession();
	return true;
}

_bool CLevel_Edit::Promote_MapPreviewEnvObjectToAdded(CGameObject* pObject)
{
	if (nullptr == pObject || nullptr == m_pMapPreviewSession)
		return false;

	CEnvObject* pEnvObject = dynamic_cast<CEnvObject*>(pObject);
	if (nullptr == pEnvObject || ENV_OBJECT_KIND::EFFECT != pEnvObject->Get_Desc().eKind)
		return false;

	if (m_pMapPreviewSession->Is_AddedObject(pObject))
		return true;

	if (!m_pMapPreviewSession->Can_DeleteAsEnvOverride(pObject))
		return false;

	_wstring strPrototypeTag;
	_wstring strLayerTag;
	_wstring strObjectTag;

	for (const auto& [strCurrentLayer, Objects] : m_Layers)
	{
		const auto Iter = find_if(
			Objects.begin(),
			Objects.end(),
			[pObject](const EDITOR_OBJECT_HANDLE& Handle)
			{
				return Handle.pObject == pObject;
			});

		if (Iter == Objects.end())
			continue;

		strPrototypeTag = Iter->strPrototypeTag;
		strLayerTag = strCurrentLayer;
		strObjectTag = Iter->strName;
		break;
	}

	if (strPrototypeTag.empty() || strLayerTag.empty() || strObjectTag.empty())
		return false;

	const _wstring strStableKey = CMap_EditFile::Make_EnvKey(pEnvObject->Get_Desc());
	if (strStableKey.empty())
		return false;

	MAP_ADD_OBJECT AddedDesc{};
	AddedDesc.strPrototypeTag = strPrototypeTag;
	AddedDesc.strLayerTag = strLayerTag;
	AddedDesc.strObjectTag = strObjectTag;
	AddedDesc.strReplacedEnvKey = strStableKey;
	AddedDesc.jObject = pObject->Serialize();

	m_pMapPreviewSession->Clear_EditedPreviewObject(pObject);

	if (!m_pMapPreviewSession->Track_DeletedPreviewObject(pObject))
		return false;

	m_pMapPreviewSession->Register_AddedObject(pObject, AddedDesc, strObjectTag);
	return true;
}

_bool CLevel_Edit::Try_RegisterAddedMapOverridePlacement(CGameObject* pObject, const _wstring& strObjectTag)
{
	if (nullptr == pObject || nullptr == m_pMapPreviewSession)
		return false;

	if (!m_pMapPreviewSession->Get_EditData().bHasMapContent)
		return false;

	if (!CMap_Loader::Is_MapLayer(m_strPendingLayer)
		&& !CLevelDesign_Registry::Is_LevelDesignLayer(m_strPendingLayer))
		return false;

	MAP_ADD_OBJECT AddedDesc{};
	AddedDesc.strPrototypeTag = m_strPendingProto;
	AddedDesc.strLayerTag = m_strPendingLayer;
	AddedDesc.strObjectTag = strObjectTag;
	AddedDesc.jObject = pObject->Serialize();

	m_MapPreviewObjects.insert(pObject);
	m_pMapPreviewSession->Register_AddedObject(pObject, AddedDesc, strObjectTag);

	MAP_EDIT_DATA MapContentDesc = Build_MapPreviewContentDescSnapshot();
	MapContentDesc.bLoadEnv = true;
	Apply_MapPreviewContentDesc(MapContentDesc, true);

	Set_MapPreviewStatus(
		L"Added map override object: " + strObjectTag
		+ L" / added=" + to_wstring(m_pMapPreviewSession->Get_AddedObjectCount()));

	Sync_MapPreviewRuntimeStateToSession();
	return true;
}

void CLevel_Edit::Try_RegisterLoadedAddedMapObject(CGameObject* pObject, const _wstring& strPrototypeTag,
	const _wstring& strLayerTag, const _wstring& strObjectTag)
{
	if (nullptr == pObject || nullptr == m_pMapPreviewSession)
		return;

	const auto& AddedMapObjects =
		m_pMapPreviewSession->Get_EditData().OverrideDesc.AddedMapObjects;

	for (const MAP_ADD_OBJECT& AddedDesc : AddedMapObjects)
	{
		if (AddedDesc.strPrototypeTag != strPrototypeTag)
			continue;
		if (AddedDesc.strLayerTag != strLayerTag)
			continue;
		if (AddedDesc.strObjectTag != strObjectTag)
			continue;

		const _wstring strDisplayName =
			strObjectTag.empty() ? strPrototypeTag : strObjectTag;

		m_pMapPreviewSession->Register_AddedObject(
			pObject,
			AddedDesc,
			strDisplayName);
		break;
	}
}

CLevel_Edit* CLevel_Edit::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CLevel_Edit* pInstance = new CLevel_Edit(pDevice, pContext);

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("Failed to Created : CLevel_Edit");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CLevel_Edit::Free()
{
	m_pMapStage = nullptr;

	Safe_Release(m_pGrid);
	Safe_Release(m_pMapPreviewSession);

	__super::Free();
}
