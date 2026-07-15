#pragma once

#include "Kirby_Deform_Model.h"

NS_BEGIN(Engine)
NS_END

NS_BEGIN(Client)

class CKirby_DeformCylinder_Demo final : public CKirby_Deform_Model
{
	GENERATED_BODY(CKirby_DeformCylinder_Demo)

public:
	struct KIRBY_DEFORMCYLINDER_DEMO_DESC : public CKirby_Deform_Model::KIRBY_DEFORM_MODEL_DESC
	{
	};

	static constexpr const wchar_t* PROTOTYPE_TAG = L"Proto_Kirby_DeformCylinder_Demo";
	static constexpr const wchar_t* Kirby_PartTag = L"Deform_Cylinder_Demo";

private:
	CKirby_DeformCylinder_Demo(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CKirby_DeformCylinder_Demo(const CKirby_DeformCylinder_Demo& Prototype);
	virtual ~CKirby_DeformCylinder_Demo() = default;

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
	static CKirby_DeformCylinder_Demo* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;

private:
	virtual void Free() override;
};

NS_END
