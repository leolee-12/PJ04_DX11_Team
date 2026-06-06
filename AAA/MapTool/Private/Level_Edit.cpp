#include "Level_Edit.h"
#include "Loader.h"
#include "EditCamera.h"
#include "Edit_Grid.h"
#include "EnvObjectLoader.h"
#include "GameContent_Log.h"
#include "MapStage.h"
#ifdef _DEBUG
#include "MapToolProfiler.h"
#endif

#include "GameObject_Factory.h"
#include "GameContent_const.h"

#include "GameInstance.h"
#include "GameInstance_Proxy.h"
#include "GameObject.h"
#include "DataExporter.h"
#include "DataLoader.h"
//#include "NavMesh_Editor.h"

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
}

namespace
{
	using namespace std::filesystem;

	constexpr wchar_t kMapModelRoot[] = L"../../Resources/Map";

	_bool Equals_NoCase(const wstring& strLeft, const wstring& strRight)
	{
		return 0 == _wcsicmp(strLeft.c_str(), strRight.c_str());
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

		const wstring strLegacyPath = L"../../Resources/Models/Test/Stage1-0/" + strSectionName + L".ysh";
		error_code ErrorCode;
		if (!exists(path(strLegacyPath), ErrorCode) || ErrorCode)
			return false;

		*pOutPath = strLegacyPath;
		return true;
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

	if (FAILED(Ready_MapStage()))
		return E_FAIL;

	if (FAILED(Ready_EnvObjects()))
		return E_FAIL;

	//m_pNavMeshEditor = CNavMesh_Editor::Create();
	//if (nullptr == m_pNavMeshEditor)
	//	return E_FAIL;


	return S_OK;
}

void CLevel_Edit::Update(_float fTimeDelta)
{
	if (m_pGameInstance_Proxy->Key_Down(DIK_F2))
		m_pGameInstance_Proxy->Toggle_DebugRender();

	if (m_pGameInstance_Proxy->Key_Down(DIK_ESCAPE))
		m_pGameInstance_Proxy->Publish(TEXT("Return_Lobby"), nullptr);

#ifdef _DEBUG
	CMapToolProfiler* pProfiler = CMapToolProfiler::GetInstance();
	if (m_pGameInstance_Proxy->Key_Down(DIK_1))
		pProfiler->Toggle_Enabled();
	if (m_pGameInstance_Proxy->Key_Down(DIK_2))
		pProfiler->Toggle_CsvEnabled();
	if (m_pGameInstance_Proxy->Key_Down(DIK_3))
		pProfiler->Reset();

	pProfiler->Update(fTimeDelta);
#endif
}

HRESULT CLevel_Edit::Render()
{
	if (m_pGrid)
	{
		const _float4x4* pView = m_pGameInstance_Proxy->Get_Matrix(D3DTS::VIEW, PROJ_TYPE::PERSPEC);
		const _float4x4* pProj = m_pGameInstance_Proxy->Get_Matrix(D3DTS::PROJ, PROJ_TYPE::PERSPEC);
		m_pGrid->Render(pView, pProj);
	}

#ifdef _DEBUG
	CMapToolProfiler::GetInstance()->Capture_Frame();
#endif

	return S_OK;
}

CGameObject* CLevel_Edit::Spawn_Object(const wstring& strProtoTag, const wstring& strLayerTag, const wstring& strName, void* pArg)
{
	auto* pReg = CGameObject_Factory::GetInstance()->Get_Registration(strProtoTag);
	if (!pReg) return nullptr;

	if (!m_pGameInstance_Proxy->Has_Prototype(ETOUI(TOOL_LEVEL::EDIT), strProtoTag))
	{
		pReg->ResourceLoader(m_pGameInstance_Proxy, m_pDevice, m_pContext);

		m_pGameInstance_Proxy->Add_Prototype(ETOUI(TOOL_LEVEL::EDIT), strProtoTag.c_str(),
			pReg->CreatorFunc(m_pDevice, m_pContext));
	}

	wstring strFinalLayer = strLayerTag;
	if (pReg->strCategory == L"CAMERA_OBJECT")
		strFinalLayer = L"Layer_Camera";

	Engine::CGameObject* pObj = { nullptr };
	m_pGameInstance_Proxy->Add_GameObject_Return(
		&pObj,
		ETOUI(TOOL_LEVEL::EDIT), strProtoTag.c_str(),
		ETOUI(TOOL_LEVEL::EDIT), strFinalLayer.c_str(), strName, pArg);

	Add_Layer(strFinalLayer);
	m_Layers[strFinalLayer].push_back({ strProtoTag, strName, pObj });

	return pObj;
}

void CLevel_Edit::Save_Level(const wstring& strFilePath, const wstring& strLevelTag)
{
	json jLevel;
	jLevel["Level_Tag"] = WstrToStr(strLevelTag);
	if (nullptr != m_pMapStage)
		jLevel["MapStage"] = m_pMapStage->Serialize();

	json jObjects = json::array();

	for (auto& [LayerTag, Objects] : m_Layers)
	{
		for (auto& handle : Objects)
		{
			if (handle.pObject == m_pMapStage)
				continue;

			json jObj = handle.pObject->Serialize();

			jObj["Object_Tag"] = WstrToStr(handle.strName);
			jObj["Prototype_Tag"] = WstrToStr(handle.strPrototypeTag);
			jObj["Layer_Tag"] = WstrToStr(LayerTag);

			jObjects.push_back(jObj);
		}
	}

	jLevel["Objects"] = jObjects;

	CDataExporter::Write_JsonFile(strFilePath.c_str(), jLevel);
}

void CLevel_Edit::Load_Level(const wstring& strFilePath)
{
	m_pSelected = nullptr;
	m_Layers.clear();
#ifdef _DEBUG
	CMapToolProfiler::GetInstance()->Set_Stage(nullptr);
#endif
	m_pMapStage = nullptr;
	m_pGameInstance_Proxy->Clear_Objects(ETOUI(TOOL_LEVEL::EDIT));

	if (FAILED(Ready_MapStage()))
	{
		MSG_BOX("Failed to ready MapStage while loading level.");
		return;
	}

	if (FAILED(Ready_EnvObjects()))
	{
		MSG_BOX("Failed to ready EnvObjects while loading level.");
		return;
	}

	string strContent = {};
	if (FAILED(CDataLoader::Read_Json(strFilePath.c_str(), &strContent)))
	{
		MSG_BOX("Failed to read level file.");
		return;
	}

	try
	{
		json jLevel = json::parse(strContent);

		if (nullptr != m_pMapStage && jLevel.contains("MapStage"))
			m_pMapStage->Deserialize(jLevel["MapStage"]);

		for (auto& jObj : jLevel["Objects"])
		{
			string strProto = jObj["Prototype_Tag"].get<string>();
			string strLayer = jObj["Layer_Tag"].get<string>();
			string strName = jObj["Object_Tag"].get<string>();

			if (0 == _wcsicmp(StrToWstr(strProto).c_str(), Client::CMapStage::PROTOTYPE_TAG))
				continue;

			wstring wProto = StrToWstr(strProto);
			wstring wLayer = StrToWstr(strLayer);
			wstring wName = StrToWstr(strName);

			CGameObject* pObj = Spawn_Object(wProto, wLayer, wName);

			if (pObj)
				pObj->Deserialize(jObj);
		}
	}
	catch (json::exception&)
	{
		MSG_BOX("JSON PARSE ERROR");
	}
}

void CLevel_Edit::Add_Layer(const wstring& strLayerTag)
{
	if (m_Layers.find(strLayerTag) == m_Layers.end())
		m_Layers[strLayerTag] = {};
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
	return S_OK;
}

void CLevel_Edit::Change_ObjectLayer(CGameObject* pObject, const wstring& strNewLayer)
{
	if (!pObject) return;

	Add_Layer(strNewLayer);

	for (auto& [LayerTag, Objects] : m_Layers)
	{
		for (auto iter = Objects.begin(); iter != Objects.end(); ++iter)
		{
			if (iter->pObject == pObject)
			{
				if (LayerTag == strNewLayer) return;

				EDITOR_OBJECT_HANDLE handle = *iter;

				Objects.erase(iter);

				m_Layers[strNewLayer].push_back(handle);

				return;
			}
		}
	}
}

void CLevel_Edit::Delete_Object(CGameObject* pObject)
{
	if (!pObject) return;

	if (m_pSelected == pObject)
		m_pSelected = nullptr;

	for (auto& [LayerTag, Objects] : m_Layers)
	{
		for (auto iter = Objects.begin(); iter != Objects.end(); ++iter)
		{
			if (iter->pObject == pObject)
			{
				Objects.erase(iter);
				m_pGameInstance_Proxy->Destroy_GameObject(pObject);
				return;
			}
		}
	}
}

void CLevel_Edit::Pick_And_Place(_fvector vOrigin, _fvector vDir)
{
	/*if (m_bNavEditMode)
	{
		_float3 fHitPos = {};
		_float  fDummy = {};
		if (m_pLumia->Pick_Floor(vOrigin, vDir, &fHitPos, &fDummy))
			m_pNavMeshEditor->OnClick(fHitPos);
		return;
	}*/

	_float3 fHitPos = {};
	if (!m_pGameInstance_Proxy->Pick_RayToPlane(vOrigin, vDir, &fHitPos))
		return;

	if (!Is_PlaceMode()) return;
	fHitPos.x = roundf(fHitPos.x);
	fHitPos.z = roundf(fHitPos.z);
	fHitPos.y = 0.f;
	Place_Object_At(fHitPos);
}

void CLevel_Edit::Place_Object_At(const _float3& vPos)
{
	wstring strName = m_strPendingProto + L"_" + to_wstring(m_iPlaceCount++);

	CGameObject* pObj = Spawn_Object(m_strPendingProto, m_strPendingLayer, strName);
	if (pObj)
	{
		pObj->Get_Transform()->Set_State(
			STATE::POSITION,
			XMVectorSet(vPos.x, vPos.y, vPos.z, 1.f)
		);
		pObj->Initialize_NaviPlacement();
	}
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

const vector<CLevel_Edit::EDITOR_OBJECT_HANDLE>* CLevel_Edit::Get_CameraLayer() const
{
	auto it = m_Layers.find(L"Layer_Camera");
	return (it != m_Layers.end()) ? &it->second : nullptr;
}

//void CLevel_Edit::Begin_NavEditMode()
//{
//	//// CLumia 캐싱, 없으면 모드 진입 거부
//	//m_pLumia = dynamic_cast<CLumia*>(
//	//    m_pGameInstance_Proxy->Find_GameObject(ETOUI(EDIT_LEVEL::EDIT), L"Default_Layer", L"Proto_Lumia_0"));
//
//	//if (!m_pLumia) return;
//
//	//m_bNavEditMode = true;
//}
//
//void CLevel_Edit::End_NavEditMode()
//{
//	m_bNavEditMode = false;
//	m_pLumia = nullptr;  // 소유권 없음, Release 불필요
//}
//
//void CLevel_Edit::Nav_Undo()
//{
//	if (m_pNavMeshEditor)
//		m_pNavMeshEditor->Undo();
//}
//
//void CLevel_Edit::Save_NavMesh(const wstring& strFilePath)
//{
//	if (m_pNavMeshEditor)
//		m_pNavMeshEditor->Save(strFilePath);
//}
//
//void CLevel_Edit::Load_NavMesh(const wstring& strFilePath)
//{
//	if (m_pNavMeshEditor)
//		m_pNavMeshEditor->Load(strFilePath);
//}
//
//void CLevel_Edit::Nav_Redo()
//{
//	if (m_pNavMeshEditor)
//		m_pNavMeshEditor->Redo();
//}

HRESULT CLevel_Edit::Ready_EditLights()
{
	LIGHT_DESC      LightDesc{};

	LightDesc.eType = LIGHT::DIRECTIONAL;
	LightDesc.vDiffuse = _float4(1.f, 1.f, 1.f, 1.f);
	LightDesc.vAmbient = _float4(1.f, 1.f, 1.f, 1.f);
	LightDesc.vSpecular = _float4(1.f, 1.f, 1.f, 1.f);
	LightDesc.vDirection = _float4(1.f, -1.f, 1.f, 0.f);

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
	m_pGrid = CEdit_Grid::Create(m_pDevice, m_pContext, 1000, 1.f);
	return (m_pGrid == nullptr) ? E_FAIL : S_OK;
}

HRESULT CLevel_Edit::Ready_MapStage()
{
	const _uint iEditLevel = ETOUI(TOOL_LEVEL::EDIT);
	const _uint iModelLevel = ETOUI(LEVEL::GAMEPLAY);

	if (!m_pGameInstance_Proxy->Has_Prototype(iEditLevel, Client::CMapSection::PROTOTYPE_TAG))
	{
		if (FAILED(m_pGameInstance_Proxy->Add_Prototype(
			iEditLevel,
			Client::CMapSection::PROTOTYPE_TAG,
			Client::CMapSection::Create(m_pDevice, m_pContext))))
			return E_FAIL;
	}

	if (!m_pGameInstance_Proxy->Has_Prototype(iEditLevel, Client::CMapStage::PROTOTYPE_TAG))
	{
		if (FAILED(m_pGameInstance_Proxy->Add_Prototype(
			iEditLevel,
			Client::CMapStage::PROTOTYPE_TAG,
			Client::CMapStage::Create(m_pDevice, m_pContext))))
			return E_FAIL;
	}

	auto AddSectionDesc = [&](Client::MAP_STAGE_DESC& StageDesc,
		const wstring& strName,
		Client::MAP_SECTION_TYPE eType,
		RENDERID eRenderID) -> HRESULT
		{
			wstring strModelPath;
			const _bool bHasLegacyPath = Try_GetLegacyMapSectionModelPath(strName, &strModelPath);
			if (!bHasLegacyPath
				&& !Try_ResolveMapSectionModelPath(strName, &strModelPath))
				return E_FAIL;

			Client::MAP_SECTION_DESC SectionDesc{};
			SectionDesc.strSectionName = strName;
			SectionDesc.strModelProtoTag = bHasLegacyPath
				? L"Prototype_Component_Model_" + strName
				: Make_MapSectionModelProtoTag(strName);
			SectionDesc.strModelPath = strModelPath;
			SectionDesc.iModelProtoLevel = iModelLevel;
			SectionDesc.eSectionType = eType;
			SectionDesc.eRenderID = eRenderID;
			SectionDesc.bCastShadow = false;
			SectionDesc.bEnableCulling = true;
			SectionDesc.bRenderable = true;

			StageDesc.SectionDescs.push_back(SectionDesc);
			return S_OK;
		};

	Client::MAP_STAGE_DESC StageDesc{};
	StageDesc.strStageName = L"Stage1_0_MapStage";
	StageDesc.iSectionProtoLevel = iEditLevel;
	StageDesc.SectionDescs.reserve(8);

	if (FAILED(AddSectionDesc(StageDesc, L"GsAllBuilding_0", Client::MAP_SECTION_TYPE::BUILDING, RENDERID::NONBLEND))) return E_FAIL;
	if (FAILED(AddSectionDesc(StageDesc, L"GsBuilding_1", Client::MAP_SECTION_TYPE::BUILDING, RENDERID::NONBLEND))) return E_FAIL;
	if (FAILED(AddSectionDesc(StageDesc, L"GsBuilding_7", Client::MAP_SECTION_TYPE::BUILDING, RENDERID::NONBLEND))) return E_FAIL;
	if (FAILED(AddSectionDesc(StageDesc, L"GsDefault_2", Client::MAP_SECTION_TYPE::GROUND, RENDERID::NONBLEND))) return E_FAIL;
	if (FAILED(AddSectionDesc(StageDesc, L"GsDefault_5", Client::MAP_SECTION_TYPE::GROUND, RENDERID::NONBLEND))) return E_FAIL;
	if (FAILED(AddSectionDesc(StageDesc, L"SeRock_3", Client::MAP_SECTION_TYPE::ROCK, RENDERID::NONBLEND))) return E_FAIL;
	if (FAILED(AddSectionDesc(StageDesc, L"SeRock_6", Client::MAP_SECTION_TYPE::ROCK, RENDERID::NONBLEND))) return E_FAIL;
	//if (FAILED(AddSectionDesc(StageDesc, L"Transparent_4", Client::MAP_SECTION_TYPE::TRANSPARENT, RENDERID::NONBLEND))) return E_FAIL;
	//StageDesc.SectionDescs.back().bRenderable = false;

	CGameObject* pStageObject = nullptr;
	if (FAILED(m_pGameInstance_Proxy->Add_GameObject_Return(
		&pStageObject,
		iEditLevel,
		Client::CMapStage::PROTOTYPE_TAG,
		iEditLevel,
		L"Layer_MapStage",
		L"MapStage",
		&StageDesc)))
		return E_FAIL;

	m_pMapStage = dynamic_cast<Client::CMapStage*>(pStageObject);
	if (nullptr == m_pMapStage)
		return E_FAIL;

	Add_Layer(L"Layer_MapStage");
	m_Layers[L"Layer_MapStage"].push_back({
		Client::CMapStage::PROTOTYPE_TAG,
		L"MapStage",
		m_pMapStage });

#ifdef _DEBUG
	CMapToolProfiler::GetInstance()->Set_Stage(m_pMapStage);
#endif

	return S_OK;
}

HRESULT CLevel_Edit::Ready_EnvObjects()
 {
	const _uint iEditLevel = ETOUI(TOOL_LEVEL::EDIT);
	Client::Set_GameContentLogSink(&Forward_GameContentLog);
	static const wchar_t* kEnvJsonPaths[] =
	{
		L"../../Resources/Map/Decor_Decor_FlipZ_FullMatrix.json",
		L"../../Resources/Map/Toy_Decor.json",
		L"../../Resources/Map/Toy_Obj.json",
		L"../../Resources/Map/Decor_Obj.json"
	};

	for (const wchar_t* pJsonPath : kEnvJsonPaths)
	{
		string strDummy;
		if (FAILED(CDataLoader::Read_Json(pJsonPath, &strDummy)))
		{
			MapTool::Log_Info(string("EnvObject source skipped: ") + WstrToStr(pJsonPath));
			continue;
		}

		if (FAILED(Client::CEnvObjectLoader::Load_FromJsonFile(
			m_pGameInstance_Proxy,
			m_pDevice,
			m_pContext,
			iEditLevel,
			pJsonPath)))
		{
			MapTool::Log_Warning(string("EnvObject source failed: ") + WstrToStr(pJsonPath));
		}
	}

	return S_OK;
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
#ifdef _DEBUG
	CMapToolProfiler::GetInstance()->Set_Stage(nullptr);
#endif
	m_pMapStage = nullptr;

	Safe_Release(m_pGrid);
	//Safe_Release(m_pNavMeshEditor);

	__super::Free();
}
