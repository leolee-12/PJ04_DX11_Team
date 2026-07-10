#pragma once
#include "GameContent_Defines.h"
#include "GameObject.h"

NS_BEGIN(Client)

// 케이지 댄스 연출 이후를 전담하는 대화 연출 감독 (대화 연출이 있는 맵에만 배치)
// 흐름: 댄스끝 신호 -> 암전 -> 재배치 신호 -> 페이드인 -> 대화 -> 보스맵 전환 신호
class CDialogue_Director final : public CGameObject
{
    GENERATED_BODY(CDialogue_Director)

public:
    static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_Dialogue_Director";

private:
    CDialogue_Director(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CDialogue_Director(const CDialogue_Director& Prototype);
    virtual ~CDialogue_Director() = default;

public:
    virtual HRESULT Initialize_Prototype() override;
    virtual HRESULT Initialize(void* pArg) override;
    virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* pOut) override
    {
        pOut->strPrototypeTag = PROTOTYPE_TAG;
    }

protected:
    virtual HRESULT Ready_Events() override;

private:
    enum class ESEQ { IDLE, FADEOUT, FADEIN, TALK, DONE };
    ESEQ m_eSeq = { ESEQ::IDLE };

public:
    static CDialogue_Director* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CGameObject* Clone(void* pArg) override;

protected:
    virtual void Free() override;
};

NS_END