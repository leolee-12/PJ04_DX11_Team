#pragma once

namespace Engine
{
#pragma pack(push, 1)
	typedef struct tagVertexMeshData
	{
		XMFLOAT3 vPosition;
		XMFLOAT3 vNormal;
		XMFLOAT2 vTexcoord;
		XMFLOAT3 vTangent;
		XMFLOAT3 vBinormal;
	}VTXMESH_DATA;

	typedef struct tagVertexAnimMeshData
	{
		XMFLOAT3 vPosition;
		XMFLOAT3 vNormal;
		XMFLOAT2 vTexcoord;
		XMFLOAT3 vTangent;
		XMFLOAT3 vBinormal;
		XMUINT4  vBlendIndex;   // 메시 로컬 본 인덱스 (0..num_bones-1)
		XMFLOAT4 vBlendWeight;
	}VTXANIMMESH_DATA;

	typedef struct tagKeyFrameData
	{
		XMFLOAT3		vScale;
		XMFLOAT4		vRotation;
		XMFLOAT3		vTranslation;
		float			fTrackPosition;
	}KEYFRAME_DATA;
#pragma pack(pop)

	typedef struct tagBoneData
	{
		string		strName;
		_float4x4	TransformationMatrix;
		_int		iParentIndex;
	}BONE_DATA;

	typedef struct tagMeshBoneData
	{
		string		strName;
		_float4x4	OffsetMatrix;
	}MESH_BONE_DATA;

	typedef struct tagMeshData
	{
		string						strName;
		_uint						iMaterialIndex;
		vector<VTXMESH_DATA>		NonAnimVertices;
		vector<VTXANIMMESH_DATA>	AnimVertices;
		vector<_uint>				Indices;
		vector<MESH_BONE_DATA>		Bones;
	}MESH_DATA;

	typedef struct tagMaterialData
	{
		vector<string> TextureNames[MTEX_TYPE_MAX];
	}MATERIAL_DATA;

	typedef struct tagChannelData
	{
		string                  strBoneName;
		vector<KEYFRAME_DATA>   KeyFrames;   
	}CHANNEL_DATA;

	typedef struct tagAnimationData
	{
		string                 strName;
		_float                 fDuration;
		_float                 fTickPerSecond;
		vector<CHANNEL_DATA>   Channels;
	}ANIMATION_DATA;

	typedef struct tagModelData
	{
		MODEL                  eType;
		vector<BONE_DATA>      Bones;         // 트리를 플래트닝한 배열
		vector<MESH_DATA>      Meshes;
		vector<MATERIAL_DATA>  Materials;
		vector<ANIMATION_DATA>   Animations;
	}MODEL_DATA;


}