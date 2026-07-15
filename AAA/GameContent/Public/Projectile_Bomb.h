#pragma once
#include "PhysicsProjectile.h"

NS_BEGIN(Engine)
class CShader;
class CModel;
class CAnimator;
class CEffect_Container;
NS_END


NS_BEGIN(Client)

class CProjectile_Bomb abstract : public CPhysicsProjectile
{
	GENERATED_BODY(CProjectile_Bomb)

protected:
	CProjectile_Bomb(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CProjectile_Bomb(const CProjectile_Bomb& Prototype);
	virtual ~CProjectile_Bomb() = default;

public:
	virtual HRESULT			Initialize(void* pArg) override;
	virtual void			Update(_float fTimeDelta) override;		// 기존 수명 방식이 아닌 애니메이션 종료 여부로 수명 주기 따져야 해서 override 
	virtual void			Late_Update(_float fTimeDelta) override;
	virtual HRESULT			Render() override;
	virtual HRESULT			Render_Shadow() override;

	void					Bomb_Explode();			// 즉시 폭발 
	virtual void			Despawn();				// 연출 없이 회수


protected:
	void					Ignite();
	void					Start_Fuse(_float fSpeed = 1.f);
	void					Pause_Fuse();
	void					Resume_Fuse();
	_float					Get_BurnRatio() const { return m_fBurnRatio; }

	void					Spawn_FuseFx();
	void					Stop_FuseFx();
	void					Update_FuseSocket();

	void					Play_BodyAnim(const _char* szClop, _bool bLoop, _float fSpeed = 1.f, _bool bRestart = true);
	void					Play_DangerGlow(_float fSpeed = 2.f);
	void					Apply_RollPose();
	void					Play_ExplodeFx();
	void					Reset_BombVisual();

	virtual HRESULT			Ready_Visual() override;
	virtual void			Tick_Visual(_float fTimeDelta) override;
	
	virtual void			On_Explode() ;
	virtual void			On_Impact() override;
	virtual void			Kill() override;

	virtual const _tchar* Get_ExplodeSFXKey() const		// 폭발 효과음 직접 넣기
	{
		return L"CharaPoppyBrosJr_BombExplode.wav";
	}

	virtual HRESULT			Ready_AnimEvents() { return S_OK; }

	virtual const _char*	Get_FuseBoneName() const { return "EffectL"; }
	virtual const _char*	Get_RollBoneName() const { return "RotL"; }

	virtual _matrix			Get_PreRotInverse() const;

	// 두 폭탄 공통 클립/레이어 규약
protected:
	static constexpr const _char*	ANIM_FUSE = "FuseBurning";
	static constexpr const _char*	ANIM_DANGER = "DangerGlow";
	static constexpr _uint			FUSE_LAYER = { 1 };

private:
	HRESULT					Bind_ShaderResources();
	void					Roll_ByMovement(_float fTimeDelta);

protected:
	CShader*				m_pShaderCom = { nullptr };
	CModel*					m_pModelCom = { nullptr };
	CAnimator*				m_pAnimatorCom = { nullptr };
	CEffect_Container*		m_pFuseFx = { nullptr };

	_float4x4				m_matFuseWorld{};
	const _float4x4*		m_pFuseBone = { nullptr };

	_float					m_fRollAngle = { 0.f };
	_float3					m_vRollAxis = { 1.f, 0.f, 0.f };
	static constexpr _float ROLL_DEG_PER_SPEED = { 45.f };

	_float					m_fBurnRatio = { 0.f };
	_float3					m_vGlow = { 0.f, 0.f, 0.f };

protected:
	virtual void			Free() override;
};

NS_END