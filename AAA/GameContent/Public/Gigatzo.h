#pragma once
#include "Monster.h"
#include "GameContent_Defines.h"

NS_BEGIN(Engine)
class CAnimator;
NS_END

NS_BEGIN(Client)
class CGigatzo_Body;
class CMonsterBrain;
class CMonster_StateMachine;
class CProjectile;

class CGigatzo final : public CMonster
{
	GENERATED_BODY(CGigatzo)

private:
	enum class PITCH { HORIZONTAL, TILT, VERTICAL };

public:
	struct GIGATZO_DESC : public CContainerObject::COTAINEROBJECT_DESC
	{

	};

	static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_Gigatzo";

private:
	CGigatzo(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CGigatzo(const CGigatzo& Prototype);
	virtual ~CGigatzo() = default;

public:
	virtual HRESULT				Initialize_Prototype() override;
	virtual HRESULT				Initialize(void* pArg) override;

public:
	virtual void				Copy_PrototypeName(
		ENGINE_OBJECT_DATA* pOutData) override
	{
		pOutData->strPrototypeTag = PROTOTYPE_TAG;
	}

	// 캡슐 크기 보고 수정
	virtual _float				Get_CapsuleRadius() const override { return 0.5f; }
	virtual _float				Get_CapsuleHeight() const override { return 0.75f; }
	virtual _float				Get_InteractRadius() const override { return 20.f; }		// 맵 데이터에서 보고 설정	

	virtual _bool				Block_Hit(const ATTACK_INFO& tInfo) override { return true; }
	virtual _bool				Get_HurtBoxDesc(CAPSULE_DESC& Out) const override { return false; }

	virtual CAnimator*			Get_BodyAnimator() const override;

	CGigatzo_Body*				Get_Body() { return m_pBody; }
	_float						Get_InitWaitDelay() const { return m_fInitWaitDelay; }

protected:
	virtual CMonsterBrain*		Create_Brain() override;
	virtual HRESULT				Create_Movement() override;
	virtual HRESULT				Ready_State() override;
	virtual HRESULT				Ready_PartObjects() override;
	virtual HRESULT				Ready_AnimEvents() override;
	virtual const _float4x4*	Get_FxParentMatrix(const _wstring& strFx) const override;

	virtual void				Update(_float fTimeDelta) override;


private:
	CGigatzo_Body*				m_pBody = { nullptr };
	PITCH						m_ePitch = { PITCH::VERTICAL };
	_float						m_fBulletSpeed = { 18.f };
	_float						m_fBulletDamage = { 1.f };
	_float						m_fBulletLife = { 4.5f };
	_float						m_fInitWaitDelay = { 0.f };
private:
	void						Read_FireParams(void* pArg);
	void						Fire_Bullet();

public:
	static CGigatzo*			Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject*		Clone(void* pArg) override;

protected:
	virtual void				Free() override;
};

NS_END