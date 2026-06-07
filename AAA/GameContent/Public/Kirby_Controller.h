#pragma once

#include "Base.h"

#include "GameContent_Defines.h"

#include "Kirby_Command.h"

NS_BEGIN(Engine)
class CGameInstance_Proxy;
NS_END

NS_BEGIN(Client)

class CKirby;

class CLIENT_DLL CKirby_Controller final : public CBase
{
private:
	CKirby_Controller();
	virtual ~CKirby_Controller() = default;

private:
	HRESULT Initialize(CKirby* m_pPlayer);

public:
	void Update_KirbyController(_float fTimeDelta);

	void Push_Command(const CKirby_Command& Command);

private:
	CGameInstance_Proxy* m_pGameInstance_Proxy = {};

	CKirby* m_pPlayer{};
	std::queue<CKirby_Command> m_CommandQueue;

public:
	static CKirby_Controller* Create(CKirby* m_pPlayer);
private:
	virtual void Free() override;
};

NS_END