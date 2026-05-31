#pragma once

#include "Base.h"

/* 1.화면에 그려져야할 객체들을 그리는 순서대로 모아놓는다. */
/* 2.보관된 순서대로 객체들의 드로우콜을 해준다. */

NS_BEGIN(Engine)

class CGameObject;
class CUIObject;
class CGameInstance_Proxy;
class CVIBuffer_Rect;
class CShader;
class CComponent;

class CRenderer final : public CBase
{
private:
	CRenderer(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CRenderer() = default;

public:
	HRESULT Initialize();
	void Add_RenderGroup(RENDERID eGroupID, CGameObject* pGameObject);
	void Add_RenderGroup_UI(RENDERUIID eGroupID, CUIObject* pUIObject);
	HRESULT Draw();

#ifdef _DEBUG
	void Add_DebugComponent(class CComponent* pComponent);
#endif

private:
	ID3D11Device* m_pDevice = { nullptr };
	ID3D11DeviceContext* m_pContext = { nullptr };

private:
	vector<CGameObject*>	m_RenderObjects[ETOUI(RENDERID::END)];
	vector<CUIObject*>		m_RenderUIs[ETOUI(RENDERUIID::END)];

	CGameInstance_Proxy*	m_pGameInstance_Proxy = { nullptr };
	CShader*				m_pShader = { nullptr };
	CVIBuffer_Rect*			m_pVIBuffer = { nullptr };
	_float4x4				m_WorldMatrix{};

	ID3D11DepthStencilView* m_pMaxDSV = { nullptr };

#ifdef _DEBUG
	list<CComponent*>		m_DebugComponents;
#endif

private:
	HRESULT Render_Priority();
	HRESULT Render_Shadow();
	HRESULT Render_NonBlend();
	HRESULT Render_Lights();
	HRESULT Render_Combined();
	HRESULT Render_NonLight();
	HRESULT Render_Blend();
	HRESULT Render_UI_BACK();
	HRESULT Render_UI_MIDDLE();
	HRESULT Render_UI_FRONT();

private:
	HRESULT Ready_DepthStencil_Buffer();
	HRESULT Change_ViewportDesc(_uint iWidth, _uint iHeight);

#ifdef _DEBUG
private:
	HRESULT Render_Debug();
#endif

public:
	static CRenderer* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void Free() override;
};

NS_END