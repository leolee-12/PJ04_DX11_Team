#pragma once
#include "LD_EventObject.h"

NS_BEGIN(Engine)
class CCollider;
NS_END

NS_BEGIN(Client)

struct LD_SPAWN_SPEC;

class CLD_DeformObject final : public CLD_EventObject
{
	GENERATED_BODY(CLD_DeformObject);

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
	DEFORM_TYPE	Get_DeformType() const { return m_tDeformObjectDesc.eDeformType; }	// 머금기 타입 확인
	_bool		Is_Available() const { return m_bAvailable; }						// 흡입 가능한 상태인지?
	HRESULT		On_DeformAcquired();												// 먹힐 때 발동 (객체 비활성)
	HRESULT		On_DeformReleased(const _float3& vWorldPosition);					// 뱉을 때 발동 (객체 활성)

private:
	LD_DEFORMOBJECT_DESC m_tDeformObjectDesc = {};

	CCollider* m_pInteractionCollider = { nullptr };
	_bool m_bAvailable = { true };

private:
	virtual HRESULT Ready_Components() override;
	HRESULT Ready_InteractionCollider();

	void Set_InteractionEnabled(_bool bEnabled);

public:
	static CLD_DeformObject* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;

protected:
	virtual void Free() override;
};

NS_END