#pragma once

#include "Launcher_Defines.h"
#include "Level.h"

NS_BEGIN(Client)

class CTitleUI;

class CLevel_Logo final : public CLevel
{
public:
	enum class LOGO_STATE { LOGO1, LOGO2, DONE };
private:
	CLevel_Logo(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CLevel_Logo() = default;

public:
	virtual HRESULT Initialize() override;
	virtual void Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	CTitleUI* m_pTitleUI_1 = { nullptr };
	CTitleUI* m_pTitleUI_2 = { nullptr };

	LOGO_STATE m_eLogoState = { LOGO_STATE::LOGO1 };

private:
	virtual HRESULT Ready_Events() override { return S_OK; }

public:
	static CLevel_Logo* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void Free() override;
};

NS_END