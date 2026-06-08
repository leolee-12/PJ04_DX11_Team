#pragma once

#include "Base.h"

#include "GameContent_Defines.h"

NS_BEGIN(Engine)
class CGameInstance_Proxy;
NS_END

NS_BEGIN(Client)

class CKirby_Command;
class CKirby_Controller;
class CKirby;

class CLIENT_DLL CKirby_InputManager final : public CBase
{
private:
	enum MOVE_DIR { TOP, DOWN, LEFT, RIGHT };

private:
	CKirby_InputManager();
	virtual ~CKirby_InputManager() = default;

private:
	HRESULT Initialize(CKirby* pKiryby, CKirby_Controller* pKirby_Controller);

public:
	void Update_KirbyInput(_float fTimeDelta);

private:
	CGameInstance_Proxy* m_pGameInstance_Proxy = {};

	CKirby_Controller* m_pKirby_Controller{};
	CKirby* m_pKiryby{};

private:
	_bool Cal_MoveDir(MOVE_DIR eMoveDir, _float3& vOutDir);
	void ProcessCommand(CKirby_Command* pCommand);

public:
	static CKirby_InputManager* Create(CKirby* pKiryby, CKirby_Controller* pKirby_Controller);
private:
	virtual void Free() override;
};

NS_END