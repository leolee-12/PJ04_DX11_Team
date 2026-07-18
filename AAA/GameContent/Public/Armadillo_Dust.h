#pragma once

#include "Effect_Container.h"

NS_BEGIN(Client)

class CArmadillo_Dust final : public CEffect_Container
{
	GENERATED_BODY(CArmadillo_Dust)

public:
	static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_Armadillo_Dust";
	static constexpr const _tchar* TEX_PROTOTAG = L"Proto_Armadillo_Texture_Dust";

private:
	CArmadillo_Dust(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CArmadillo_Dust(const CArmadillo_Dust& Prototype);
	virtual ~CArmadillo_Dust() = default;

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
	static CArmadillo_Dust* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;

private:
	virtual void Free() override;
};

NS_END
