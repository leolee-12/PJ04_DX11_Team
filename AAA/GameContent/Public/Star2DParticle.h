#pragma once

#include "GameContent_Defines.h"

#include "Effect_RectParticle.h"

NS_BEGIN(Client)

class CStar2DParticle final : public CEffect_RectParticle
{
	GENERATED_BODY(CStar2DParticle)

public:
	static constexpr const wchar_t* PROTOTYPE_TAG = L"Proto_Star2DParticle";

private:
	CStar2DParticle(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CStar2DParticle(const CStar2DParticle& Prototype);
	virtual ~CStar2DParticle() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;

public:
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;
	virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override { pOutData->strPrototypeTag = PROTOTYPE_TAG; }

public:
	static CStar2DParticle* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
private:
	virtual void Free();
};

NS_END