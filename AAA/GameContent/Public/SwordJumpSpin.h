#pragma once

#include "Effect_Container.h"

NS_BEGIN(Client)

class CSwordJumpSpin final : public CEffect_Container
{
	GENERATED_BODY(CSwordJumpSpin)

public:
	static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_SwordJumpSpin";
	static constexpr const _tchar* MODEL_PROTO_TAG = L"Prototype_Component_Model_SwordJumpSpin_Common_Curve03";
	static constexpr const _tchar* TAIL_TEXTURE_PROTO_TAG = L"Prototype_Component_Texture_SwordJumpSpin_CommonTail";
	static constexpr const _tchar* SCROLL_TEXTURE_PROTO_TAG = L"Prototype_Component_Texture_SwordJumpSpin_CommonScroll06";
	static constexpr const _tchar* OUTER_PART_TAG = L"CurveOuter";
	static constexpr const _tchar* CORE_PART_TAG = L"CurveCore";

private:
	CSwordJumpSpin(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CSwordJumpSpin(const CSwordJumpSpin& Prototype);
	virtual ~CSwordJumpSpin() = default;

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
	static CSwordJumpSpin* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;

private:
	virtual void Free() override;
};

NS_END
