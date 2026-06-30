#pragma once
#include "Panel.h"

NS_BEGIN(AnimUITool)

class CPanel_Animation final : public CPanel
{
private:
	CPanel_Animation(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CPanel_Animation() = default;

public:
	virtual void				Render() override;

private:
	CGameObject* m_pPrevActor = { nullptr };

private:
	_int						m_iPrevClip = { -1 };
	_int						m_iSelEvent = { -1 };
	_char						m_szEventPath[MAX_PATH] = {};

	CModel*						m_pCachedAnimModel = { nullptr };
	_int						m_iCachedAnimCount = { -1 };
	_char						m_szAnimSearch[MAX_PATH] = {};
	string						m_strCachedAnimSearch = {};
	vector<string>				m_AnimNames;
	vector<string>				m_AnimNamesLower;
	vector<_uint>				m_FilteredAnimIndices;


private:
	virtual void				Update(_float fTimeDelta) override;
	void						Render_EventTimeline();

	void						Rebuild_AnimNameCache(CModel* pModel);
	void						Rebuild_AnimFilter();

public:
	static CPanel_Animation* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);

protected:
	virtual void				Free() override;
};

NS_END