#pragma once

#include "AnimUITool_Defines.h"
#include "Base.h"

NS_BEGIN(Engine)
class CGameInstance_Proxy;
NS_END

NS_BEGIN(AnimUITool)
class CPanel_Manager;
class CPanel_Viewport;
class CPanel_UICanvas;
class CLevel_Tool;

class CAnimUITool_App final : public CBase
{
private:
	CAnimUITool_App();
	virtual ~CAnimUITool_App() = default;

public:
	HRESULT						Initialize();
	void						Update(_float fTimeDelta);
	HRESULT						Render();

	void						OnResize(_uint iWidth, _uint iHeight);

private:
	CGameInstance_Proxy*		m_pGameInstance_Proxy = { nullptr };
	ID3D11Device*				m_pDevice = { nullptr };
	ID3D11DeviceContext*		m_pContext = { nullptr };

	CPanel_Manager*				m_pPanel_Manager = { nullptr };
	CPanel_Viewport*			m_pViewportPanel = { nullptr };
	CPanel_UICanvas*			m_pUICanvasPanel = { nullptr };
	CLevel_Tool*				m_pToolLevel = { nullptr };


	ID3D11RenderTargetView*		m_pRTV = { nullptr };
	ID3D11ShaderResourceView*	m_pSRV = { nullptr };
	ID3D11DepthStencilView*		m_pDSV = { nullptr };

	_uint						m_iViewportWidth = { 1600 };
	_uint						m_iViewportHeight = { 900 };

private:
	HRESULT						Init_Engine();
	HRESULT						Init_ImGui();
	void						Render_DockSpace();

	HRESULT						Ready_EditRTV();
	void						Editor_BeginDraw();

public:
	static CAnimUITool_App*		Create();

protected:
	virtual void				Free() override;
};

NS_END