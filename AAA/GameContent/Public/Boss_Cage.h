#pragma once
#include "GameContent_Defines.h"
#include "ContainerObject.h"

NS_BEGIN(Engine)
class CCollider;
NS_END

NS_BEGIN(Client)

class CBoss_Cage_Body;
class CCage_WaddleDee;

// 골드 케이지 컨테이너: 트랜스폼/부착 로직 소유. 메쉬는 CBoss_Cage_Body 파트가 담당
class CBoss_Cage final : public CContainerObject
{
    GENERATED_BODY(CBoss_Cage)

public:
    static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_Boss_Cage";
    static constexpr const _float  CLEAR_ANIM_SPEED = 2.f;

    enum class CAGE_STATE
    {
        HANGING,      // 고릴라 목에 매달림 (본 부착, 케이지 애님 정지)
        DESCEND,      // 보스 사망 후 공중 스폰 -> 하강 중
        FLOAT_IDLE,   // 목표 높이 도달, 공중에서 대기
        BREAKING,     // 커비 충돌, 부서지는 연출 재생 중
        BROKEN,       // 부서짐 완료, 웨이들디 춤 대기
    };

private:
    CBoss_Cage(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CBoss_Cage(const CBoss_Cage& Prototype);
    virtual ~CBoss_Cage() = default;

public:
    virtual HRESULT Initialize_Prototype() override;
    virtual HRESULT Initialize(void* pArg) override;
    virtual void    Priority_Update(_float fTimeDelta) override;
    virtual void    Update(_float fTimeDelta) override;
    virtual void    Late_Update(_float fTimeDelta) override;

    virtual void    Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override
    {
        pOutData->strPrototypeTag = PROTOTYPE_TAG;
    }

public:
    void  Attach_To_Bone(const _float4x4* pBoneMatrix, const _float4x4* pOwnerWorld, _fmatrix OffsetMatrix = XMMatrixIdentity());
    void  Detach();     // 분리 후엔 마지막 합성 월드가 트랜스폼에 남음 (낙하 연출 등에 사용)
    _bool Is_Attached() const { return m_pAttachBone != nullptr; }

    void  Rescue_WaddleDees();
    _bool Is_RescueDone() const;

public:
    void       Start_Descend(_fvector vLookTarget, _float fSpawnHeightOffset = 10.f);
    void       Start_Descend_InPlace(_float fSpawnHeightOffset = 10.f);
    void       Break();
    CAGE_STATE Get_CageState() const { return m_eState; }

    virtual void On_Deserialized() override;

private:
    virtual HRESULT Ready_Events() override;
    HRESULT Ready_PartObjects();
    HRESULT Ready_BreakTrigger();
    void Fire_CutsceneCamera(const _tchar* szTrack, CAnimator* pProgress);
    CAnimator* Get_DeeAnimator() const;

private:
    const _float4x4* m_pAttachBone = { nullptr };
    const _float4x4* m_pAttachOwnerWorld = { nullptr };
    _float4x4        m_AttachOffset = {};

    CCage_WaddleDee* m_WaddleDees[3] = {};
    CBoss_Cage_Body* m_pBody = { nullptr };
    CCollider* m_pBreakTrigger = { nullptr };

    CAGE_STATE m_eState = { CAGE_STATE::HANGING };
    _bool      m_bAnimPrimed = { false };           // 본 1프레임 갱신 후 Pause 완료 여부
    _float     m_fFloatHeight = { 0.f };
    _float     m_fDescendSpeed = { 3.f };

    _bool      m_bHeadTurnFired = { false };


public:
    static CBoss_Cage* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CGameObject* Clone(void* pArg) override;

protected:
    virtual void Free() override;
};

NS_END