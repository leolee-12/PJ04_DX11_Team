#pragma once
#include "Base.h"
#include "LevelDesign_Registry.h"

NS_BEGIN(Client)

class CLevelDesign_ProtoRegister final : public CBase
{
private:
	CLevelDesign_ProtoRegister(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);

	virtual ~CLevelDesign_ProtoRegister() = default;

public:
	HRESULT Ready_Prototypes(const LD_RUNTIME_LEVELS& Levels, const LD_PACKAGE& Package);

private:
	ID3D11Device* m_pDevice = nullptr;
	ID3D11DeviceContext* m_pContext = nullptr;
	CGameInstance_Proxy* m_pProxy = nullptr;

private:
	HRESULT Ensure_Resources(_uint iPrototypeLevel, const LD_SPAWN_SPEC& Spec);

public:
	static CLevelDesign_ProtoRegister* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);

private:
	virtual void Free() override;
};

NS_END