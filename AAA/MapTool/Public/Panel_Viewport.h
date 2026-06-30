#pragma once
#include "Panel.h"

NS_BEGIN(Engine)
class CGameObject;
NS_END

NS_BEGIN(MapTool)

class CPanel_Viewport final : public CPanel
{
private:
	struct VIEWPORT_DRAW_DESC
	{
		ID3D11DeviceContext* pContext;
		ID3D11BlendState* pBlendState;
	};

private:
	CPanel_Viewport(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CPanel_Viewport() = default;

public:
	HRESULT			Initialize();
	virtual void	Render() override;

private:
	// 오프스크린 씬 텍스처는 CEditInstance 가 보유(App 소유). 매 프레임 그쪽에서 읽어온다.
	static constexpr _float	m_fTargetAspect = { 1600.f / 900.f };
	ID3D11BlendState*		m_pViewportOpaqueBlend = { nullptr };
	VIEWPORT_DRAW_DESC		m_ViewportDrawDesc = {};
	_bool						m_bWasGizmoUsing = { false };

private:
	static void	Viewport_DisableBlend(const ImDrawList*, const ImDrawCmd* cmd);
	void		Draw_Gizmo(CGameObject* pSelected, const ImVec2& vImagePos, const ImVec2& vImageSize);

public:
	static CPanel_Viewport*		Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);

protected:
	virtual void				Free() override;
};

NS_END
