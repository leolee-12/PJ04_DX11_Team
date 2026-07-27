#pragma once

#include "Effect_Container.h"

NS_BEGIN(Client)

class CMetaSlash1 final : public CEffect_Container
{
	GENERATED_BODY(CMetaSlash1)

public:
	static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_MetaSlash1";
	static constexpr const _tchar* MODEL_PROTO_TAG_RING = L"Prototype_Component_Model_MetaSlash1_Common_Ring03";
	static constexpr const _tchar* SPIN_TEXTURE_PROTO_TAG = L"Prototype_Component_Texture_MetaSlash1_CommonSpin01";
	static constexpr const _tchar* CIRCLE05_TEXTURE_PROTO_TAG = L"Prototype_Component_Texture_MetaSlash1_CommonCircle05";
	static constexpr const _tchar* CIRCLE06_TEXTURE_PROTO_TAG = L"Prototype_Component_Texture_MetaSlash1_CommonCircle06";

	static constexpr const _tchar* WARP_COLOR_PART_TAG = L"Wind_Warp_Color";
	static constexpr const _tchar* WIND_PART_TAG = L"WindL";

private:
	CMetaSlash1(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CMetaSlash1(const CMetaSlash1& Prototype);
	virtual ~CMetaSlash1() = default;

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
	static CMetaSlash1* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;

private:
	virtual void Free() override;
};

NS_END
