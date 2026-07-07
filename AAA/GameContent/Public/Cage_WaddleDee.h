#pragma once
#include "GameContent_Defines.h"
#include "MonsterPart.h"

NS_BEGIN(Client)

// 케이지에 갇힌 웨이들디. 갇힌 동안 = 케이지 본 추종 / 구출 후 = 케이지 월드만 적용받으며 춤
class CCage_WaddleDee final : public CMonsterPart
{
    GENERATED_BODY(CCage_WaddleDee)

public:
    static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_Cage_WaddleDee";
    static constexpr const _tchar* MODEL_PROTO_TAG = L"Prototype_Component_Model_WaddleDee"; // 실제 모델 태그로

    enum class DEE_POS : _uint { FRONT, LEFT, RIGHT, END };
    enum class STATE { CAGED, RESCUED, DANCE, DONE };

    struct WADDLEDEE_DESC : public CMonsterPart::MONSTERPART_DESC
    {
        DEE_POS   ePos = { DEE_POS::FRONT };
        _float4x4 InitialLocal;   // 소켓(또는 케이지 루트) 기준 자리 오프셋
    };

private:
    CCage_WaddleDee(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CCage_WaddleDee(const CCage_WaddleDee& Prototype);
    virtual ~CCage_WaddleDee() = default;

public:
    virtual HRESULT Initialize_Prototype() override;
    virtual HRESULT Initialize(void* pArg) override;
    virtual void    Update(_float fTimeDelta) override;
    virtual HRESULT Render() override;

public:
    virtual void  Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override { pOutData->strPrototypeTag = PROTOTYPE_TAG; }

public:
    void    Rescue();
    _bool   Is_Done() const { return STATE::DONE == m_eState; }
    DEE_POS Get_Pos() const { return m_ePos; }

private:
    virtual HRESULT Ready_Components() override;
    void            Play_RandomCageWait();

private:
    DEE_POS m_ePos = { DEE_POS::FRONT };
    STATE   m_eState = { STATE::CAGED };
    _float  m_fDanceTimer = { 0.f };
    _uint m_iCagedClipIdx = { 0 };

public:
    static CCage_WaddleDee* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CGameObject* Clone(void* pArg) override;

protected:
    virtual void Free() override;
};

NS_END