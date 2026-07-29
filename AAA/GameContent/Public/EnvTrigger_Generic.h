#pragma once
#include "EnvObject_Trigger.h"

NS_BEGIN(Client)

class CEnvTrigger_Generic final : public CEnvObject_Trigger
{
	GENERATED_BODY(CEnvTrigger_Generic)

public:
	static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_EnvTrigger_Generic";
	static constexpr const _tchar* LEGACY_PROTOTYPE_TAG = CEnvObject_Trigger::LEGACY_PROTOTYPE_TAG;

private:
	CEnvTrigger_Generic(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CEnvTrigger_Generic(const CEnvTrigger_Generic& Prototype);
	virtual ~CEnvTrigger_Generic() = default;

protected:
	virtual void OnTriggerEnter(CCollider* pOther) override;
	virtual void OnTriggerStay(CCollider* pOther) override;
	virtual void OnTriggerExit(CCollider* pOther) override;

public:
	static CEnvTrigger_Generic* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;

protected:
	virtual void Free() override;
};

NS_END