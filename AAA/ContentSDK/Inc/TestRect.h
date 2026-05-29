#pragma once

#include "GameContent_Defines.h"
#include "GameObject.h"

NS_BEGIN(Engine)
class CShader;
class CVIBuffer_Rect;
NS_END

NS_BEGIN(Client)

class CTestRect final : public CGameObject
{
	GENERATED_BODY(CTestRect)

public:
	typedef struct tagTRectDesc : public CGameObject::GAMEOBJECT_DESC
	{

	}TRECT_DESC;

	static constexpr const wchar_t* PROTOTYPE_TAG = L"Proto_TestRect";

protected:
	CTestRect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CTestRect(const CTestRect& Prototype);
	virtual ~CTestRect() = default;

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
	CVIBuffer_Rect* m_pVIBuffer = { nullptr };
	
private:
	virtual HRESULT Ready_Events() override { return S_OK; }
	HRESULT Ready_Components();
	HRESULT Bind_ShaderResources();

public:
	static CTestRect* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
protected:
	virtual void Free();
};

NS_END