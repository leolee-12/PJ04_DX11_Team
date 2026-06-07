#pragma once

#include "Base.h"

#include "GameContent_Defines.h"

NS_BEGIN(Engine)
class CGameInstance_Proxy;
NS_END

NS_BEGIN(Client)

class CKirby_Controller;

class CLIENT_DLL CKirby_InputManager final : public CBase
{
private:
	CKirby_InputManager();
	virtual ~CKirby_InputManager() = default;

private:
	HRESULT Initialize(CKirby_Controller* pKirby_Controller);

public:
	void Update_KirbyInput(_float fTimeDelta);

private:
	CGameInstance_Proxy* m_pGameInstance_Proxy = {};

	CKirby_Controller* m_pKirby_Controller{};

public:
	static CKirby_InputManager* Create(CKirby_Controller* pKirby_Controller);
private:
	virtual void Free() override;
};

NS_END