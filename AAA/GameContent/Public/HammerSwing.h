#pragma once

#include "Effect_Container.h"

NS_BEGIN(Client)

class CHammerSwing final : public CEffect_Container
{
	GENERATED_BODY(CHammerSwing)

public:
	static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_HammerSwing";
	static constexpr const _tchar* MODEL_PROTO_TAG = L"Prototype_Component_Model_HammerSwing";
	static constexpr const _tchar* WARP_PART_TAG = L"TailModelWarp";
	static constexpr const _tchar* COLOR_PART_TAG = L"TailModel";
	static constexpr const _tchar* YELLOW_PART_TAG = L"TailModelYellow";

private:
	CHammerSwing(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CHammerSwing(const CHammerSwing& Prototype);
	virtual ~CHammerSwing() = default;

protected:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;

public:
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;
	virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override
	{
		pOutData->strPrototypeTag = PROTOTYPE_TAG;
	}

private:
	HRESULT Ready_EffectPartObjects();

public:
	static CHammerSwing* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;

private:
	virtual void Free() override;
};

NS_END
