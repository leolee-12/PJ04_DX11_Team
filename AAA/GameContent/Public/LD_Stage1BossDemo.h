#pragma once
#include "LD_EventObject.h"

NS_BEGIN(Client)

struct LD_SPAWN_SPEC;

class CLD_Stage1BossDemo final : public CLD_EventObject
{
	GENERATED_BODY(CLD_Stage1BossDemo)

public:
	static constexpr const _tchar* OBJECT_NAME = L"Level1BossDemoBg";
	static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_LevelDesign_Stage1BossDemo";
	static constexpr const _tchar* MODEL_PROTO_TAG = L"Proto_Component_Model_Level1BossDemoBg";
	static constexpr const _tchar* LAYER_TAG = L"Layer_LevelDesign_Gimmick";

private:
	CLD_Stage1BossDemo(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CLD_Stage1BossDemo(const CLD_Stage1BossDemo& Prototype);
	virtual ~CLD_Stage1BossDemo() = default;

	virtual HRESULT Validate_Initialized() override;

public:
	virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override;

	static void Register_LevelDesignSpecs();
	static _bool Build_Desc(const LD_OBJECT_DESC& CommonDesc, const json& jEntry, const LD_SPAWN_SPEC& Spec, LD_OBJECT_ENTRY* pOutEntry);
	static CGameObject* Create_Prototype(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);

private:
	virtual HRESULT Ready_Events() override;
	virtual HRESULT Ready_Components() override;

	virtual void On_AnimEvent(const ANIM_EVENT& AnimEvent, ANIM_EVENT_PHASE ePhase) override;

	HRESULT Ready_Stage1BossDemo();
	void On_Event(const _wstring& strEventTag);

public:
	static CLD_Stage1BossDemo* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
};

NS_END