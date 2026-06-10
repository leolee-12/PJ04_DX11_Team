#pragma once
#include "Editor_Defines.h"
#include "Level.h"

NS_BEGIN(Engine)
class CGameObject;
class CCamera;
NS_END

NS_BEGIN(Client)
class CMapStage;
NS_END

NS_BEGIN(Editor)
class CEditCamera;
class CEdit_Grid;
class CMap_PreviewSession;

class CLevel_Edit final : public CLevel
{
public:
	typedef struct tagObjectHandle
	{
		_wstring		strPrototypeTag;
		_wstring		strName;
		CGameObject*	pObject;
	}EDITOR_OBJECT_HANDLE;

	static constexpr const _tchar* OBJECT_LAYER_TAG = L"Layer_LiveObject";

private:
	CLevel_Edit(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CLevel_Edit() = default;

public:
	virtual HRESULT	Initialize();
	virtual void	Update(_float fTimeDelta) override;
	virtual HRESULT	Render() override;

public:
	CGameObject*	Spawn_Object(const wstring& strProtoTag, const wstring& strLayerTag, const wstring& strName, void* pArg = nullptr);
	void			Save_Level(const wstring& strFilePath, const wstring& strLevelTag);
	void			Load_Level(const wstring& strFilePath);

	void			Save_LiveObjects(const wstring& strFilePath, const wstring& strLevelTag);
	void			Load_LiveObjects(const wstring& strFilePath);

public:
	void	Add_Layer(const wstring& strLayerTag);
	HRESULT	Remove_Layer(const wstring& strLayerTag);
	void	Change_ObjectLayer(CGameObject* pObject, const wstring& strNewLayer);
	void    Delete_Object(CGameObject* pObject);

public:
	void Pick_And_Place(_fvector vOrigin, _fvector vDir);
	void Place_Object_At(const _float3& vPos);
	void Begin_PlaceMode(const wstring& strProtoTag, const wstring& strLayerTag);
	void End_PlaceMode();
	_bool Is_PlaceMode() const { return m_ePlaceMode == PLACE_MODE::PENDING; }

public:
	const unordered_map<wstring, vector<EDITOR_OBJECT_HANDLE>>& Get_Layers() { return m_Layers; }
	CGameObject* Get_Selected() { return m_pSelected; }
	const wstring& Get_PendingProto() const { return m_strPendingProto; }

public:
	void	Set_Selected(CGameObject* pSelected) { m_pSelected = pSelected; }
	void	Set_CameraActive(_bool b);

	void	Preview_Camera(CGameObject* pCam);
	void	Back_To_Edit();
	const vector<EDITOR_OBJECT_HANDLE>* Get_CameraLayer() const;

public:	// Map Preview - Public Func
	HRESULT			Load_MapPreview(_uint iPresetIndex);
	HRESULT			Load_MapPreviewStage(_uint iPresetIndex);
	HRESULT			Load_MapPreviewEnv(_uint iPresetIndex);
	void			Clear_MapPreview();
	void			Clear_MapPreviewStage();
	void			Clear_MapPreviewEnv();
	_uint			Get_MapPreviewPresetCount() const;
	const _char*	Get_MapPreviewPresetLabel(_uint iPresetIndex) const;
	_bool			Is_MapPreviewLoaded() const;
	const _wstring& Get_MapPreviewStatus() const;
	const _wstring& Get_LoadedMapPreviewStageName() const;
	_uint			Get_MapPreviewEnvCreatedCount() const;
	const CMap_PreviewSession* Get_MapPreviewSession() const { return m_pMapPreviewSession; }
	HRESULT Restore_DeletedMapPreviewEnv(const _wstring& strStableKey);
	HRESULT Restore_AllDeletedMapPreviewEnv();
	HRESULT Save_MapOverride();

	// Effect
public:
	HRESULT Save_Selected_Effect(const wstring& strFilePath);
	HRESULT Load_Selected_Effect(const wstring& strFilePath);


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

	// Map Preview - Members
	CMap_PreviewSession* m_pMapPreviewSession = { nullptr };
	CMapStage*	m_pMapStage = { nullptr };
	unordered_set<CGameObject*> m_MapPreviewObjects;

private:
	virtual HRESULT Ready_Events() override { return S_OK; }
	HRESULT	 Ready_EditLights();
	HRESULT  Ready_EditCamera();
	HRESULT  Ready_EditGrid();

	// Map Preview - Private Func
	void		Clear_MapPreviewLayer(const _wstring& strLayerTag);
	void		Add_MapPreviewObjectHandle(const _wstring& strPrototypeTag, const _wstring& strLayerTag, const _wstring& strObjectTag, CGameObject* pObject);
	static void On_MapPreviewObjectCreated(void* pContext, CGameObject* pObject,
		const _wstring& strPrototypeTag, const _wstring& strLayerTag, const _wstring& strObjectTag);

	_bool	Should_SkipMapObjectForLevelSave(CGameObject* pObject) const;
	void	Append_MapLevelData(json* pInOutLevel) const;
	HRESULT	Load_MapLevelContentFromJson(const json& jLevel);
	HRESULT	Apply_MapStageOverrideFromJson(const json& jLevel);
	_bool	Handle_MapSpecificDeletion(CGameObject* pObject);
	_bool	Try_RegisterAddedMapOverridePlacement(CGameObject* pObject, const _wstring& strObjectTag);
	void	Try_RegisterLoadedAddedMapObject(CGameObject* pObject, const _wstring& strPrototypeTag, const _wstring& strLayerTag, const _wstring& strObjectTag);

public:
	static CLevel_Edit* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void Free() override;
};

NS_END
