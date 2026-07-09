#pragma once
#include "LevelDesignObject.h"

NS_BEGIN(Engine)
class CCollider;
NS_END

NS_BEGIN(Client)

class CLevelDesign_FallBorder : public CLevelDesignObject
{
	GENERATED_BODY(CLevelDesign_FallBorder)

public:
	static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_LevelDesign_FallBorder";
	static constexpr const _tchar* OBJECT_NAME = L"FallBorder";
	static constexpr const _tchar* LAYER_TAG = L"Layer_LevelDesign_Volume";

private:
	CLevelDesign_FallBorder(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CLevelDesign_FallBorder(const CLevelDesign_FallBorder& Prototype);
	virtual ~CLevelDesign_FallBorder() = default;

	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual HRESULT Validate_Initialized() override;

public:
	virtual void    Late_Update(_float fTimeDelta) override;
	virtual void    Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override;

private:
	CCollider* m_pTrigger = { nullptr };

private:
	HRESULT Ready_Components(const LD_PARSED_OBJECT& Desc);
	void    SetUp_Collider_Callback();
	void    Handle_TriggerEnter(CCollider* pOther);

public:
	static void Register_LevelDesignSpecs();
	static CGameObject* Create_Prototype(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	static CLevelDesign_FallBorder* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;

protected:
	virtual void Free() override;
};

NS_END
