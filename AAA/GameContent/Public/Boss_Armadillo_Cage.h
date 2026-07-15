#pragma once
#include "MonsterPart.h"

NS_BEGIN(Engine)
class CAnimator;
NS_END

NS_BEGIN(Client)

// 잡기 상태 때 띄우는 케이지. 본 부착이 아니라 아르마딜로 컨테이너 피벗에 그대로 붙고,
// 애니메이션은 Body 애니메이터에 매 프레임 동기(클립명 동일 전제)한다.
class CBoss_Armadillo_Cage final : public CMonsterPart
{
    GENERATED_BODY(CBoss_Armadillo_Cage)

private:
    CBoss_Armadillo_Cage(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CBoss_Armadillo_Cage(const CBoss_Armadillo_Cage& Prototype);
    virtual ~CBoss_Armadillo_Cage() = default;

public:
    struct BOSS_ARMADILLO_CAGE_DESC : public CMonsterPart::MONSTERPART_DESC {};

    static constexpr const wchar_t* PROTOTYPE_TAG = L"Proto_Boss_Armadillo_Cage";
    static constexpr const wchar_t* MODEL_PROTO_TAG = L"Prototype_Component_Model_Boss_Armadillo_Cage";
    static constexpr const wchar_t* PART_TAG = L"Cage";

public:
    virtual HRESULT Initialize(void* pArg) override;
    virtual void    Update(_float fTimeDelta) override;   // 자체 시간진행 안 함(동기 전용)
    virtual void    Copy_PrototypeName(ENGINE_OBJECT_DATA* p) override { p->strPrototypeTag = PROTOTYPE_TAG; }

private:
    HRESULT Ready_Components();

public:
    static CBoss_Armadillo_Cage* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CBoss_Armadillo_Cage* Clone(void* pArg) override;
    virtual void Free() override;
};

NS_END