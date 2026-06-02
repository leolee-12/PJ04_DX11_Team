#pragma once

#include "Editor_Defines.h"
#include "Base.h"

NS_BEGIN(Engine)
class CGameInstance_Proxy;
NS_END


NS_BEGIN(Editor)

class CImGui_Manager;

class CEditorApp final : public CBase
{
private:
	CEditorApp();
	virtual ~CEditorApp() = default;

public:
	HRESULT Initialize();
	void Update(_float fTimeDelta);
	HRESULT Render();


private:
	ID3D11Device* m_pDevice = { nullptr };
	ID3D11DeviceContext* m_pContext = { nullptr };
	CGameInstance_Proxy* m_pGameInstance_Proxy = { nullptr };

	CImGui_Manager* m_pImGui_Manager = { nullptr };

	ID3D11RenderTargetView* m_pRTV = { nullptr };
	ID3D11ShaderResourceView* m_pSRV = { nullptr };
	ID3D11DepthStencilView* m_pDSV = { nullptr };
private:
	HRESULT Start_Level();
	HRESULT Ready_SharedResources();

	HRESULT Ready_EditRTV();

	void Editor_BeginDraw();

public:
	static CEditorApp* Create();
	virtual void Free() override;
};

NS_END