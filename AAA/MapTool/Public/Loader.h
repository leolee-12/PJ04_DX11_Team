#pragma once
#include "MapTool_Defines.h"
#include "Base.h"
#include <concurrent_queue.h>

NS_BEGIN(Engine)
class CGameInstance_Proxy;
NS_END

NS_BEGIN(MapTool)

class CLoader final : public CBase
{
private:
	CLoader(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CLoader() = default;

public:
	static constexpr _uint	WORKER_COUNT = 4;

public:
	HRESULT Initialize(TOOL_LEVEL eNextLevelID);
	HRESULT Loading();

	_bool	isFinished() const
	{
		return m_iFinishedWorkCount.load() == m_iTotalWorkCount;
	}
	_float	Get_Progress() const
	{
		return m_iTotalWorkCount ? (_float)m_iFinishedWorkCount.load() / (_float)m_iTotalWorkCount : 1.f;
	}
	_uint   Get_FailedWorkCount() const
	{
		return m_iFailedWorkCount.load();
	}

#ifdef _DEBUG
public:
	void	Show();
#endif

private:
	ID3D11Device*			m_pDevice = { nullptr };
	ID3D11DeviceContext*	m_pContext = { nullptr };
	CGameInstance_Proxy*	m_pGameInstance_Proxy = { nullptr };
	TOOL_LEVEL				m_eNextLevelID = { TOOL_LEVEL::END };

	HANDLE					m_hThreads[WORKER_COUNT] = { };

	Concurrency::concurrent_queue<function<HRESULT()>> m_WorkQueue;

	_uint					m_iTotalWorkCount = { 0 };
	atomic<_uint>			m_iFinishedWorkCount = { 0 };

	_tchar					m_szLoadingText[MAX_PATH] = {};
	atomic<_uint>			m_iFailedWorkCount = { 0 };

private:
	void	Add_Work(function<HRESULT()>&& func);
	HRESULT	Ready_WorkQueue();
	HRESULT	Ready_Resources_For_Edit();
	HRESULT	Ready_TexPool_For_Edit();

public:
	static CLoader* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, TOOL_LEVEL eNextLevelID);
	virtual void Free() override;
};

NS_END
