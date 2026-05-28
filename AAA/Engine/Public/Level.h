#pragma once

/* 클라이언트 개발자가 앞으로 제작할 모든 레벨클래스들의 부모가 되는 클래스 */

#include "Base.h"

NS_BEGIN(Engine)

class CGameInstance_Proxy;

class ENGINE_DLL CLevel abstract : public CBase
{
protected:
	CLevel(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CLevel() = default;

public:
	virtual HRESULT Initialize();
	virtual void Update(_float fTimeDelta);
	virtual HRESULT Render();

protected:
	ID3D11Device*			m_pDevice = { nullptr };
	ID3D11DeviceContext*	m_pContext = { nullptr };
	CGameInstance_Proxy*	m_pGameInstance_Proxy = { nullptr };

protected: // 이벤트 버스 헬퍼
	void	Subscribe_Event(const wstring& strEventTag, function<void(void*)> Handler);
	void	UnSubscribe_Event(const wstring& strEventTag);

	virtual HRESULT Ready_Events() = 0;

private:
	unordered_map<wstring, SUBHANDLE> m_Subhandles;

public:
	virtual void Free() override;
};

NS_END