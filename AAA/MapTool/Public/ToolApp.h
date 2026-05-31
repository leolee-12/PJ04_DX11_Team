#pragma once
#include "MapTool_Defines.h"
#include "Base.h"

NS_BEGIN(Engine)
class CGameInstance_Proxy;
NS_END

NS_BEGIN(MapTool)
class CEditInstance;

class CToolApp final : public CBase
{
private:
	CToolApp();
	virtual ~CToolApp() = default;

public:
	HRESULT	Initialize();
	void	Update(_float fTimeDelta);
	HRESULT	Render();

private:
	CGameInstance_Proxy*		m_pGI_Proxy = { nullptr };
	ID3D11Device*				m_pDevice = { nullptr };
	ID3D11DeviceContext*		m_pContext = { nullptr };

	CEditInstance*				m_pEditInstance = { nullptr };

	ID3D11RenderTargetView*		m_pRTV = { nullptr };
	ID3D11ShaderResourceView*	m_pSRV = { nullptr };
	ID3D11DepthStencilView*		m_pDSV = { nullptr };

	static constexpr _uint		m_iViewportWidth = { 1600 };
	static constexpr _uint		m_iViewportHeight = { 900 };

private:
	HRESULT Ready_Engine();
	HRESULT	Init_ImGui();
	HRESULT	Ready_EditRTV();
	void	Editor_BeginDraw();
	void	Draw_LoadingOverlay();

public:
	static CToolApp* Create();

protected:
	virtual void Free() override;
};

NS_END
