#pragma once
#include "Panel.h"

NS_BEGIN(Engine)
class CGameObject;
class IReflectable;
NS_END

NS_BEGIN(MapTool)

class CPanel_Inspector final : public CPanel
{
private:
	CPanel_Inspector(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CPanel_Inspector() = default;

public:
	virtual void				Render() override;

private:
	void						Draw_Properties(IReflectable* pHolder);
	void						Draw_Transform(CGameObject* pObject, const string& strSuffix = "");

private:
	// 회전 편집용 오일러 누적값 (오브젝트별). 선택된 오브젝트에 대해서만 사용.
	unordered_map<CGameObject*, _float3> m_RotEditEuler;

public:
	static CPanel_Inspector*	Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);

protected:
	virtual void				Free() override;
};

NS_END
