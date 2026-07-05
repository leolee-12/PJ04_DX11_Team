#pragma once
#include "GameContent_Defines.h"
#include "Monster.h"

NS_BEGIN(Engine)
class CAnimator;
NS_END

NS_BEGIN(Client)

// Part 들 전방선언
class CNormalEnemy_Body;

class CNormalEnemy final : public CMonster
{
	GENERATED_BODY(CNormalEnemy)

public:
	struct NORMALENEMY_DESC : public CContainerObject::COTAINEROBJECT_DESC
	{

	};

	static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_NormalEnemy";

private:
	CNormalEnemy(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CNormalEnemy(const CNormalEnemy& Prototype);
	virtual ~CNormalEnemy() = default;

protected:
	virtual HRESULT				Initialize_Prototype() override;
	virtual HRESULT				Initialize(void* pArg) override;

public:
	virtual void				Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override
	{
		pOutData->strPrototypeTag = PROTOTYPE_TAG;
	}

	virtual _float				Get_CapsuleRadius() const override { return 0.5f; }
	virtual _float				Get_CapsuleHeight() const override { return 1.f; }
	virtual _float				Get_InteractRadius() const override { return 14.f; }
	virtual _bool				Get_HurtBoxDesc(CAPSULE_DESC& Out) const override;

	virtual CAnimator*			Get_BodyAnimator() const override;

protected:
	virtual CMonsterBrain*		Create_Brain() override;
	virtual HRESULT				Ready_State() override;
	virtual HRESULT				Ready_AnimEvents() override;

	virtual void				Apply_AIVariation(const _wstring& strVariation) override;

	virtual void				On_Exit(MONSTER_STATE_TYPE eNextState) override;

private:
	HRESULT						Ready_PartObjects();

	virtual void				On_Deserialized() override;

private:
	// Body 추가 
	CNormalEnemy_Body*			m_pBody = { nullptr };

public:
	static CNormalEnemy*		Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject*		Clone(void* pArg) override;

protected:
	virtual void				Free() override;

};

NS_END