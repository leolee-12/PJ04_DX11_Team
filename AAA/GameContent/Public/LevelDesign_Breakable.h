#pragma once
#include "LevelDesignObject.h"

NS_BEGIN(Engine)
class CShader;
class CModel;
NS_END

NS_BEGIN(physx)
class PxRigidStatic;
NS_END

NS_BEGIN(Client)

class CLevelDesign_Breakable final : public CLevelDesignObject
{
	GENERATED_BODY(CLevelDesign_Breakable)

public:
	static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_LevelDesign_Breakable";
	static constexpr const _tchar* STARBLOCK_H1W1_MODEL_PROTO_TAG = L"Prototype_Component_Model_LevelDesign_Star_H1W1";
	static constexpr const _tchar* STARBLOCK_H2W2_MODEL_PROTO_TAG = L"Prototype_Component_Model_LevelDesign_Star_H2W2";
	static constexpr const _tchar* STARBLOCK_H3W3_MODEL_PROTO_TAG = L"Prototype_Component_Model_LevelDesign_Star_H3W3";
	static constexpr const _tchar* STARBLOCK_MODEL_PROTO_TAG = STARBLOCK_H1W1_MODEL_PROTO_TAG;

private:
	CLevelDesign_Breakable(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CLevelDesign_Breakable(const CLevelDesign_Breakable& Prototype);
	virtual ~CLevelDesign_Breakable() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void    Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;
	virtual void    Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override;

	static void					Ready_BreakableCatalog();
	static LD_BREAKABLE_TYPE	Resolve_Breakable(const _wstring& wstrObjName);

public:
	const LD_BREAKABLE_OBJECT_DESC&	Get_BreakableDesc() const { return m_tBreakableDesc; }

private:
	CShader* m_pShaderCom = { nullptr };
	CModel* m_pModelCom = { nullptr };
	physx::PxRigidStatic* m_pPhysicsActor = { nullptr };

	LD_BREAKABLE_OBJECT_DESC m_tBreakableDesc = {};

	static vector<pair<_wstring, LD_BREAKABLE_TYPE>> s_BreakableCatalog;

private:
	virtual	HRESULT	Validate_Desc() override;

	HRESULT			Ready_Components();
	HRESULT			Ready_PhysicsActor_Box();
	void			Release_PhysicsActor();
	HRESULT			Bind_ShaderResources();
	const _tchar*	Resolve_ModelProtoTag() const;
	
public:
	static CLevelDesign_Breakable* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;

protected:
	virtual void Free() override;
};

NS_END