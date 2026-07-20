#pragma once

#include "Kirby_Deform_Model.h"

NS_BEGIN(Engine)
NS_END

NS_BEGIN(Client)

class CKirby_DeformRollerCoaster_Main final : public CKirby_Deform_Model
{
	GENERATED_BODY(CKirby_DeformRollerCoaster_Main)

public:
	struct KIRBY_DEFORMROLLERCOASTER_MAIN_DESC : public CKirby_Deform_Model::KIRBY_DEFORM_MODEL_DESC
	{
	};

	static constexpr const wchar_t* PROTOTYPE_TAG = L"Proto_Kirby_DeformRollerCoaster_Main";
	static constexpr const wchar_t* Kirby_PartTag = L"Deform_RollerCoaster_Main";

private:
	enum DEFORM_ROLLERCOASTER_MESH
	{
		MESH_KIRBY,
		MESH_ROLLERCOASTER,
		MESH_END
	};

private:
	CKirby_DeformRollerCoaster_Main(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CKirby_DeformRollerCoaster_Main(const CKirby_DeformRollerCoaster_Main& Prototype);
	virtual ~CKirby_DeformRollerCoaster_Main() = default;

private:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;

public:
	virtual HRESULT Render() override;
	virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override { pOutData->strPrototypeTag = PROTOTYPE_TAG; }

	virtual HRESULT Ready_AnimEvents(CKirby* pKirby) override;

private:
	HRESULT Ready_Components();

public:
	static CKirby_DeformRollerCoaster_Main* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;

private:
	virtual void Free() override;
};

NS_END
