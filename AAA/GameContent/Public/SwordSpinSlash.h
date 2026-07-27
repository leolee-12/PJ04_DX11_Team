#pragma once

#include "Effect_Container.h"

NS_BEGIN(Client)

class CSwordSpinSlash final : public CEffect_Container
{
	GENERATED_BODY(CSwordSpinSlash)

public:
	static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_SwordSpinSlash";
	static constexpr const _tchar* MODEL_PROTO_TAG_RING = L"Prototype_Component_Model_SwordSpinSlash_Common_Ring03";
	static constexpr const _tchar* MODEL_PROTO_TAG_RING_HIGH = L"Prototype_Component_Model_SwordSpinSlash_Common_Ring03High";
	static constexpr const _tchar* DISTORTION_TEXTURE_PROTO_TAG = L"Prototype_Component_Texture_SwordSpinSlash_IndirectWarpRing2Normal";
	static constexpr const _tchar* SPIN_TEXTURE_PROTO_TAG = L"Prototype_Component_Texture_SwordSpinSlash_CommonSpin02";
	static constexpr const _tchar* MASK_TEXTURE_PROTO_TAG = L"Prototype_Component_Texture_SwordSpinSlash_CommonCircle05";
	static constexpr const _tchar* WARP_PART_TAG = L"Spin1Warp";
	static constexpr const _tchar* SPIN_PART_TAG = L"Spin1";

private:
	CSwordSpinSlash(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CSwordSpinSlash(const CSwordSpinSlash& Prototype);
	virtual ~CSwordSpinSlash() = default;

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
	static CSwordSpinSlash* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;

private:
	virtual void Free() override;
};

NS_END
