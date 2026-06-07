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

    physx::PxRigidStatic* Add_StaticActor(physx::PxTriangleMesh* pMesh, _fmatrix WorldMatrix);
    void                  Remove_StaticActor(physx::PxRigidStatic* pActor);

    physx::PxController* Create_CapsuleController(const _float3& vPos, _float fRadius, _float fHeight);
    void                 Release_Controller(physx::PxController* pCtrl);

    physx::PxRigidStatic* Cook_StaticMesh(
        const _float3* pVertices, _uint iNumVertices,
        const _uint* pIndices, _uint iNumIndices,
        _fmatrix WorldMatrix);

    physx::PxPhysics* Get_Physics() const { return m_pPhysics; }
    physx::PxScene* Get_Scene()   const { return m_pScene; }

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

    physx::PxDefaultAllocator      m_Allocator;
    physx::PxDefaultErrorCallback  m_ErrorCallback;

    _bool   m_bExtensionsInited = { false };

    ID3D11Device* m_pDevice = { nullptr };
    ID3D11DeviceContext* m_pContext = { nullptr };

    PrimitiveBatch<VertexPositionColor>* m_pBatch = { nullptr };
    BasicEffect* m_pEffect = { nullptr };
    ID3D11InputLayout* m_pInputLayout = { nullptr };
    _bool                                m_bDebugDraw = { false };

public:
    static CPhysX_Manager* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual void Free() override;
};

NS_END