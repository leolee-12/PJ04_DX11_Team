#pragma once

#include "Kirby_Form.h"

NS_BEGIN(Engine)
class CShader;
class CModel;
class CAnimator;
class CTexture;
NS_END

NS_BEGIN(Client)

class CKirby_DeformCar_Main final : public CKirby_Form
{
	GENERATED_BODY(CKirby_DeformCar_Main)

public:
	struct KIRBY_DEFORMCAR_MAIN_DESC : public CKirby_Form::KIRBY_FORM_DESC
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
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;
	virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override { pOutData->strPrototypeTag = PROTOTYPE_TAG; }

private:
	HRESULT Ready_Components();

	virtual HRESULT Render_KirbyMesh(_uint iMeshIndex) override;

public:
	static CKirby_DeformCar_Main* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
protected:
	virtual void Free();
};

NS_END
