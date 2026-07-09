#pragma once
#include "Ability_Bubble.h"
#include "Inhalable.h"

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

private:
	CGameObject*				m_pCaptor = { nullptr };

	_bool						m_bCaptured = { false };
	_float						m_fPullSpeed = { 0.f };
	_float						m_fScaleRatio = { 1.f };

	static constexpr _float		s_fBlinkTime = { 16.f };			// º¸¸é¼­ Æ©´×
	static constexpr _float		s_fDeSpawnTime = { 20.f };
	static constexpr _float		s_fPullAccel = { 40.f };
	static constexpr _float		s_fMinScaleRatio = { 0.45f };		// º¸¸é¼­ Æ©´×
	static constexpr _float		s_fShrinkLerp = { 2.f };

public:
	static CDroppedBubble*		Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject*		Clone(void* pArg) override;

protected:
	virtual void				Free() override;

};

NS_END
