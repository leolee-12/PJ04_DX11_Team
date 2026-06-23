#pragma once

#include "GameContent_Defines.h"

#include "Effect_RectParticle.h"

NS_BEGIN(Client)

class CTestParticle final : public CEffect_RectParticle
{
	GENERATED_BODY(CEffect_RectParticle)

public:
	struct TEST_PARTICLE_DESC : public CEffect_RectParticle::EFFECT_RECTPARTICLE_DESC
	{
	};

	static constexpr const wchar_t* PROTOTYPE_TAG = L"Proto_TestParticle";

private:
	CTestParticle(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CTestParticle(const CTestParticle& Prototype);
	virtual ~CTestParticle() = default;

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
	static CTestParticle* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
private:
	virtual void Free();
};

NS_END