#pragma once

#include "Kirby_HitBox_Model.h"

NS_BEGIN(Engine)
NS_END

NS_BEGIN(Client)

class CKirby_DeformCar_Main final : public CKirby_HitBox_Model
{
	GENERATED_BODY(CKirby_DeformCar_Main)

public:
	struct KIRBY_DEFORMCAR_MAIN_DESC : public CKirby_HitBox_Model::KIRBY_HITBOX_MODEL_DESC
	{
	};

	static constexpr const wchar_t* PROTOTYPE_TAG = L"Proto_Kirby_DeformCar_Main";
	static constexpr const wchar_t* Kirby_PartTag = L"Deform_Car_Main";

private:
	enum DEFORMCAR_MAIN_MESH { MESH_CAR, MESH_KIRBY, MESH_TIRES, MESH_END };

	CKirby_DeformCar_Main(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CKirby_DeformCar_Main(const CKirby_DeformCar_Main& Prototype);
	virtual ~CKirby_DeformCar_Main() = default;

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
	static CKirby_DeformCar_Main* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
private:
	virtual void Free();
};

NS_END
