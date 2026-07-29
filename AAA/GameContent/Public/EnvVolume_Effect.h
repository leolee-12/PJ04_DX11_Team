#pragma once
#include "EnvObject_Trigger.h"

NS_BEGIN(Engine)
class CEffect_Container;
NS_END

NS_BEGIN(Client)

class CEnvVolume_Effect final : public CEnvObject_Trigger
{
	GENERATED_BODY(CEnvVolume_Effect)

	PROPERTY(_wstring, m_strEffectId, L"Effect Id", L"Effect Volume")
		PROPERTY(_wstring, m_strEffectKind, L"Effect Kind", L"Effect Volume")
		PROPERTY(_float3, m_vEmitPos, L"Emit Position", L"Effect Volume")

public:
	static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_EnvVolume_Effect";

private:
	CEnvVolume_Effect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CEnvVolume_Effect(const CEnvVolume_Effect& Prototype);
	virtual ~CEnvVolume_Effect() = default;

public:
	virtual HRESULT Initialize(void* pArg) override;

private:
	_bool m_bActive = { false };

	// Effect pool-owned weak reference. Do not Safe_Release.
	Engine::CEffect_Container* m_pSpawnedEffect = { nullptr };

private:
	virtual void OnTriggerEnter(CCollider* pOther) override;
	virtual void OnTriggerExit(CCollider* pOther) override;

#ifdef _DEBUG
protected:
	virtual _wstring Get_DebugLabel() const override;
#endif

public:
	static CEnvVolume_Effect* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;

protected:
	virtual void Free() override;
};

NS_END