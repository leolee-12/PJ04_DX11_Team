#pragma once
#include "LevelDesignObject.h"

NS_BEGIN(Engine)
class CCollider;
NS_END

NS_BEGIN(Client)

class CLD_CamPivot final : public CLevelDesignObject
{
	GENERATED_BODY(CLD_CamPivot)

public:
	static constexpr const _tchar* OBJECT_NAME = L"DemoCameraPivot";
	static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_LevelDesign_CameraPivot";
	static constexpr const _tchar* LAYER_TAG = L"Layer_LevelDesign_Guide";

private:
	CLD_CamPivot(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CLD_CamPivot(const CLD_CamPivot& Prototype);
	virtual ~CLD_CamPivot() = default;

	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual HRESULT Validate_Initialized() override;

public:
	virtual void Late_Update(_float fTimeDelta) override;
	virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override;

	const _float4x4* Get_PivotWorldMatrixPtr() const { return &m_matPivotWorld; }

#pragma region Editable
	virtual HRESULT On_EditTransformChanged() override;
#pragma endregion

private:
	_float4x4 m_matPivotWorld = {};

#ifdef _DEBUG
	CCollider* m_pDebugCollider = { nullptr };
#endif

private:
	void Sync_PivotWorldMatrix();

#ifdef _DEBUG
	HRESULT Ready_DebugCollider();
#endif

public:
	static void Register_LevelDesignSpecs();
	static CGameObject* Create_Prototype(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);

	static CLD_CamPivot* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;

protected:
	virtual void Free() override;
};

NS_END