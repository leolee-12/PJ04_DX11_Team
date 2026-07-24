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
class CComputeShader;

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
	void Add_DebugTextComponent(class CComponent* pComponent);
	void Toggle_DebugRender() { m_bDebugRender = !m_bDebugRender; }
	_bool IsOn_DebugRender() { return m_bDebugRender; }
#endif

public:
	void Bind_RenderTarget(ID3D11RenderTargetView* pRTV, ID3D11DepthStencilView* pDSV, _uint iWidth, _uint iHeight);

private:
	ID3D11Device* m_pDevice = { nullptr };
	ID3D11DeviceContext* m_pContext = { nullptr };

private:
	vector<CGameObject*>	m_RenderObjects[ETOUI(RENDERID::END)];
	vector<CUIObject*>		m_RenderUIs[ETOUI(RENDERUIID::END)];

	CGameInstance_Proxy*	m_pGameInstance_Proxy = { nullptr };
	CShader*				m_pShaderDeferred = { nullptr };
	CShader*				m_pShaderPost = { nullptr };
	CVIBuffer_Rect*			m_pVIBuffer = { nullptr };
	_float4x4				m_WorldMatrix{};

	ID3D11DepthStencilView* m_pMaxDSV = { nullptr };

#ifdef _DEBUG
	list<CComponent*>		m_DebugComponents;
	list<CComponent*>       m_DebugTextComponents;
	_bool					m_bDebugRender = { false };
#endif

	ID3D11RenderTargetView* m_pOutRTV = { nullptr };
	ID3D11DepthStencilView* m_pOutDSV = { nullptr };
	_uint                   m_iOutWidth = { 0 };
	_uint                   m_iOutHeight = { 0 };

	_uint					m_iRTWidth = {};  
	_uint					m_iRTHeight = {};

	// 볼류메트릭포그

	CComputeShader* m_pCSInject = { nullptr };
	CComputeShader* m_pCSIntegrate = { nullptr };
	ID3D11Buffer* m_pFroxelCB = { nullptr };

	ID3D11Texture3D* m_pScatterTex = { nullptr };
	ID3D11UnorderedAccessView* m_pScatterUAV = { nullptr };
	ID3D11ShaderResourceView* m_pScatterSRV = { nullptr };

	ID3D11Texture3D* m_pIntegTex = { nullptr };
	ID3D11UnorderedAccessView* m_pIntegUAV = { nullptr };
	ID3D11ShaderResourceView* m_pIntegSRV = { nullptr };

	//froxel
	ID3D11SamplerState*		m_pShadowSampler = { nullptr };
	_float                  m_fFogTime = { 0.f };


private:
	HRESULT Render_Priority();
	HRESULT Render_Sky();
	HRESULT Render_Shadow();
	HRESULT Render_ShadowBlur();
	HRESULT Render_NonBlend();
	HRESULT Render_Decals();
	HRESULT Render_SSAO();
	HRESULT Render_Lights();
	HRESULT Render_VolumetricFog();
	HRESULT Render_Combined();
	HRESULT Render_SpotlightDarken();
	HRESULT Render_SSR();
	HRESULT Render_DoF();
	HRESULT Render_Effect_HDR();
	HRESULT Render_Distortion();
	HRESULT Render_DistortionApply();
	HRESULT Render_Bloom();

	HRESULT Render_NonLight();
	HRESULT Render_Blend();

	HRESULT Render_Occlusion();

	HRESULT Render_UI_BACK();
	HRESULT Render_UI_MIDDLE();
	HRESULT Render_UI_FRONT();

	HRESULT Render_Curtain();

	HRESULT Render_Flash();

	_uint Render_Width()  const;
	_uint Render_Height() const;

private:
	HRESULT Ready_DepthStencil_Buffer();
	HRESULT Ready_Froxel_Volumes();
	HRESULT Change_ViewportDesc(_uint iWidth, _uint iHeight);

#ifdef _DEBUG
private:
	HRESULT Render_Debug();
	HRESULT Render_DebugText();
#endif

	void	Reset_RS();

public:
	static CRenderer* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void Free() override;
};

NS_END