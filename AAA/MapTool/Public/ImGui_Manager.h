#pragma once
#include "MapTool_Defines.h"
#include "Base.h"

NS_BEGIN(MapTool)

class CImGui_Manager final : public CBase
{
private:
	CImGui_Manager(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CImGui_Manager() = default;

public:
	HRESULT				Initialize();
	void				Begin_Frame();
	void				Draw_LoadingOverlay(_float fProgress);
	void				Render();

public:
	static CImGui_Manager* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);

private:
	ID3D11Device*			m_pDevice = { nullptr };
	ID3D11DeviceContext*	m_pContext = { nullptr };

	_bool				m_bContextInitialized = { false };
	_bool				m_bWin32Initialized = { false };
	_bool				m_bDX11Initialized = { false };

protected:
	virtual void		Free() override;
};

NS_END
