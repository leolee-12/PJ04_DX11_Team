#pragma once

#include "GameContent_Defines.h"
#include "GameObject.h"

NS_BEGIN(Engine)
class CShader;
class CModel;
class CNavigation;
NS_END

NS_BEGIN(Client)

class CTestFiona final : public CGameObject
{
	GENERATED_BODY(CTestFiona)
	PROPERTY(ANIM_INDEX, m_iAnimationIndex, L"Animation_Index", L"Animation")

public:
	typedef struct tagMonsterDesc : public CGameObject::GAMEOBJECT_DESC
	{

	}MONSTER_DESC;

	static constexpr const wchar_t* PROTOTYPE_TAG = L"Proto_TestFiona";
	
protected:
	CTestFiona(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CTestFiona(const CTestFiona& Prototype);
	virtual ~CTestFiona() = default;

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
	CShader*			m_pShaderCom = { nullptr };	
	CModel*				m_pModelCom = { nullptr };	

	CNavigation*		m_pNavigationCom = { nullptr };

private:
	virtual HRESULT Ready_Events() override { return S_OK; }
	HRESULT Ready_Components();
	HRESULT Bind_ShaderResources();

public:
	static CTestFiona* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free();
};

NS_END