#pragma once
#include "LevelDesignObject.h"
#include "Sound_Handle.h"

NS_BEGIN(Engine)
class CCollider;
NS_END

NS_BEGIN(Client)

class CLD_AudioArea final : public CLevelDesignObject
{
	GENERATED_BODY(CLD_AudioArea)

public:
	static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_LevelDesign_AudioArea";
	static constexpr const _tchar* LAYER_TAG = L"Layer_LevelDesign_Audio";

private:
	enum class MODE
	{
		BGM_REQUEST,
		AMBIENT_LOOP,
		UNKNOWN
	};

private:
	CLD_AudioArea(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CLD_AudioArea(const CLD_AudioArea& Prototype);
	virtual ~CLD_AudioArea() = default;

	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual HRESULT Validate_Initialized() override;

public:
	virtual void Late_Update(_float fTimeDelta) override;
	virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override;

private:
	LD_AUDIO_AREA_DESC m_tAudioAreaDesc = {};
	MODE m_eMode = { MODE::UNKNOWN };

	CCollider* m_pTrigger = { nullptr };

	_bool m_bInside = { false };
	_bool m_bAmbientStopping = { false };

	CSound_Handle m_AmbientHandle = {};
	_float m_fCurrentVolume = { 0.f };

private:
	HRESULT Ready_Trigger();
	void Update_Trigger();
	void SetUp_Collider_Callback();

	void Handle_TriggerEnter(CCollider* pOther);
	void Handle_TriggerStay(CCollider* pOther);
	void Handle_TriggerExit(CCollider* pOther);

	_bool Is_TriggerActivator(CCollider* pOther) const;
	MODE Resolve_Mode() const;

	void Request_BGM();
	void Release_BGM();

	void Start_Ambient();
	void Update_Ambient(_float fTimeDelta);
	void Stop_Ambient();
	void Update_AmbientFadeOut(_float fTimeDelta);

	_wstring Resolve_SoundKey() const;
	_float Get_FadeInSeconds() const;
	_float Get_InactivateSeconds() const;

public:
	static void Register_LevelDesignSpecs();
	static CGameObject* Create_Prototype(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	static CLD_AudioArea* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;

protected:
	virtual void Free() override;
};

NS_END
