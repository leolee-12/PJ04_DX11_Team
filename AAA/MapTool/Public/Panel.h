#pragma once
#include "MapTool_Defines.h"
#include "Base.h"

#include "imgui.h"

NS_BEGIN(Engine)
class CGameInstance_Proxy;
NS_END

NS_BEGIN(MapTool)

class CPanel_Manager;
class CEditInstance;

class CPanel abstract : public CBase
{
protected:
	CPanel(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CPanel() = default;

public:
	virtual HRESULT			Initialize(CPanel_Manager* pPanelManager);
	virtual void			Update(_float fTimeDelta) {}
	virtual void			Render() PURE;

	_bool					Is_Open() const { return m_bOpen; }
	void					Set_Open(_bool bOpen) { m_bOpen = bOpen; }
	_bool*					Get_OpenPtr() { return &m_bOpen; }
	const _char*			Get_Name() const { return m_szName; }

protected:
	_bool					Begin_Panel();
	void					End_Panel();

protected:
	ID3D11Device*			m_pDevice = { nullptr };
	ID3D11DeviceContext*	m_pContext = { nullptr };
	CGameInstance_Proxy*	m_pGI_Proxy = { nullptr };
	CPanel_Manager*			m_pPanel_Manager = { nullptr };

	_char					m_szName[MAX_PATH] = {};
	ImGuiWindowFlags		m_iWindowFlags = { 0 };
	_bool					m_bOpen = { true };

protected:
	virtual void			Free() override;
};

NS_END
