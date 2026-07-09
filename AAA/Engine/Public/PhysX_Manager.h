#pragma once
#include "Base.h"
#include "Engine_Defines.h"

#pragma warning(push, 0)
#ifdef new
#undef new
#endif

#include <PhysX/PxPhysicsAPI.h>

#if defined(_DEBUG) && defined(DBG_NEW)
#define new DBG_NEW            
#endif
#pragma warning(pop)

NS_BEGIN(Engine)

class ENGINE_DLL CPhysX_Manager final : public CBase
{
private:
    CPhysX_Manager(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual ~CPhysX_Manager() = default;

private:
    HRESULT Initialize();

public:
    void    Simulate(_float fTimeDelta);          // 매 프레임 simulate/fetchResults
    void    Reset_For_SceneChange();              // 레벨 바뀌면 static actor 정리

    physx::PxTriangleMesh* Cook_TriangleMesh(
        const _float3* pPositions, _uint iNumVertices,
        const _uint* pIndices, _uint iNumIndices,
        _bool bFlipWinding = true);

    physx::PxRigidStatic* Create_StaticActor(physx::PxTriangleMesh* pMesh, _fmatrix WorldMatrix);
    physx::PxRigidStatic* Create_StaticBox(const _float3& vLocalCenter, const _float3& vLocalHalfExtents, _fmatrix WorldMatrix);
    physx::PxRigidStatic* Cook_StaticMesh(const _float3* pVertices, _uint iNumVertices, const _uint* pIndices, _uint iNumIndices, _fmatrix WorldMatrix);
    void                  Remove_StaticActor(physx::PxRigidStatic* pActor);
    HRESULT               Refresh_StaticActorPose(physx::PxRigidStatic* pActor, _fmatrix WorldMatrix);
    HRESULT               Refresh_StaticBoxPose(physx::PxRigidStatic* pActor, const _float3& vLocalCenter, _fmatrix WorldMatrix);

    physx::PxController* Create_CapsuleController(const _float3& vPos, _float fRadius, _float fHeight);
    void                 Release_Controller(physx::PxController* pCtrl);

    physx::PxPhysics* Get_Physics() const { return m_pPhysics; }
    physx::PxScene* Get_Scene()   const { return m_pScene; }

public:
    // 씬 오버랩 쿼리. 겹친 액터가 하나라도 있으면 true
    _bool Overlap_Box(const _float3& vCenter, const _float3& vHalfExtents, const _float4& qRot,
        vector<physx::PxActor*>* pOutActors = nullptr, _bool bStatic = true, _bool bDynamic = true);
    _bool Overlap_Sphere(const _float3& vCenter, _float fRadius,
        vector<physx::PxActor*>* pOutActors = nullptr, _bool bStatic = true, _bool bDynamic = true);
    _bool Overlap_Capsule(const _float3& vCenter, _float fRadius, _float fHalfHeight,
        vector<physx::PxActor*>* pOutActors = nullptr, _bool bStatic = true, _bool bDynamic = true);

    _bool Sweep_Box(const _float3& vCenter, const _float3& vHalfExtents, const _float4& qRot,
        const _float3& vDir, _float fMaxDist,
        _float3* pOutNormal = nullptr, _float* pOutDist = nullptr,
        _bool bStatic = true, _bool bDynamic = false);

    _bool Sweep_Sphere(const _float3& vCenter, _float fRadius,
        const _float3& vDir, _float fMaxDist,
        _float3* pOutNormal = nullptr, _float* pOutDist = nullptr,
        _bool bStatic = true, _bool bDynamic = false);

    _bool Sweep_Capsule(const _float3& vCenter, _float fRadius, _float fHalfHeight,
        const _float3& vDir, _float fMaxDist,
        _float3* pOutNormal = nullptr, _float* pOutDist = nullptr,
        _bool bStatic = true, _bool bDynamic = false);

public:
    physx::PxRigidDynamic* Create_DynamicBox(const _float3& vPos, const _float4& qRot, const _float3& vHalfExtents, _float fDensity = 10.f);
    physx::PxRigidDynamic* Create_DynamicSphere(const _float3& vPos, _float fRadius, _float fDensity = 10.f);
    physx::PxRigidDynamic* Create_DynamicCapsule(const _float3& vPos, const _float4& qRot, _float fRadius, _float fHalfHeight, _float fDensity = 10.f);
    physx::PxConvexMesh*   Cook_ConvexMesh(const _float3* pPositions, _uint iNumVertices);
    physx::PxRigidDynamic* Create_DynamicConvex(physx::PxConvexMesh* pMesh, _fmatrix WorldMatrix, _float fDensity = 10.f);
    void                   Remove_DynamicActor(physx::PxRigidDynamic* pActor);

public:
    void  Render_Debug(_fmatrix ViewMatrix, _fmatrix ProjMatrix);   
    void  Toggle_DebugDraw();                                       
    _bool Is_DebugDraw() const { return m_bDebugDraw; }

private:
    physx::PxFoundation* m_pFoundation = { nullptr };
    physx::PxPhysics* m_pPhysics = { nullptr };
    physx::PxScene* m_pScene = { nullptr };
    physx::PxDefaultCpuDispatcher* m_pDispatcher = { nullptr };
    physx::PxControllerManager* m_pCCTManager = { nullptr };
    physx::PxMaterial* m_pDefaultMtrl = { nullptr };

    vector<physx::PxRigidStatic*> m_StaticActors;
    vector<physx::PxController*> m_Controllers;
    vector<physx::PxRigidDynamic*> m_Dynamics;

    physx::PxDefaultAllocator      m_Allocator;
    physx::PxDefaultErrorCallback  m_ErrorCallback;

    _bool   m_bExtensionsInited = { false };

    ID3D11Device* m_pDevice = { nullptr };
    ID3D11DeviceContext* m_pContext = { nullptr };

    PrimitiveBatch<VertexPositionColor>* m_pBatch = { nullptr };
    BasicEffect* m_pEffect = { nullptr };
    ID3D11InputLayout* m_pInputLayout = { nullptr };
    _bool                                m_bDebugDraw = { false };

private:
    physx::PxRigidDynamic* Finalize_Dynamic(const physx::PxTransform& pose,
        const physx::PxGeometry& geom, _float fDensity,
        const physx::PxTransform& localPose =
        physx::PxTransform(physx::PxIdentity));

    _bool Overlap_Internal(const physx::PxGeometry& Geom, const physx::PxTransform& Pose,
        vector<physx::PxActor*>* pOutActors, _bool bStatic, _bool bDynamic);

    _bool Sweep_Internal(const physx::PxGeometry& Geom, const physx::PxTransform& Pose,
        const _float3& vDir, _float fMaxDist,
        _float3* pOutNormal, _float* pOutDist, _bool bStatic, _bool bDynamic);


public:
    static CPhysX_Manager* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual void Free() override;
};

NS_END