#pragma once

#include "Effect_Container.h"

NS_BEGIN(Client)

class CSwordSuperSpinSlash final : public CEffect_Container
{
	GENERATED_BODY(CSwordSuperSpinSlash)

public:
	static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_SwordSuperSpinSlash";
	static constexpr const _tchar* MODEL_PROTO_TAG_RING = L"Prototype_Component_Model_SwordSuperSpinSlash_Common_Ring03";
	static constexpr const _tchar* SPIN01_TEXTURE_PROTO_TAG = L"Prototype_Component_Texture_SwordSuperSpinSlash_CommonSpin01";
	static constexpr const _tchar* SPIN06_TEXTURE_PROTO_TAG = L"Prototype_Component_Texture_SwordSuperSpinSlash_CommonSpin06";
	static constexpr const _tchar* CIRCLE04_TEXTURE_PROTO_TAG = L"Prototype_Component_Texture_SwordSuperSpinSlash_CommonCircle04";

	static constexpr const _tchar* OUTLINE_PART_TAG = L"Spin1_Outline";
	static constexpr const _tchar* HUKAI_PART_TAG = L"Spin1_hukai";
	static constexpr const _tchar* WHITE_LINE_PART_TAG = L"Spin1_WhiteLine";

private:
	CSwordSuperSpinSlash(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CSwordSuperSpinSlash(const CSwordSuperSpinSlash& Prototype);
	virtual ~CSwordSuperSpinSlash() = default;

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
	static CSwordSuperSpinSlash* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;

private:
	virtual void Free() override;
};

NS_END
