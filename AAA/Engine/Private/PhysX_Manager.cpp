#include "PhysX_Manager.h"

using namespace physx;

//#define PX_RELEASE(x) if(x){ x->release(); x = nullptr; }

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
    for (PxRigidStatic* pActor : m_StaticActors) {
        if (m_pScene) m_pScene->removeActor(*pActor);
        pActor->release();
    }
    m_StaticActors.clear();
}

PxTriangleMesh* CPhysX_Manager::Cook_TriangleMesh(const _float3* pPositions, _uint iNumVertices, const _uint* pIndices, _uint iNumIndices, _bool bFlipWinding)
{
    if (nullptr == m_pPhysics || nullptr == pPositions || nullptr == pIndices ||
        0 == iNumVertices || iNumIndices < 3)
        return nullptr;

    const _uint iNumTris = iNumIndices / 3;

    // DX(좌수) 데이터 → PhysX 와인딩 정합: 삼각형 1·2 인덱스 스왑
    vector<PxU32> Indices(iNumIndices);
    if (bFlipWinding)
        for (_uint i = 0; i < iNumTris; ++i) {
            Indices[i * 3 + 0] = pIndices[i * 3 + 0];
            Indices[i * 3 + 1] = pIndices[i * 3 + 2];
            Indices[i * 3 + 2] = pIndices[i * 3 + 1];
        }
    else
        memcpy(Indices.data(), pIndices, sizeof(PxU32) * iNumIndices);

    PxTriangleMeshDesc desc;
    desc.points.count = iNumVertices;
    desc.points.stride = sizeof(_float3);
    desc.points.data = pPositions;
    desc.triangles.count = iNumTris;
    desc.triangles.stride = 3 * sizeof(PxU32);
    desc.triangles.data = Indices.data();

    PxCookingParams params(m_pPhysics->getTolerancesScale());
    return PxCreateTriangleMesh(params, desc, m_pPhysics->getPhysicsInsertionCallback());
}

PxRigidStatic* CPhysX_Manager::Add_StaticActor(PxTriangleMesh* pMesh, _fmatrix WorldMatrix)
{
    if (nullptr == m_pPhysics || nullptr == m_pScene || nullptr == pMesh)
        return nullptr;

    // 월드행렬 분해 → (T,R)=PxTransform, S=PxMeshScale (강체 포즈엔 스케일 못 넣음)
    XMVECTOR vScale, vQuat, vTrans;
    if (!XMMatrixDecompose(&vScale, &vQuat, &vTrans, WorldMatrix))
        return nullptr;

    _float3 vS; XMStoreFloat3(&vS, vScale);
    _float4 qR; XMStoreFloat4(&qR, vQuat);
    _float3 vT; XMStoreFloat3(&vT, vTrans);

    PxTransform pose(PxVec3(vT.x, vT.y, vT.z), PxQuat(qR.x, qR.y, qR.z, qR.w));
    PxTriangleMeshGeometry geom(pMesh, PxMeshScale(PxVec3(vS.x, vS.y, vS.z), PxQuat(PxIdentity)));

    PxRigidStatic* pActor = m_pPhysics->createRigidStatic(pose);
    if (nullptr == pActor) return nullptr;

    if (nullptr == PxRigidActorExt::createExclusiveShape(*pActor, geom, *m_pDefaultMtrl)) {
        pActor->release();
        return nullptr;
    }

    m_pScene->addActor(*pActor);
    m_StaticActors.push_back(pActor);
    return pActor;
}

void CPhysX_Manager::Remove_StaticActor(PxRigidStatic* pActor)
{
    if (nullptr == pActor) return;
    auto it = find(m_StaticActors.begin(), m_StaticActors.end(), pActor);
    if (it != m_StaticActors.end()) m_StaticActors.erase(it);
    if (m_pScene) m_pScene->removeActor(*pActor);
    pActor->release();
}

PxRigidStatic* CPhysX_Manager::Cook_StaticMesh(
    const _float3* /*pVertices*/, _uint /*iNumVertices*/,
    const _uint*   /*pIndices*/, _uint /*iNumIndices*/,
    _fmatrix /*WorldMatrix*/)
{
    // TODO(§5): triangle mesh cooking  다음 마일스톤
    return nullptr;
}

PxController* CPhysX_Manager::Create_CapsuleController(
    const _float3& /*vPos*/, _float /*fRadius*/, _float /*fHeight*/)
{
    // TODO(§6): 캡슐 CCT 생성  다음 마일스톤
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
    Reset_For_SceneChange();
    PX_RELEASE(m_pCCTManager);
    PX_RELEASE(m_pScene);
    PX_RELEASE(m_pDispatcher);
    PX_RELEASE(m_pDefaultMtrl);
    if (m_bExtensionsInited) { PxCloseExtensions(); m_bExtensionsInited = false; }
    PX_RELEASE(m_pPhysics);
    PX_RELEASE(m_pFoundation);
}