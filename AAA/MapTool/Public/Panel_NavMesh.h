#pragma once
#include "Panel.h"

/* CPanel_NavMesh
   - NavMesh 편집 패널 슬롯만 도입한 상태(보류). 내비게이션을 사용하지 않을 가능성이 높아
     실제 편집 기능은 비워둔다.
   - 본격 구현이 필요하면 docs/Panel_MapTool.* (POINT/SELECT/MOVE/REMOVE 툴모드,
     스냅, 공유정점 동기화, 스크린 오버레이, .nav 저장/로드)을 이 패널로 이식하면 된다. */

NS_BEGIN(MapTool)

class CPanel_NavMesh final : public CPanel
{
private:
	CPanel_NavMesh(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CPanel_NavMesh() = default;

public:
	virtual void				Render() override;

private:
	_bool						m_bEnabled = { false };

public:
	static CPanel_NavMesh*		Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);

protected:
	virtual void				Free() override;
};

NS_END
