#pragma once
#include "GameContent_Defines.h"
#include "Monster.h"

NS_BEGIN(Engine)
class CAnimator;
NS_END

NS_BEGIN(Client)
class CCappy_Brain;
class CCappy_Body;
class CCappy_Hat;

class CCappy final : public CMonster
{
	GENERATED_BODY(CCappy)

public:
	struct CAPPY_DESC : public CContainerObject::COTAINEROBJECT_DESC
	{

	};

	static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_Cappy";

private:
	CCappy(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CCappy(const CCappy& Prototype);
	virtual ~CCappy() = default;

protected:
	virtual HRESULT				Initialize_Prototype() override;
	virtual HRESULT				Initialize(void* pArg) override;
	virtual void				Update(_float fTimeDelta) override;

public:
	virtual void				Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override
	{
		pOutData->strPrototypeTag = PROTOTYPE_TAG;
	}

	virtual _float				Get_CapsuleRadius() const override { return 0.5f; }
	virtual _float				Get_CapsuleHeight() const override { return 0.75f; }
	virtual _float				Get_InteractRadius() const override { return (m_iAIType == 0) ? 15.f : 10.f; }
	virtual _bool				Get_HurtBoxDesc(CAPSULE_DESC& Out) const override;

	virtual CAnimator*			Get_BodyAnimator() const override;
	virtual _bool				Can_BeInhaled(const INHALE_QUERY& q) const override;
	virtual void				On_Swallowed() override;
	virtual void				On_SpatBegin() override;
	virtual void				On_SpatEnd() override;

public:
	CCappy_Body*				Get_Body() { return m_pBody; }
	CCappy_Hat*					Get_Hat() { return m_pHat; }

protected:
	virtual CMonsterBrain*		Create_Brain() override;
	virtual HRESULT				Ready_State() override;
	virtual HRESULT				Ready_AnimEvents() override;

	virtual void				Apply_AIVariation(const _wstring& strVariation) override;

private:
	virtual HRESULT				Ready_PartObjects() override;

	virtual void				On_Deserialized() override;
	void						Sync_PartFlags();

	_bool						Hat_Out() const;
	void						Check_HatlessReflex();

private:
	CCappy_Body*				m_pBody = { nullptr };
	CCappy_Hat*					m_pHat = { nullptr };

	_bool						m_bDeferFinal = { false };

public:
	static CCappy*				Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual	CGameObject*		Clone(void* pArg) override;

protected:
	virtual void				Free() override;


};

NS_END