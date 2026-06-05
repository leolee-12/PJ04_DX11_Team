#pragma once
#include "GameContent_Defines.h"
#include "GameObject.h"

NS_BEGIN(Engine)

class CGameInstance_Proxy;
class CShader;
class CModel;

NS_END

NS_BEGIN(Client)

class CSkySphere final : public CGameObject
{
	GENERATED_BODY(CSkySphere)
	PROPERTY(_uint, m_iModelIndex, L"ModelIndex", L"Model")

protected:
	CSkySphere(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CSkySphere(const CSkySphere& Prototype);
	virtual ~CSkySphere() = default;

public:
	static constexpr const wchar_t* PROTOTYPE_TAG = L"Proto_SkySphere";

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;
	virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override
	{
		pOutData->strPrototypeTag = PROTOTYPE_TAG;
	}

protected:
	CModel* m_pSphereModels[ETOUI(SKYTYPE::END)] = {nullptr};
	CShader* m_pShaderCom = { nullptr };

protected:
	HRESULT Ready_Components();
	HRESULT Bind_ShaderResources();

public:
	static CSkySphere* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg);
	virtual void Free() override;
};

NS_END