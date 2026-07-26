#pragma once

#include "Effect_Container.h"

NS_BEGIN(Client)

class CMetaSwordSpin final : public CEffect_Container
{
	GENERATED_BODY(CMetaSwordSpin)

public:
	static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_MetaSwordSpin";
	static constexpr const _tchar* MODEL_PROTO_TAG = L"Prototype_Component_Model_MetaSword_Common_Curve03";
	static constexpr const _tchar* TAIL_TEXTURE_PROTO_TAG = L"Prototype_Component_Texture_MetaSword_CommonTail";
	static constexpr const _tchar* SCROLL_TEXTURE_PROTO_TAG = L"Prototype_Component_Texture_MetaSword_CommonScroll06";
	static constexpr const _tchar* OUTER_PART_TAG = L"CurveOuter";
	static constexpr const _tchar* CORE_PART_TAG = L"CurveCore";

private:
	CMetaSwordSpin(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CMetaSwordSpin(const CMetaSwordSpin& Prototype);
	virtual ~CMetaSwordSpin() = default;

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
	static CMetaSwordSpin* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;

private:
	virtual void Free() override;
};

NS_END
