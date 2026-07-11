#pragma once
#include "CutsceneActor.h"

NS_BEGIN(Client)

// 대화 연출용 왜들디. 로직 없는 순수 배우 - 재생 지시는 SequencePlayer가 내린다
class CDialogueDee final : public CCutsceneActor
{
    GENERATED_BODY(CDialogueDee)

public:
    static constexpr const wchar_t* PROTOTYPE_TAG = L"Proto_DialogueDee";

private:
    CDialogueDee(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CDialogueDee(const CDialogueDee& Prototype);
    virtual ~CDialogueDee() = default;

public:
    virtual HRESULT Initialize(void* pArg) override;
    virtual HRESULT Render() override;
    virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* p) override { p->strPrototypeTag = PROTOTYPE_TAG; }

protected:
    virtual HRESULT Ready_Components() override;

public:
    static CDialogueDee* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CGameObject* Clone(void* pArg) override;
    virtual void Free() override;
};

NS_END