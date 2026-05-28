#pragma once
#include "Base.h"

NS_BEGIN(Engine)

class CEventBus final : public CBase
{
private:
	CEventBus();
	virtual ~CEventBus();

public:
	SUBHANDLE Subscribe(const wstring& strEventType, function<void(void*)> func);
	void Publish(const wstring& strEventTag, void* pData);
	void Unsubscribe(SUBHANDLE SubHandle);
	void Clear_All();

private:                                
	typedef struct tagHandlerSlot {     
		function<void(void*)> func;     
		_uint uiVersion = 0;            
		_bool bAlive = true;            
	}HANDLERSLOT;                       

private:
	unordered_map<wstring, vector<HANDLERSLOT>> m_hmapHandlers;   
	vector<SUBHANDLE>							m_vecUnsubscribeQueue;                      
	//_bool										m_bRunning = { false };
	// Publish 도중 내부에서 Publish 호출시 내부 루프 끝나고 나오면서 플래그가 꺼지는문제
	// _uint로 대체해서 ++ -- 사용
	_uint m_iRunningDepth = {};


public:
	static CEventBus* Create();
	virtual void Free() override;
};

NS_END

