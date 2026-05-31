#include "PhysX_Manager.h"

using namespace physx;

#define PX_RELEASE(x) if(x){ x->release(); x = nullptr; }

HRESULT CPhysX_Manager::Initialize()
{
    m_pFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, m_Allocator, m_ErrorCallback);
    if (nullptr == m_pFoundation) return E_FAIL;

    m_pPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *m_pFoundation, PxTolerancesScale());
    if (nullptr == m_pPhysics) return E_FAIL;

    if (false == PxInitExtensions(*m_pPhysics, nullptr)) return E_FAIL;
    m_bExtensionsInited = true;

    PxSceneDesc sceneDesc(m_pPhysics->getTolerancesScale());
    sceneDesc.gravity = PxVec3(0.f, -9.81f, 0.f);
    m_pDispatcher = PxDefaultCpuDispatcherCreate(2);
    sceneDesc.cpuDispatcher = m_pDispatcher;
    sceneDesc.filterShader = PxDefaultSimulationFilterShader;

    m_pScene = m_pPhysics->createScene(sceneDesc);
    if (nullptr == m_pScene) return E_FAIL;

    m_pCCTManager = PxCreateControllerManager(*m_pScene);
    m_pDefaultMtrl = m_pPhysics->createMaterial(0.5f, 0.5f, 0.1f); // staticFric, dynFric, restitution

    return S_OK;
}

void CPhysX_Manager::Simulate(_float fTimeDelta)
{
    if (nullptr == m_pScene || fTimeDelta <= 0.f) return;

    m_pScene->simulate(fTimeDelta);
    m_pScene->fetchResults(true);
}

void CPhysX_Manager::Reset_For_SceneChange()
{
    // TODO(¡×5 ÀÌÈÄ): ÀÌ ¾À¿¡ µî·ÏµÈ static actor Á¤¸®
}

PxRigidStatic* CPhysX_Manager::Cook_StaticMesh(
    const _float3* /*pVertices*/, _uint /*iNumVertices*/,
    const _uint*   /*pIndices*/, _uint /*iNumIndices*/,
    _fmatrix /*WorldMatrix*/)
{
    // TODO(¡×5): triangle mesh cooking  ´ÙÀ½ ¸¶ÀÏ½ºÅæ
    return nullptr;
}

PxController* CPhysX_Manager::Create_CapsuleController(
    const _float3& /*vPos*/, _float /*fRadius*/, _float /*fHeight*/)
{
    // TODO(¡×6): Ä¸½¶ CCT »ý¼º  ´ÙÀ½ ¸¶ÀÏ½ºÅæ
    return nullptr;
}

CPhysX_Manager* CPhysX_Manager::Create()
{
    CPhysX_Manager* pInstance = new CPhysX_Manager();
    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Create : CPhysX_Manager");
        Safe_Release(pInstance);          
    }
    return pInstance;
}

void CPhysX_Manager::Free()
{
    PX_RELEASE(m_pCCTManager);
    PX_RELEASE(m_pScene);
    PX_RELEASE(m_pDispatcher);
    PX_RELEASE(m_pDefaultMtrl);
    if (m_bExtensionsInited) { PxCloseExtensions(); m_bExtensionsInited = false; }
    PX_RELEASE(m_pPhysics);
    PX_RELEASE(m_pFoundation);
}