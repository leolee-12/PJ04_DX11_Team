#pragma once
#include "LevelDesignObject.h"
#include "Effect_Loader.h"

NS_BEGIN(Engine)
class CShader;
class CModel;
class CCollider;
NS_END

NS_BEGIN(Client)
struct LD_SPAWN_SPEC;

class CLevelDesign_Food : public CLevelDesignObject
{
	GENERATED_BODY(CLevelDesign_Food)

public:
	static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_LevelDesign_Food";

private:
	CLevelDesign_Food(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CLevelDesign_Food(const CLevelDesign_Food& Prototype);
	virtual ~CLevelDesign_Food() = default;

	virtual HRESULT	Initialize_Prototype() override;
	virtual HRESULT	Initialize(void* pArg) override;
	virtual HRESULT	Validate_Initialized() override;

public:
	virtual void	Late_Update(_float fTimeDelta) override;
	virtual HRESULT	Render() override;
	virtual HRESULT Render_Shadow() override;
	virtual void	Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override;

	virtual _bool   Is_CullTransformDynamic() const override { return m_bPickingUp || m_bInhaleDisplaced; }

	static void Register_LevelDesignSpecs();
	static _bool Build_Desc(const LD_OBJECT_DESC& CommonDesc, const json& jEntry, const LD_SPAWN_SPEC& Spec, LD_OBJECT_ENTRY* pOutEntry);
	static CGameObject* Create_Prototype(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);

private:
	CShader*			m_pShaderCom = { nullptr };
	CModel*				m_pModelCom = { nullptr };
	CCollider*			m_pHurtBox = { nullptr };
	FX_HANDLE			m_ItemEffectHandle = {};

	LD_FOOD_DESC		m_tFoodDesc = {};
	_bool				m_bPickingUp = { false };
	_float				m_fPickupElapsed = {};
	_float3				m_vPickupStartPos = {};
	_float3				m_vPickupTargetPos = {};

	// »Ì¿‘ ¿Œ∑¬
	_bool				m_bInhalePullRequested = { false };
	_bool				m_bInhaleDisplaced = { false };
	_float				m_fInhalePullSpeed = {};
	_float				m_fInhaleGraceTime = {};
	CGameObject*		m_pInhaler = { nullptr };

private:
	virtual void  On_LDEventReceived(const _wstring& strEventTag) override;

	HRESULT			Ready_Components();
	HRESULT			Ready_RenderComponents();
	HRESULT			Ready_Effect();
	void			Release_Effect();
	HRESULT			Bind_ShaderResources();
	HRESULT			Render_Model();
	const _tchar*	Resolve_ModelProtoTag() const;

	HRESULT			Ready_HurtBox();
	void			SetUp_Collider_Callback();
	void			Handle_Pickup(CCollider* pOther);
	void			Begin_Pickup();
	void			Update_Pickup(_float fTimeDelta);

	void			Handle_InhalePull(CCollider* pOther);
	void			Update_InhalePull(_float fTimeDelta);

public:
	static CLevelDesign_Food* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;

protected:
	virtual void Free() override;
};

NS_END
