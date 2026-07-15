#pragma once
#include "LevelDesignObject.h"

NS_BEGIN(physx)
class PxRigidStatic;
class PxTriangleMesh;
NS_END

NS_BEGIN(Client)

class CLevelDesign_Boundary final : public CLevelDesignObject
{
	GENERATED_BODY(CLevelDesign_Boundary)

public:
	static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_LevelDesign_Boundary";
	static constexpr const _tchar* OBJECT_NAME = L"InvisibleCollision";
	static constexpr const _tchar* LAYER_TAG = L"Layer_LevelDesign_Volume";

private:
	CLevelDesign_Boundary(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CLevelDesign_Boundary(const CLevelDesign_Boundary& Prototype);
	virtual ~CLevelDesign_Boundary() = default;

	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual HRESULT Validate_Initialized() override;

public:
	virtual void    Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override;

private:
	physx::PxRigidStatic* m_pRigidStatic = { nullptr };
	physx::PxTriangleMesh* m_pCollisionMesh = { nullptr };

private:
	virtual HRESULT On_EditTransformChanged() override;
	virtual void On_LDEventReceived(const _wstring& strEventTag) override;

	HRESULT Ready_Components(const LD_PARSED_OBJECT& Desc);
	HRESULT Ready_RigidStatic(const LD_PARSED_OBJECT& Desc);
	HRESULT Ready_RigidStatic_FromPoints(const LD_PARSED_OBJECT& Desc);
	HRESULT Ready_RigidStatic_FromBox();
	void    Release_RigidStatic();

public:
	static void Register_LevelDesignSpecs();
	static CGameObject* Create_Prototype(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	static CLevelDesign_Boundary* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;

protected:
	virtual void Free() override;
};

NS_END
