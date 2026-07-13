#pragma once

#include "Launcher_Defines.h"
#include "Base.h"
#include <concurrent_queue.h>

/* 데드락 깊게 이해하기 */
/* 상호참조와 비슷한 느낌의 과정으로 데드락이 발생함 */


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
	static constexpr _uint MAX_WORKER_COUNT = 4;

public:
	HRESULT Initialize(LEVEL eNextLevelID, _bool Initialzed);
	HRESULT Collect_And_StartWorkers();
	HRESULT Loading();
	_bool isFinished()
	{
		return m_bJobsReady.load() && m_iFinishedWorkCount.load() == m_iTotalWorkCount;
	}

	_float Get_Progress() const {
		if (!m_bJobsReady.load() || 0 == m_iTotalWorkCount) return 0.f;
		return (_float)m_iFinishedWorkCount.load() / (_float)m_iTotalWorkCount;
	}

#ifdef _DEBUG
public:
	void Show();
#endif

private:
	ID3D11Device*			m_pDevice = { nullptr };
	ID3D11DeviceContext*	m_pContext = { nullptr };
	CGameInstance_Proxy*	m_pGameInstance_Proxy = { nullptr };
	LEVEL					m_eNextLevelID = { LEVEL::END };

	HANDLE					m_hThreads[MAX_WORKER_COUNT] = {};
	_uint					m_iWorkerCount = { 1 };

	Concurrency::concurrent_queue<function<HRESULT()>> m_WorkQueue;

	atomic<_uint>           m_iTotalWorkCount = {0};
	atomic<_uint>			m_iFinishedWorkCount = {0};

private:
	_tchar					m_szLoadingText[MAX_PATH] = {};
	_bool					m_isFinished = { };

	HANDLE					m_hCollector = {};
	_bool					m_bNeedStatic = { false };
	atomic<_bool>			m_bJobsReady = { false };

private:
	void	Add_Work(function<HRESULT()>&& func);
	HRESULT Read_Manifest(const _tchar* path, const LEVEL eLevel);

	HRESULT Ready_StaticResources();
	HRESULT Ready_WorkQueue();

public:
	static CLoader* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, LEVEL eNextLevelID, _bool Initialized = true);
	virtual void Free() override;
};

NS_END