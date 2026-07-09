#pragma once
#include "Component.h"

namespace physx { class PxController; class PxControllerFilterCallback; }

NS_BEGIN(Engine)
class CGameObject;

class ENGINE_DLL CController final : public CComponent
{
public:
    typedef struct tagControllerDesc
    {
        _float3      vFootPos = {};
        _float       fRadius = { 0.5f };
        _float       fHeight = { 1.f };
        CGameObject* pOwner = { nullptr };
    } CONTROLLER_DESC;

private:
    CController(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CController(const CController& Prototype);
    virtual ~CController() = default;

public:
    virtual HRESULT Initialize_Prototype() override;
    virtual HRESULT Initialize(void* pArg) override;   // 내부에서 PxController 생성 + setUserData(this)

public:
    // 이동 (raw move 래핑). 충돌플래그 반환, grounded 갱신. ※ 풀 응집 단계에서 사용
    _uint   Move(_fvector vDisp, _float fMinDist, _float fTimeDelta);
    _bool   Is_Grounded() const { return m_bGrounded; }

    // 리사이즈
    _bool Set_CapsuleSize(_float fRadius, _float fHeight);

    // 위치
    void    Set_FootPosition(_fvector vPos);
    _vector Get_FootPosition() const;

    // 토글 (셰이프 플래그 + CCT-CCT 솔리드 한 번에)
    void    Set_Enabled(_bool bEnable);
    _bool   Is_Solid() const { return m_bSolid; }      // ★ 전역 필터가 읽음
    void    Set_Solid(_bool b) { m_bSolid = b; }

    void    Controller_Release();                                  // 매니저에서 제거

    CGameObject* Get_Owner() const { return m_pOwner; }
    physx::PxController* Get_Raw()  const { return m_pController; }  // 점진 마이그레이션용

private:
    physx::PxController* m_pController = { nullptr };
    CGameObject* m_pOwner = { nullptr };
    _bool               m_bSolid = { true };
    _bool               m_bGrounded = { false };

public:
    static CController* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CComponent* Clone(void* pArg) override;
    virtual void Free() override;
};

// 전역 CCT-CCT 필터 (userData=CController → Is_Solid 판정)
ENGINE_DLL physx::PxControllerFilterCallback& Get_CCTFilter();

NS_END