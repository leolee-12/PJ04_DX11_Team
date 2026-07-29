#pragma once

#include "Effect_Container.h"

NS_BEGIN(Client)

class CWheelHammer final : public CEffect_Container
{
	GENERATED_BODY(CWheelHammer)

public:
	static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_WheelHammer";
	static constexpr const _tchar* MODEL_PROTO_TAG = L"Prototype_Component_Model_WheelHammer";
	static constexpr const _tchar* WARP_PART_TAG_A = L"TailModelWarpA";
	static constexpr const _tchar* COLOR_PART_TAG_A = L"TailModelA";
	static constexpr const _tchar* WARP_PART_TAG_B = L"TailModelWarpB";
	static constexpr const _tchar* COLOR_PART_TAG_B = L"TailModelB";

private:
	CWheelHammer(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CWheelHammer(const CWheelHammer& Prototype);
	virtual ~CWheelHammer() = default;

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
	static CWheelHammer* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;

private:
	virtual void Free() override;
};

NS_END
