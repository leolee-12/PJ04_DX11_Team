#include "EventBus.h"

CEventBus::CEventBus()
{
}

CEventBus::~CEventBus()
{
}

SUBHANDLE CEventBus::Subscribe(const wstring& strEventType, function<void(void*)> func)
{
	auto [iter, inserted] = m_hmapHandlers.try_emplace(strEventType);

	auto& vec = iter->second;

	if (inserted) {					// 키값이 없어서 새 요소 삽입을 실행했다면
		vec.reserve(16);	// 해당 요소(벡터)의 카파시티의 크기를 16으로 할당해라
	}

	_uint uiIndex(0), uiVersion(0);

	for (_uint i = 0; i < (_uint)vec.size(); ++i)
	{
		if (vec[i].bAlive == false)
		{
			uiIndex = i;
			uiVersion = vec[i].uiVersion;
			vec[i].bAlive = true;
			vec[i].func = move(func);
			return { strEventType, uiIndex, uiVersion };
		}
	}

	uiIndex = (_uint)vec.size();			// 반환할 핸들내부의 인덱스 변수설정
	uiVersion = 1;							// 반환할 핸들내부, 보관할 핸들러슬롯 구조체의 버전 설정

	iter->second.push_back({ move(func), uiVersion, true }); // 핸들러 삽입

	return { strEventType, uiIndex, uiVersion };			// 해당 핸들러의 핸들 반환.
}

void CEventBus::Publish(const wstring& strEventTag, void* pData)
{
	auto iter = m_hmapHandlers.find(strEventTag);
	if (iter == m_hmapHandlers.end()) { return; }

	++m_iRunningDepth;

	for (auto& slot : iter->second)		
	{
		if (slot.bAlive)				
			slot.func(pData);
	}									
										
	--m_iRunningDepth;

	if (m_iRunningDepth == 0)
	{
		for (const auto& Handles : m_vecUnsubscribeQueue) {
			Unsubscribe(Handles);
		}
		m_vecUnsubscribeQueue.clear();
	}
}

void CEventBus::Unsubscribe(SUBHANDLE SubHandle)
{
	auto iter = m_hmapHandlers.find(SubHandle.strEventType);
	if (iter == m_hmapHandlers.end()) { return; }

	auto& vec = iter->second;
	if (SubHandle.uiIndex >= vec.size()) { return; }		// 인덱스가 요소의 개수보다 크거나 같으면 범위초과(사이즈가 10이면 해당 벡터의 마지막 인덱스는 9이기 때문)

	auto& slot = vec[SubHandle.uiIndex];					// 해당 인덱스를 통해서 벡터의 요소에 접근 (핸들러슬롯)

	//슬롯이 살아있지 않거나, 슬롯의 버전이 핸들의 버전이랑 일치하지않으면 접근금지.
	if (slot.bAlive == false || slot.uiVersion != SubHandle.uiVersion) { return; }

	if (m_iRunningDepth > 0) {										// 이벤트 발생 함수에서 목록순회중이라면.
		m_vecUnsubscribeQueue.push_back(SubHandle);					// 지연처리 컨테이너에 해당하는 핸들을 저장하고 리턴함.
		return;
	}

	slot.bAlive = false;									// 비활성화된 슬롯으로 전환
	slot.uiVersion++;										// 버전 갱신
	slot.func = nullptr;
}

void CEventBus::Clear_All()
{
	m_hmapHandlers.clear();
	m_vecUnsubscribeQueue.clear();
}

CEventBus* CEventBus::Create()
{
	return new CEventBus();
}

void CEventBus::Free()
{
	__super::Free();
	m_hmapHandlers.clear();
	m_vecUnsubscribeQueue.clear();
}
