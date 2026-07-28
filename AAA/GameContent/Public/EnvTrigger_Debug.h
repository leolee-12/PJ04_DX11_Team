#pragma once
#include "EnvTrigger_EventPublisher.h"

NS_BEGIN(Client)

class CEnvTrigger_Debug final : public CEnvTrigger_EventPublisher
{
	GENERATED_BODY(CEnvTrigger_Debug)

	PROPERTY(_wstring, m_strPayload, L"Payload", L"Debug")
	PROPERTY(_wstring, m_strExitPayload, L"Exit Payload", L"Debug")

public:
	static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_EnvTrigger_Debug";

private:
	CEnvTrigger_Debug(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CEnvTrigger_Debug(const CEnvTrigger_Debug& Prototype);
	virtual ~CEnvTrigger_Debug() = default;

protected:
	virtual void* Get_EnterPayload() override;
	virtual void* Get_ExitPayload() override;

private:
	TRIGGER_EVENT_PAYLOAD ePayload = {};

public:
	static CEnvTrigger_Debug* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

NS_END