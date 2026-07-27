#pragma once

#include "Effect_Container.h"

NS_BEGIN(Client)

class CMetaSuperSpinSlash final : public CEffect_Container
{
	GENERATED_BODY(CMetaSuperSpinSlash)

public:
	static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_MetaSuperSpinSlash";
	static constexpr const _tchar* MODEL_PROTO_TAG_RING = L"Prototype_Component_Model_MetaSuperSpinSlash_Common_Ring03";
	static constexpr const _tchar* SPIN01_TEXTURE_PROTO_TAG = L"Prototype_Component_Texture_MetaSuperSpinSlash_CommonSpin01";
	static constexpr const _tchar* SPIN06_TEXTURE_PROTO_TAG = L"Prototype_Component_Texture_MetaSuperSpinSlash_CommonSpin06";
	static constexpr const _tchar* CIRCLE04_TEXTURE_PROTO_TAG = L"Prototype_Component_Texture_MetaSuperSpinSlash_CommonCircle04";
	static constexpr const _tchar* CIRCLE05_TEXTURE_PROTO_TAG = L"Prototype_Component_Texture_MetaSuperSpinSlash_CommonCircle05";
	static constexpr const _tchar* CIRCLE01_TEXTURE_PROTO_TAG = L"Prototype_Component_Texture_MetaSuperSpinSlash_CommonCircle01";

	static constexpr const _tchar* OUTLINE_DARK_PART_TAG = L"Spin1_Outline_Dark";
	static constexpr const _tchar* OUTLINE_COPY_PART_TAG = L"Spin1_Outline_Copy1";
	static constexpr const _tchar* HUKAI_COPY_PART_TAG = L"Spin1_hukai_Copy1";
	static constexpr const _tchar* OUTLINE_PART_TAG = L"Spin1_Outline";
	static constexpr const _tchar* HUKAI_PART_TAG = L"Spin1_hukai";
	static constexpr const _tchar* SPIN3_PART_TAG = L"Spin3";
	static constexpr const _tchar* WHITE_LINE_PART_TAG = L"Spin1_WhiteLine";
	static constexpr const _tchar* SHINE_POWDER_PART_TAG = L"ShinePowder";

private:
	CMetaSuperSpinSlash(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CMetaSuperSpinSlash(const CMetaSuperSpinSlash& Prototype);
	virtual ~CMetaSuperSpinSlash() = default;

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
	static CMetaSuperSpinSlash* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;

private:
	virtual void Free() override;
};

NS_END
