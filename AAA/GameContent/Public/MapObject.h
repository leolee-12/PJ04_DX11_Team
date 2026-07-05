#pragma once

#include "GameContent_Defines.h"
#include "GameObject.h"

NS_BEGIN(Engine)
class CShader;
class CModel;
NS_END

NS_BEGIN(Client)

// 맵 구조물 공통 베이스: Shader_Map 기반 PBR G-buffer + 오버레이(DirtParts) 메커니즘.
// moss/dirt 같은 구조물별 레이어는 파생에서 Bind_MeshLayers 오버라이드로 추가.
class CLIENT_DLL CMapObject abstract : public CGameObject
{
	GENERATED_BODY_ABSTRACT(CMapObject)

protected:
	CMapObject(ID3D11Device * pDevice, ID3D11DeviceContext * pContext);
	CMapObject(const CMapObject& Prototype);
	virtual ~CMapObject() = default;

	virtual HRESULT	Initialize_Prototype() override;
	virtual HRESULT	Initialize(void* pArg) override;
	virtual HRESULT	Validate_Initialized();

public:
	virtual void    Priority_Update(_float fTimeDelta) override;
	virtual void    Update(_float fTimeDelta) override;
	virtual void    Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

protected:
	CShader* m_pShaderCom = { nullptr };
	CModel* m_pModelCom = { nullptr };

	// 파생이 바꾸는 지점
	virtual const _tchar* Get_ModelProtoTag() const = 0;   // 구조물별 .ysh 모델 프로토타입
	virtual _uint Get_ModelProtoLevel() const = 0;
	virtual void  Bind_MeshLayers(_uint iMesh) {}          // 베이스: no-op(순수 PBR)
	virtual HRESULT Bind_WorldMatrix();

protected:
	virtual HRESULT Ready_Events() override { return S_OK; }
	virtual _bool Should_RenderMesh(_uint iMesh) const { return true; }
	HRESULT Ready_MapComponents();
	HRESULT Bind_ShaderResources();

public:
	virtual void Free() override;
};

NS_END
