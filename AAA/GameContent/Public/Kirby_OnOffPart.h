#pragma once

#include "GameContent_Defines.h"

#include "PartObject.h"

NS_BEGIN(Engine)
class CShader;
class CModel;
class CAnimator;
NS_END

NS_BEGIN(Client)

class CKirby_OnOffPart abstract : public CPartObject
{
	GENERATED_BODY_ABSTRACT(CKirby_OnOffPart)

public:
	struct KIRBY_ONONFFPART_DESC : public CPartObject::PARTOBJECT_DESC
	{
		const _float4x4* pSocketBoneMatrix{ nullptr };
	};

protected:
	CKirby_OnOffPart(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CKirby_OnOffPart(const CKirby_OnOffPart& Prototype);
	virtual ~CKirby_OnOffPart() = default;

protected:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;

public:
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

public:
	virtual void PartOnOff(_bool bOn) { m_bOn = bOn; }

	void Set_SocketBoneMatrix(const _float4x4* pSocketBoneMatrix) { m_pSocketBoneMatrix = pSocketBoneMatrix; }

protected:
	const _float4x4* m_pSocketBoneMatrix{ nullptr };

	_bool m_bOn{};

protected:
	virtual void Free();
};

NS_END