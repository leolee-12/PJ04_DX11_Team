#pragma once
#include "LD_EventObject.h"
#include "Deformable.h"

NS_BEGIN(Engine)
class CCollider;
NS_END

NS_BEGIN(Client)

struct LD_SPAWN_SPEC;

class CLD_DeformObject final
	: public CLD_EventObject
	, public IDeformable
{
	GENERATED_BODY(CLD_DeformObject)

public:
	static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_LevelDesign_DeformObject";
	static constexpr const _tchar* LAYER_TAG = L"Layer_LevelDesign_Gimmick";


private:
	CLD_DeformObject(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CLD_DeformObject(const CLD_DeformObject& Prototype);
	virtual ~CLD_DeformObject() = default;

	virtual HRESULT Initialize(void* pArg) override;
	virtual HRESULT Validate_Initialized() override;

public:
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override;

	static void Register_LevelDesignSpecs();
	static _bool Build_Desc(const LD_OBJECT_DESC& CommonDesc, const json& jEntry, const LD_SPAWN_SPEC& Spec, LD_OBJECT_ENTRY* pOutEntry);
	static CGameObject* Create_Prototype(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);

public:
	_bool		Is_Available() const { return m_bAvailable; }
	HRESULT		On_DeformAcquired();
	HRESULT		On_DeformReleased(const _float3& vWorldPosition);

#pragma region Deformable
	virtual DEFORM_TYPE				Get_DeformType() const override { return m_tDeformObjectDesc.eDeformType; }
	virtual DEFORM_OBJECT_KIND		Get_DeformKind() const override { return m_eKind; }
	virtual _bool					Request_Deform(const _float4x4* AnchorWorld) override;
	virtual void					End_Deform(const _float4x4* AnchorWorld) override;
#pragma endregion

private:
	enum class DEFORM_OBJECT_STATE { IDLE, CAPTURED, ACQUIRED, FALLING, LANDING };

	LD_DEFORMOBJECT_DESC m_tDeformObjectDesc = {};

	CCollider* m_pTrigger = { nullptr };
	_bool m_bAvailable = { true };

	DEFORM_OBJECT_STATE m_eState = { DEFORM_OBJECT_STATE::IDLE };
	DEFORM_OBJECT_KIND m_eKind = { DEFORM_OBJECT_KIND::MOBILE };
	_float4x4 m_AnchorWorld = {};
	_bool m_bAlignDone = { false };
	_float m_fPullSpeed = { 0.f };
	_float m_fVerticalVelocity = { 0.f };

private:
	virtual HRESULT Ready_Components() override;
	virtual void On_Deserialized() override;
	HRESULT Ready_Trigger();

	void Set_TriggerEnabled(_bool bEnabled);
	void Change_State(DEFORM_OBJECT_STATE eState);
	void Update_Captured(_float fTimeDelta);
	void Update_Falling(_float fTimeDelta);

public:
	static CLD_DeformObject* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;

protected:
	virtual void Free() override;
};

NS_END