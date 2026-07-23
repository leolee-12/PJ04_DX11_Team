#pragma once
#include "Editor_Defines.h"
#include "Base.h"

#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"
#include "ImGuizmo.h"
#include "Property.h"

NS_BEGIN(Engine)
class CGameInstance_Proxy;
class CGameObject;
class IReflectable;
class CAnimator;
class CModel;
NS_END

NS_BEGIN(Client)
class CMapSection;
NS_END

NS_BEGIN(Editor)

class CLevel_Edit;

class CImGui_Manager final : public CBase
{
	DECLARE_SINGLETON(CImGui_Manager)
	
private:
	CImGui_Manager();
	virtual ~CImGui_Manager();

public:
	HRESULT ImGui_Initialize(ID3D11Device** ppDevice, ID3D11DeviceContext** ppContext, CLevel_Edit* pLevelEdit, ID3D11ShaderResourceView** ppSRV);
	void ImGui_Render();

private:
	void Draw_Toolbar();
	void Draw_Hierarchy();
	void Draw_Gizmo();
	void Draw_Inspector();
	void Draw_Properties(IReflectable* pHolder);
	void Draw_Property(IReflectable* pHolder, const FPROPERTY& prop);
	void Draw_Palette();
	void Draw_Viewport();
	void Draw_Transform(CGameObject* pObject, const string& strSuffix = "");

	void Draw_AnimatorEditor(CModel* pModel, CAnimator* pAnimator);
	
	void Draw_ShaderGlobals();
	void Draw_MeshLayerPanel(CGameObject* pObj);

public:
	virtual void Free() override;

private:
	CLevel_Edit* m_pLevel_Edit = { nullptr };
	CGameInstance_Proxy* m_pGameInstance_Proxy = { nullptr };
	ImGuizmo::OPERATION m_eGizmoOp = {};
	ID3D11ShaderResourceView* m_pSRV = { nullptr };
	_bool m_bKeyInputEnabled = { false };

	ID3D11DeviceContext* m_pContext = { nullptr };
	ID3D11BlendState* m_pOpaqueBlend = { nullptr };

	struct VIEWPORT_DRAW { ID3D11DeviceContext* pContext; ID3D11BlendState* pBlend; };
	VIEWPORT_DRAW m_ViewportDraw{};

	unordered_map<CGameObject*, _float3> m_RotEditEuler;

private:
	static void Viewport_DisableBlend(const ImDrawList*, const ImDrawCmd* cmd);
};

NS_END
