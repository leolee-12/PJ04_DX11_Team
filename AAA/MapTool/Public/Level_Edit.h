#pragma once
#include "MapTool_Defines.h"
#include "Level.h"

NS_BEGIN(Engine)
class CGameObject;
class CCamera;
NS_END

NS_BEGIN(Client)
class CLumia;
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

	// Getter & Setter
	const unordered_map<wstring, vector<EDITOR_OBJECT_HANDLE>>& Get_Layers() { return m_Layers; }
	CGameObject* Get_Selected() { return m_pSelected; }
	const wstring& Get_PendingProto() const { return m_strPendingProto; }
	const vector<EDITOR_OBJECT_HANDLE>* Get_CameraLayer() const;

	void Set_Selected(CGameObject* pSelected) { m_pSelected = pSelected; }
	void Set_CameraActive(_bool b);

	void Preview_Camera(CGameObject* pCam);
	void Back_To_Edit();

	// NavMesh
	//void  Begin_NavEditMode();
	//void  End_NavEditMode();
	//_bool Is_NavEditMode()  const { return m_bNavEditMode; }
	//void  Nav_Undo();
	//void  Save_NavMesh(const wstring& strFilePath);
	//void  Load_NavMesh(const wstring& strFilePath);
	//const CNavMesh_Editor* Get_NavMeshEditor() const { return m_pNavMeshEditor; }
	//void Nav_Redo();

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

	//CNavMesh_Editor* m_pNavMeshEditor = { nullptr };
	_bool			 m_bNavEditMode = { false };
	CLumia* m_pLumia = { nullptr };

private:
	virtual HRESULT Ready_Events() override { return S_OK; }
	HRESULT	 Ready_EditLights();
	HRESULT  Ready_EditCamera();
	HRESULT  Ready_EditGrid();

public:
	static CLevel_Edit* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void Free() override;
};

NS_END