#pragma once
#include "MapTool_Defines.h"
#include "Base.h"
#include <concurrent_queue.h>

NS_BEGIN(Engine)
class CGameInstance_Proxy;
NS_END


NS_BEGIN(Client)

class CLoader final : public CBase
{
private:
	CLoader(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CLoader() = default;

public: 
	static constexpr _uint	WORKER_COUNT = 4;

public:
	HRESULT Initialize(EDIT_LEVEL eNextLevelID);
	HRESULT Loading();
	_bool isFinished()
	{
		return m_iFinishedWorkCount.load() == m_iTotalWorkCount;
	}

#ifdef _DEBUG
public:
	void Show();
#endif

private:
	ID3D11Device*			m_pDevice = { nullptr };
	ID3D11DeviceContext*	m_pContext = { nullptr };
	//CGameInstance*			m_pGameInstance = { nullptr };
	CGameInstance_Proxy*	m_pGameInstance_Proxy = { nullptr };
	EDIT_LEVEL					m_eNextLevelID = { EDIT_LEVEL::END };

	HANDLE					m_hThreads[WORKER_COUNT] = { };

	Concurrency::concurrent_queue<function<HRESULT()>> m_WorkQueue;

	_uint                   m_iTotalWorkCount = {0};
	atomic<_uint>			m_iFinishedWorkCount = {0};	


	/*CRITICAL_SECTION		m_CriticalSection = { };*/

private:
	_tchar					m_szLoadingText[MAX_PATH] = {};
	_bool					m_isFinished = { };

private:
	void	Add_Work(function<HRESULT()>&& func);
	HRESULT Ready_WorkQueue();
	HRESULT Ready_Resources_For_Logo();
	HRESULT Ready_Resources_For_GamePlay();

public:
	static CLoader* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, EDIT_LEVEL eNextLevelID);
	virtual void Free() override;
};

NS_END