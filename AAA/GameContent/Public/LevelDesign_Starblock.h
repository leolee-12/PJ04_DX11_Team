#pragma once
#include "LevelDesignObject.h"
#include "Damageable.h"
#include "Inhalable.h"

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
	: public CLevelDesignObject
	, public IDamageable
	, public IInhalable
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

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void    Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;
	virtual void    Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override;

	// Damageable
	virtual void Damaged(const ATTACK_INFO& tInfo) override;

	// Inhalable
	virtual _bool Can_BeInhaled(const INHALE_QUERY& q) const override;
	virtual void  Be_Captured(CGameObject* pInhaler) override;
	virtual COPY_ABILITY_TYPE Get_CopyAbility() const override;
	virtual void  Be_Spat(_fvector vPos, _fvector vDir, _float fSpeed) override;
	virtual CGameObject* Get_GameObject() override;

public:
	const LD_BREAKABLE_DESC& Get_BreakableDesc() const { return m_tBreakableDesc; }

private:
	CShader* m_pShaderCom = { nullptr };
	CModel* m_pModelCom = { nullptr };
	CCollider* m_pCollider = { nullptr };
	physx::PxRigidStatic* m_pPhysicsActor = { nullptr };

	LD_BREAKABLE_DESC m_tBreakableDesc = {};

private:
	virtual	HRESULT	Validate_Desc() override;

	HRESULT			Ready_Components();
	HRESULT			Ready_PhysicsActor_Box();
	void			Release_PhysicsActor();
	HRESULT			Bind_ShaderResources();
	const _tchar*	Resolve_ModelProtoTag() const;

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