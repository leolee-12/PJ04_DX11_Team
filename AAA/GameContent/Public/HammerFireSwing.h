#pragma once

#include "Effect_Container.h"

NS_BEGIN(Client)

class CHammerFireSwing final : public CEffect_Container
{
	GENERATED_BODY(CHammerFireSwing)

public:
	static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_HammerFireSwing";
	static constexpr const _tchar* MAIN_PART_TAG = L"FireMain";
	static constexpr const _tchar* CORE_PART_TAG = L"FireCore";
	static constexpr const _tchar* ACCENT_PART_TAG = L"FireAccent";

private:
	CHammerFireSwing(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CHammerFireSwing(const CHammerFireSwing& Prototype);
	virtual ~CHammerFireSwing() = default;

protected:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;

public:
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;
	virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override { pOutData->strPrototypeTag = PROTOTYPE_TAG; }

private:
	HRESULT Ready_EffectPartObjects();

public:
	static CHammerFireSwing* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;

private:
	virtual void Free() override;
};

NS_END
