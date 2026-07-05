#pragma once
#include "Monster.h"
#include "GameContent_Defines.h"

NS_BEGIN(Engine)
class CAnimator;
NS_END

NS_BEGIN(Client)
class CPoppyBrosJr_Body;
class CMonsterBrain;
class CMonster_StateMachine;
class CProjectile;

class CPoppyBrosJr final : public CMonster
{
	GENERATED_BODY(CPoppyBrosJr)

public:
	struct POPPYBROSJR_DESC : public CContainerObject::COTAINEROBJECT_DESC
	{

	};

	static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_PoppyBrosJr";

private:
	CPoppyBrosJr(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CPoppyBrosJr(const CPoppyBrosJr& Prototype);
	virtual ~CPoppyBrosJr() = default;

public:
	virtual HRESULT		Initialize_Prototype() override;
	virtual HRESULT		Initialize(void* pArg) override;

public:
	virtual void		Copy_PrototypeName(
		ENGINE_OBJECT_DATA* pOutData) override
	{
		pOutData->strPrototypeTag = PROTOTYPE_TAG;
	}

	virtual _float Get_CapsuleRadius() const override { return 0.5f; }
	virtual _float Get_CapsuleHeight() const override { return 0.75f; }
	virtual _float Get_InteractRadius() const override
	{
		return (m_iAIType == 0) ? 34.f : 14.f;
	}
	virtual _bool			Get_HurtBoxDesc(CAPSULE_DESC& Out) const override;

	virtual CAnimator*		Get_BodyAnimator() const override;

	CPoppyBrosJr_Body*		Get_Body() { return m_pBody; }

	_bool					Has_Bomb() const { return m_pHeldBomb ? true : false; }
	void					Release_Bomb();

protected:
	virtual CMonsterBrain*	Create_Brain() override;
	virtual HRESULT			Ready_State() override;
	virtual HRESULT			Ready_PartObjects() override;
	virtual HRESULT			Ready_AnimEvents() override;

	virtual void			Apply_AIVariation(const _wstring& strVariation) override;

private:
	CPoppyBrosJr_Body*		m_pBody = { nullptr };

	CProjectile*			m_pHeldBomb = { nullptr };

	_int					m_iThrowLv = { 1 };

private:
	void					Attach_Bomb();
	void					Throw_Bomb();
	void					Set_ThrowLevel(const _wstring& strThrowLv);

public:
	static CPoppyBrosJr*	Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject*	Clone(void* pArg) override;

protected:
	virtual void			Free() override;
};

NS_END