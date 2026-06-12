#pragma once
#include "AnimUITool_Defines.h"
#include "Base.h"

NS_BEGIN(Engine)
class CGameObject;
class CUIContainerObject;
class CUIPartObject;
NS_END

NS_BEGIN(AnimUITool)

class CPanel;
class CLevel_Tool;

class CPanel_Manager final : public CBase
{
private:
	CPanel_Manager(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CPanel_Manager() = default;

public:
	HRESULT						Initialize();
	void						Update(_float fTimeDelta);
	void						Render();

	TOOL_MODE					Get_WorkMode() const { return m_eWorkMode; }
	void						Set_WorkMode(TOOL_MODE eMode) { m_eWorkMode = eMode; }

	HRESULT						Add_Panel(const _wstring& strPanelTag, CPanel* pPanel);
	CPanel* Get_Panel(const _wstring& strPanelTag);

	CGameObject* Get_Selected() const { return m_pSelected; }
	void						Set_Selected(CGameObject* pObject);
	void						Clear_Selected();

	ANIM_CONTEXT& Get_Context() { return m_Context; }
	void						Bind_Preview(CGameObject* pOwner);

	UI_CONTEXT& Get_UIContext() { return m_UIContext; }
	const UI_CONTEXT& Get_UIContext() const { return m_UIContext; }

	void						Set_UISelected(CUIContainerObject* pContainer, CUIPartObject* pPart, const _wstring& strPartTag);

	void						Set_Level(CLevel_Tool* pLevel) { m_pLevel = pLevel; }
	CLevel_Tool* Get_Level() const { return m_pLevel; }
	void						Load_Preview(const _wstring& strYshPath);
	void						Clear_Preview();
	void						Clear_UISelected();
	_bool						Validate_UISelection();

	void						Load_UI_ByPath(const _wstring& strFullPath);

	void						Bind_ForAnim(CGameObject* pObj);

private:
	ID3D11Device* m_pDevice = { nullptr };
	ID3D11DeviceContext* m_pContext = { nullptr };

	map<_wstring, CPanel*>		m_Panels;
	CGameObject* m_pSelected = { nullptr };

	CLevel_Tool* m_pLevel = { nullptr };

	ANIM_CONTEXT				m_Context;
	UI_CONTEXT					m_UIContext;

	TOOL_MODE					m_eWorkMode = { TOOL_MODE::ANIMATION };

private:
	void						Render_DockSpace();
	void						Render_ModeBar();
	_bool						Is_PanelAllowedInCurrentMode(const _wstring& strPanelTag) const;

public:
	static CPanel_Manager* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);

protected:
	virtual void				Free() override;
};

NS_END