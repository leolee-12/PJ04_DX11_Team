#pragma once

#include "GameContent_Defines.h"
#include "Engine_Defines.h"

typedef struct tagComponentDesc
{
	_uint iLevelID;
	const _tchar* szProtoTag;
} COMPONENT_DESC;

typedef struct tagShaderDesc
{
	_uint iLevelID;
	const _tchar* szProtoTag;
	const _tchar* szFileTag;
} SHADER_DESC;

typedef struct tagTexDesc
{
	_uint iLevelID;
	const _tchar* szProtoTag;
	const _tchar* szFileTag;
	_uint  iNumTex;
} TEXTURE_DESC;



namespace ProtoDesc
{
	inline constexpr COMPONENT_DESC VI_Rect = { ETOUI(LEVEL::STATIC), TEXT("Prototype_Component_VIBuffer_Rect") };
	inline constexpr COMPONENT_DESC VI_Trail = { ETOUI(LEVEL::STATIC), TEXT("Prototype_Component_VIBuffer_Trail") };
	inline constexpr COMPONENT_DESC VI_Point = { ETOUI(LEVEL::STATIC), TEXT("Prototype_Component_VIBuffer_Point") };
	inline constexpr COMPONENT_DESC VI_CrackInstance = { ETOUI(LEVEL::STATIC), TEXT("Prototype_Component_VIBuffer_CrackInstance") };

	inline constexpr SHADER_DESC Shader_VtxTex = { ETOUI(LEVEL::STATIC), TEXT("Prototype_Component_Shader_VtxTex"), TEXT("../Bin/ShaderFiles/Shader_VtxTex.hlsl") };
	inline constexpr SHADER_DESC Shader_VtxAnimMesh = { ETOUI(LEVEL::STATIC), TEXT("Prototype_Component_Shader_VtxAnimMesh"), TEXT("../Bin/ShaderFiles/Shader_VtxAnimMesh.hlsl") };
	inline constexpr SHADER_DESC Shader_VtxMesh = { ETOUI(LEVEL::STATIC), TEXT("Prototype_Component_Shader_VtxMesh"), TEXT("../Bin/ShaderFiles/Shader_VtxMesh.hlsl") };
	inline constexpr SHADER_DESC Shader_VtxNorTex = { ETOUI(LEVEL::STATIC), TEXT("Prototype_Component_Shader_VtxNorTex"), TEXT("../Bin/ShaderFiles/Shader_VtxNorTex.hlsl") };


	inline constexpr SHADER_DESC Shader_AnimMesh_PBR = { ETOUI(LEVEL::STATIC), TEXT("Prototype_Component_Shader_AnimMesh_PBR"), TEXT("../Bin/ShaderFiles/Shader_AnimMesh_PBR.hlsl") };
	inline constexpr SHADER_DESC Shader_NonAnimMesh_PBR = { ETOUI(LEVEL::STATIC), TEXT("Prototype_Component_Shader_NonAnimMesh_PBR"), TEXT("../Bin/ShaderFiles/Shader_NonAnim_PBR.hlsl") };
	inline constexpr SHADER_DESC Shader_Map = { ETOUI(LEVEL::STATIC), TEXT("Prototype_Component_Shader_Map"), TEXT("../Bin/ShaderFiles/Shader_Map.hlsl") };

	inline constexpr SHADER_DESC Shader_UI = { ETOUI(LEVEL::STATIC), TEXT("Prototype_Component_Shader_UI"), TEXT("../Bin/ShaderFiles/Shader_UI.hlsl") };




	//Sample
	inline constexpr TEXTURE_DESC Texture_LoadingUI = { ETOUI(LEVEL::LOADING), TEXT("Prototype_Component_Texture_LoadingUI"), TEXT("../../Resources/Textures/LoadingUI/LoadingUI_%d.png"), 3 };
	inline constexpr COMPONENT_DESC Model_Slash = { ETOUI(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Model_Slash") };
	
	// Sample Mask
	inline constexpr TEXTURE_DESC Texture_Common_Flash02 = { ETOUI(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Texture_Common_Flash02"), TEXT("../../Resources/Models/Test/SmokeSphereOriginal/common_flash02.png"), 1 };
	inline constexpr TEXTURE_DESC Texture_TestMask = { ETOUI(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Texture_Texture_TestMask"), TEXT("../../Resources/Models/Test/SmokeSphereOriginal/TestMask.png"), 1 };

}

using namespace ProtoDesc;