#pragma once
#include "Panel.h"
#include "AnimUITool_Defines.h"

NS_BEGIN(Engine)
class CGameObject;
class IReflectable;
NS_END

NS_BEGIN(AnimUITool)

class CPanel_Inspector final : public CPanel
{
private:
	CPanel_Inspector(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CPanel_Inspector() = default;

public:
	virtual void				Render() override;

private:
	void						Render_Model();
	void						Render_Transform(CGameObject* pObject);
	void						Render_RenderDebug();
	void						Render_Properties(IReflectable* pHolder);
	void						Render_Bones();
	void						Render_Meshs();

private:
	unordered_map<CGameObject*, _float3>	m_RotEditEuler;

public:
	static CPanel_Inspector*	Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);

protected:
	virtual void				Free() override;
};

NS_END