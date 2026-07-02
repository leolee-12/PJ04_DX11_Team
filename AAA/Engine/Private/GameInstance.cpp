#include "GameInstance.h"
#include "Graphic_Device.h"
#include "Timer_Manager.h"
#include "Level_Manager.h"
#include "Prototype_Manager.h"
#include "Object_Manager.h"
#include "Renderer.h"
#include "Camera_Manager.h"
#include "Input_Device.h"
#include "Light_Manager.h"
#include "Picking_Utils.h"
#include "Font_Manager.h"
#include "EventBus.h"
#include "Collision_Manager.h"
#include "Sound_Manager.h"
#include "Target_Manager.h"
#include "Shadow_Dir.h"
#include "Effect_Manager.h"
#include "PhysX_Manager.h"
#include "Environment_Manager.h"
#include "ShaderGlobal_Manager.h"
#include "Culling_Manager.h"
#include "Texture_Hub.h"
#include "Effect_Allocator.h"

CGameInstance* CGameInstance::m_pInstance = { nullptr };
CGameInstance_Proxy* CGameInstance::m_pGameInstance_Proxy = { nullptr };

using namespace physx;

CGameInstance::CGameInstance()
{
}

#pragma region ENGINE

HRESULT CGameInstance::Initialize_Engine(const ENGINE_DESC& EngineDesc, ID3D11Device** ppDevice, ID3D11DeviceContext** ppContext)
{
    GetInstance();

    m_pGameInstance_Proxy = CGameInstance_Proxy::Create(m_pInstance);
    if (nullptr == m_pGameInstance_Proxy)
        return E_FAIL;

    m_pInstance->m_pGraphic_Device = CGraphic_Device::Create(EngineDesc.hWnd, EngineDesc.eWinMode, EngineDesc.iViewportWidth, EngineDesc.iViewportHeight, ppDevice, ppContext);
    if (nullptr == m_pInstance->m_pGraphic_Device)
        return E_FAIL;    

    m_pInstance->m_pCamera_Manager = CCamera_Manager::Create(static_cast<_float>(EngineDesc.iViewportWidth), static_cast<_float>(EngineDesc.iViewportHeight));
    if (nullptr == m_pInstance->m_pCamera_Manager)
        return E_FAIL;

    m_pInstance->m_pCulling_Manager = CCulling_Manager::Create();   // WY
    if (nullptr == m_pInstance->m_pCulling_Manager)
        return E_FAIL;

    m_pInstance->m_pTimer_Manager = CTimer_Manager::Create();
    if (nullptr == m_pInstance->m_pTimer_Manager)
        return E_FAIL;

    m_pInstance->m_pLevel_Manager = CLevel_Manager::Create();
    if (nullptr == m_pInstance->m_pLevel_Manager)
        return E_FAIL;

    m_pInstance->m_pPrototype_Manager = CPrototype_Manager::Create(EngineDesc.iNumLevels);
    if (nullptr == m_pInstance->m_pPrototype_Manager)
        return E_FAIL;

    m_pInstance->m_pTexture_Hub = CTexture_Hub::Create(*ppDevice, *ppContext);    // WY
    if (nullptr == m_pInstance->m_pTexture_Hub)
        return E_FAIL;

    m_pInstance->m_pObject_Manager = CObject_Manager::Create(EngineDesc.iNumLevels);
    if (nullptr == m_pInstance->m_pObject_Manager)
        return E_FAIL;

    m_pInstance->m_pTarget_Manager = CTarget_Manager::Create(*ppDevice, *ppContext);
    if (nullptr == m_pInstance->m_pTarget_Manager)
        return E_FAIL;

    m_pInstance->m_pRenderer = CRenderer::Create(*ppDevice, *ppContext);
    if (nullptr == m_pInstance->m_pRenderer)
        return E_FAIL;

    m_pInstance->m_pInput_Device = CInput_Device::Create(EngineDesc.hInstance, EngineDesc.hWnd);
    if (nullptr == m_pInstance->m_pInput_Device)
        return E_FAIL;

    m_pInstance->m_pLight_Manager = CLight_Manager::Create(*ppDevice, *ppContext);
    if (nullptr == m_pInstance->m_pLight_Manager)
        return E_FAIL;

    m_pInstance->m_pFont_Manager = CFont_Manager::Create(*ppDevice, *ppContext);
    if (nullptr == m_pInstance->m_pFont_Manager)
        return E_FAIL;

    m_pInstance->m_pEventBus = CEventBus::Create();
    if (nullptr == m_pInstance->m_pEventBus)
        return E_FAIL;

    m_pInstance->m_pCollision_Manager = CCollision_Manager::Create();
    if (nullptr == m_pInstance->m_pCollision_Manager)
        return E_FAIL;

    m_pInstance->m_pSound_Manager = CSound_Manager::Create();
    if (nullptr == m_pInstance->m_pSound_Manager)
        return E_FAIL;

    m_pInstance->m_pShadow_Dir = CShadow_Dir::Create();
    if (nullptr == m_pInstance->m_pShadow_Dir)
        return E_FAIL;

    m_pInstance->m_pShadow_Blob = CShadow_Dir::Create();
    if (nullptr == m_pInstance->m_pShadow_Blob)
        return E_FAIL;

    m_pInstance->m_pEffect_Manager = CEffect_Manager::Create(*ppDevice, *ppContext);
    if (nullptr == m_pInstance->m_pEffect_Manager)
        return E_FAIL;

    m_pInstance->m_pPhysX_Manager = CPhysX_Manager::Create(*ppDevice, *ppContext);
    if (nullptr == m_pInstance->m_pPhysX_Manager)
        return E_FAIL;

    m_pInstance->m_pEnvironment_Manager = CEnvironment_Manager::Create(*ppDevice, *ppContext);
    if (nullptr == m_pInstance->m_pEnvironment_Manager)
        return E_FAIL;

    m_pInstance->m_pShaderGlobal_Manager = CShaderGlobal_Manager::Create();
    if (nullptr == m_pInstance->m_pShaderGlobal_Manager)
        return E_FAIL;

    m_pInstance->m_RandomGenerator.seed(random_device{}());

    return S_OK;
}

void CGameInstance::Update_Engine(_float fTimeDelta, _float fRawTimeDelta)
{
    ++m_iFrameIndex;

    m_pLevel_Manager->Apply_ReservedLevel();

	m_pInput_Device->Update();

    // 전역상수값들은 현재 연출로만쓰고있어서 언스케일 시간
    // 물결이나 실제 타임스케일 영향받아야하면 그때 추가 ㅇㅇ
    m_pShaderGlobal_Manager->Tick(fRawTimeDelta);

    m_pObject_Manager->Priority_Update(fTimeDelta);
    m_pObject_Manager->Update(fTimeDelta);
	m_pCamera_Manager->Update();
    m_pCulling_Manager->Update();

    m_pSound_Manager->Set_ListenerPos(*m_pCamera_Manager->Get_CamPosition());
    m_pSound_Manager->Update();
    m_pObject_Manager->Late_Update(fTimeDelta);

    m_pPhysX_Manager->Simulate(fTimeDelta);

    m_pCollision_Manager->Check_Collisions(fTimeDelta);

	m_pObject_Manager->Flush_DeadObjects();

    m_pLevel_Manager->Update(fTimeDelta);
}

HRESULT CGameInstance::Begin_Draw()
{
    _float4     vColor = _float4(0.f, 1.f, 0.f, 1.f);

    if (FAILED(m_pGraphic_Device->Bind_BackBuffer()))
		return E_FAIL;

    if (FAILED(m_pGraphic_Device->Clear_BackBuffer_View(&vColor)))
        return E_FAIL;

    if (FAILED(m_pGraphic_Device->Clear_DepthStencil_View()))
        return E_FAIL;

    return S_OK;
}

HRESULT CGameInstance::Draw()
{
    if (FAILED(m_pRenderer->Draw()))
        return E_FAIL;

    if (FAILED(m_pLevel_Manager->Render()))
        return E_FAIL;

    return S_OK;
}

HRESULT CGameInstance::End_Draw()
{
    return m_pGraphic_Device->Present();    
}

void CGameInstance::Clear_Resources(_int iLevelIndex)
{
    if (-1 == iLevelIndex)
        return;

    m_pEffect_Manager->Clear_Level(iLevelIndex);
    m_pObject_Manager->Clear(iLevelIndex);
    m_pPrototype_Manager->Clear(iLevelIndex);
}
_float CGameInstance::RandomFloat(_float fMin, _float fMax) const
{
	uniform_real_distribution<_float> dist(fMin, fMax);
    return dist(m_RandomGenerator);
}
_int CGameInstance::RandomInt(_int iMin, _int iMax) const
{
	uniform_int_distribution<_int> dist(iMin, iMax);
    return dist(m_RandomGenerator);
}
_uint64 CGameInstance::Get_FrameIndex()
{
    return m_iFrameIndex;
}
#pragma endregion

#pragma region DEVICE
HRESULT CGameInstance::Bind_BackBuffer()
{
    return m_pGraphic_Device->Bind_BackBuffer();
}

HRESULT CGameInstance::OnResize(_uint iWidth, _uint iHeight)
{
	return m_pGraphic_Device->OnResize(iWidth, iHeight);
}
_float CGameInstance::Get_WindowWidth()
{
    return m_pGraphic_Device->Get_CurWidth();
}
_float CGameInstance::Get_WindowHeight()
{
    return m_pGraphic_Device->Get_CurHeight();
}
#pragma endregion

#pragma region TIMER_MANAGER

_float CGameInstance::Get_TimeDelta(const _wstring& strTimerTag)
{
    return m_pTimer_Manager->Get_TimeDelta(strTimerTag);
}

HRESULT CGameInstance::Add_Timer(const _wstring& strTimerTag)
{
    return m_pTimer_Manager->Add_Timer(strTimerTag);
}

void CGameInstance::Compute_Timer(const _wstring& strTimerTag)
{
    m_pTimer_Manager->Compute_Timer(strTimerTag);
}
#pragma endregion

#pragma region LEVEL_MANAGER
HRESULT CGameInstance::Change_Level(_int iNewLevelIndex, CLevel* pNewLevel)
{
    return m_pLevel_Manager->Change_Level(iNewLevelIndex, pNewLevel);
}
#pragma endregion

#pragma region PROTO_MANAGER
HRESULT CGameInstance::Add_Proto(_uint iLevelIndex, const _wstring& strPrototypeTag, CBase* pPrototype)
{
	return m_pPrototype_Manager->Add_Prototype(iLevelIndex, strPrototypeTag, pPrototype);
}

CBase* CGameInstance::Clone_Prototype(PROTOTYPE eType, _uint iLevelIndex, const _wstring& strPrototypeTag, void* pArg)
{
	return m_pPrototype_Manager->Clone_Prototype(eType, iLevelIndex, strPrototypeTag, pArg);
}

_bool CGameInstance::Has_Prototype(_uint iLevelIndex, const _wstring& strPrototypeTag)
{
    return m_pPrototype_Manager->Has_Prototype(iLevelIndex, strPrototypeTag);
}
#pragma endregion

#pragma region OBJECT_MANAGER
HRESULT CGameInstance::Add_GameObject(_uint iPrototypeLevelIndex, const _wstring& strPrototypeTag, _uint iLayerLevelIndex, const _wstring& strLayerTag, const _wstring& strObjectTag, void* pArg)
{
    return m_pObject_Manager->Add_GameObject(iPrototypeLevelIndex, strPrototypeTag, iLayerLevelIndex, strLayerTag, strObjectTag, pArg);
}

HRESULT CGameInstance::Add_GameObject_Return(CGameObject** ppOut, _uint iPrototypeLevelIndex, const _wstring& strPrototypeTag, _uint iLayerLevelIndex, const _wstring& strLayerTag, const _wstring& strObjectTag, void* pArg)
{
    return m_pObject_Manager->Add_GameObject_Return(ppOut, iPrototypeLevelIndex, strPrototypeTag, iLayerLevelIndex, strLayerTag, strObjectTag, pArg);
}
void CGameInstance::Clear_Objects(_int iLevelIndex)
{
    m_pObject_Manager->Clear(iLevelIndex);
}
void CGameInstance::Destroy_GameObject(CGameObject* pGameObject)
{
	m_pObject_Manager->Request_Destroy(pGameObject);
}
CGameObject* CGameInstance::Find_GameObject(_uint iLevelIndex, const _wstring& strLayerTag, const _wstring& strObjectTag)
{
    return m_pObject_Manager->Find_GameObject(iLevelIndex, strLayerTag, strObjectTag);
}
#pragma endregion

#pragma region RENDERER
void CGameInstance::Add_RenderGroup(RENDERID eGroupID, CGameObject* pGameObject)
{
    m_pRenderer->Add_RenderGroup(eGroupID, pGameObject);
}
void CGameInstance::Add_RenderGroup_UI(RENDERUIID eGroupID, CUIObject* pUIObject)
{
    m_pRenderer->Add_RenderGroup_UI(eGroupID, pUIObject);
}
#pragma endregion

#pragma region INPUT_DEVICE
_byte CGameInstance::Get_DIKeyState(_ubyte byKeyID)
{
    return m_pInput_Device->Get_DIKeyState(byKeyID);
}

_byte CGameInstance::Get_DIMouseState(DIMB eMouse)
{
    return m_pInput_Device->Get_DIMouseState(eMouse);
}

_long CGameInstance::Get_DIMouseMove(DIMM eMouseState)
{
    return m_pInput_Device->Get_DIMouseMove(eMouseState);
}

void	CGameInstance::Disable_InputDeveice()
{
    m_pInput_Device->Disable_InputDeveice();
}

void	CGameInstance::Enable_InputDeveice()
{
	m_pInput_Device->Enable_InputDeveice();
}

_bool CGameInstance::Key_Down(_ubyte byKeyID)
{
    return m_pInput_Device->Key_Down(byKeyID);
}
_bool CGameInstance::Key_Up(_ubyte byKeyID)
{
    return m_pInput_Device->Key_Up(byKeyID);
}
_bool CGameInstance::Key_Pressing(_ubyte byKeyID)
{
    return m_pInput_Device->Key_Pressing(byKeyID);
}
_bool CGameInstance::Mouse_Down(DIMB eMouse)
{
    return m_pInput_Device->Mouse_Down(eMouse);
}
_bool CGameInstance::Mouse_Up(DIMB eMouse)
{
    return m_pInput_Device->Mouse_Up(eMouse);
}
_bool CGameInstance::Mouse_Pressing(DIMB eMouse)
{
    return m_pInput_Device->Mouse_Pressing(eMouse);
}
POINT CGameInstance::Get_MousePos()
{
    return m_pInput_Device->Get_MousePos();
}
#pragma endregion

#pragma region CAMERA_MANAGER
const _float4x4* CGameInstance::Get_Matrix(D3DTS eState, PROJ_TYPE eType) const
{
    return m_pCamera_Manager->Get_Matrix(eState, eType);
}
const _float4x4* CGameInstance::Get_InverseMatrix_Prespec(D3DTS eState) const
{
    return m_pCamera_Manager->Get_InverseMatrix_Prespec(eState);
}
const _float4* CGameInstance::Get_CamPosition() const
{
    return m_pCamera_Manager->Get_CamPosition();
}
void CGameInstance::Set_Transform(D3DTS eState, PROJ_TYPE eType, _fmatrix StateMatrix)
{
    return m_pCamera_Manager->Set_Transform(eState, eType, StateMatrix);
}
void CGameInstance::Set_Transform(D3DTS eState, PROJ_TYPE eType, const _float4x4& StateMatrix)
{
    return m_pCamera_Manager->Set_Transform(eState, eType, StateMatrix);
}
#pragma endregion

#pragma region LIGHT_MANAGER

const LIGHT_DESC* CGameInstance::Get_LightDesc(_uint iIndex)
{
    return m_pLight_Manager->Get_LightDesc(iIndex);
}

HRESULT CGameInstance::Add_Light(const LIGHT_DESC& LightDesc)
{
    return m_pLight_Manager->Add_Light(LightDesc);
}

void CGameInstance::Compute_PickingRay(_float fNdcX, _float fNdcY, _vector* pOutOrigin, _vector* pOutDir, PROJ_TYPE eType)
{
    const _float4x4* pView = Get_Matrix(D3DTS::VIEW, eType);
    const _float4x4* pProj = Get_Matrix(D3DTS::PROJ, eType);
    if (!pView || !pProj) return;

    _matrix matInvProj = XMMatrixInverse(nullptr, XMLoadFloat4x4(pProj));
    _matrix matInvView = XMMatrixInverse(nullptr, XMLoadFloat4x4(pView));

    _vector rayView = XMVector4Transform(
        XMVectorSet(fNdcX, fNdcY, 0.f, 1.f), matInvProj);
    rayView = XMVectorSetW(rayView, 0.f);

    *pOutDir = XMVector3Normalize(XMVector3TransformNormal(rayView, matInvView));
    *pOutOrigin = matInvView.r[3];
}

_bool CGameInstance::Pick_RayToPlane(_vector vOrigin, _vector vDir, _float3* pOutHit, _float fPlaneY)
{
    const _float4x4* pProj = Get_Matrix(D3DTS::PROJ, PROJ_TYPE::PERSPEC);
    _float fFar = pProj->m[3][2] / (1.f - pProj->m[2][2]);


    return CPicking_Utils::Pick_RayToPlane(vOrigin, vDir, pOutHit, fPlaneY, fFar);
}

_bool CGameInstance::Pick_RayToMesh(_vector vOrigin, _vector vDir, const _float3* pVertices, _uint iVertexCount, const _uint* pIndices, _uint iIndexCount, _float3* pOutHit, _float* pOutDist)
{
    return CPicking_Utils::RayToMesh(vOrigin, vDir, pVertices, iVertexCount, pIndices, iIndexCount, pOutHit, pOutDist);
}

#pragma endregion

#pragma region FONT_MANAGER
HRESULT CGameInstance::Add_Font(const _wstring& strFontTag, const _tchar* pFontFilePath)
{
    return m_pFont_Manager->Add_Font(strFontTag, pFontFilePath);
}

HRESULT CGameInstance::Draw_Text(const _wstring& strFontTag, const _tchar* pText, const _float2& vPosition, _fvector vColor, _float fRotation, const _float2& vScale, TEXT_ALIGN eAlign)
{
    return m_pFont_Manager->Draw(strFontTag, pText, vPosition, vColor, fRotation,  vScale, eAlign);
}

_float2 CGameInstance::Measure_Text(const _wstring& strFontTag, const _tchar* pText)
{
    return m_pFont_Manager->Measure(strFontTag, pText);
}

#pragma endregion

#pragma region EVENTBUS
SUBHANDLE CGameInstance::Subscribe(const wstring& EventTag, function<void(void*)> Handler)
{
    return m_pEventBus->Subscribe(EventTag, Handler);
}
void CGameInstance::UnSubscribe(const SUBHANDLE& Handle)
{
    m_pEventBus->Unsubscribe(Handle);
}
void CGameInstance::Publish(const wstring& EventTag, void* pData)
{
    m_pEventBus->Publish(EventTag, pData);
}
void CGameInstance::Clear_EventBus()
{
    m_pEventBus->Clear_All();
}
#pragma endregion

#pragma region COLLISION_MANAGER
void	CGameInstance::Register_Collider(CCollider* pCollider, _uint Group)
{
    m_pCollision_Manager->RegisterCollider(pCollider, Group);
}
void	CGameInstance::Request_Unregister(CCollider* pCollider, _uint Group)
{
    m_pCollision_Manager->RequestUnregister(pCollider, Group);
}
void	CGameInstance::Immediate_Unregister(CCollider* pCollider, _uint Group)
{
    m_pCollision_Manager->ImmediateUnregister(pCollider, Group);
}
void	CGameInstance::Add_CollisionPool(_uint SrcGroup, _uint DstGroup)
{
    m_pCollision_Manager->Add_CollisionPool(SrcGroup, DstGroup);
}
void	CGameInstance::Reset_For_SceneChange()
{
    m_pCollision_Manager->Reset_For_SceneChange();
    m_pPhysX_Manager->Reset_For_SceneChange();
}
void	CGameInstance::Clear_CollisionPool()
{
    m_pCollision_Manager->Clear_CollisionPool();
}
#pragma endregion

#pragma region SOUND_MANAGER
void CGameInstance::Play_SFX(const TCHAR* pSoundKey, float fVolume, ESoundBus eBus)
{
    m_pSound_Manager->PlaySFX(pSoundKey, fVolume, eBus);
}
void CGameInstance::Play_SFX3D(const TCHAR* pSoundKey, _fvector vSoundPos, float fVolume, ESoundBus eBus)
{
    m_pSound_Manager->PlaySFX3D(pSoundKey, vSoundPos, fVolume, eBus);
}
void CGameInstance::Play_BGM(const TCHAR* pSoundKey, float fVolume, bool bLoop)
{
    m_pSound_Manager->PlayBGM(pSoundKey, fVolume, bLoop);
}
void CGameInstance::Stop_BGM()
{
    m_pSound_Manager->StopBGM();
}
void CGameInstance::Set_BusVolume(ESoundBus eBus, float fVolume)
{
    m_pSound_Manager->SetBusVolume(eBus, fVolume);
}
void CGameInstance::Stop_Bus(ESoundBus eBus)
{
    m_pSound_Manager->StopBus(eBus);
}
void CGameInstance::Stop_SoundAll()
{
    m_pSound_Manager->StopAll();
}
#pragma endregion

#pragma region TARGET_MANAGER
HRESULT CGameInstance::Add_RenderTarget(const _wstring& strTargetTag, _uint iWidth, _uint iHeight, DXGI_FORMAT ePixelFormat, const _float4& vClearColor)
{
    return m_pTarget_Manager->Add_RenderTarget(strTargetTag, iWidth, iHeight, ePixelFormat, vClearColor);
}
HRESULT CGameInstance::Add_MRT(const _wstring& strMRTTag, const _wstring& strTargetTag)
{
    return m_pTarget_Manager->Add_MRT(strMRTTag, strTargetTag);
}
HRESULT CGameInstance::Begin_MRT(const _wstring& strMRTTag, ID3D11DepthStencilView* pDSV, _bool bBindDSV, _bool bClear)
{
    return m_pTarget_Manager->Begin_MRT(strMRTTag, pDSV, bBindDSV, bClear);
}
HRESULT CGameInstance::End_MRT()
{
    return m_pTarget_Manager->End_MRT();
}

#ifdef _DEBUG
HRESULT CGameInstance::Ready_RT_Debug(const _wstring& strTargetTag, _float fX, _float fY, _float fSizeX, _float fSizeY)
{
    return m_pTarget_Manager->Ready_Debug(strTargetTag, fX, fY, fSizeX, fSizeY);
}
HRESULT CGameInstance::Render_RT_Debug(const _wstring& strMRTTag, class CShader* pShader, class CVIBuffer_Rect* pVIBuffer)
{
    return m_pTarget_Manager->Render_Debug(strMRTTag, pShader, pVIBuffer);
}
#endif

#pragma endregion

#pragma region CULLING_MANAGER
_bool CGameInstance::Update_CullingView(CULLING_VIEW eView, const CULLING_VIEW_DESC& Desc)
{
    if (nullptr == m_pCulling_Manager)
        return false;

    return m_pCulling_Manager->Update_View(eView, Desc);
}

_bool CGameInstance::Should_CullAABB(CULLING_VIEW eView, const BoundingBox& WorldBounds) const
{
    if (nullptr == m_pCulling_Manager)
        return false;

    return m_pCulling_Manager->Should_CullAABB(eView, WorldBounds);
}

_bool CGameInstance::Should_CullByDistance(const BoundingBox& WorldBounds, _float fCullDistance) const
{
    if (nullptr == m_pCulling_Manager)
        return false;

    return m_pCulling_Manager->Should_CullByDistance(WorldBounds, fCullDistance);
}

_bool XM_CALLCONV CGameInstance::IsIn_CullingView_WorldSpace(CULLING_VIEW eView, _fvector vWorldPos, _float fRange) const
{
    if (nullptr == m_pCulling_Manager)
        return true;

    return m_pCulling_Manager->IsIn_WorldSpace(eView, vWorldPos, fRange);
}

_bool CGameInstance::IsIn_CullingView_AABB(CULLING_VIEW eView, const BoundingBox& WorldBounds) const
{
    if (nullptr == m_pCulling_Manager)
        return true;

    return m_pCulling_Manager->IsIn_WorldSpace_AABB(eView, WorldBounds);
}
#pragma endregion

#pragma region TEXTURE_HUB
HRESULT CGameInstance::LoadOrGet_TextureFromHub(const _tchar* pTexturePath, TEXTURE_HANDLE* pOutHandle)
{
    if (nullptr == m_pTexture_Hub)
        return E_FAIL;

    return m_pTexture_Hub->LoadOrGet(pTexturePath, pOutHandle);
}

HRESULT CGameInstance::Register_TextureNameInHub(const _tchar* pTextureName, TEXTURE_HANDLE Handle)
{
    if (nullptr == m_pTexture_Hub)
        return E_FAIL;

    return m_pTexture_Hub->Register_TextureName(Handle, pTextureName);
}

HRESULT CGameInstance::Get_TextureFromHub(const _tchar* pTextureName, TEXTURE_HANDLE* pOutHandle) const
{
    if (nullptr == m_pTexture_Hub)
        return E_FAIL;

    return m_pTexture_Hub->Get(pTextureName, pOutHandle);
}

HRESULT CGameInstance::Bind_TextureFromHub(CShader* pShader, const _char* pConstantName, TEXTURE_HANDLE Handle)
{
    if (nullptr == m_pTexture_Hub)
        return E_FAIL;

    return m_pTexture_Hub->Bind_ShaderResource(pShader, pConstantName, Handle);
}

HRESULT CGameInstance::Bind_DefaultTextureFromHub(CShader* pShader, const _char* pConstantName, DEFAULT_TEXTURE eKind)
{
    return m_pTexture_Hub->Bind_DefaultShaderResource(pShader, pConstantName, eKind);
}

TEXTURE_HUB_STATS CGameInstance::Get_TextureHubStats() const
{
    if (nullptr == m_pTexture_Hub)
        return {};

    return m_pTexture_Hub->Get_Stats();
}
#pragma endregion

#pragma region PHYSIX_MANAGER

PxTriangleMesh* CGameInstance::Cook_TriangleMesh(const _float3* p, _uint nv, const _uint* idx, _uint ni, _bool bFlip)
{
    return m_pPhysX_Manager->Cook_TriangleMesh(p, nv, idx, ni, bFlip);
}
PxRigidStatic* CGameInstance::Create_StaticActor(PxTriangleMesh* pMesh, _fmatrix W)
{
    return m_pPhysX_Manager->Create_StaticActor(pMesh, W);
}
PxRigidStatic* CGameInstance::Create_StaticBox(const _float3& vLocalCenter, const _float3& vLocalHalfExtents, _fmatrix WorldMatrix)
{
    return m_pPhysX_Manager->Create_StaticBox(vLocalCenter, vLocalHalfExtents, WorldMatrix);
}
void CGameInstance::Remove_StaticActor(PxRigidStatic* pActor)
{
    m_pPhysX_Manager->Remove_StaticActor(pActor);
}
PxController* CGameInstance::Create_CapsuleController(const _float3& vFootPos, _float fRadius, _float fHeight)
{
    return m_pPhysX_Manager->Create_CapsuleController(vFootPos, fRadius, fHeight);
}
void CGameInstance::Release_Controller(PxController* pCtrl)
{
    m_pPhysX_Manager->Release_Controller(pCtrl);
}
void CGameInstance::Toggle_PhysXDebug()
{
    m_pPhysX_Manager->Toggle_DebugDraw();
}
_bool CGameInstance::Is_PhysXDebug() const
{
    return m_pPhysX_Manager->Is_DebugDraw();
}
void CGameInstance::Render_PhysXDebug(_fmatrix V, _fmatrix P)
{
    m_pPhysX_Manager->Render_Debug(V, P);
}
#pragma endregion




CGameInstance* CGameInstance::GetInstance()
{
    if (m_pInstance == nullptr)
    {
        m_pInstance = new CGameInstance;
    }
    return m_pInstance;
}

void CGameInstance::DestroyInstance()
{
    if (m_pInstance)
    {
        m_pInstance->Free();
        delete m_pInstance;
        m_pInstance = nullptr;
    }
}

void CGameInstance::Free()
{
	m_pGameInstance_Proxy->Disconnect();

    Safe_Release(m_pShaderGlobal_Manager);
    Safe_Release(m_pEnvironment_Manager);
    Safe_Release(m_pShadow_Dir);
    Safe_Release(m_pShadow_Blob);
    Safe_Release(m_pTarget_Manager);
    Safe_Release(m_pSound_Manager);
    Safe_Release(m_pCollision_Manager);
    Safe_Release(m_pEventBus);
    Safe_Release(m_pFont_Manager);
	Safe_Release(m_pLight_Manager);
	Safe_Release(m_pInput_Device);
    Safe_Release(m_pRenderer);
    Safe_Release(m_pObject_Manager);
    Safe_Release(m_pEffect_Manager);
    Safe_Release(m_pLevel_Manager);
    Safe_Release(m_pTexture_Hub);
    Safe_Release(m_pPrototype_Manager);
    
    CEffect_Allocator::DestroyInstance();

    Safe_Release(m_pPhysX_Manager);
    Safe_Release(m_pTimer_Manager);
    Safe_Release(m_pCulling_Manager);
    Safe_Release(m_pCamera_Manager);
    Safe_Release(m_pGameInstance_Proxy);
    Safe_Release(m_pGraphic_Device);
}
