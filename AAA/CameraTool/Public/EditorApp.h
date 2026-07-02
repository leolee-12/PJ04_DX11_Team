#pragma once

#include "Editor_Defines.h"
#include "Base.h"

NS_BEGIN(Engine)
class CGameInstance_Proxy;
NS_END


NS_BEGIN(Editor)

class CImGui_Manager;
class CLevel_Edit;

class CEditorApp final : public CBase
{
private:
	CEditorApp();
	virtual ~CEditorApp() = default;

public:
	HRESULT Initialize();
	void Update(_float fTimeDelta, _float fRawTimeDelta);
	HRESULT Render();


private:
	ID3D11Device* m_pDevice = { nullptr };
	ID3D11DeviceContext* m_pContext = { nullptr };
	CGameInstance_Proxy* m_pGameInstance_Proxy = { nullptr };

	CImGui_Manager* m_pImGui_Manager = { nullptr };

	ID3D11RenderTargetView* m_pRTV = { nullptr };
	ID3D11ShaderResourceView* m_pSRV = { nullptr };
	ID3D11DepthStencilView* m_pDSV = { nullptr };

	ID3D11RenderTargetView*		m_pRTV2 = { nullptr };
	ID3D11ShaderResourceView*	m_pSRV2 = { nullptr };
	ID3D11DepthStencilView*		m_pDSV2 = { nullptr };

	CLevel_Edit* m_pLevel_Edit = { nullptr };
	_bool		 m_bPreviewFrame = { false };

private:
	HRESULT Start_Level();
	HRESULT Ready_SharedResources();
	HRESULT Ready_EditRTV();
	HRESULT Make_OffscreenRTV(ID3D11RenderTargetView** ppRTV, ID3D11ShaderResourceView** ppSRV, ID3D11DepthStencilView** ppDSV);
	void Editor_BeginDraw();

public:
	static CEditorApp* Create();
	virtual void Free() override;
};

NS_END