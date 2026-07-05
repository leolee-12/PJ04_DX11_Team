#pragma once
#include "LevelDesignObject.h"

NS_BEGIN(Engine)
class CCollider;
NS_END

NS_BEGIN(Client)

class CLevelDesign_Unsupported final : public CLevelDesignObject
{
	GENERATED_BODY(CLevelDesign_Unsupported)

public:
	static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_LevelDesign_Unsupported";

private:
	CLevelDesign_Unsupported(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CLevelDesign_Unsupported(const CLevelDesign_Unsupported& Prototype);
	virtual ~CLevelDesign_Unsupported() = default;

	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual HRESULT Validate_Initialized() override;

public:
	virtual void    Late_Update(_float fTimeDelta) override;
	virtual void    Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override;

#ifdef _DEBUG
private:
	CCollider* m_pDebugCollider = nullptr;

	HRESULT Ready_DebugCollider(const LD_OBJECT_DESC* pObjectDesc);
#endif

public:
	static CLevelDesign_Unsupported* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;

protected:
	virtual void Free() override;
};

NS_END