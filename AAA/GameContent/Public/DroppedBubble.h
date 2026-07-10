#pragma once
#include "Ability_Bubble.h"
#include "Inhalable.h"

NS_BEGIN(Engine)
class CController;
NS_END

NS_BEGIN(Client)

class CDroppedBubble final : public CAbility_Bubble, public IInhalable
{
	GENERATED_BODY(CDroppedBubble)

public:
	static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_DroppedBubble";

private:
	CDroppedBubble(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CDroppedBubble(const CDroppedBubble& Prototype);
	virtual ~CDroppedBubble() = default;

public:
	virtual HRESULT				Initialize(void* pArg) override;
	virtual void				Update(_float fTimeDelta) override;
	virtual void				Activate(const _float3& vPos) override;
	virtual void				Launch(const _float3& vDir) override;

	virtual void				Copy_PrototypeName(ENGINE_OBJECT_DATA* pOut) override
	{
		pOut->strPrototypeTag = PROTOTYPE_TAG;
	}

	virtual _bool				Can_BeInhaled(const INHALE_QUERY& q) const override;
	virtual void				Be_Captured(CGameObject* pInhaler) override;
	virtual COPY_ABILITY_TYPE	Get_CopyAbility() const	override { return m_eAbility; }
	virtual CGameObject*		Get_GameObject() override { return this; }
	virtual _float3				Get_SpatPivotOffset() const { return _float3(0.f, 0.f, 0.f); }

	virtual void				On_SpatBegin() override {};
	virtual void				On_SpatEnd() override {};

	void						On_Swallowed();

protected:
	virtual void				SetUp_Collider_CallBack() override {};

private:
	void						Update_Captured(_float fTimeDelta);
	void						Despawn();
	HRESULT						Ready_Controller();
	void						Update_Movement(_float fTimeDelta);

	void                        On_Launched();
	void                        On_Bounce(_int iCount);
	void                        On_Rest();

private:
	CGameObject*				m_pCaptor = { nullptr };
	CController*				m_pController = { nullptr };

	_float3						m_vVelocity = { 0.f, 0.f, 0.f };
	_bool                       m_bIsGrounded = { false };
	_int                        m_iBounceCount = { 0 };
	_bool						m_bCaptured = { false };
	_float						m_fPullSpeed = { 0.f };
	_float						m_fScaleRatio = { 1.f };

	static constexpr _float		s_fBlinkTime = { 6.f };				//	원본 (20초 기준) - 16 - 18초 -> 6초로 수정
	static constexpr _float		s_fDeSpawnTime = { 10.f };			// 원본은 20초지만 너무 길어서 10초로 수정
	static constexpr _float		s_fPullAccel = { 40.f };
	static constexpr _float		s_fMinScaleRatio = { 0.45f };		// 보면서 튜닝
	static constexpr _float		s_fShrinkLerp = { 2.f };

	static constexpr _float     s_fLaunchSpeed = { 4.f };
	static constexpr _float     s_fPopUpSpeed = { 6.f };
	static constexpr _float     s_fGravity = { -12.f };
	static constexpr _float     s_fAirDrag = { 1.2f };
	static constexpr _float     s_fRestitution = { 0.8f };
	static constexpr _float     s_fHorizDamp = { 0.85f };
	static constexpr _float     s_fRestSpeed = { 1.5f };
	static constexpr _int       s_iMaxBounce = { 6 };

	static constexpr _float		s_fFootOffset = { 0.45f };

public:
	static CDroppedBubble*		Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject*		Clone(void* pArg) override;

protected:
	virtual void				Free() override;

};

NS_END
