#pragma once

#include "GameContent_Defines.h"
#include "ContainerObject.h"
#include "Damageable.h"

NS_BEGIN(Client)

class CCharacter abstract : public CContainerObject, public IDamageable
{
	GENERATED_BODY_ABSTRACT(CContainerObject)

protected:
	CCharacter(ID3D11Device * pDevice, ID3D11DeviceContext * pContext);
	CCharacter(const CCharacter& Prototype);
	virtual ~CCharacter() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

public:
	virtual void Damaged(const ATTACK_INFO& tInfo) override;

protected:
	_float						m_fMaxHP = { 100.f };
	_float						m_fCurHP = { 100.f };

protected:
	virtual _bool Block_Hit(const ATTACK_INFO& tInfo) { return false; }
	virtual void  On_Damaged(const ATTACK_INFO& tInfo) {}
	virtual void  On_Death(const ATTACK_INFO& tInfo) {}

public:
	virtual CGameObject* Clone(void* pArg) = 0;
protected:
	virtual void Free() override;
};

NS_END