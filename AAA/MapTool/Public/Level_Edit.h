#pragma once
#include "MapTool_Defines.h"
#include "Level.h"
#include "Map_LevelContent.h"

NS_BEGIN(Engine)
class CGameObject;
class CCamera;
NS_END

NS_BEGIN(Client)
class CLumia;
class CMapStage;
class CMap_EditSession;
NS_END

NS_BEGIN(MapTool)
class CEditCamera;
class CEdit_Grid;

class CLevel_Edit final : public CLevel
{
public:
	struct EDITOR_OBJECT_HANDLE
	{
		wstring			strPrototypeTag;
		wstring			strName;
		CGameObject*	pObject;
	};

private:
	CLevel_Edit(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CLevel_Edit() = default;

public:
	virtual HRESULT Initialize();
	virtual void Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

public:
	CGameObject* Spawn_Object(const wstring& strProtoTag, const wstring& strLayerTag, const wstring& strName, void* pArg = nullptr);

public:
	// Layer
	void Add_Layer(const wstring& strLayerTag);
	HRESULT Remove_Layer(const wstring& strLayerTag);
	void Change_ObjectLayer(CGameObject* pObject, const wstring& strNewLayer);
	void Delete_Object(CGameObject* pObject);

	// Grid & Placement
	void Pick_And_Place(_fvector vOrigin, _fvector vDir);
	void Pick_And_SelectEnvObject(_fvector vOrigin, _fvector vDir);
	void Place_Object_At(const _float3& vPos);
	void Begin_PlaceMode(const wstring& strProtoTag, const wstring& strLayerTag);
	void End_PlaceMode();
	_bool Is_PlaceMode() const { return m_ePlaceMode == PLACE_MODE::PENDING; }

	// Map
	HRESULT Load_MapPreview(_uint iPresetIndex);
	HRESULT Load_MapPreviewStage(_uint iPresetIndex);
	HRESULT Load_MapPreviewEnv(_uint iPresetIndex);
	void    Clear_MapPreview();
	void    Clear_MapPreviewStage();
	void    Clear_MapPreviewEnv();

	const _wstring& Get_MapPreviewStatus() const;
	_bool   Is_MapStageLoaded() const;
	_bool   Is_MapEnvLoaded() const;
	_bool   Is_MapPreviewObject(CGameObject* pObject) const
	{
		return nullptr != pObject
			&& m_MapPreviewObjects.find(pObject) != m_MapPreviewObjects.end();
	}

	const CMap_EditSession* Get_MapPreviewSession() const { return m_pMapPreviewSession; }
	HRESULT Restore_DeletedMapPreviewEnv(const _wstring& strStableKey);
	HRESULT Restore_AllDeletedMapPreviewEnv();
	HRESULT Save_MapOverride();

	// Getter & Setter
	const unordered_map<wstring, vector<EDITOR_OBJECT_HANDLE>>& Get_Layers() { return m_Layers; }
	CGameObject* Get_Selected() { return m_pSelected; }
	const wstring& Get_PendingProto() const { return m_strPendingProto; }
	const vector<EDITOR_OBJECT_HANDLE>* Get_CameraLayer() const;

	void Set_Selected(CGameObject* pSelected);
	void Set_CameraActive(_bool b);

	void Preview_Camera(CGameObject* pCam);
	void Back_To_Edit();

public:       // Hierarchy
	_uint Get_HierarchyRevision() const { return m_iHierarchyRevision; }
	_uint Get_SelectionRevision() const { return m_iSelectionRevision; }

private:
	CEditCamera* m_pCamera = { nullptr };
	CGameObject* m_pSelected = { nullptr };
	unordered_map<wstring, vector<EDITOR_OBJECT_HANDLE>> m_Layers;

	CEdit_Grid* m_pGrid = { nullptr };

	enum class PLACE_MODE { NONE, PENDING };
	PLACE_MODE m_ePlaceMode = { PLACE_MODE::NONE };
	wstring m_strPendingProto = {};
	wstring m_strPendingLayer = {};
	_uint	m_iPlaceCount = {};

	// Map
	unordered_set<CGameObject*> m_MapPreviewObjects;
	CMapStage* m_pMapStage = { nullptr };
	CMap_EditSession* m_pMapPreviewSession = { nullptr };

	// Hierarchy
	_uint m_iHierarchyRevision = {};
	_uint m_iSelectionRevision = {};

private:
	virtual HRESULT Ready_Events() override { return S_OK; }
	HRESULT	 Ready_EditLights();
	HRESULT  Ready_EditCamera();
	HRESULT  Ready_EditGrid();
	HRESULT  Ready_MapStage();
	HRESULT  Ready_EnvObjects(vector<ENV_OBJECT_DESC>* pOutDeletedEnvDescs = nullptr);

	_bool XM_CALLCONV Pick_EnvObjectByRay(_fvector vOrigin, _fvector vDir, CGameObject** ppOutObject, _float3* pOutHit, _float* pOutDistance);

	void Clear_MapPreviewLayer(const _wstring& strLayerTag);
	void Add_MapPreviewObjectHandle(const _wstring& strPrototypeTag, const _wstring& strLayerTag,
		const _wstring& strObjectTag, CGameObject* pObject);
	static void On_MapPreviewObjectCreated(void* pContext, CGameObject* pObject,
		const _wstring& strPrototypeTag, const _wstring& strLayerTag, const _wstring& strObjectTag);
	static void On_EnvObjectCreated(void* pContext, CGameObject* pObject,
		const _wstring& strPrototypeTag, const _wstring& strLayerTag, const _wstring& strObjectTag);

	HRESULT Prepare_MapContentForPreviewLoad(_uint iPresetIndex, _bool bPresetChanged, _bool bPreserveEnvRuntimeState);
	MAP_LEVEL_CONTENT_DESC Build_MapPreviewContentDescSnapshot() const;
	void    Apply_MapPreviewContentDesc(const MAP_LEVEL_CONTENT_DESC& Desc, _bool bPreserveRuntimeState = false);
	const _wstring& Get_MapPreviewLoadedStageNameRef() const;
	_uint   Get_MapPreviewEnvCreatedCountInternal() const;
	void    Set_MapPreviewStageRuntime(_bool bLoaded, const wstring& strStageName = L"");
	void    Set_MapPreviewEnvRuntime(_bool bLoaded, _uint iCreatedCount);
	void    Set_MapPreviewStatus(const wstring& strStatus);
	void    Sync_MapPreviewRuntimeStateToSession();
	_bool   Handle_MapSpecificDeletion(CGameObject* pObject);
	_bool   Try_RegisterAddedMapOverridePlacement(CGameObject* pObject, const _wstring& strObjectTag);
	void    Try_RegisterLoadedAddedMapObject(
		CGameObject* pObject,
		const _wstring& strPrototypeTag,
		const _wstring& strLayerTag,
		const _wstring& strObjectTag);

  private:	// Hierarchy
		void Mark_HierarchyDirty() { ++m_iHierarchyRevision; }

public:
	static CLevel_Edit* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void Free() override;
};

NS_END
