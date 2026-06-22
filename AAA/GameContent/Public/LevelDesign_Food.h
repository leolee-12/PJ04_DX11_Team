#pragma once
#include "LevelDesignObject.h"

NS_BEGIN(Engine)
class CShader;
class CModel;
NS_END

NS_BEGIN(Client)

class CLevelDesign_Food : public CLevelDesignObject
{
	GENERATED_BODY(CLevelDesign_Food);

public:
	static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_LevelDesign_Food";
	static constexpr const _tchar* ENERGY_DRINK_MODEL_PROTO_TAG = L"Proto_Component_Model_Food_EnergyDrink";
	static constexpr const _tchar* DINNER_ROAST_CHICKEN_MODEL_PROTO_TAG = L"Proto_Component_Model_Food_RoastChicken";
	static constexpr const _tchar* FRUIT_CHERRY_MODEL_PROTO_TAG = L"Proto_Component_Model_Food_Cherry";
	static constexpr const _tchar* VEGETABLE_CARROT_MODEL_PROTO_TAG = L"Proto_Component_Model_Food_Carrot";
	static constexpr const _tchar* SWEETS_DOUGHNUT_MODEL_PROTO_TAG = L"Proto_Component_Model_Food_Doughnut";
	static constexpr const _tchar* FRUIT_BANANA_MODEL_PROTO_TAG = L"Proto_Component_Model_Food_Banana";

private:
	CLevelDesign_Food(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CLevelDesign_Food(const CLevelDesign_Food& Prototype);
	virtual ~CLevelDesign_Food() = default;

public:
	virtual HRESULT	Initialize_Prototype() override;
	virtual HRESULT	Initialize(void* pArg) override;
	virtual void	Late_Update(_float fTimeDelta) override;
	virtual HRESULT	Render() override;
	virtual void	Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override;

	static LD_FOOD_TYPE Resolve_FoodType(const _wstring& wstrObjName);

private:
	CShader*		m_pShaderCom = { nullptr };
	CModel*			m_pModelCom = { nullptr };

	LD_FOOD_DESC	m_tFoodDesc = {};
	_uint			m_iModelProtoLevel = { ETOUI(LEVEL::GAMEPLAY) };

private:
	virtual HRESULT	Validate_Desc() override;

	HRESULT			Ready_Components();
	HRESULT			Bind_ShaderResources();
	HRESULT			Render_Model();
	const _tchar*	Resolve_ModelProtoTag() const;

public:
	static CLevelDesign_Food* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;

protected:
	virtual void Free() override;
};

NS_END