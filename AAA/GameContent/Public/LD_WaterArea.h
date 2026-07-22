#pragma once
#include "LevelDesignObject.h"
#include "Water_Defines.h"

NS_BEGIN(Engine)
class CShader;
class CModel;
NS_END

NS_BEGIN(Client)

struct LD_SPAWN_SPEC;

class CLD_WaterArea final : public CLevelDesignObject
{
	GENERATED_BODY(CLD_WaterArea)

public:
	static constexpr const _tchar* OBJECT_NAME = L"WaterArea";
	static constexpr const _tchar* SECOND_OBJECT_NAME = L"SecondWaterArea";
	static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_LevelDesign_WaterArea";
	static constexpr const _tchar* MODEL_PROTO_TAG = L"Proto_Component_Model_WaterArea";
	static constexpr const _tchar* LAYER_TAG = L"Layer_LevelDesign_Volume";

private:
	CLD_WaterArea(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CLD_WaterArea(const CLD_WaterArea& Prototype);
	virtual ~CLD_WaterArea() = default;

	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual HRESULT Validate_Initialized() override;

public:
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;
	virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override;
	virtual json Serialize() const override;
	virtual void Deserialize_Internal(const json& j) override;
	virtual _bool Is_CullingEnabled() const override { return false; }

#pragma region Editable
public:
	virtual _bool Get_EditDesc(EDITABLE_DESC* pOutDesc) const override;
	virtual HRESULT Apply_EditCustomDesc(const EDIT_CUSTOM_DESC& Desc) override;
#pragma endregion

	static void Register_LevelDesignSpecs();
	static _bool Make_DefaultDesc(const LD_OBJECT_DESC& CommonDesc, _uint iModelProtoLevel, const LD_SPAWN_SPEC& Spec, LD_OBJECT_ENTRY* pOutEntry);
	static _bool Build_Desc(const LD_OBJECT_DESC& CommonDesc, const json& jEntry, const LD_SPAWN_SPEC& Spec, LD_OBJECT_ENTRY* pOutEntry);
	static CGameObject* Create_Prototype(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);

private:
	CShader* m_pShaderCom = { nullptr };
	CModel* m_pModelCom = { nullptr };

	LD_SURFACE_AREA_DESC m_tSurfaceAreaDesc = {};
	WATER_RENDER_DESC m_tWaterRenderDesc = {};

private:
	HRESULT Ready_RenderComponents();
	HRESULT Bind_ShaderResources();
	HRESULT Render_Model();

public:
	static CLD_WaterArea* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;

protected:
	virtual void Collect_EditModelSlots(vector<EDITABLE_MODEL_SLOT>* pOutSlots) const override;
	virtual void Free() override;
};

NS_END