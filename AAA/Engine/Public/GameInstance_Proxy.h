#pragma once
#include "Prototype_Manager.h"

NS_BEGIN(physx)
class PxTriangleMesh;
class PxRigidStatic;
class PxController;
class PxRigidDynamic;
class PxConvexMesh;
NS_END

NS_BEGIN(Engine)

class CGameObject;
class CUIObject;
class CGameInstance;
class CLevel;
enum class CULLING_VIEW;

class ENGINE_DLL CGameInstance_Proxy final : public CBase
{
private:
    CGameInstance_Proxy() = default;
    virtual ~CGameInstance_Proxy() = default;

public:
    void    Disconnect() { m_pOwner = nullptr; }
    _bool   IsConnected() const { return m_pOwner != nullptr; }

#pragma region ENGINE
public: // Engine
    void Update_Engine(_float fTimeDelta);
    HRESULT Begin_Draw();
    HRESULT Draw();
    HRESULT End_Draw();
    void    Bind_RenderTarget(ID3D11RenderTargetView* pRTV, ID3D11DepthStencilView* pDSV, _uint w, _uint h);

    void    Clear_Resources(_int iLevelIndex);

    _float RandomFloat(_float fMin, _float fMax) const;
    _int   RandomInt(_int iMin, _int iMax) const;

    _int64 Get_FrameIndex();
#pragma endregion

#pragma region GRAPHICDEVICE
public: // GraphicDevice
    HRESULT Bind_BackBuffer();
    HRESULT OnResize(_uint iWidth, _uint iHeight);
    _float Get_WindowWidth();
    _float Get_WindowHeight();
#pragma endregion

#pragma region TIMERMANAGER
public: //TimerManager
    _float Get_TimeDelta(const _wstring& strTimerTag);
    HRESULT	Add_Timer(const _wstring& strTimerTag);
    void Compute_Timer(const _wstring& strTimerTag);
    _float Get_RawTimeDelta(const _wstring& strTimerTag);
    void   Set_TimeScale(_float fScale);
    _float Get_TimeScale() const;
#pragma endregion

#pragma region LEVELMANAGER
public: // LevelManager
    HRESULT Change_Level(_int iNewLevelIndex, CLevel* pNewLevel);
#pragma endregion

#pragma region PROTOTYPEMANAGER
public: // PrototypeManager
    HRESULT Add_Prototype(_uint iLevelIndex, const _wstring& strTag, CBase* pProto);
    CBase* Clone_Prototype(PROTOTYPE eType, _uint iLevelIndex, const _wstring& strTag, void* pArg = nullptr);
    _bool Has_Prototype(_uint iLevelIndex, const _wstring& strPrototypeTag);
#pragma endregion

#pragma region OBJECTMANAGER
public: //ObjectManager
    HRESULT Add_GameObject(_uint iProtoLevel, const _wstring& strProtoTag,
        _uint iLayerLevel, const _wstring& strLayerTag, const _wstring& strObjectTag, void* pArg = nullptr);
    HRESULT Add_GameObject_Return(CGameObject** ppOut, _uint iPrototypeLevelIndex, const _wstring& strPrototypeTag, 
        _uint iLayerLevelIndex, const _wstring& strLayerTag, const _wstring& strObjectTag, void* pArg = nullptr);
    void	Clear_Objects(_int iLevelIndex);
    void    Destroy_GameObject(CGameObject* pGameObject);
    CGameObject* Find_GameObject(_uint iLevelIndex, const _wstring& strLayerTag, const _wstring& strObjectTag);

    template<typename T>
    T* Find_GameObject(_uint iLevelIndex, const _wstring& strLayerTag, const _wstring& strObjectTag)
    {
        return dynamic_cast<T*>(Find_GameObject(iLevelIndex, strLayerTag, strObjectTag));
    }
#pragma endregion

#pragma region RENDERER
public: //Renderer
    void    Add_RenderGroup(RENDERID eGroupID, CGameObject* pGameObject);
    void    Add_RenderGroup_UI(RENDERUIID eGroupID, CUIObject* pUIObject);
#ifdef _DEBUG
    void    Add_DebugComponent(class CComponent* pComponent);
    void    Toggle_DebugRender();
    _bool   IsOn_DebugRender();
#endif
#pragma endregion

#pragma region INPUTDEVICE
public: // InputDevice
    _byte	Get_DIKeyState(_ubyte byKeyID);
    _byte	Get_DIMouseState(DIMB eMouse);
    _long	Get_DIMouseMove(DIMM eMouseState);
    void	Disable_InputDeveice();
    void	Enable_InputDeveice();
    _bool Key_Down(_ubyte byKeyID);
    _bool Key_Up(_ubyte byKeyID);
    _bool Key_Pressing(_ubyte byKeyID);
    _bool Mouse_Down(DIMB eMouse);
    _bool Mouse_Up(DIMB eMouse);
    _bool Mouse_Pressing(DIMB eMouse);
    POINT Get_MousePos();
#pragma endregion

#pragma region CAMERAMANAGER
public: // CameraManager
    const _float4x4* Get_Matrix(D3DTS eState, PROJ_TYPE eType) const;
    const _float4x4* Get_InverseMatrix_Prespec(D3DTS eState) const;
    const _float4* Get_CamPosition() const;
    const _float4* Get_CamLook()  const;  
    const _float4* Get_CamRight() const;
    void Set_Transform(D3DTS eState, PROJ_TYPE eType, _fmatrix StateMatrix);
    void Set_Transform(D3DTS eState, PROJ_TYPE eType, const _float4x4& StateMatrix);
#pragma endregion

#pragma region FRUSTUM_MANAGER
  public: // FrustumManager
      _bool Update_CullingView(CULLING_VIEW eView, const CULLING_VIEW_DESC& Desc);
      _bool Should_CullAABB(CULLING_VIEW eView, const BoundingBox& WorldBounds) const;
      _bool Should_CullByDistance(const BoundingBox& WorldBounds, _float fCullDistance) const;
      _bool XM_CALLCONV IsIn_CullingView_WorldSpace(CULLING_VIEW eView, _fvector vWorldPos, _float fRange = 0.f) const;
      _bool IsIn_CullingView_AABB(CULLING_VIEW eView, const BoundingBox& WorldBounds) const;
#pragma endregion

#pragma region LIGHTMANAGER
public: // LightManager
    const LIGHT_DESC* Get_LightDesc(_uint iIndex);
    HRESULT Add_Light(const LIGHT_DESC& LightDesc);
    HRESULT Render_Light(class CShader* pShader, class CVIBuffer_Rect* pVIBuffer);
    HRESULT Clear_Lights();
#pragma endregion

#pragma region PICKING
public: // Picking
    void Compute_PickingRay(_float fNdcX, _float fNdcY,_vector* pOutOrigin, _vector* pOutDir, PROJ_TYPE  eType = PROJ_TYPE::PERSPEC);
    _bool Pick_RayToPlane(_vector vOrigin, _vector vDir, _float3* pOutHit, _float fPlaneY = 0.f);
    _bool Pick_RayToMesh(_vector vOrigin, _vector vDir, const _float3* pVertices, _uint iVertexCount, const _uint* pIndices, _uint iIndexCount, _float3* pOutHit, _float* pOutDist = nullptr);
#pragma endregion

#pragma region FONT_MANAGER
    HRESULT Add_Font(const _wstring& strFontTag, const _tchar* pFontFilePath);
    HRESULT Draw_Text(const _wstring& strFontTag, const _tchar* pText, const _float2& vPosition,
        _fvector vColor = XMVectorSet(1.f, 1.f, 1.f, 1.f), _float fRotation = 0.f, const _float2& vScale = _float2(1.f, 1.f), TEXT_ALIGN eAlign = TEXT_ALIGN::CENTER);
    HRESULT Draw_Text_Raw(const _wstring& tag, const _tchar* p, const _float2& pos, _fvector col, const _float2& scl,
        TEXT_ALIGN a);
    _float2 Measure_Text(const _wstring& strFontTag, const _tchar* pText);
#pragma endregion

#pragma region EVENTBUS
    SUBHANDLE Subscribe(const wstring& EventTag, function<void(void*)> Handler);
    void	  Publish(const wstring& EventTag, void* pData);
    void	  UnSubscribe(const SUBHANDLE& Handle);
    void	  Clear_EventBus();
#pragma endregion

#pragma region COLLISION_MANAGER
    void	Register_Collider(CCollider* pCollider, _uint Group);
    void	Request_Unregister(CCollider* pCollider, _uint Group);
    void	Immediate_Unregister(CCollider* pCollider, _uint Group);
    void	Add_CollisionPool(_uint SrcGroup, _uint DstGroup);
    void	Reset_For_SceneChange();
    void	Clear_CollisionPool();
#pragma endregion

#pragma region SOUND_MANAGER
    void Play_Sound(const TCHAR* pSoundKey, _uint iChannelIndex, float fVolume);
    void Play_Sound3D(const TCHAR* pSoundKey, _uint iChannelIndex, float fVolume, _fvector vSoundPos);
    void Play_BGM(const TCHAR* pSoundKey, _uint iChannelIndex, float fVolume);
    void Stop_Sound(_uint iChannelIndex);
    void Stop_SoundAll();
    void Set_Channel_Volume(_uint iChannelIndex, float fVolume);
#pragma endregion

#pragma region TARGET_MANAGER
    HRESULT Add_RenderTarget(const _wstring& strTargetTag, _uint iWidth, _uint iHeight, DXGI_FORMAT ePixelFormat, const _float4& vClearColor);
    HRESULT Add_MRT(const _wstring& strMRTTag, const _wstring& strTargetTag);
    HRESULT Begin_MRT(const _wstring& strMRTTag, ID3D11DepthStencilView* pDSV = nullptr, _bool bBindDSV = true, _bool bClear = true);
    HRESULT End_MRT();
    HRESULT Bind_RT_ShaderResource(const _wstring& strTargetTag, class CShader* pShader, const _char* pConstantName);
    HRESULT Bind_RT_CSResource(const _wstring& strTargetTag, _uint iSlot);

#ifdef _DEBUG
public:
    HRESULT Ready_RT_Debug(const _wstring& strTargetTag, _float fX, _float fY, _float fSizeX, _float fSizeY);
    HRESULT Render_RT_Debug(const _wstring& strMRTTag, class CShader* pShader, class CVIBuffer_Rect* pVIBuffer);
#endif
#pragma endregion

#pragma region SHADOW
    const _float4x4* Get_Shadow_Transform(D3DTS eState) const;
    HRESULT Add_ShadowLight(const SHADOW_LIGHT_DESC& ShadowDesc);
    HRESULT Update_ShadowLight(const SHADOW_LIGHT_DESC& Desc);
#pragma endregion

#pragma region EFFECT_MANAGER
    HRESULT Spawn_Effect(_uint iLevel, const _wstring& strEffectKey, const _wstring& strProtoTag, 
        const CEffect_Container::EFFECT_CONTAINER_DESC& desc, const json* pConfig = nullptr, CEffect_Container** ppOut = nullptr);

    void Set_EffectPrototypeLevel(_uint iLevel);

    CShader* Get_2DShader();
    CShader* Get_MeshShader();
#pragma endregion

#pragma region EDITMODE
  public:
      void  Set_EditMode(_bool bEdit);
      _bool Is_EditMode() const;
#pragma endregion

#pragma region ENVIRONMENT_MANAGER
  public:
      HRESULT Register_Environment(const _wstring& tag, const _tchar* d, const _tchar* s, const _tchar* l, _float i = 1.f);
      HRESULT Set_CurrentEnvironment(const _wstring& tag);
      const ENVIRONMENT_DESC& Get_CurrentEnvironment() const;
#pragma endregion

#pragma region SHADERGLOBAL_MANAGER
  public:
      HRESULT Bind_ShaderGlobals(class CShader* pShader, const string& strName);
      HRESULT Bind_ShaderGlobals(class CShader* pShader, initializer_list<const _char*> Names);
      void    Set_ShaderGlobal(const string& strName, const _float4& vValue);
      vector<GLOBAL_DESC>& Get_ShaderGlobals();
      const _float4* Get_ShaderGlobal(const string& strName) const;
#pragma endregion

#pragma region PHYSIX_MANAGER
      physx::PxTriangleMesh* Cook_TriangleMesh(const _float3* pPositions, _uint iNumVertices, const _uint* pIndices, _uint iNumIndices, _bool bFlipWinding = true);
      physx::PxRigidStatic*  Create_StaticActor(physx::PxTriangleMesh* pMesh, _fmatrix WorldMatrix);
      physx::PxRigidStatic*  Create_StaticBox(const _float3& vLocalCenter, const _float3& vLocalHalfExtents, _fmatrix WorldMatrix);
      void                   Remove_StaticActor(physx::PxRigidStatic* pActor);

      physx::PxController*   Create_CapsuleController(const _float3& vFootPos, _float fRadius, _float fHeight);
      void                   Release_Controller(physx::PxController* pCtrl);

      void  Toggle_PhysXDebug();
      _bool Is_PhysXDebug() const;
      void  Render_PhysXDebug(_fmatrix ViewMatrix, _fmatrix ProjMatrix);

      physx::PxRigidDynamic* Create_DynamicBox(const _float3& vPos, const _float4& qRot, const _float3& vHalfExtents, _float fDensity = 10.f);
      physx::PxRigidDynamic* Create_DynamicSphere(const _float3& vPos, _float fRadius, _float fDensity = 10.f);
      physx::PxRigidDynamic* Create_DynamicCapsule(const _float3& vPos, const _float4& qRot, _float fRadius, _float fHalfHeight, _float fDensity = 10.f);
      physx::PxConvexMesh*   Cook_ConvexMesh(const _float3* pPositions, _uint iNumVertices);
      physx::PxRigidDynamic* Create_DynamicConvex(physx::PxConvexMesh* pMesh, _fmatrix WorldMatrix, _float fDensity = 10.f);
      void                   Remove_DynamicActor(physx::PxRigidDynamic* pActor);
#pragma endregion

#pragma region TEXTURE_HUB
  public:
      HRESULT LoadOrGet_TextureFromHub(const _tchar* pTexturePath, TEXTURE_HANDLE* pOutHandle);
      HRESULT Register_TextureNameInHub(const _tchar* pTextureName, TEXTURE_HANDLE Handle);
      HRESULT Get_TextureFromHub(const _tchar* pTextureName, TEXTURE_HANDLE* pOutHandle) const;
      HRESULT Bind_TextureFromHub(class CShader* pShader, const _char* pConstantName, TEXTURE_HANDLE Handle);
      HRESULT Bind_DefaultTextureFromHub(class CShader* pShader, const _char* pConstantName, DEFAULT_TEXTURE eKind);
      TEXTURE_HUB_STATS Get_TextureHubStats() const;
#pragma endregion

private:
    CGameInstance* m_pOwner = { nullptr };

public:
    static CGameInstance_Proxy* Create(CGameInstance* pOwner);
    virtual void Free() override {}
};

NS_END
