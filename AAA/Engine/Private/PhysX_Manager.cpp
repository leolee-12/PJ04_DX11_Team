#include "PhysX_Manager.h"

using namespace physx;


CPhysX_Manager::CPhysX_Manager(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : m_pDevice{ pDevice }
    , m_pContext{ pContext }
{
    Safe_AddRef(m_pDevice);
    Safe_AddRef(m_pContext);
}

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
    m_pCCTManager->setOverlapRecoveryModule(true);   // ★ 침투 시 표면으로 밀어냄
    m_pCCTManager->setPreciseSweeps(true);
    m_pDefaultMtrl = m_pPhysics->createMaterial(0.5f, 0.5f, 0.1f); // staticFric, dynFric, restitution

    m_pBatch = new PrimitiveBatch<VertexPositionColor>(m_pContext);
    m_pEffect = new BasicEffect(m_pDevice);
    m_pEffect->SetVertexColorEnabled(true);

    const void* pSB = nullptr; size_t nSB = 0;
    m_pEffect->GetVertexShaderBytecode(&pSB, &nSB);
    m_pDevice->CreateInputLayout(VertexPositionColor::InputElements,
        VertexPositionColor::InputElementCount, pSB, nSB, &m_pInputLayout);

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

PxController* CPhysX_Manager::Create_CapsuleController(const _float3& vFootPos, _float fRadius, _float fHeight)
{
    if (nullptr == m_pCCTManager || nullptr == m_pDefaultMtrl)
        return nullptr;

    PxCapsuleControllerDesc desc;
    desc.radius = fRadius;
    desc.height = fHeight;                 // 두 반구 사이 원통 높이(전체높이 = height + 2*radius)
    desc.material = m_pDefaultMtrl;
    desc.upDirection = PxVec3(0.f, 1.f, 0.f);
    desc.slopeLimit = cosf(PxPi / 4.f);        // 45도까지 오를 수 있음
    desc.stepOffset = 0.3f;                    // 계단 턱 높이
    desc.contactOffset = 0.05f;
    desc.climbingMode = PxCapsuleClimbingMode::eCONSTRAINED;
    // desc.position은 캡슐 '중심'. 발 위치로 받았으니 중심 = 발 + 위로(height/2 + radius)
    desc.position = PxExtendedVec3(vFootPos.x, vFootPos.y + fHeight * 0.5f + fRadius, vFootPos.z);

    PxController* pCtrl = m_pCCTManager->createController(desc);
    if (nullptr == pCtrl)
        return nullptr;

    m_Controllers.push_back(pCtrl);
    return pCtrl;
}

void CPhysX_Manager::Release_Controller(physx::PxController* pCtrl)
{
    if (nullptr == pCtrl)
        return;

    auto it = find(m_Controllers.begin(), m_Controllers.end(), pCtrl);
    if (it != m_Controllers.end())
        m_Controllers.erase(it);

    pCtrl->release();
}

void CPhysX_Manager::Render_Debug(_fmatrix ViewMatrix, _fmatrix ProjMatrix)
{
    if (!m_bDebugDraw || nullptr == m_pScene || nullptr == m_pBatch)
        return;

    const PxRenderBuffer& rb = m_pScene->getRenderBuffer();
    const PxU32 nbLines = rb.getNbLines();
    if (0 == nbLines)
        return;

    m_pEffect->SetWorld(XMMatrixIdentity());
    m_pEffect->SetView(ViewMatrix);
    m_pEffect->SetProjection(ProjMatrix);
    m_pContext->IASetInputLayout(m_pInputLayout);
    m_pEffect->Apply(m_pContext);

    const PxDebugLine* pLines = rb.getLines();

    m_pBatch->Begin();
    for (PxU32 i = 0; i < nbLines; ++i)
    {
        const PxDebugLine& L = pLines[i];
        VertexPositionColor v0(XMFLOAT3(L.pos0.x, L.pos0.y, L.pos0.z), XMFLOAT4(0.f, 1.f, 0.f, 1.f));
        VertexPositionColor v1(XMFLOAT3(L.pos1.x, L.pos1.y, L.pos1.z), XMFLOAT4(0.f, 1.f, 0.f, 1.f));
        m_pBatch->DrawLine(v0, v1);
    }
    m_pBatch->End();
}

void CPhysX_Manager::Toggle_DebugDraw()
{
    m_bDebugDraw = !m_bDebugDraw;
    if (m_pScene)
    {
        const PxReal s = m_bDebugDraw ? 1.f : 0.f;
        m_pScene->setVisualizationParameter(PxVisualizationParameter::eSCALE, s);
        m_pScene->setVisualizationParameter(PxVisualizationParameter::eCOLLISION_SHAPES, s);
    }
}

CPhysX_Manager* CPhysX_Manager::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CPhysX_Manager* pInstance = new CPhysX_Manager(pDevice, pContext);
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

    Safe_Release(m_pInputLayout);
    Safe_Delete(m_pEffect);
    Safe_Delete(m_pBatch);
    Safe_Release(m_pContext);
    Safe_Release(m_pDevice);

    for (PxController* pCtrl : m_Controllers)
        pCtrl->release();
    m_Controllers.clear();

    PX_RELEASE(m_pCCTManager);
    PX_RELEASE(m_pScene);
    PX_RELEASE(m_pDispatcher);
    PX_RELEASE(m_pDefaultMtrl);
    if (m_bExtensionsInited) 
    { 
        PxCloseExtensions(); 
        m_bExtensionsInited = false; 
    }
    PX_RELEASE(m_pPhysics);
    PX_RELEASE(m_pFoundation);
}