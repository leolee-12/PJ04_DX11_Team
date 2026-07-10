#include "Controller.h"
#include "GameInstance.h"
#include "GameObject.h"

#pragma warning(push, 0)
#ifdef new
#undef new
#endif
#include <PhysX/PxPhysicsAPI.h>
#if defined(_DEBUG) && defined(DBG_NEW)
#define new DBG_NEW
#endif
#pragma warning(pop)

// 전역 CCT-CCT 필터 
namespace
{
    class CCCTFilter : public physx::PxControllerFilterCallback
    {
    public:
        virtual bool filter(const physx::PxController& a, const physx::PxController& b) override
        {
            auto* pA = static_cast<CController*>(a.getUserData());
            auto* pB = static_cast<CController*>(b.getUserData());
            _bool aSolid = (pA == nullptr) ? true : pA->Is_Solid();
            _bool bSolid = (pB == nullptr) ? true : pB->Is_Solid();
            return aSolid && bSolid;   // 하나라도 비-solid면 통과
        }
    };
}

ENGINE_DLL physx::PxControllerFilterCallback& Engine::Get_CCTFilter()
{
    static CCCTFilter s_Filter;
    return s_Filter;
}

CController::CController(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CComponent{ pDevice, pContext } {
}

CController::CController(const CController& Prototype)
    : CComponent(Prototype) {
}

HRESULT CController::Initialize_Prototype() { return S_OK; }

HRESULT CController::Initialize(void* pArg)
{
    auto pDesc = static_cast<CONTROLLER_DESC*>(pArg);
    if (nullptr == pDesc) return E_FAIL;

    m_pOwner = pDesc->pOwner;

    m_pController = m_pGameInstance_Proxy->Create_CapsuleController(
        pDesc->vFootPos, pDesc->fRadius, pDesc->fHeight);
    if (nullptr == m_pController) return E_FAIL;

    m_pController->setUserData(this);     // ★ 필터가 이 포인터로 Is_Solid 읽음
    return S_OK;
}

_uint CController::Move(_fvector vDisp, _float fMinDist, _float fTimeDelta)
{
    if (nullptr == m_pController) return 0;

    _float3 vD; XMStoreFloat3(&vD, vDisp);

    physx::PxControllerFilters filters;
    filters.mFilterFlags = physx::PxQueryFlag::eSTATIC | physx::PxQueryFlag::eDYNAMIC;
    filters.mCCTFilterCallback = &Get_CCTFilter();

    physx::PxControllerCollisionFlags flags =
        m_pController->move(physx::PxVec3(vD.x, vD.y, vD.z), fMinDist, fTimeDelta, filters);

    m_bGrounded = flags.isSet(physx::PxControllerCollisionFlag::eCOLLISION_DOWN);
    return (_uint)(physx::PxU32)flags;
}

_bool CController::Set_CapsuleSize(_float fRadius, _float fHeight)
{
    if (m_pController ==  nullptr)
        return false;

    auto* pCapsule = static_cast<physx::PxCapsuleController*>(m_pController);

    // 크기 변경 전 발 위치 보존
    const physx::PxExtendedVec3 vFootPos = pCapsule->getFootPosition();

    if (!pCapsule->setRadius(fRadius))
        return false;

    if (!pCapsule->setHeight(fHeight))
        return false;

    pCapsule->setFootPosition(vFootPos);

    return true;
}

void CController::Set_FootPosition(_fvector vPos)
{
    if (nullptr == m_pController) return;
    _float3 p; XMStoreFloat3(&p, vPos);
    m_pController->setFootPosition(physx::PxExtendedVec3(p.x, p.y, p.z));
}

_vector CController::Get_FootPosition() const
{
    if (nullptr == m_pController) return XMVectorZero();
    const physx::PxExtendedVec3& f = m_pController->getFootPosition();
    return XMVectorSet((_float)f.x, (_float)f.y, (_float)f.z, 1.f);
}

void CController::Set_Enabled(_bool bEnable)
{
    m_bSolid = bEnable;                              // CCT-CCT
    if (nullptr == m_pController) return;

    physx::PxRigidDynamic* pActor = m_pController->getActor();
    if (nullptr == pActor) return;

    const physx::PxU32 n = pActor->getNbShapes();
    if (0 == n) return;

    std::vector<physx::PxShape*> shapes(n);
    pActor->getShapes(shapes.data(), n);
    for (physx::PxShape* s : shapes)
        if (s)
        {
            s->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, bEnable);
            s->setFlag(physx::PxShapeFlag::eSCENE_QUERY_SHAPE, bEnable);
        }
}

void CController::Controller_Release()
{
    if (nullptr == m_pController) return;
    m_pGameInstance_Proxy->Release_Controller(m_pController);
    m_pController = nullptr;
}

CController* CController::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CController* p = new CController(pDevice, pContext);
    if (FAILED(p->Initialize_Prototype())) { MSG_BOX("Failed to Create: CController"); Safe_Release(p); }
    return p;
}

CComponent* CController::Clone(void* pArg)
{
    CController* p = new CController(*this);
    if (FAILED(p->Initialize(pArg))) { MSG_BOX("Failed to Clone: CController"); Safe_Release(p); }
    return p;
}

void CController::Free()
{
    Controller_Release();
    __super::Free();
}