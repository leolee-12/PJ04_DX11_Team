#pragma once
#include "MapTool_Defines.h"
#include "Base.h"

NS_BEGIN(MapTool)
class CPanel;

class CPanel_Manager final : public CBase
{
private:
	CPanel_Manager(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CPanel_Manager() = default;

public:
	HRESULT						Initialize();
	void						Update(_float fTimeDelta);
	void						Render();

	HRESULT						Add_Panel(const _wstring& strPanelTag, CPanel* pPanel);

private:
	ID3D11Device*				m_pDevice = { nullptr };
	ID3D11DeviceContext*		m_pContext = { nullptr };

	map<_wstring, CPanel*>		m_Panels;
	_bool						m_bLayoutBuilt = { false };

private:
	void						Render_DockSpace();

public:
	static CPanel_Manager*		Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);

protected:
	virtual void				Free() override;
};

NS_END
