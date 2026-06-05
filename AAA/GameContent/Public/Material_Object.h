#pragma once

#include "GameContent_Defines.h"
#include "GameObject.h"

NS_BEGIN(Engine)
class CShader;
class CModel;
NS_END

NS_BEGIN(Client)

class CMaterial_Object final : public CGameObject
{
	GENERATED_BODY(CMaterial_Object)
	PROPERTY(_float4, m_vAlbedo, L"AlbedoColor(RGBA)", L"Color")
	PROPERTY(_float4, m_vEmissiveColor, L"EmissiveColor(RGBA)", L"Color")
	PROPERTY(_float3, m_vMRA, L"MRA(Metalness, Roughness, AO)", L"Material")

public:
	typedef struct tagMonsterDesc : public CGameObject::GAMEOBJECT_DESC
	{

	}MONSTER_DESC;

	static constexpr const wchar_t* PROTOTYPE_TAG = L"Proto_MaterialObject";

protected:
	CMaterial_Object(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CMaterial_Object(const CMaterial_Object& Prototype);
	virtual ~CMaterial_Object() = default;

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

private:
	virtual HRESULT Ready_Events() override { return S_OK; }
	HRESULT Ready_Components();
	HRESULT Bind_ShaderResources();

public:
	static CMaterial_Object* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free();
};

NS_END