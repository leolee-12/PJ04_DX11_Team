#pragma once
#include "GameContent_Defines.h"
#include "EnvObject.h"

NS_BEGIN(Engine)
class CCollider;
NS_END

NS_BEGIN(Client)

class CTestTriggerBox final : public CEnvObject
{
	GENERATED_BODY(CTestTriggerBox)

	PROPERTY(_wstring, m_strTriggerId, L"Trigger Id", L"Trigger")
	PROPERTY(_float3, m_vTriggerCenterLocal, L"Trigger Center", L"Trigger");
	PROPERTY(_float3, m_vTriggerSize, L"Trigger Size", L"Trigger");

public:
	static constexpr const wchar_t* PROTOTYPE_TAG = L"Proto_TestTriggerBox";

private:
	CTestTriggerBox(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CTestTriggerBox(const CTestTriggerBox& Prototype);
	virtual ~CTestTriggerBox() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override { pOutData->strPrototypeTag = PROTOTYPE_TAG; }

	virtual void Enter_Collision(CGameObject* pOther) override;
	virtual void On_Collision(CGameObject* pOther) override;
	virtual void Exit_Collision(CGameObject* pOther) override;

private:
	CCollider* m_pCollider = { nullptr };

private:
	HRESULT Ready_TriggerCollider();
	void Build_DefaultDesc(ENV_OBJECT_DESC* pOutDesc);

public:
	static CTestTriggerBox* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;

protected:
	virtual void Free() override;
};

NS_END