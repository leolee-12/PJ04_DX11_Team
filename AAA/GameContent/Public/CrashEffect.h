#pragma once
#include "Effect_Container.h"

NS_BEGIN(Client)

class CCrashEffect final : public CEffect_Container
{
	GENERATED_BODY(CCrashEffect)

public:
	static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_CrashEffect";

	static constexpr const _tchar* MODEL_GROUND_SMOKE_TAG = L"Prototype_Component_Model_Crash_GroundSmoke";
	static constexpr const _tchar* MODEL_SPHERE_TAG = L"Prototype_Component_Model_Crash_Sphere";
	static constexpr const _tchar* MODEL_RING_TAG = L"Prototype_Component_Model_Crash_Ring";
	static constexpr const _tchar* MODEL_CIRCLE_TAG = L"Prototype_Component_Model_Crash_Circle";

	static constexpr const _tchar* TEXTURE_SLASH_TAG = L"Prototype_Component_Texture_Crash_Slash";
	static constexpr const _tchar* TEXTURE_WAVE_TAG = L"Prototype_Component_Texture_Crash_Wave";
	static constexpr const _tchar* TEXTURE_RING04_TAG = L"Prototype_Component_Texture_Crash_Ring04";
	static constexpr const _tchar* TEXTURE_CIRCLE01_TAG = L"Prototype_Component_Texture_Crash_Circle01";
	static constexpr const _tchar* TEXTURE_STAR00_TAG = L"Prototype_Component_Texture_Crash_Star00";
	static constexpr const _tchar* TEXTURE_CIRCLE02_TAG = L"Prototype_Component_Texture_Crash_Circle02";
	static constexpr const _tchar* TEXTURE_SHINE_CIRCLE_TAG = L"Prototype_Component_Texture_Crash_ShineCircle";
	static constexpr const _tchar* TEXTURE_CIRCLE06_TAG = L"Prototype_Component_Texture_Crash_Circle06";
	static constexpr const _tchar* TEXTURE_RING03_TAG = L"Prototype_Component_Texture_Crash_Ring03";
	static constexpr const _tchar* TEXTURE_CIRCLE07_TAG = L"Prototype_Component_Texture_Crash_Circle07";
	static constexpr const _tchar* TEXTURE_CIRCLE_GLOW_TAG = L"Prototype_Component_Texture_Crash_CircleGlow";
	static constexpr const _tchar* TEXTURE_WARP_TAG = L"Prototype_Component_Texture_Crash_Warp";

private:
	CCrashEffect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CCrashEffect(const CCrashEffect& Prototype);
	virtual ~CCrashEffect() = default;

protected:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;

public:
	virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override { pOutData->strPrototypeTag = PROTOTYPE_TAG; }

private:
	HRESULT Ready_EffectPartObjects();

public:
	static CCrashEffect* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;

private:
	virtual void Free() override;
};

NS_END