#pragma once
#include "LD_EventObject.h"

NS_BEGIN(Engine)
class CCollider;
NS_END

NS_BEGIN(Client)

struct LD_SPAWN_SPEC;

class CLD_SlopeBoardC final : public CLD_EventObject
{
	GENERATED_BODY(CLD_SlopeBoardC);

public:
	static constexpr const _tchar* OBJECT_NAME = L"SlopeBoardC";
	static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_LevelDesign_SlopeBoardC";
	static constexpr const _tchar* MODEL_PROTO_TAG = L"Proto_Component_Model_SlopeBoard_C";
	static constexpr const _tchar* LAYER_TAG = L"Layer_LevelDesign_Gimmick";

private:
	enum class STATE { IDLE, PLAYING, PLAYED };

private:
	CLD_SlopeBoardC(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CLD_SlopeBoardC(const CLD_SlopeBoardC& Prototype);
	virtual ~CLD_SlopeBoardC() = default;

	virtual HRESULT Validate_Initialized() override;

public:
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override;

	static void Register_LevelDesignSpecs();
	static _bool Build_Desc(const LD_OBJECT_DESC& CommonDesc, const json& jEntry, const LD_SPAWN_SPEC& Spec, LD_OBJECT_ENTRY* pOutEntry);
	static CGameObject* Create_Prototype(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);

private:
	CCollider* m_pInteractionCollider = { nullptr };

	STATE m_eState = { STATE::IDLE };
	_float m_fEventAnimSpeed = { 1.5f };

private:
	virtual HRESULT Ready_Components() override;
	HRESULT Ready_RenderComponents();
	HRESULT Ready_InteractionCollider();

	void Handle_Interaction(CCollider* pOther);

public:
	static CLD_SlopeBoardC* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
};

NS_END