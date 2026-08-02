#pragma once
#include "Effect_Container.h"

NS_BEGIN(Client)

class CVanishEffect final : public CEffect_Container
{
	GENERATED_BODY(CVanishEffect)

	PROPERTY(_float, m_fVanishMoveSpeed, L"Vanish Move Speed", L"Vanish Move");
	PROPERTY(_float, m_fVanishMoveStartRatio, L"Vanish Move Start Ratio", L"Vanish Move");
	PROPERTY(_float, m_fVanishMoveEndRatio, L"Vanish Move End Ratio", L"Vanish Move");

public:
	static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_VanishEffect";

private:
	CVanishEffect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CVanishEffect(const CVanishEffect& Prototype);
	virtual ~CVanishEffect() = default;

protected:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;

public:
	virtual void Update(_float fTimeDelta) override;
	virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override { pOutData->strPrototypeTag = PROTOTYPE_TAG; }

private:
	HRESULT Ready_EffectPartObjects();
	void Update_Move(_float fTimeDelta);
	void Init_PropertyValue();

public:
	static CVanishEffect* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;

private:
	virtual void Free() override;
};

NS_END