#ifndef Engine_Struct_h__
#define Engine_Struct_h__

namespace Engine
{
	typedef struct tagEngineDesc
	{
		HINSTANCE		hInstance;
		HWND			hWnd;
		WINMODE			eWinMode;
		unsigned int	iViewportWidth, iViewportHeight;
		unsigned int	iNumLevels;
	}ENGINE_DESC;

	typedef struct tagLightDesc
	{
		LIGHT			eType;
		XMFLOAT4		vDiffuse, vAmbient, vSpecular;

		XMFLOAT4		vDirection;
		XMFLOAT4		vPosition;
		float			fRange;
	}LIGHT_DESC;

	typedef struct tagEnvironmentDesc
	{
		ID3D11ShaderResourceView* pDiffuseSRV = { nullptr };  // irradiance cube  (BC6H HDR)
		ID3D11ShaderResourceView* pSpecularSRV = { nullptr };  // prefiltered cube (BC6H HDR, mip=roughness)
		ID3D11ShaderResourceView* pColorGradeLUT = { nullptr };  // ★ 3D LUT (R8G8B8A8_UNORM, 16³)
		unsigned int              iSpecularMip = { 1 };        // 스페큘러 큐브 밉 수
		float                     fIntensity = { 1.f };      // 맵별 IBL 세기
	}ENVIRONMENT_DESC;

	typedef struct tagShadowLightDesc
	{
		XMFLOAT4		vEye, vAt;
		float			fFovy, fNear, fFar;
		float			fWidth, fHeight;
	}SHADOW_LIGHT_DESC;

	typedef struct tagKeyFrame
	{
		XMFLOAT3		vScale;
		XMFLOAT4		vRotation;
		XMFLOAT3		vTranslation;
		float			fTrackPosition;
	}KEYFRAME;

	typedef struct tagSubscriptionHandle {
		wstring			strEventType;
		unsigned int	uiIndex;
		unsigned int	uiVersion;
	}SUBHANDLE;

	struct GLOBAL_DESC
	{
		string  strShaderName;            // 셰이더 변수명 "g_fSSAORadius"
		string  strLabel;                 // 에디터 표시명 "SSAO Radius"
		GVAL    eType = GVAL::FLOAT;
		XMFLOAT4 vValue = {};              // FLOAT은 .x만 사용
		XMFLOAT2 vRange = { 0.f, 1.f };    // 에디터 슬라이더 min/max
	};

	static const unsigned int FROXEL_W = 160;
	static const unsigned int FROXEL_H = 90;
	static const unsigned int FROXEL_D = 64;

	typedef struct tagFroxelCB
	{
		XMFLOAT4X4 mCamViewInv;
		XMFLOAT4X4 mCamProjInv;
		XMFLOAT4X4 mShadowView;
		XMFLOAT4X4 mShadowProj;

		XMFLOAT4 vCamPos;       
		XMFLOAT4 vLightDir;     
		XMFLOAT4 vLightColor;   
		XMFLOAT4 vFogScatter;   
		XMFLOAT4 vFogParams;    
		XMFLOAT4 vFogParams2;   
		XMFLOAT4 vGridParams;   
	}FROXEL_CB;

	struct MESH_LAYER_IDX
	{
		int				iPass = { -1 };						// -1 = default
		unsigned int    iUVIndex = { 0 };					// Base UV: Diffuse / Normal / MRA 공용
		unsigned int    iUnknownUVIndex = { 0 };			// Unknown 전용
		unsigned int    iExtraUVIndex[4] = { 0, 0, 0, 0 };	// ExtraR/G/B/A 전용

		unsigned int	iFlags = { 0 };				// shader-specific option bits
		unsigned int	idx[MTEX_TYPE_MAX] = { 0 };	// MTEX_TYPE 별 slotArrayIndex

		bool			bUseUVTransform = { false };
		XMFLOAT2		vUVScale = { 0.075f, 0.075f };
		XMFLOAT2		vUVScaleNormal = { 0.075f, 0.075f };
		XMFLOAT2		vUVScaleMaterial = { 0.075f, 0.075f };
		float			fUVRotate = 0.f;
		XMFLOAT2		vUVOffset = { 0.f, 0.f };

		float			fNormalStrength = 1.f;
		float			fMaskStrength = 1.f;

		int				iExtraBind[4] = { -1, -1, -1, -1 };

		unsigned int    iExtraTexType[4] = {
				static_cast<unsigned int>(MTEX_TYPE::UNKNOWN),
				static_cast<unsigned int>(MTEX_TYPE::UNKNOWN),
				static_cast<unsigned int>(MTEX_TYPE::UNKNOWN),
				static_cast<unsigned int>(MTEX_TYPE::UNKNOWN)
		};
	};

	struct TEXTURE_HUB_STATS
	{
		unsigned int iCachedSRVCount = {};
		unsigned int iCacheReuseCount = {};
		unsigned int iFirstLoadRequestCount = {};
		unsigned int iLoadFailCount = {};
	};

	struct CULLING_VIEW_DESC
	{
		const XMFLOAT4X4*	pView = { nullptr };
		const XMFLOAT4X4*	pProj = { nullptr };
		float				fCullMargin = { 0.f };
	};

#ifdef _DEBUG
	struct FRUSTUM_CULLING_STATS
	{
		unsigned int   iTestedAABB = {};
		unsigned int   iCulledAABB = {};
		unsigned int   iPassedAABB = {};
		unsigned int   iDisabledPolicy = {};
		unsigned int   iInvalidViewFailOpen = {};
		unsigned int   iInvalidBoundsFailOpen = {};
	};
#endif

	typedef struct tagAnimEvent
	{
		int         iEventType = 0;          // 의미는 클라이언트 enum(EANIM_EVENT)이 결정
		float       fTriggerProgress = 0.f;  // 시작 지점 (0~1)

		bool        bIsRange = false;        // true면 구간 이벤트
		float       fEndProgress = 0.f;      // 구간 끝 (start <= end)

		string      strParam;                // 이펙트 프로토타입 태그 / 소켓 본 이름 (자유도 위해 string)
		int         iIntParam = 0;
		XMFLOAT3    vOffset = {};

		bool        bActive = false;         // 런타임 전용 (직렬화 X)
	}ANIM_EVENT;

	typedef struct tagAnimEventTrack
	{
		string              strAnimName;
		vector<ANIM_EVENT>  Events;          // fTriggerProgress 오름차순 유지
	}ANIM_EVENT_TRACK;

	typedef struct tagVertexPosition
	{
		XMFLOAT3		vPosition;

		static const unsigned int		iNumElements = { 1 };

		static constexpr D3D11_INPUT_ELEMENT_DESC   Elements[] = {
			{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		};
	}VTXPOS;

	typedef struct tagVertexPositionColor
	{
		XMFLOAT3		vPosition;
		XMFLOAT4		vColor;

		static const unsigned int		iNumElements = { 2 };

		static constexpr D3D11_INPUT_ELEMENT_DESC   Elements[] = {
			{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
		};
	}VTXCOLOR;

	typedef struct tagVertexPositionTexcoord
	{
		XMFLOAT3		vPosition;
		XMFLOAT2		vTexcoord;

		static const unsigned int		iNumElements = { 2 };

		static constexpr D3D11_INPUT_ELEMENT_DESC   Elements[] = {
			{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
		};
	}VTXTEX;

	typedef struct tagVertexPositionNormalTexcoord
	{
		XMFLOAT3		vPosition;
		XMFLOAT3		vNormal;
		XMFLOAT2		vTexcoord;

		static const unsigned int		iNumElements = { 3 };

		static constexpr D3D11_INPUT_ELEMENT_DESC   Elements[] = {
			{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0}
		};
	}VTXNORTEX;

	typedef struct tagEffectMesh
	{
		XMFLOAT3		vPosition;
		XMFLOAT3		vNormal;
		XMFLOAT2		vTexcoord;

		XMFLOAT3		vTangent;
		XMFLOAT3		vBinormal;

		static const unsigned int		iNumElements = { 5 };

		static constexpr D3D11_INPUT_ELEMENT_DESC   Elements[] = {
			{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 44, D3D11_INPUT_PER_VERTEX_DATA, 0},
		};
	}VTXEFFECTMESH;

	typedef struct tagVertexMesh
	{
		XMFLOAT3          vPosition;
		XMFLOAT3          vNormal;
		XMFLOAT2          vTexcoord;
		XMFLOAT2          vTexcoord1;
		XMFLOAT2          vTexcoord2;
		XMFLOAT2          vTexcoord3;

		XMFLOAT4          vTangent;
		XMFLOAT4          vBinormal;

		static const unsigned int         iNumElements = { 8 };   // 5 -> 8

		static constexpr D3D11_INPUT_ELEMENT_DESC   Elements[] = {
			{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,	0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT,	0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,		0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"TEXCOORD", 1, DXGI_FORMAT_R32G32_FLOAT,		0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"TEXCOORD", 2, DXGI_FORMAT_R32G32_FLOAT,		0, 40, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"TEXCOORD", 3, DXGI_FORMAT_R32G32_FLOAT,		0, 48, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"TANGENT",  0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 56, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"BINORMAL", 0, DXGI_FORMAT_R32G32B32A32_FLOAT,	0, 72, D3D11_INPUT_PER_VERTEX_DATA, 0},
		};
	}VTXMESH;

	struct VTXMESH_INSTANCED
	{
		static const unsigned int	iNumElements = { 12 };

		static constexpr D3D11_INPUT_ELEMENT_DESC   Elements[] = {
			{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,	0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT,	0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,		0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"TEXCOORD", 1, DXGI_FORMAT_R32G32_FLOAT,		0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"TEXCOORD", 2, DXGI_FORMAT_R32G32_FLOAT,		0, 40, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"TEXCOORD", 3, DXGI_FORMAT_R32G32_FLOAT,		0, 48, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"TANGENT",  0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 56, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"BINORMAL", 0, DXGI_FORMAT_R32G32B32A32_FLOAT,	0, 72, D3D11_INPUT_PER_VERTEX_DATA, 0},

			{"WORLD",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1,  0, D3D11_INPUT_PER_INSTANCE_DATA, 1},
			{"WORLD",    1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 16, D3D11_INPUT_PER_INSTANCE_DATA, 1},
			{"WORLD",    2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 32, D3D11_INPUT_PER_INSTANCE_DATA, 1},
			{"WORLD",    3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 48, D3D11_INPUT_PER_INSTANCE_DATA, 1},
		};
	};

	typedef struct tagVertexAnimationMesh
	{
		XMFLOAT3		vPosition;
		XMFLOAT3		vNormal;
		XMFLOAT2		vTexcoord;
		XMFLOAT2		vTexcoord1;
		XMFLOAT2		vTexcoord2;
		XMFLOAT2		vTexcoord3;

		XMFLOAT4		vTangent;
		XMFLOAT4		vBinormal;

		XMUINT4			vBlendIndex;
		XMFLOAT4		vBlendWeight;

		static const unsigned int		iNumElements = { 10 };

		static constexpr D3D11_INPUT_ELEMENT_DESC   Elements[] = {
			{"POSITION",	0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"NORMAL",		0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"TEXCOORD",    0, DXGI_FORMAT_R32G32_FLOAT,	   0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"TEXCOORD",    1, DXGI_FORMAT_R32G32_FLOAT,	   0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"TEXCOORD",    2, DXGI_FORMAT_R32G32_FLOAT,	   0, 40, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"TEXCOORD",    3, DXGI_FORMAT_R32G32_FLOAT,	   0, 48, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"TANGENT",		0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 56, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"BINORMAL",	0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 72, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"BLENDINDEX",  0, DXGI_FORMAT_R32G32B32A32_UINT,  0, 88, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"BLENDWEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 104, D3D11_INPUT_PER_VERTEX_DATA, 0},
		};
	}VTXANIMMESH;

	typedef struct tagVertexMapMesh
	{
		XMFLOAT3          vPosition;
		XMFLOAT3          vNormal;
		XMFLOAT2          vTexcoord;
		XMFLOAT2          vTexcoord1;
		XMFLOAT2          vTexcoord2;
		XMFLOAT2          vTexcoord3;
		XMFLOAT4          vTangent;
		XMFLOAT4          vBinormal;
		XMFLOAT4          vColor;
		XMFLOAT4          vColor1;
		XMFLOAT4          vColor2;

		static const unsigned int         iNumElements = { 11 };

		static constexpr D3D11_INPUT_ELEMENT_DESC   Elements[] = {
				{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0,   0, D3D11_INPUT_PER_VERTEX_DATA, 0},
				{"NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT,    0,  12, D3D11_INPUT_PER_VERTEX_DATA, 0},
				{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,               0,  24, D3D11_INPUT_PER_VERTEX_DATA, 0},
				{"TEXCOORD", 1, DXGI_FORMAT_R32G32_FLOAT,               0,  32, D3D11_INPUT_PER_VERTEX_DATA, 0},
				{"TEXCOORD", 2, DXGI_FORMAT_R32G32_FLOAT,               0,  40, D3D11_INPUT_PER_VERTEX_DATA, 0},
				{"TEXCOORD", 3, DXGI_FORMAT_R32G32_FLOAT,               0,  48, D3D11_INPUT_PER_VERTEX_DATA, 0},
				{"TANGENT",  0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0,  56, D3D11_INPUT_PER_VERTEX_DATA, 0},
				{"BINORMAL", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0,  72, D3D11_INPUT_PER_VERTEX_DATA, 0},
				{"COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0,  88, D3D11_INPUT_PER_VERTEX_DATA, 0},
				{"COLOR",    1, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 104, D3D11_INPUT_PER_VERTEX_DATA, 0},
				{"COLOR",    2, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 120, D3D11_INPUT_PER_VERTEX_DATA, 0},
		};
	}VTXMAPMESH;

	typedef struct tagVertexParticleInstance
	{
		XMFLOAT4			vRight;
		XMFLOAT4			vUp;
		XMFLOAT4			vLook;
		XMFLOAT4			vTranslation;
		XMFLOAT2			vLifeTime;
	}VTXPARTICLE_INSTANCE;

	typedef struct tagVertexRectInstanceDesc
	{
		static const unsigned int		iNumElements = { 7 };

		static constexpr D3D11_INPUT_ELEMENT_DESC   Elements[] = {
			{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},

			{"TEXCOORD", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1},
			{"TEXCOORD", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 16, D3D11_INPUT_PER_INSTANCE_DATA, 1},
			{"TEXCOORD", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 32, D3D11_INPUT_PER_INSTANCE_DATA, 1},
			{"TEXCOORD", 4, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 48, D3D11_INPUT_PER_INSTANCE_DATA, 1},
			{"TEXCOORD", 5, DXGI_FORMAT_R32G32_FLOAT, 1, 64, D3D11_INPUT_PER_INSTANCE_DATA, 1},
		};
	}VTXRECT_INSTANCE_DESC;

	typedef struct tagVertexPointInstanceDesc
	{
		static const unsigned int		iNumElements = { 6 };

		static constexpr D3D11_INPUT_ELEMENT_DESC   Elements[] = {
			{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},

			{"WORLD", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1},
			{"WORLD", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 16, D3D11_INPUT_PER_INSTANCE_DATA, 1},
			{"WORLD", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 32, D3D11_INPUT_PER_INSTANCE_DATA, 1},
			{"WORLD", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 48, D3D11_INPUT_PER_INSTANCE_DATA, 1},

			{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 1, 64, D3D11_INPUT_PER_INSTANCE_DATA, 1},
		};
	}VTXPOINT_INSTANCE_DESC;

	typedef struct tagVertexTrail
	{
		XMFLOAT3          vPosition;
		XMFLOAT2          vTexcoord;     /* x: 길이(0=꼬리,1=선두), y: 폭(0=base,1=tip) */
		float             fAge;          /* 정점 생성 후 경과 시간(초) */

		static const unsigned int         iNumElements = { 3 };

		static constexpr D3D11_INPUT_ELEMENT_DESC   Elements[] = {
			{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"TEXCOORD", 1, DXGI_FORMAT_R32_FLOAT,       0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0},
		};
	}VTXTRAIL;
}


#endif // Engine_Struct_h__
