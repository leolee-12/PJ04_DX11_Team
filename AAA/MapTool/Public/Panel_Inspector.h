#pragma once
#include "Panel.h"

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
	void    Draw_Properties(IReflectable* pHolder);
	_bool   Draw_Transform(CGameObject* pObject, const string& strSuffix = "");
	void    Draw_EnvObjectEditPanel(CLevel_Edit* pLevel, CGameObject* pObject);
	void    Draw_MapSectionEditPanel(CLevel_Edit* pLevel, CMapStage* pMapStage, CMapSection* pSection);
	void    Draw_MeshLayerPanel(CGameObject* pObject);
	void    Draw_MapStageSections(CMapStage* pMapStage);
	void    Draw_MapSectionRenderOptions(CMapSection* pSection);

private:
	unordered_map<CGameObject*, _float3>	m_RotEditEuler;
	unordered_map<CGameObject*, _bool>		m_EnvCollMeshEditStates;
	unordered_map<CMapSection*, _bool>		m_MapCollMeshEditStates;
	unordered_map<CGameObject*, _bool>		m_EnvNearAlphaEditStates;

	CMapSection* m_pFocusedMapSection = { nullptr };

private:
	_bool*	Resolve_EnvCollMeshEditState(CLevel_Edit* pLevel, CEnvObject* pEnvObject);
	_bool*	Resolve_MapCollMeshEditState(CLevel_Edit* pLevel, CMapStage* pMapStage, CMapSection* pSection);
	_bool*	Resolve_EnvNearAlphaEditState(CLevel_Edit* pLevel, CEnvObject* pEnvObject);
	void	Clear_EnvCollMeshEditState(CGameObject* pObject);
	void	Clear_MapCollMeshEditState(CMapSection* pSection);
	void	Clear_EnvNearAlphaEditState(CGameObject* pObject);

public:
	static CPanel_Inspector* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);

protected:
	virtual void	Free() override;
};

NS_END