#pragma once

#include "GameContent_Defines.h"

#include "Effect_Mesh.h"

NS_BEGIN(Client)

class CCommon_SpinSlashTrail final : public CEffect_Mesh
{
	GENERATED_BODY(CCommon_SpinSlashTrail)

public:
	struct COMMON_SPINSSLASHTRAIL_DESC : public CEffect_Mesh::EFFECT_MESH_DESC
	{

	};

	static constexpr const wchar_t* PROTOTYPE_TAG = L"Proto_Common_SpinSlashTrail";

private:
	CCommon_SpinSlashTrail(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CCommon_SpinSlashTrail(const CCommon_SpinSlashTrail& Prototype);
	virtual ~CCommon_SpinSlashTrail() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;

public:
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;
	virtual void Effect_Start() override;
	virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override { pOutData->strPrototypeTag = PROTOTYPE_TAG; }

public:
	void Start_FadeOut(_float fFadeOutDuration = 0.3f);
	_bool Is_FadingOut() const { return m_bFadeOutActive; }
	_bool Is_FadeOutFinished() const { return m_bFadeOutFinished; }

	static CCommon_SpinSlashTrail* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;

private:
	_bool m_bFadeOutActive{};
	_bool m_bFadeOutFinished{};
	_float m_fFadeOutDuration{ 0.3f };
	_float m_fAccFadeOutTime{};
	_float m_fFadeOutStartAlpha{ 1.f };
	_bool m_bInitialAlphaCached{};
	_float m_fInitialAlpha{ 1.f };

private:
	virtual void Free();
};

NS_END
