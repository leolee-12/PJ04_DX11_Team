#pragma once

#include "GameContent_Defines.h"
#include "Movement.h"

NS_BEGIN(Client)

class CLIENT_DLL CMonster_Movement final : public CMovement
{
	GENERATED_BODY(CMonster_Movement)

private:
	CMonster_Movement(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CMonster_Movement(const CMonster_Movement& Prototype);
	virtual ~CMonster_Movement() = default;

public:
	virtual HRESULT				Initialize_Prototype() override;
	virtual HRESULT				Initialize(void* pArg) override;

public:
	static CMonster_Movement*	Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CComponent*			Clone(void* pArg) override;

private:
	virtual void				Free() override;
};

NS_END