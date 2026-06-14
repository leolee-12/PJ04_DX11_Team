#pragma once
#include "EnvObject.h"

NS_BEGIN(Engine)
class CCollider;
NS_END

NS_BEGIN(Client)

class CLIENT_DLL CEnvObject_Trigger final : public CEnvObject
{
public:
	static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_EnvObject_Effect";

private:
	CEnvObject_Trigger(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CEnvObject_Trigger(const CEnvObject_Trigger& Prototype);
	virtual ~CEnvObject_Trigger() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override;

	virtual void Enter_Collision(CGameObject* pOther) override;
	virtual void On_Collision(CGameObject* pOther) override;
	virtual void Exit_Collision(CGameObject* pOther) override;

private:
	CCollider* m_pCollider = { nullptr };

private:
	HRESULT Ready_TriggerCollider();

public:
	static CEnvObject_Trigger* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;

protected:
	virtual void Free() override;
};

NS_END
