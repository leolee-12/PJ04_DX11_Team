#pragma once

#include "Effect_Container.h"
NS_BEGIN(Engine)
class CEffect_Part;
NS_END

NS_BEGIN(Client)

class CFaintEffect final : public CEffect_Container
{
	GENERATED_BODY(CFaintEffect)

public:
	static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_FaintEffect";
	static constexpr const _tchar* STAR_MODEL_TAG  = L"Prototype_Component_Model_FaintStar";
	static constexpr const _tchar* SMOKE_MODEL_TAG = L"Prototype_Component_Model_FaintSmoke";

private:
	CFaintEffect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CFaintEffect(const CFaintEffect& Prototype);
	virtual ~CFaintEffect() = default;

protected:
	virtual HRESULT				Initialize_Prototype() override;
	virtual HRESULT				Initialize(void* pArg) override;

public:
	virtual void				Priority_Update(_float fTimeDelta) override;
	virtual void				Update(_float fTimeDelta) override;
	virtual void				Late_Update(_float fTimeDelta) override;
	virtual HRESULT				Render() override;
	virtual void				Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override
	{
		pOutData->strPrototypeTag = PROTOTYPE_TAG;
	}

private:
	HRESULT						Ready_EffectPartObjects();
	void						Update_ContainerTilt(_float fTimeDelta);

private:
	_float							m_fTiltAccTime = { 0.f };

	static constexpr const _float   TILT_ANGLE_DEG = { 40.f };
	static constexpr const _float   TILT_PRECESS_DEG_SEC = { 60.f };
	static constexpr const _float   TILT_WOBBLE_CYCLES = { 6.f };
	static constexpr const _float   TILT_WOBBLE_DEPTH = { 0.45f };

public:
	static CFaintEffect* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;

private:
	virtual void				Free() override;
};

NS_END