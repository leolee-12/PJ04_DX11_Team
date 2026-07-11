#pragma once
#include "GameContent_Defines.h"
#include "GameObject.h"

NS_BEGIN(Client)

class CSequencePlayer;

// 대화 씬 재배치 담당. 배치된 자리(자기 트랜스폼)가 대화 씬 앵커.
// 암전 완료 신호를 받아 커비/웨이들디 배치 신호를 쏘고 곧바로 페이드인 발행
class CDialogue_Arranger final : public CGameObject
{
    GENERATED_BODY(CDialogue_Arranger)

    PROPERTY(_float, m_fActorGap, L"Actor Gap", L"Dialogue")   // 커비-웨이들디 간격
    PROPERTY(_wstring, m_strDialogueId, L"Dialogue Id", L"Dialogue")

public:
    static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_Dialogue_Arranger";

private:
    CDialogue_Arranger(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CDialogue_Arranger(const CDialogue_Arranger& Prototype);
    virtual ~CDialogue_Arranger() = default;

public:
    virtual HRESULT Initialize_Prototype() override;
    virtual HRESULT Initialize(void* pArg) override;
    virtual void    Update(_float fTimeDelta) override;
    virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* pOut) override
    {
        pOut->strPrototypeTag = PROTOTYPE_TAG;
    }

protected:
    virtual HRESULT Ready_Events() override;

private:
    _bool  m_bArranged = { false };
    _bool  m_bFinishNotified = { false };
    CSequencePlayer* m_pPlayer = { nullptr };

public:
    static CDialogue_Arranger* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CGameObject* Clone(void* pArg) override;

protected:
    virtual void Free() override;
};

NS_END