#pragma once
#include "LD_EventObject.h"

NS_BEGIN(Client)

struct LD_SPAWN_SPEC;

class CLD_GarageRadio final : public CLD_EventObject
{
	GENERATED_BODY(CLD_GarageRadio)

public:
	static constexpr const _tchar* OBJECT_NAME = L"GarageRadio";
	static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_LevelDesign_GarageRadio";
	static constexpr const _tchar* MODEL_PROTO_TAG = L"Proto_Component_Model_GarageRadio";
	static constexpr const _tchar* LAYER_TAG = L"Layer_LevelDesign_Gimmick";

private:
	enum class STATE { IDLE, PLAYING };

private:
	CLD_GarageRadio(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CLD_GarageRadio(const CLD_GarageRadio& Prototype);
	virtual ~CLD_GarageRadio() = default;

	virtual HRESULT Initialize(void* pArg) override;
	virtual HRESULT Validate_Initialized() override;

public:
	virtual void    Update(_float fTimeDelta) override;
	virtual void    Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override;

	static void			Register_LevelDesignSpecs();
	static _bool		Build_Desc(const LD_OBJECT_DESC& CommonDesc, const json& jEntry, const LD_SPAWN_SPEC& Spec, LD_OBJECT_ENTRY* pOutEntry);

private:
	STATE m_eState = { STATE::IDLE };

private:
	virtual HRESULT Ready_Events() override;

	void On_Event();

public:
	static CGameObject*	Create_Prototype(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	static CLD_GarageRadio* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
};

NS_END