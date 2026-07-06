#pragma once
#include "LD_EventObject.h"

NS_BEGIN(Engine)
class CCollider;
NS_END

NS_BEGIN(Client)

struct LD_SPAWN_SPEC;

class CLD_SlopeBoardA final : public CLD_EventObject
{
	GENERATED_BODY(CLD_SlopeBoardA);

public:
	static constexpr const _tchar* OBJECT_NAME = L"SlopeBoardA";
	static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_LevelDesign_SlopeBoardA";
	static constexpr const _tchar* MODEL_PROTO_TAG = L"Proto_Component_Model_SlopeBoard_A";
	static constexpr const _tchar* LAYER_TAG = L"Layer_LevelDesign_Gimmick";

private:
	enum class STATE { IDLE, PLAYING, BROKEN };

private:
	CLD_SlopeBoardA(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CLD_SlopeBoardA(const CLD_SlopeBoardA& Prototype);
	virtual ~CLD_SlopeBoardA() = default;

	virtual HRESULT Validate_Initialized() override;

public:
	virtual void	Update(_float fTimeDelta) override;
	virtual void	Late_Update(_float fTimeDelta) override;
	virtual void	Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override;

	static void		Register_LevelDesignSpecs();
	static _bool	Build_Desc(const LD_OBJECT_DESC& CommonDesc, const json& jEntry, const LD_SPAWN_SPEC& Spec, LD_OBJECT_ENTRY* pOutEntry);
	static CGameObject* Create_Prototype(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);

private:
	CCollider* m_pInteractionTrigger = { nullptr };
	_bool m_bInteractionTriggerRegistered = { false };

	STATE m_eState = { STATE::IDLE };
	_float m_fElapsed = { 0.f };
	_bool m_bEndAnimationPlayed = { false };

private:
	virtual HRESULT Ready_Components() override;
	virtual _bool Should_RenderMesh(_uint iMeshIndex) const override;
	virtual HRESULT Bind_BoneMatrices(_uint iMeshIndex) override;

	HRESULT Ready_SlopeBoard();
	HRESULT Ready_RigidStatic_FromMeshAABB();
	HRESULT Update_RigidStatic_ToFallenPose();
	HRESULT Ready_InteractionTrigger();

	void SetUp_InteractionTriggerCallback();
	void Handle_InteractionTrigger(CCollider* pOther);
	void Unregister_InteractionTrigger(_bool bImmediate);
	void On_Event();

public:
	static CLD_SlopeBoardA* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;

protected:
	virtual void Free() override;
};

NS_END