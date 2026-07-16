#pragma once
#include "Character.h"

NS_BEGIN(Client)

class CWaddleDee_Body;

class CWaddleDee final : public CCharacter
{
	GENERATED_BODY(CWaddleDee)

	PROPERTY(_wstring, m_strFixedAnim, L"FixedAnim", L"NPC")
	PROPERTY(_float, m_fInteractRadius, L"InteractRadius", L"NPC")

public:
	static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_WaddleDee";

	enum class WADDLEDEE_STATE : _uint
	{
		IDLE,
		GREET
	};

private:
	CWaddleDee(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CWaddleDee(const CWaddleDee& Prototype);
	virtual ~CWaddleDee() = default;

public:
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;

	virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override
	{
		pOutData->strPrototypeTag = PROTOTYPE_TAG;
	}

private:
	virtual void On_Deserialized() override;

	HRESULT Ready_PartObjects();
	void Change_State(WADDLEDEE_STATE eState);
	_bool Find_Player();
	void Check_Interact();

	void Update_Idle();
	void Play_Idle();
	void Update_Greet();

private:
	CWaddleDee_Body* m_pBody = { nullptr };

	WADDLEDEE_STATE m_eState = { WADDLEDEE_STATE::IDLE };
	_wstring m_strAppliedFixedAnim = {};
	_float m_fGreetCooldown = { 0.f };
	CGameObject* m_pPlayer = { nullptr };

public:
	static CWaddleDee* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;

protected:
	virtual void Free() override;
};

NS_END