#pragma once
#include "LevelDesignObject.h"
#include "Damageable.h"

NS_BEGIN(Engine)
class CShader;
class CModel;
class CAnimator;
class CCollider;
NS_END

NS_BEGIN(Client)

struct LD_SPAWN_SPEC;

class CLD_ArrowBoard final : public CLevelDesignObject, public IDamageable
{
	GENERATED_BODY(CLD_ArrowBoard);

public:
	static constexpr const _tchar* OBJECT_NAME = L"ArrowBoard";
	static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_LevelDesign_ArrowBoard";
	static constexpr const _tchar* MODEL_PROTO_TAG = L"Proto_Component_Model_ArrowBoard";
	static constexpr const _tchar* LAYER_TAG = L"Layer_LevelDesign_Gimmick";

private:
	CLD_ArrowBoard(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CLD_ArrowBoard(const CLD_ArrowBoard& Prototype);
	virtual ~CLD_ArrowBoard() = default;

	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual HRESULT Validate_Initialized() override;

public:
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;
	virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override;

	virtual void Damaged(const ATTACK_INFO& tInfo) override;

	static void Register_LevelDesignSpecs();
	static _bool Build_Desc(const LD_OBJECT_DESC& CommonDesc, const json& jEntry, const LD_SPAWN_SPEC& Spec, LD_OBJECT_ENTRY* pOutEntry);
	static CGameObject* Create_Prototype(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);

private:
	CShader* m_pShaderCom = { nullptr };
	CModel* m_pModelCom = { nullptr };
	CAnimator* m_pAnimatorCom = { nullptr };
	CCollider* m_pHurtBox = { nullptr };

	LD_EVENTOBJECT_DESC m_tArrowBoardDesc = {};

	_bool m_bHurtBoxRegistered = { false };
	_float m_fArrowDeg = { 0.f };

private:
	HRESULT Ready_RenderComponents();
	HRESULT Ready_HurtBox();

	HRESULT Bind_ShaderResources();
	HRESULT Render_Model();

	void SetUp_HurtBoxCallback();
	void Handle_HurtBox(CCollider* pOther);
	void Unregister_HurtBox(_bool bImmediate);

public:
	static CLD_ArrowBoard* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;

protected:
	virtual void Free() override;
};

NS_END