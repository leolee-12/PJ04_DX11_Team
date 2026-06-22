#pragma once
#include "LevelDesignObject.h"

NS_BEGIN(Engine)
class CShader;
class CModel;
NS_END

NS_BEGIN(Client)
struct LD_SPAWN_SPEC;

class CLevelDesign_Point : public CLevelDesignObject
{
	GENERATED_BODY(CLevelDesign_Point);

public:
	static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_LevelDesign_Point";
	static constexpr const _tchar* YELLOW_MODEL_PROTO_TAG = L"Proto_Component_Model_Point_Yellow";
	static constexpr const _tchar* BLUE_MODEL_PROTO_TAG = L"Proto_Component_Model_Point_Blue";
	static constexpr const _tchar* GREEN_MODEL_PROTO_TAG = L"Proto_Component_Model_Point_Green";
	static constexpr const _tchar* RED_MODEL_PROTO_TAG = L"Proto_Component_Model_Point_Red";
	static constexpr const _tchar* COIN_CLUSTER_S_MODEL_PROTO_TAG = L"Proto_Component_Model_Point_CoinCluster_S";
	static constexpr const _tchar* COIN_CLUSTER_M_MODEL_PROTO_TAG = L"Proto_Component_Model_Point_CoinCluster_M";
	static constexpr const _tchar* COIN_CLUSTER_L_MODEL_PROTO_TAG = L"Proto_Component_Model_Point_CoinCluster_L";

private:
	CLevelDesign_Point(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CLevelDesign_Point(const CLevelDesign_Point& Prototype);
	virtual ~CLevelDesign_Point() = default;

public:
	virtual HRESULT	Initialize_Prototype() override;
	virtual HRESULT	Initialize(void* pArg) override;
	virtual void	Late_Update(_float fTimeDelta) override;
	virtual HRESULT	Render() override;
	virtual void	Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override;

	static LD_POINT_TYPE Resolve_PointType(const _wstring& wstrObjName);
	static void Register_LevelDesignSpecs();
	static _bool Build_Desc(const LD_OBJECT_DESC& CommonDesc, const json& jEntry, const LD_SPAWN_SPEC& Spec, LD_OBJECT_ENTRY* pOutEntry);
	static CGameObject* Create_Prototype(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);

private:
	CShader*	m_pShaderCom = nullptr;
	CModel*		m_pModelCom = nullptr;

	LD_POINT_DESC	m_tPointDesc = {};
	_uint			m_iModelProtoLevel = { ETOUI(LEVEL::GAMEPLAY) };

private:
	virtual HRESULT	Validate_Desc() override;

	HRESULT			Ready_Components();
	HRESULT			Bind_ShaderResources();
	HRESULT			Render_Model();
	const _tchar*	Resolve_ModelProtoTag() const;

public:
	static CLevelDesign_Point* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;

protected:
	virtual void Free() override;
};

NS_END