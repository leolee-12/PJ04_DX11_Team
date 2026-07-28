#pragma once
#include "EnvTrigger_EventPublisher.h"

NS_BEGIN(Client)

class CEnvTrigger_LevelChange final : public CEnvTrigger_EventPublisher
{
	GENERATED_BODY(CEnvTrigger_LevelChange)

	PROPERTY(_float, m_fSFXDuration, L"SFX Duration", L"LevelChange")

public:
	static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_EnvTrigger_LevelChange";

private:
	CEnvTrigger_LevelChange(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CEnvTrigger_LevelChange(const CEnvTrigger_LevelChange& Prototype);
	virtual ~CEnvTrigger_LevelChange() = default;

protected:
	virtual void* Get_EnterPayload() override;

private:
	FADEOUT_PAYLOAD ePayload = {};

public:
	static CEnvTrigger_LevelChange* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

NS_END