#pragma once
#include "LevelDesignObject.h"
#include "Effect_Loader.h"

NS_BEGIN(Engine)
class CCollider;
NS_END

NS_BEGIN(Client)

class CLD_LensFlare final : public CLevelDesignObject
{
	GENERATED_BODY(CLD_LensFlare)

public:
	static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_LevelDesign_LensFlare";
	static constexpr const _tchar* OBJECT_NAME = L"LensFlare";
	static constexpr const _tchar* LAYER_TAG = L"Layer_LevelDesign_Guide";

private:
	CLD_LensFlare(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CLD_LensFlare(const CLD_LensFlare& Prototype);
	virtual ~CLD_LensFlare() = default;

	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual HRESULT Validate_Initialized() override;

public:
	virtual void Late_Update(_float fTimeDelta) override;
	virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override;

#pragma region Editable
	virtual HRESULT On_EditTransformChanged() override;
#pragma endregion

private:
	LD_EFFECT_AREA_DESC m_tEffectAreaDesc = {};

	CCollider* m_pTrigger = { nullptr };
	FX_HANDLE m_LensFlareHandle = {};

	_float4x4 m_matAreaLocal = {};
	_float4x4 m_matEffectAnchorLocal = {};
	_float4x4 m_matEffectAnchorWorld = {};

	_uint m_iActivatorCount = { 0u };

private:
	HRESULT Ready_Components(const LD_PARSED_OBJECT& Desc);
	HRESULT Ready_Trigger();
	_bool Build_LocalMatrices(const LD_PARSED_OBJECT& Desc);
	void Update_RuntimeMatrices();
	void Update_Trigger();
	void SetUp_Collider_Callback();

	void Handle_TriggerEnter(CCollider* pOther);
	void Handle_TriggerExit(CCollider* pOther);
	_bool Is_TriggerActivator(const CCollider* pOther) const;

	void Start_LensFlare();
	void Stop_LensFlare();

public:
	static void Register_LevelDesignSpecs();
	static CGameObject* Create_Prototype(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	static CLD_LensFlare* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;

protected:
	virtual void Free() override;
};

NS_END