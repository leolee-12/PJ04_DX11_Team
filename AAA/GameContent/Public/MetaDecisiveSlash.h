#pragma once

#include "Effect_Container.h"

NS_BEGIN(Client)

class CMetaDecisiveSlash final : public CEffect_Container
{
	GENERATED_BODY(CMetaDecisiveSlash)

public:
	static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_MetaDecisiveSlash";
	static constexpr const _tchar* MODEL_PROTO_TAG_RING = L"Prototype_Component_Model_MetaDecisiveSlash_Common_Ring03";
	static constexpr const _tchar* SPIN_TEXTURE_PROTO_TAG = L"Prototype_Component_Texture_MetaDecisiveSlash_CommonSpin01";
	static constexpr const _tchar* CIRCLE05_TEXTURE_PROTO_TAG = L"Prototype_Component_Texture_MetaDecisiveSlash_CommonCircle05";
	static constexpr const _tchar* CIRCLE06_TEXTURE_PROTO_TAG = L"Prototype_Component_Texture_MetaDecisiveSlash_CommonCircle06";

	static constexpr const _tchar* DEEP_BLUE_PART_TAG = L"DeepBlue";
	static constexpr const _tchar* SKY_BLUE_PART_TAG = L"SkyBlue";
	static constexpr const _tchar* WHITE_SLASH_PART_TAG = L"WhiteSlash";

private:
	CMetaDecisiveSlash(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CMetaDecisiveSlash(const CMetaDecisiveSlash& Prototype);
	virtual ~CMetaDecisiveSlash() = default;

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
	static CMetaDecisiveSlash* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;

private:
	virtual void Free() override;
};

NS_END
