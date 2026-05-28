#pragma once

#include "Launcher_Defines.h"
#include "Base.h"

NS_BEGIN(Engine)
class CGameInstance_Proxy;
NS_END

NS_BEGIN(Client)
class CDebugDraw_Manager;
NS_END


NS_BEGIN(Launcher)

class CMainApp final : public CBase
{
private:
	CMainApp();
	virtual ~CMainApp() = default;

public:
	HRESULT Initialize();
	void Update(_float fTimeDelta);
	HRESULT Render();


private:
	ID3D11Device* m_pDevice = { nullptr };
	ID3D11DeviceContext* m_pContext = { nullptr };
	CGameInstance_Proxy* m_pGameInstance_Proxy = { nullptr };
#ifdef _DEBUG
	CDebugDraw_Manager* m_pDebugDraw_Manager = { nullptr };
#endif

private:
	HRESULT Ready_Prototype_For_Static();
	HRESULT Start_Logo();

public:
	static CMainApp* Create();
	virtual void Free() override;
};

NS_END