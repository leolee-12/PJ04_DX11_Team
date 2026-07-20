#pragma once
#include "GameContent_Defines.h"
#include "ContainerObject.h"
#include "Inhalable.h"

NS_BEGIN(Engine)
class CCollider;
NS_END

NS_BEGIN(Client)

class CExcalibur_Body;
class CExcalibur_GetIt;


// 조우 전 바닥에 꽂힌 검: 순수 컨테이너 (몬스터 라이프사이클 없음)
class CExcalibur final : public CContainerObject, public IInhalable
{
    GENERATED_BODY(CExcalibur)

public:
    static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_Excalibur";

private:
    CExcalibur(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CExcalibur(const CExcalibur& Prototype);
    virtual ~CExcalibur() = default;

public:
    virtual HRESULT Initialize_Prototype() override;
    virtual HRESULT Initialize(void* pArg) override;
    virtual void    Update(_float fTimeDelta) override;
    virtual void    Late_Update(_float fTimeDelta) override;

    virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* p) override { p->strPrototypeTag = PROTOTYPE_TAG; }

public:
    _bool Can_BeInhaled(const INHALE_QUERY& q) const override { return false; }
    void Be_Captured(CGameObject* pInhaler) override {}
    COPY_ABILITY_TYPE Get_CopyAbility() const override { return COPY_ABILITY_TYPE::SWORD; }
    CGameObject* Get_GameObject() override { return this; }
    void On_SpatBegin() override {}
    void On_SpatEnd() override {}

public:
    void Show_GetIt(_bool bOn);

private:
    HRESULT Ready_PartObjects();
    HRESULT Ready_Collider();

private:
    CExcalibur_Body* m_pBody = { nullptr };
    CExcalibur_GetIt* m_pGetIt = { nullptr };
    CCollider* m_pHurtBox = { nullptr };

public:
    static CExcalibur* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CGameObject* Clone(void* pArg) override;

protected:
    virtual void Free() override;

    
};

NS_END