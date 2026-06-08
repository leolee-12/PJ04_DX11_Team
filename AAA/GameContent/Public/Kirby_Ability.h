#pragma once

#include "Base.h"

#include "GameContent_Defines.h"

#include "Kirby_Command.h"

NS_BEGIN(Engine)
NS_END

NS_BEGIN(Client)

enum class KIRBY_ABILITY_TYPE
{
	NORMAL
};

class CKirby;

class CLIENT_DLL CKirby_Ability abstract : public CBase
{
protected:
	CKirby_Ability();
	virtual ~CKirby_Ability() = default;

public:
	virtual KIRBY_ABILITY_TYPE Get_AbilityType() = 0;

	virtual void On_Attack(CKirby* pKirby) = 0;

protected:
	virtual void Free() override;
};

NS_END