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
	PROPERTY(_int, m_iCollisionLayer, L"Collision Layer", L"Trigger")

	PROPERTY(_bool, m_bDebugDrawTrigger, L"Debug Draw", L"Trigger")
	PROPERTY(_wstring, m_strDebugTextFontTag, L"Debug Text Font", L"Trigger")
	PROPERTY(_float, m_fDebugTextScale, L"Debug Text Scale", L"Trigger")
	PROPERTY(_float4, m_vDebugTextColor, L"Debug Text Color", L"Trigger")
	PROPERTY(_float4, m_vDebugBoxColor, L"Debug Box Color", L"Trigger")

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
	
#pragma region Editable
	virtual HRESULT On_EditTransformChanged() override;
#pragma endregion

	void Mark_TriggerDirty();

protected:
	CCollider* m_pCollider = { nullptr };
	_uint m_iRegisteredCollisionLayer = { UINT_MAX };

	_bool m_bTriggerShapeDirty = { true };
	_bool m_bTriggerTransformDirty = { true };
	_bool m_bTriggerDebugStyleDirty = { true };
	_bool m_bTriggerAreaValid = { false };

protected:
	virtual void OnTriggerEnter(CCollider* pOther);
	virtual void OnTriggerStay(CCollider* pOther);
	virtual void OnTriggerExit(CCollider* pOther);

	_bool Is_PlayerActivator(const CCollider* pOther) const;

	HRESULT Ready_TriggerCollider();
	void    Refresh_TriggerCollisionLayer();
	void	SetUp_Collider_Callback();
	void    Refresh_TriggerCollider();

#ifdef _DEBUG
protected:
	void	Refresh_TriggerDebugStyle();
	virtual _wstring	Get_DebugLabel() const;
#endif

protected:
	virtual void Free() override;
};

NS_END
