#pragma once

#include "GameContent_Defines.h"
#include "GameObject.h"

NS_BEGIN(Engine)
class CShader;
class CModel;
class CAnimator;
NS_END

NS_BEGIN(Client)

class CTestMarb1e final : public CGameObject
{
	GENERATED_BODY(CTestMarb1e)

public:
	typedef struct tagMonsterDesc : public CGameObject::GAMEOBJECT_DESC
	{

	}MONSTER_DESC;

	static constexpr const wchar_t* PROTOTYPE_TAG = L"Proto_TestMarb1e";

protected:
	CTestMarb1e(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CTestMarb1e(const CTestMarb1e& Prototype);
	virtual ~CTestMarb1e() = default;

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


private:
	CShader* m_pShaderCom = { nullptr };
	CModel* m_pModelCom = { nullptr };
	CAnimator* m_pAnimatorCom = { nullptr };


private:
	virtual HRESULT Ready_Events() override { return S_OK; }
	HRESULT Ready_Components();
	HRESULT Bind_ShaderResources();

public:
	static CTestMarb1e* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free();
};

NS_END