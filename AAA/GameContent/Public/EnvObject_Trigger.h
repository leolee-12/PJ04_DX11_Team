#pragma once
#include "EnvObject.h"

NS_BEGIN(Engine)
class CCollider;
NS_END

NS_BEGIN(Client)

class CLIENT_DLL CEnvObject_Trigger abstract : public CEnvObject
{
	GENERATED_BODY_ABSTRACT(CEnvObject_Trigger)

	PROPERTY(_wstring, m_strTriggerId, L"Trigger Id", L"Trigger")
	PROPERTY(_float3, m_vAreaCenter, L"Area Center", L"Trigger")
	PROPERTY(_float3, m_vAreaSize, L"Area Size", L"Trigger")
	PROPERTY(_float4, m_vAreaRot, L"Area Rotation", L"Trigger")
	PROPERTY(_bool, m_bDebugDrawTrigger, L"Debug Draw", L"Trigger")

public:
	static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_EnvObject_Trigger";
	static constexpr const _tchar* LEGACY_PROTOTYPE_TAG = L"Proto_EnvObject_Effect";

protected:
	CEnvObject_Trigger(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CEnvObject_Trigger(const CEnvObject_Trigger& Prototype);
	virtual ~CEnvObject_Trigger() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override;

protected:
	virtual void OnTriggerEnter(CCollider* pOther);
	virtual void OnTriggerStay(CCollider* pOther);
	virtual void OnTriggerExit(CCollider* pOther);

protected:
	CCollider* m_pCollider = { nullptr };

protected:
	HRESULT Ready_TriggerCollider();
	void	SetUp_Collider_Callback();

protected:
	virtual void Free() override;
};

NS_END
