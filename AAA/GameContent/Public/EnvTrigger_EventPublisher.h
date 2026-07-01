#pragma once
#include "EnvObject_Trigger.h"

NS_BEGIN(Client)

class CLIENT_DLL CEnvTrigger_EventPublisher final : public CEnvObject_Trigger
{
	GENERATED_BODY(CEnvTrigger_EventPublisher)

	PROPERTY(_wstring, m_strEventTag, L"Event Tag", L"Event Publisher")
	PROPERTY(_wstring, m_strPayload, L"Payload", L"Event Publisher")
	PROPERTY(_bool, m_bPublishOnce, L"Publish Once", L"Event Publisher")

public:
	static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_EnvTrigger_EventPublisher";

private:
	CEnvTrigger_EventPublisher(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CEnvTrigger_EventPublisher(const CEnvTrigger_EventPublisher& Prototype);
	virtual ~CEnvTrigger_EventPublisher() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override;

private:
	_bool m_bPublished = { false };

private:
	virtual void OnTriggerEnter(CCollider* pOther) override;
	virtual void OnTriggerStay(CCollider* pOther) override;
	virtual void OnTriggerExit(CCollider* pOther) override;

	_bool Is_TriggerActivator(CCollider* pOther) const;

public:
	static CEnvTrigger_EventPublisher* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;

protected:
	virtual void Free() override;
};

NS_END