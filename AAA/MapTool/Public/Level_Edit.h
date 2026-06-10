#pragma once
#include "MapTool_Defines.h"
#include "Level.h"

NS_BEGIN(Engine)
class CGameObject;
class CCamera;
NS_END

NS_BEGIN(Client)
class CLumia;
class CMapStage;
NS_END

NS_BEGIN(MapTool)
class CEditCamera;
class CEdit_Grid;
//class CNavMesh_Editor;

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
	void Save_Level(const wstring& strFilePath, const wstring& strLevelTag);
	void Load_Level(const wstring& strFilePath);

public:
	// Layer
	void Add_Layer(const wstring& strLayerTag);
	HRESULT Remove_Layer(const wstring& strLayerTag);
	void Change_ObjectLayer(CGameObject* pObject, const wstring& strNewLayer);
	void Delete_Object(CGameObject* pObject);

	// Grid & Placement
	void Pick_And_Place(_fvector vOrigin, _fvector vDir);
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

	_uint   Get_MapPreviewPresetCount() const;
	const _char* Get_MapPreviewPresetLabel(_uint iPresetIndex) const;
	const _wstring& Get_MapPreviewStatus() const { return m_strMapPreviewStatus; }
	_bool   Is_MapStageLoaded() const { return m_bMapStageLoaded; }
	_bool   Is_MapEnvLoaded() const { return m_bMapEnvLoaded; }

	// Getter & Setter
	const unordered_map<wstring, vector<EDITOR_OBJECT_HANDLE>>& Get_Layers() { return m_Layers; }
	CGameObject* Get_Selected() { return m_pSelected; }
	const wstring& Get_PendingProto() const { return m_strPendingProto; }
	const vector<EDITOR_OBJECT_HANDLE>* Get_CameraLayer() const;

	void Set_Selected(CGameObject* pSelected) { m_pSelected = pSelected; }
	void Set_CameraActive(_bool b);

	void Preview_Camera(CGameObject* pCam);
	void Back_To_Edit();

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
	wstring m_strMapPreviewStatus = { L"Map preset not loaded." };
	wstring m_strLoadedMapStageName = {};
	_uint   m_iEnvObjCreatedCount = {};
	_int    m_iLoadedMapPresetIndex = { -1 };
	unordered_set<CGameObject*> m_MapPreviewObjects;
	_bool   m_bMapStageLoaded = { false };
	_bool   m_bMapEnvLoaded = { false };

	_bool m_bNavEditMode = { false };
	CLumia* m_pLumia = { nullptr };
	Client::CMapStage* m_pMapStage = { nullptr };

private:
	virtual HRESULT Ready_Events() override { return S_OK; }
	HRESULT	 Ready_EditLights();
	HRESULT  Ready_EditCamera();
	HRESULT  Ready_EditGrid();
	HRESULT  Ready_MapStage();
	HRESULT  Ready_EnvObjects();

private:
	void Clear_MapPreviewLayer(const _wstring& strLayerTag);
	void Add_MapPreviewObjectHandle(const _wstring& strPrototypeTag, const _wstring& strLayerTag,
		const _wstring& strObjectTag, CGameObject* pObject);
	static void On_MapPreviewObjectCreated(void* pContext, CGameObject* pObject,
		const _wstring& strPrototypeTag, const _wstring& strLayerTag, const _wstring& strObjectTag);
	static void On_EnvObjectCreated(void* pContext, CGameObject* pObject,
		const _wstring& strPrototypeTag, const _wstring& strLayerTag, const _wstring& strObjectTag);

public:
	static CLevel_Edit* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void Free() override;
};

NS_END
