#pragma once
#include "Panel.h"

NS_BEGIN(MapTool)

class CPanel_Toolbar final : public CPanel
{
private:
	CPanel_Toolbar(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CPanel_Toolbar() = default;

public:
	virtual void				Render() override;

private:
	static constexpr _uint		BUF_SIZE = { 64 };
	_char						m_szSaveName[BUF_SIZE] = {};
	_char						m_szLoadName[BUF_SIZE] = {};
	_char						m_szLayerName[BUF_SIZE] = {};
	_bool						m_bKeyInputEnabled = { false };

private:
	void						Draw_FileButtons(class CLevel_Edit* pLevel);
	void						Draw_GizmoButtons();
	void						Draw_CameraButtons(class CLevel_Edit* pLevel);

public:
	static CPanel_Toolbar*		Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);

protected:
	virtual void				Free() override;
};

NS_END
