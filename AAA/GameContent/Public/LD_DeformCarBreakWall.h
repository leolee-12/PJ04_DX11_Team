#pragma once
#include "LD_EventObject.h"

NS_BEGIN(Engine)
class CCollider;
NS_END

NS_BEGIN(Client)

struct LD_SPAWN_SPEC;

class CLD_DeformCarBreakWall final : public CLD_EventObject
{
	GENERATED_BODY(CLD_DeformCarBreakWall);

public:
	static constexpr const _tchar* OBJECT_NAME = L"DemoDeformCarFirst";
	static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_LevelDesign_DeformCarBreakWall";
	static constexpr const _tchar* MODEL_PROTO_TAG = L"Proto_Component_Model_DeformCarBreakWall";
	static constexpr const _tchar* LAYER_TAG = L"Layer_LevelDesign_Gimmick";

private:
	enum class STATE { IDLE, BREAKING, BROKEN };

private:
	CLD_DeformCarBreakWall(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CLD_DeformCarBreakWall(const CLD_DeformCarBreakWall& Prototype);
	virtual ~CLD_DeformCarBreakWall() = default;

	virtual HRESULT Validate_Initialized() override;

public:
	virtual void    Update(_float fTimeDelta) override;
	virtual void    Late_Update(_float fTimeDelta) override;
	virtual void    Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override;

	static void		Register_LevelDesignSpecs();
	static _bool	Build_Desc(const LD_OBJECT_DESC& CommonDesc, const json& jEntry, const LD_SPAWN_SPEC& Spec, LD_OBJECT_ENTRY* pOutEntry);
	static CGameObject*	Create_Prototype(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);

private:
	CCollider*	m_pBoostTrigger = { nullptr };
	_bool		m_bBoostTriggerRegistered = { false };
	STATE		m_eState = { STATE::IDLE };
	_bool		m_bMeshHiddenAtFrame = { false };

private:
	virtual HRESULT Ready_Components() override;

	HRESULT	Ready_DeformCarBreakWall();
	HRESULT	Ready_WallRigidStatic(const BoundingBox& LocalBounds);
	HRESULT	Ready_BoostTrigger(const BoundingBox& LocalBounds);

	void	SetUp_BoostTriggerCallback();
	void	Handle_BoostTrigger(CCollider* pOther);
	void	Unregister_BoostTrigger(_bool bImmediate);
	void	On_Event();

public:
	static CLD_DeformCarBreakWall* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;

protected:
	virtual void Free() override;
};

NS_END