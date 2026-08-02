#pragma once
#include "MapTool_Defines.h"
#include "Base.h"

/* CEditInstance
	- 맵툴 패널 매니저의 퍼사드이자 패널/레벨/로더가 공유하는 "툴 UI 상태 허브" (MapTool 로컬 싱글톤, 엔진 무관).
	- 선택/배치/레이어 등 "레벨 데이터"는 CLevel_Edit 가 계속 소유한다.
	- 여기서는 패널끼리 직접 묶이지 않도록, 현재 편집 레벨 포인터와 씬 SRV/로딩 진행률만 중계한다.
	- ImGui/ImGuizmo 헤더 의존을 두지 않는다(로더/레벨도 include 하므로).
	- 보관 포인터(레벨/SRV)는 소유권이 다른 곳(엔진/App)에 있으므로 약참조(AddRef 안 함).
     CEditInstance 수명 ⊂ 소유자 수명. */

NS_BEGIN(Engine)
class CGameObject;
NS_END

NS_BEGIN(MapTool)
class CLevel_Edit;
class CImGui_Manager;
class CPanel_Manager;

class CEditInstance final : public CBase
{
	DECLARE_SINGLETON(CEditInstance)

private:
	CEditInstance() = default;
	virtual ~CEditInstance() = default;

public:
	HRESULT			Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	void			ImGui_BeginFrame();
	void			Render_UI();
	void			ImGui_Render();
	void			Update_Panels(_float fTimeDelta);
	void			Render_Panels();

	// 현재 편집 레벨 (약참조)
	void			Set_Level(CLevel_Edit* pLevel) { m_pLevel = pLevel; }
	CLevel_Edit*	Get_Level() const { return m_pLevel; }
	CGameObject*	Get_Selected() const;

	// 오프스크린 씬 텍스처 (App 소유, 약참조)
	void			Set_SceneSRV(ID3D11ShaderResourceView* pSRV) { m_pSceneSRV = pSRV; }
	ID3D11ShaderResourceView* Get_SceneSRV() const { return m_pSceneSRV; }

	// 로딩 진행률 (Loader 갱신 -> ImGui 매니저 오버레이가 표시)
	void			Set_Loading(_bool bLoading) { m_bLoading = bLoading; }
	_bool			Is_Loading() const { return m_bLoading; }
	void			Set_LoadProgress(_float fProgress) { m_fLoadProgress = fProgress; }

private:
	CImGui_Manager*				m_pImGui_Manager = { nullptr };
	CPanel_Manager*				m_pPanelManager = { nullptr };

	CLevel_Edit*				m_pLevel = { nullptr };
	ID3D11ShaderResourceView*	m_pSceneSRV = { nullptr };

	_bool						m_bLoading = { true };
	_float						m_fLoadProgress = { 0.f };

private:
	virtual void Free() override;
};

NS_END
