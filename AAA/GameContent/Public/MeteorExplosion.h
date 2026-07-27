#pragma once

#include "Effect_Container.h"
NS_BEGIN(Engine)
class CEffect_Part;
NS_END

NS_BEGIN(Client)

class CMeteorExplosion final : public CEffect_Container
{
	GENERATED_BODY(CMeteorExplosion)

public:
	static constexpr const _tchar* PROTOTYPE_TAG				= L"Proto_MeteorExplosion";
	static constexpr const _tchar* PIECE_SMALL_MODEL_TAG		= L"Prototype_Component_Model_MeteorPieceSmall";
	static constexpr const _tchar* PIECE_COOL_MODEL_TAG			= L"Prototype_Component_Model_MeteorPieceCool";

private:
	CMeteorExplosion(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CMeteorExplosion(const CMeteorExplosion& Prototype);
	virtual ~CMeteorExplosion() = default;

protected:
	virtual HRESULT				Initialize_Prototype() override;
	virtual HRESULT				Initialize(void* pArg) override;

public:
	virtual void				Priority_Update(_float fTimeDelta) override;
	virtual void				Update(_float fTimeDelta) override;
	virtual void				Late_Update(_float fTimeDelta) override;
	virtual HRESULT				Render() override;
	virtual void				Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override
	{
		pOutData->strPrototypeTag = PROTOTYPE_TAG;
	}

private:
	HRESULT						Ready_EffectPartObjects();

public:
	static CMeteorExplosion*	Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject*		Clone(void* pArg) override;

private:
	virtual void				Free() override;
};

NS_END