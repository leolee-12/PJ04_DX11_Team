#pragma once

#include "GameContent_Defines.h"
#include "GameObject.h"

NS_BEGIN(Engine)
class CShader;
class CTexture;
class CVIBuffer_Terrain;
NS_END

NS_BEGIN(Client)

class CTerrain final : public CGameObject
{
	typedef struct tagTerrainDesc : public CGameObject::GAMEOBJECT_DESC
	{

	}TERRAIN_DESC;

	static constexpr const wchar_t* PROTOTYPE_TAG = L"Proto_Terrain";
	
protected:
	CTerrain(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CTerrain(const CTerrain& Prototype);
	virtual ~CTerrain() = default;

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
	CTexture*			m_pTextureCom = { nullptr };
	CVIBuffer_Terrain*	m_pVIBufferCom = { nullptr };	

private:
	virtual HRESULT Ready_Events() override { return S_OK; }
	HRESULT Ready_Components();
	HRESULT Bind_ShaderResources();


public:
	static CTerrain* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free();
};

NS_END