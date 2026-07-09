#pragma once
#include "LevelDesignObject.h"

NS_BEGIN(Engine)
class CShader;
class CModel;
class CAnimator;
class CCollider;
NS_END

NS_BEGIN(Client)

struct LD_SPAWN_SPEC;

class CLD_PopFlower final : public CLevelDesignObject
{
	GENERATED_BODY(CLD_PopFlower);

public:
	static constexpr const _tchar* OBJECT_NAME = L"PopFlower";
	static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_LevelDesign_PopFlower";
	static constexpr const _tchar* MODEL_PROTO_TAG = L"Proto_Component_Model_PopFlower";
	static constexpr const _tchar* LAYER_TAG = L"Layer_LevelDesign_Gimmick";

private:
	enum class STATE
	{
		IDLE,
		BLOOMING,
		BLOOMED
	};

private:
	CLD_PopFlower(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CLD_PopFlower(const CLD_PopFlower& Prototype);
	virtual ~CLD_PopFlower() = default;

	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual HRESULT Validate_Initialized() override;

public:
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;
	virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override;

	static void Register_LevelDesignSpecs();
	static _bool Build_Desc(const LD_OBJECT_DESC& CommonDesc, const json& jEntry, const LD_SPAWN_SPEC& Spec, LD_OBJECT_ENTRY* pOutEntry);
	static CGameObject* Create_Prototype(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);

private:
	CShader* m_pShaderCom = { nullptr };
	CModel* m_pModelCom = { nullptr };
	CAnimator* m_pAnimatorCom = { nullptr };
	CCollider* m_pTrigger = { nullptr };

	LD_EVENTOBJECT_DESC m_tPopFlowerDesc = {};
	STATE m_eState = { STATE::IDLE };

	_int m_iBudMeshIndex = { -1 };
	_int m_iBloomMeshIndex = { -1 };
	_bool m_bPlayerOverlapping = { false };

private:
	HRESULT Ready_Components();
	HRESULT Ready_RenderComponents();
	HRESULT Ready_Trigger();
	HRESULT Ready_InitialState();

	HRESULT Bind_ShaderResources();
	HRESULT Render_Model();

	void SetUp_Collider_Callback();
	void Handle_TriggerEnter(CCollider* pOther);
	void Handle_TriggerStay(CCollider* pOther);
	void Handle_TriggerExit(CCollider* pOther);
	void Grant_Reward(CCollider* pOther);
	_bool Play_Animation(const _char* pAnimName, _bool bLoop);
	_bool Is_PlayerCollider(CCollider* pOther) const;
	void Update_AnimationSpeed();
	_int Find_MeshIndex_ByName(const _char* pMeshName) const;
	_bool Should_RenderMesh(_uint iMeshIndex) const;

public:
	static CLD_PopFlower* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;

protected:
	virtual void Free() override;
};

NS_END
