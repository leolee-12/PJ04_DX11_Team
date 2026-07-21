#pragma once
#include "Panel.h"
#include "Editable.h"

NS_BEGIN(Engine)
class CGameObject;
class IReflectable;
NS_END

NS_BEGIN(Client)
class CMapStage;
class CMapSection;
class CEnvObject;
NS_END

NS_BEGIN(MapTool)
class CLevel_Edit;

class CPanel_Inspector final : public CPanel
{
private:
	CPanel_Inspector(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CPanel_Inspector() = default;

public:
	virtual void	Render() override;

private:
	_bool   Draw_Properties(IReflectable* pHolder);
	_bool   Draw_Transform(CGameObject* pObject, const string & strSuffix = "");
	void    Draw_EditableObjectPolicyPanel(CGameObject* pObject);
	void    Draw_EditableCustomPanel(CGameObject* pObject);
	void    Draw_EnvObjectEditPanel(CLevel_Edit* pLevel, CGameObject* pObject);
	void    Draw_MapSectionEditPanel(CLevel_Edit* pLevel, CMapStage* pMapStage, CGameObject* pObject);
	void    Draw_LevelDesignEventPanel(CLevel_Edit* pLevel, CGameObject* pObject);
	void    Draw_MeshLayerPanel(CGameObject* pObject);
	void    Draw_MapStageSections(CMapStage* pMapStage);
	void    Draw_MapSectionRenderOptions(CMapSection* pSection);

#ifdef _DEBUG
	void	Draw_MapSectionViewFilter(CMapStage* pMapStage, CMapSection* pSection, _int iSelectedMeshIndex);
#endif

private:
	unordered_map<CGameObject*, _float3>	m_RotEditEuler;
	unordered_map<CGameObject*, _bool>		m_EnvNearAlphaEditStates;
	unordered_map<_wstring, _uint> m_SelectedModelSlotByEditableKey;
	unordered_map<_wstring, _int> m_SelectedMeshByEditableSlotKey;
	unordered_map<_wstring, EDIT_OBJECT_POLICY> m_EditablePolicyDrafts;
	unordered_map<_wstring, pair<string, string>> m_LevelDesignEventDrafts;
	EDIT_CUSTOM_DESC m_EditCustomClipboard = monostate{};

	CMapSection* m_pFocusedMapSection = { nullptr };

#ifdef _DEBUG
private:
	_bool	m_bEditorSoloSection = false;
	_bool	m_bEditorSoloMesh = false;
#endif

private:
	_bool*	Resolve_EnvNearAlphaEditState(CLevel_Edit* pLevel, CEnvObject* pEnvObject);
	void    Clear_EnvNearAlphaEditState(CGameObject* pObject);

public:
	static CPanel_Inspector* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);

protected:
	virtual void	Free() override;
};

NS_END