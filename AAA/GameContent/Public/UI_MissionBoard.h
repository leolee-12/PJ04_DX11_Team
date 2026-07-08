#pragma once
#include "GameContent_Defines.h"
#include "UI_CoordinatorContainer.h"

NS_BEGIN(Client)

class CLIENT_DLL CUI_MissionBoard final : public CUICoordinatorContainer
{
    GENERATED_BODY(CUI_MissionBoard)

public:
    static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_UI_MissionBoard";

private:
    CUI_MissionBoard(ID3D11Device*, ID3D11DeviceContext*);
    CUI_MissionBoard(const CUI_MissionBoard&);
    virtual ~CUI_MissionBoard() = default;

public:
    virtual HRESULT Initialize(void* pArg) override;      // 자식 구성 or Deserialize 로 로드
    virtual void    Update(_float) override;              // __super::Update + 순차연출
    virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* p) override { p->strPrototypeTag = PROTOTYPE_TAG; }

protected:
    virtual HRESULT Ready_Events() override;              // StageClear_SequenceFinished 구독

private:
    enum class EBOARD { IDLE, APPEARING, SUCCESS_SEQ, DONE };
    EBOARD m_eBoard = { EBOARD::IDLE };
    _float m_fTimer = { 0.f };
    _uint  m_iSeq = { 0 };

private:
    void Start_Sequence();
    void Build_Children();

public:
    static CUI_MissionBoard* Create(ID3D11Device*, ID3D11DeviceContext*);
    virtual CGameObject* Clone(void* pArg) override;
protected:
    virtual void Free() override;
};
NS_END