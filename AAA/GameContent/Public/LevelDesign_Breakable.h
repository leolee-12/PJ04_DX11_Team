#pragma once
#include "LevelDesignObject.h"
#include "Damageable.h"

NS_BEGIN(Engine)
class CShader;
class CModel;
class CAnimator;
class CCollider;
NS_END

NS_BEGIN(physx)
class PxRigidStatic;
NS_END

NS_BEGIN(Client)
struct LD_SPAWN_SPEC;

class CLevelDesign_Breakable final
	: public CLevelDesignObject
	, public IDamageable
{
	GENERATED_BODY(CLevelDesign_Breakable)

public:
	static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_LevelDesign_Breakable";
	static constexpr const _tchar* WOODBOX_MODEL_PROTO_TAG = L"Prototype_Component_Model_BoxWood";
	static constexpr const _tchar* PLASTICBOX_MODEL_PROTO_TAG = L"Prototype_Component_Model_BoxPlastic";
	static constexpr const _tchar* BOXIRON_MODEL_PROTO_TAG = L"Prototype_Component_Model_BoxIron";
	static constexpr const _tchar* BREAKABLE_ROCK_S_MODEL_PROTO_TAG = L"Prototype_Component_Model_BreakableRock_S";
	static constexpr const _tchar* BREAKABLE_ROCK_M_MODEL_PROTO_TAG = L"Prototype_Component_Model_BreakableRock_M";

private:
	CLevelDesign_Breakable(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CLevelDesign_Breakable(const CLevelDesign_Breakable& Prototype);
	virtual ~CLevelDesign_Breakable() = default;

	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual	HRESULT	Validate_Initialized() override;

public:
	virtual void    Update(_float fTimeDelta) override;
	virtual void    Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;
	virtual HRESULT Render_Shadow() override;
	virtual void    Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override;

	// Damageable
	virtual void Damaged(const ATTACK_INFO& tInfo) override;

	static void					Register_LevelDesignSpecs();
	static _bool				Build_Desc(const LD_OBJECT_DESC& CommonDesc, const json& jEntry, const LD_SPAWN_SPEC& Spec, LD_OBJECT_ENTRY* pOutEntry);
	static CGameObject*			Create_Prototype(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);

public:
	const LD_BREAKABLE_DESC&	Get_BreakableDesc() const { return m_tBreakableDesc; }

private:
	enum class BREAKABLE_STATE { INTACT, BREAKING, DESTROYED };

	CShader* m_pShaderCom = { nullptr };
	CModel* m_pModelCom = { nullptr };
	CAnimator* m_pAnimatorCom = { nullptr };
	CCollider* m_pHurtBox = { nullptr };
	physx::PxRigidStatic* m_pRigidStatic = { nullptr };

	LD_BREAKABLE_DESC m_tBreakableDesc = {};
	_uint m_iBreakAnimIndex = { LD_INVALID_ID };
	_uint m_iBaseMeshIndex = { LD_INVALID_ID };
	BREAKABLE_STATE m_eState = { BREAKABLE_STATE::INTACT };

private:
	HRESULT			Ready_Components();
	HRESULT			Ready_HurtBox();
	HRESULT			Ready_RigidStatic();
	void			Release_RigidStatic();

	HRESULT			Bind_ShaderResources();
	_bool			Should_RenderMesh(_uint iMeshIndex) const;
	const _tchar*	Resolve_ModelProtoTag() const;
	
public:
	static CLevelDesign_Breakable* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;

protected:
	virtual void Free() override;
};

NS_END
