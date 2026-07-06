#pragma once

#include "Engine_Defines.h"
#include "GameInstance_Proxy.h"
#include "Prototype_Manager.h"

NS_BEGIN(physx)
class PxTriangleMesh;
class PxRigidStatic;
class PxController;
NS_END

NS_BEGIN(Engine)

class CGraphic_Device;
class CTimer_Manager;
class CLevel_Manager;
class CPrototype_Manager;
class CObject_Manager;
class CRenderer;
class CLevel;
class CGameObject;
class CBase;
class CCamera_Manager;
class CInput_Device;
class CLight_Manager;
class CFont_Manager;
class CUIObject;
class CEventBus;
class CCollision_Manager;
class CSound_Manager;
class CTarget_Manager;
class CShadow_Dir;
class CEffect_Manager;
class CPhysX_Manager;
class CEnvironment_Manager;
class CShaderGlobal_Manager;
class CCulling_Manager;
class CTexture_Hub;
class CProfiler_Manager;

class ENGINE_DLL CGameInstance 
{
	friend class CGameInstance_Proxy;

private:										
	CGameInstance();
	CGameInstance(const CGameInstance&) = delete;					
	CGameInstance& operator = (const CGameInstance&) = delete;
	~CGameInstance() = default;

#pragma region ENGINE
public:
	static HRESULT Initialize_Engine(const ENGINE_DESC& EngineDesc, ID3D11Device** ppDevice, ID3D11DeviceContext** ppContext);
	static void	   DestroyInstance();
	static CGameInstance_Proxy* GetProxy() 
	{ 
		m_pGameInstance_Proxy->AddRef();
		return m_pGameInstance_Proxy;
	}

private:
	void Update_Engine(_float fTimeDelta, _float fRawTimeDelta);
	HRESULT Begin_Draw();
	HRESULT Draw();
	HRESULT End_Draw();
	void Clear_Resources(_int iLevelIndex);

	_float RandomFloat(_float fMin, _float fMax) const;
	_int   RandomInt(_int iMin, _int iMax) const;
	
	_uint64 Get_FrameIndex();
#pragma endregion

#pragma region DEVICE
	HRESULT Bind_BackBuffer();
	HRESULT OnResize(_uint iWidth, _uint iHeight);
	_float Get_WindowWidth();
	_float Get_WindowHeight();
#pragma endregion

#pragma region TIMER_MANAGER
private:
	_float Get_TimeDelta(const _wstring& strTimerTag);
	HRESULT	Add_Timer(const _wstring& strTimerTag);
	void Compute_Timer(const _wstring& strTimerTag);
#pragma endregion

#pragma region LEVEL_MANAGER
	HRESULT Change_Level(_int iNewLevelIndex, CLevel* pNewLevel);
#pragma endregion

#pragma region PROTOTYPE_MANAGER
	HRESULT Add_Proto(_uint iLevelIndex, const _wstring& strPrototypeTag, CBase* pPrototype);
	CBase* Clone_Prototype(PROTOTYPE eType, _uint iLevelIndex, const _wstring& strPrototypeTag, void* pArg = nullptr);
	_bool Has_Prototype(_uint iLevelIndex, const _wstring& strPrototypeTag);
#pragma endregion

#pragma region OBJECT_MANAGER
	HRESULT Add_GameObject(_uint iPrototypeLevelIndex, const _wstring& strPrototypeTag, _uint iLayerLevelIndex, const _wstring& strLayerTag, const _wstring& strObjectTag, void* pArg);
	HRESULT Add_GameObject_Return (CGameObject** ppOut, _uint iPrototypeLevelIndex, const _wstring& strPrototypeTag, _uint iLayerLevelIndex, const _wstring& strLayerTag, const _wstring& strObjectTag, void* pArg);
	void	Clear_Objects(_int iLevelIndex);
	void    Destroy_GameObject(CGameObject* pGameObject);
	CGameObject* Find_GameObject(_uint iLevelIndex, const _wstring& strLayerTag, const _wstring& strObjectTag);
#pragma endregion

#pragma region RENDERER
	void Add_RenderGroup(RENDERID eGroupID, CGameObject* pGameObject);
	void Add_RenderGroup_UI(RENDERUIID eGroupID, CUIObject* pUIObject);
#pragma endregion

#pragma region INPUT_DEVICE
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

#pragma region CAMERA_MANAGER
	const _float4x4* Get_Matrix(D3DTS eState, PROJ_TYPE eType) const;
	const _float4x4* Get_InverseMatrix_Prespec(D3DTS eState) const;
	const _float4* Get_CamPosition() const;
	void Set_Transform(D3DTS eState, PROJ_TYPE eType, _fmatrix StateMatrix);
	void Set_Transform(D3DTS eState, PROJ_TYPE eType, const _float4x4& StateMatrix);
#pragma endregion

#pragma region FRUSTUM_MANAGER
	_bool Update_CullingView(CULLING_VIEW eView, const CULLING_VIEW_DESC& Desc);
	_bool Should_CullAABB(CULLING_VIEW eView, const BoundingBox& WorldBounds) const;
	_bool Should_CullByDistance(const BoundingBox& WorldBounds, _float fCullDistance) const;
	_bool XM_CALLCONV IsIn_CullingView_WorldSpace(CULLING_VIEW eView, _fvector vWorldPos, _float fRange = 0.f) const;
	_bool IsIn_CullingView_AABB(CULLING_VIEW eView, const BoundingBox& WorldBounds) const;
#pragma endregion

#pragma region LIGHT_MANAGER
	const LIGHT_DESC* Get_LightDesc(_uint iIndex);
	HRESULT Add_Light(const LIGHT_DESC& LightDesc);
#pragma endregion

#pragma region PICKING
	void Compute_PickingRay(
		_float      fNdcX,
		_float      fNdcY,
		_vector* pOutOrigin,
		_vector* pOutDir,
		PROJ_TYPE  eType = PROJ_TYPE::PERSPEC
	);

	_bool Pick_RayToPlane(
		_vector vOrigin,
		_vector vDir,
		_float3* pOutHit,
		_float    fPlaneY = 0.f
	);

	_bool Pick_RayToMesh(
		_vector       vOrigin,
		_vector       vDir,
		const _float3* pVertices, _uint iVertexCount,
		const _uint* pIndices, _uint iIndexCount,
		_float3* pOutHit,
		_float* pOutDist = nullptr
	);
#pragma endregion

#pragma region FONT_MANAGER
	HRESULT Add_Font(const _wstring& strFontTag, const _tchar* pFontFilePath);
	HRESULT Draw_Text(const _wstring& strFontTag, const _tchar* pText, const _float2& vPosition,
		_fvector vColor = XMVectorSet(1.f, 1.f, 1.f, 1.f), _float fRotation = 0.f, const _float2& vScale = _float2(1.f, 1.f), TEXT_ALIGN eAlign = TEXT_ALIGN::CENTER);
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
	CSound_Handle Play_SFX(const TCHAR* pSoundKey, float fVolume = 1.f, ESoundBus eBus = ESoundBus::SFX);
	CSound_Handle Play_SFX3D(const TCHAR* pSoundKey, _fvector vSoundPos, float fVolume = 1.f, ESoundBus eBus = ESoundBus::SFX);
	CSound_Handle Play_SFX_Loop(const TCHAR* pSoundKey, float fVolume = 1.f, ESoundBus eBus = ESoundBus::SFX);
	CSound_Handle Play_SFX3D_Loop(const TCHAR* pSoundKey, _fvector vSoundPos, float fVolume = 1.f,	ESoundBus eBus = ESoundBus::SFX);
	CSound_Handle Play_SFX_Section_Loop(const TCHAR* pSoundKey, float fLoopStart, float fLoopEnd, float fVolume = 1.f, ESoundBus eBus = ESoundBus::SFX);

	void Play_BGM(const TCHAR* pSoundKey, float fVolume = 1.f, bool bLoop = true);
	void Play_BGM_Section(const TCHAR* pSoundKey, float fStart01, float fEnd01, float fVolume = 1.f);
	void Fade_BGM_Out(float fSeconds);
	void Play_BGM_Fade(const TCHAR* pSoundKey, float fSeconds, float fVolume = 1.f);
	void Stop_BGM();
	void Set_BusVolume(ESoundBus eBus, float fVolume);
	void Stop_Bus(ESoundBus eBus);
	void Stop_SoundAll();
#pragma endregion

#pragma region TARGET_MANAGER
	HRESULT Add_RenderTarget(const _wstring& strTargetTag, _uint iWidth, _uint iHeight, DXGI_FORMAT ePixelFormat, const _float4& vClearColor);
	HRESULT Add_MRT(const _wstring& strMRTTag, const _wstring& strTargetTag);
	HRESULT Begin_MRT(const _wstring& strMRTTag, ID3D11DepthStencilView* pDSV, _bool bBindDSV, _bool bClear);
	HRESULT End_MRT();

#ifdef _DEBUG
	HRESULT Ready_RT_Debug(const _wstring& strTargetTag, _float fX, _float fY, _float fSizeX, _float fSizeY);
	HRESULT Render_RT_Debug(const _wstring& strMRTTag, class CShader* pShader, class CVIBuffer_Rect* pVIBuffer);
#endif
#pragma endregion

#pragma region EDITMODE
	  void  Set_EditMode(_bool bEdit) { m_bEditMode = bEdit; }
	  _bool Is_EditMode() const		  { return m_bEditMode; }
#pragma endregion

#pragma region TEXTURE_HUB
	  HRESULT LoadOrGet_TextureFromHub(const _tchar* pTexturePath, TEXTURE_HANDLE* pOutHandle);
	  HRESULT Register_TextureNameInHub(const _tchar* pTextureName, TEXTURE_HANDLE Handle);
	  HRESULT Get_TextureFromHub(const _tchar* pTextureName, TEXTURE_HANDLE* pOutHandle) const;
	  HRESULT Bind_TextureFromHub(class CShader* pShader, const _char* pConstantName, TEXTURE_HANDLE Handle);
	  HRESULT Bind_DefaultTextureFromHub(class CShader* pShader, const _char* pConstantName, DEFAULT_TEXTURE eKind);
#pragma endregion

#pragma region PHYSIX_MANAGER
	  physx::PxTriangleMesh* Cook_TriangleMesh(const _float3* pPositions, _uint iNumVertices, const _uint* pIndices, _uint iNumIndices, _bool bFlipWinding = true);
	  physx::PxRigidStatic*  Create_StaticActor(physx::PxTriangleMesh* pMesh, _fmatrix WorldMatrix);
	  HRESULT				 Refresh_StaticBoxPose(physx::PxRigidStatic* pActor, const _float3& vLocalCenter, _fmatrix WorldMatrix);
	  HRESULT				 Refresh_StaticActorPose(physx::PxRigidStatic* pActor, _fmatrix WorldMatrix);
	  physx::PxRigidStatic*	 Create_StaticBox(const _float3& vLocalCenter, const _float3& vLocalHalfExtents, _fmatrix WorldMatrix);
	  void                   Remove_StaticActor(physx::PxRigidStatic* pActor);

	  physx::PxController* Create_CapsuleController(const _float3& vFootPos, _float fRadius, _float fHeight);
	  void				   Release_Controller(physx::PxController* pCtrl);
						   
	  void				   Toggle_PhysXDebug();
	  _bool				   Is_PhysXDebug() const;
	  void				   Render_PhysXDebug(_fmatrix ViewMatrix, _fmatrix ProjMatrix);
#pragma endregion




private:
	CGraphic_Device*			m_pGraphic_Device = { nullptr };
	CTimer_Manager*				m_pTimer_Manager = { nullptr };
	CLevel_Manager*				m_pLevel_Manager = { nullptr };
	CPrototype_Manager*			m_pPrototype_Manager = { nullptr };
	CObject_Manager*			m_pObject_Manager = { nullptr };
	CRenderer*					m_pRenderer = { nullptr };
	CCamera_Manager*			m_pCamera_Manager = { nullptr };
	CInput_Device*				m_pInput_Device = { nullptr };
	CLight_Manager*				m_pLight_Manager = { nullptr };
	CFont_Manager*				m_pFont_Manager = { nullptr };
	CEventBus*					m_pEventBus = { nullptr };
	CCollision_Manager*			m_pCollision_Manager = { nullptr };
	CSound_Manager*				m_pSound_Manager = { nullptr };
	CTarget_Manager*			m_pTarget_Manager = { nullptr };
	CShadow_Dir*				m_pShadow_Dir = { nullptr };
	CShadow_Dir*				m_pShadow_Blob = { nullptr };
	CEffect_Manager*			m_pEffect_Manager = { nullptr };
	CPhysX_Manager*				m_pPhysX_Manager = { nullptr };
	CEnvironment_Manager*		m_pEnvironment_Manager = { nullptr };
	CShaderGlobal_Manager*		m_pShaderGlobal_Manager = { nullptr };
	CCulling_Manager*			m_pCulling_Manager = { nullptr };
	CTexture_Hub*				m_pTexture_Hub = { nullptr };
	CProfiler_Manager*			m_pProfiler_Manager = { nullptr };

	mutable mt19937             m_RandomGenerator;
	_bool						m_bEditMode = { false };
	_uint64						m_iFrameIndex = { 0 };

private:
	static CGameInstance* m_pInstance;
	static CGameInstance_Proxy* m_pGameInstance_Proxy;

private:
	static CGameInstance* GetInstance();
	void Free();
};

NS_END