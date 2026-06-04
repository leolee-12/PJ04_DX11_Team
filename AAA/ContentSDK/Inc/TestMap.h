#pragma once

#include "GameContent_Defines.h"
#include "GameObject.h"

NS_BEGIN(Engine)
class CShader;
class CModel;
NS_END

NS_BEGIN(Client)

class CTestMap final : public CGameObject
{
public:
	struct TESTMAP_DESC : public CGameObject::GAMEOBJECT_DESC
	{
	};

	static constexpr const wchar_t* PROTOTYPE_TAG = L"Proto_TestMap";

protected:
	CTestMap(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const _wstring& strPrototypeTag, const _wstring& strModelTag);
	CTestMap(const CTestMap& Prototype);
	virtual ~CTestMap() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;
	virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override
	{
		if (nullptr == pOutData)
			return;

		pOutData->strPrototypeTag = m_wstrPrototypeTag;
	}


private:
	CShader* m_pShaderCom = { nullptr };
	CModel* m_pModelCom = { nullptr };
	
	_wstring m_wstrPrototypeTag = {};
	_wstring m_wstrModelTag = {};

private:
	virtual HRESULT Ready_Events() override { return S_OK; }
	HRESULT Ready_Components();
	HRESULT Bind_ShaderResources();

public:
	static CTestMap* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const _wstring& strPrototypeTag, const _wstring& strModelTag);
	virtual CGameObject* Clone(void* pArg) override;

private:
	virtual void Free();
};

NS_END