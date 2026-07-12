#pragma once
#include "LevelDesignObject.h"
#include "Damageable.h"
#include "LD_Inhalable.h"

NS_BEGIN(Engine)
class CShader;
class CModel;
class CCollider;
NS_END

NS_BEGIN(physx)
class PxRigidStatic;
NS_END

NS_BEGIN(Client)
struct LD_SPAWN_SPEC;

class CLevelDesign_Starblock final
	: public CLD_Inhalable
	, public IDamageable
{
	GENERATED_BODY(CLevelDesign_Starblock)

public:
	static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_LevelDesign_Starblock";
	static constexpr const _tchar* STARBLOCK_H1W1_MODEL_PROTO_TAG = L"Prototype_Component_Model_Star_H1W1";
	static constexpr const _tchar* STARBLOCK_H2W2_MODEL_PROTO_TAG = L"Prototype_Component_Model_Star_H2W2";
	static constexpr const _tchar* STARBLOCK_H3W3_MODEL_PROTO_TAG = L"Prototype_Component_Model_Star_H3W3";
	static constexpr const _tchar* STARBLOCK_MODEL_PROTO_TAG = STARBLOCK_H1W1_MODEL_PROTO_TAG;

private:
	CLevelDesign_Starblock(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CLevelDesign_Starblock(const CLevelDesign_Starblock& Prototype);
	virtual ~CLevelDesign_Starblock() = default;

	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual	HRESULT	Validate_Initialized() override;

public:
	virtual void    Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;
	virtual HRESULT Render_Shadow() override;
	virtual void    Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override;

	// Damageable
	virtual void Damaged(const ATTACK_INFO& tInfo) override;

	// Inhalable
	virtual void Be_Captured(CGameObject* pInhaler) override;
	virtual void On_SpatEnd() override;
	virtual _float3 Get_SpatPivotOffset() const override { return m_vSpatPivot; }

public:
	const LD_BREAKABLE_DESC& Get_BreakableDesc() const { return m_tBreakableDesc; }

private:
	CShader* m_pShaderCom = { nullptr };
	CModel* m_pModelCom = { nullptr };
	physx::PxRigidStatic* m_pRigidStatic = { nullptr };

	LD_BREAKABLE_DESC m_tBreakableDesc = {};

	_float3 m_vSpatPivot{};
	_bool m_bCullTransformDynamic = { false };

private:
	HRESULT			Ready_Components();
	HRESULT			Ready_RigidStatic();
	void			Release_RigidStatic();
	HRESULT			Bind_ShaderResources();
	const _tchar*	Resolve_ModelProtoTag() const;
	void			Compute_SpatPivot();

	virtual void    SetUp_Collider_Callback() override;

public:
	static void			Register_LevelDesignSpecs();
	static _bool		Build_Desc(const LD_OBJECT_DESC& CommonDesc, const json& jEntry, const LD_SPAWN_SPEC& Spec, LD_OBJECT_ENTRY* pOutEntry);
	static CGameObject* Create_Prototype(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);

	static CLevelDesign_Starblock* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;

protected:
	virtual void Free() override;
};

NS_END
