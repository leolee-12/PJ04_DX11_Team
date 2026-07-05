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

	inline constexpr SHADER_DESC Shader_MtrlTest = { ETOUI(LEVEL::STATIC), TEXT("Prototype_Component_Shader_MaterialTest"), TEXT("../Bin/ShaderFiles/Shader_MaterialTest.hlsl") };

	inline constexpr SHADER_DESC Shader_VtxTex = { ETOUI(LEVEL::STATIC), TEXT("Prototype_Component_Shader_VtxTex"), TEXT("../Bin/ShaderFiles/Shader_VtxTex.hlsl") };
	inline constexpr SHADER_DESC Shader_VtxAnimMesh = { ETOUI(LEVEL::STATIC), TEXT("Prototype_Component_Shader_VtxAnimMesh"), TEXT("../Bin/ShaderFiles/Shader_VtxAnimMesh.hlsl") };
	inline constexpr SHADER_DESC Shader_VtxMesh = { ETOUI(LEVEL::STATIC), TEXT("Prototype_Component_Shader_VtxMesh"), TEXT("../Bin/ShaderFiles/Shader_VtxMesh.hlsl") };
	inline constexpr SHADER_DESC Shader_VtxNorTex = { ETOUI(LEVEL::STATIC), TEXT("Prototype_Component_Shader_VtxNorTex"), TEXT("../Bin/ShaderFiles/Shader_VtxNorTex.hlsl") };
	inline constexpr SHADER_DESC Shader_Kirby = { ETOUI(LEVEL::STATIC), TEXT("Prototype_Component_Shader_Kirby"), TEXT("../Bin/ShaderFiles/Shader_Kirby.hlsl") };
	inline constexpr SHADER_DESC Shader_Gorilla = { ETOUI(LEVEL::STATIC), TEXT("Prototype_Component_Shader_Gorilla"), TEXT("../Bin/ShaderFiles/Shader_Gorilla.hlsl") };
	inline constexpr SHADER_DESC Shader_Monster = { ETOUI(LEVEL::STATIC), TEXT("Prototype_Component_Shader_Monster"), TEXT("../Bin/ShaderFiles/Shader_Monster.hlsl") };
	inline constexpr SHADER_DESC Shader_Bomb = { ETOUI(LEVEL::STATIC), TEXT("Prototype_Component_Shader_Bomb"), TEXT("../Bin/ShaderFiles/Shader_Bomb.hlsl") };
	inline constexpr SHADER_DESC Shader_EffectRock = { ETOUI(LEVEL::STATIC), TEXT("Prototype_Component_Shader_EffectRock"), TEXT("../Bin/ShaderFiles/Shader_EffectRock.hlsl") };
	inline constexpr SHADER_DESC Shader_Ring = { ETOUI(LEVEL::STATIC), TEXT("Prototype_Component_Shader_Ring"), TEXT("../Bin/ShaderFiles/Shader_Ring.hlsl") };


	inline constexpr SHADER_DESC Shader_AnimMesh_PBR = { ETOUI(LEVEL::STATIC), TEXT("Prototype_Component_Shader_AnimMesh_PBR"), TEXT("../Bin/ShaderFiles/Shader_AnimMesh_PBR.hlsl") };
	inline constexpr SHADER_DESC Shader_NonAnimMesh_PBR = { ETOUI(LEVEL::STATIC), TEXT("Prototype_Component_Shader_NonAnimMesh_PBR"), TEXT("../Bin/ShaderFiles/Shader_NonAnim_PBR.hlsl") };
	inline constexpr SHADER_DESC Shader_Map = { ETOUI(LEVEL::STATIC), TEXT("Prototype_Component_Shader_Map"), TEXT("../Bin/ShaderFiles/Shader_Map.hlsl") };
	inline constexpr SHADER_DESC Shader_MapEx = { ETOUI(LEVEL::STATIC), TEXT("Prototype_Component_Shader_MapEx"), TEXT("../Bin/ShaderFiles/Shader_MapEx.hlsl") };
	inline constexpr SHADER_DESC Shader_EnvInstance = { ETOUI(LEVEL::STATIC), TEXT("Prototype_Component_Shader_EnvInstance"), TEXT("../Bin/ShaderFiles/Shader_EnvInstance.hlsl") };
	inline constexpr SHADER_DESC Shader_SkySphere = { ETOUI(LEVEL::STATIC), TEXT("Prototype_Component_Shader_SkySphere"), TEXT("../Bin/ShaderFiles/Shader_SkySphere.hlsl") };

	inline constexpr SHADER_DESC Shader_UI = { ETOUI(LEVEL::STATIC), TEXT("Prototype_Component_Shader_UI"), TEXT("../Bin/ShaderFiles/Shader_UI.hlsl") };


	//Sample
	inline constexpr TEXTURE_DESC Texture_LoadingUI = { ETOUI(LEVEL::STATIC), TEXT("Prototype_Component_Texture_LoadingUI"), TEXT("../../Resources/Textures/LoadingUI/LoadingUI_%d.png"), 3 };
	inline constexpr COMPONENT_DESC Model_Slash = { ETOUI(LEVEL::STATIC), TEXT("Prototype_Component_Model_Slash") };

	// Sample Mask
	inline constexpr TEXTURE_DESC Texture_TestMask = { ETOUI(LEVEL::STATIC), TEXT("Prototype_Component_Texture_Texture_TestMask"), TEXT("../../Resources/Test/Test/SmokeSphereOriginal/TestMask.png"), 1 };

	// Inhale Mask
	inline constexpr TEXTURE_DESC Texture_Wind01 = { ETOUI(LEVEL::STATIC), TEXT("Prototype_Component_Texture_Wind01"), TEXT("../../Resources/YSE/Effect/InhaleEffect2M/wind01.png"), 1 };
	inline constexpr TEXTURE_DESC Texture_Wind02 = { ETOUI(LEVEL::STATIC), TEXT("Prototype_Component_Texture_Wind02"), TEXT("../../Resources/YSE/Effect/InhaleEffect2M/wind02.png"), 1 };

	// SwordSlash1
	inline constexpr TEXTURE_DESC Texture_Common_Ring02 = { ETOUI(LEVEL::STATIC), TEXT("Prototype_Component_Texture_Common_Ring02"), TEXT("../../Resources/YSE/Effect/Common_Ring03/common_ring02.png"), 1 };
	inline constexpr TEXTURE_DESC Texture_SwordSlash2 = { ETOUI(LEVEL::STATIC), TEXT("Prototype_Component_Texture_SwordSlash2"), TEXT("../../Resources/YSE/Effect/Common_Ring03/SwordSlash2.png"), 1 };

	// JumpSlash
	inline constexpr TEXTURE_DESC Texture_Common_JumpSlash = { ETOUI(LEVEL::STATIC), TEXT("Prototype_Component_Texture_Common_JumpSlash"), TEXT("../../Resources/YSE/Effect/Common_JumpSlash/common_ring02.png"), 1 };
	inline constexpr TEXTURE_DESC Texture_Common_Flash02 = { ETOUI(LEVEL::STATIC), TEXT("Prototype_Component_Texture_Common_Flash02"), TEXT("../../Resources/YSE/Effect/Common_Curve03/common_flash02.png"), 1 };

	// SpinSlash
	inline constexpr TEXTURE_DESC Texture_Common_SpinSlash_1 = { ETOUI(LEVEL::STATIC), TEXT("Prototype_Component_Texture_Common_SpinSlash_1"), TEXT("../../Resources/YSE/Effect/Common_SpinSlash/common_ring02.png"), 1 };
	inline constexpr TEXTURE_DESC Texture_Common_SpinSlash_2 = { ETOUI(LEVEL::STATIC), TEXT("Prototype_Component_Texture_Common_SpinSlash_2"), TEXT("../../Resources/YSE/Effect/Common_SpinSlash/SpinSlash_2.png"), 1 };
	// SpinSlashTrail
	inline constexpr TEXTURE_DESC Texture_Common_SpinSlashTrail = { ETOUI(LEVEL::STATIC), TEXT("Prototype_Component_Texture_Common_SpinSlashTrail"), TEXT("../../Resources/YSE/Effect/Common_SpinSlashTrail/common_ring02.png"), 1 };
	// Charge
	inline constexpr TEXTURE_DESC Texture_ChargeNoise = { ETOUI(LEVEL::STATIC), TEXT("Prototype_Component_Texture_ChargeNoise"), TEXT("../../Resources/YSE/Effect/SwordCharge/ChargeNoise.png"), 1 };
	// MilkyWayMask
	inline constexpr TEXTURE_DESC Texture_MilkyWayMask = { ETOUI(LEVEL::STATIC), TEXT("Prototype_Component_Texture_MilkyWayMask"), TEXT("../../Resources/YSE/Effect/CarMilkyWay/MilkyWayMask.png"), 1 };
	inline constexpr TEXTURE_DESC Texture_Gradiant = { ETOUI(LEVEL::STATIC), TEXT("Prototype_Component_Texture_Gradiant"), TEXT("../../Resources/YSE/Effect/CarMilkyWay/Gradiant.png"), 1 };

	// Star2D
	inline constexpr TEXTURE_DESC Texture_Star2D = { ETOUI(LEVEL::STATIC), TEXT("Prototype_Component_Texture_Star2D"), TEXT("../../Resources/YSE/Effect/Star2D/common_star01.png"), 1 };

	// Hit 
	inline constexpr TEXTURE_DESC Texture_CommonHit01 = { ETOUI(LEVEL::STATIC), TEXT("Prototype_Component_Texture_CommonHit01"), TEXT("../../Resources/CHJ/Effect/Damage/common_hit01.png"), 1 };
	inline constexpr TEXTURE_DESC Texture_CommonRing01 = { ETOUI(LEVEL::STATIC), TEXT("Prototype_Component_Texture_CommonRing01"), TEXT("../../Resources/CHJ/Effect/Common/common_ring01.dds"), 1 };

	//sky
	inline constexpr COMPONENT_DESC Model_SkyTest = { ETOUI(LEVEL::STATIC), TEXT("Prototype_Component_Model_SkyTest") };

	// Collider
	inline constexpr COMPONENT_DESC Collider_Sphere = { ETOUI(LEVEL::STATIC), TEXT("Prototype_Component_Collider_Sphere") };
	inline constexpr COMPONENT_DESC Collider_AABB = { ETOUI(LEVEL::STATIC), TEXT("Prototype_Component_Collider_AABB") };
	inline constexpr COMPONENT_DESC Collider_OBB = { ETOUI(LEVEL::STATIC), TEXT("Prototype_Component_Collider_OBB") };
	inline constexpr COMPONENT_DESC Collider_Capsule = { ETOUI(LEVEL::STATIC), TEXT("Prototype_Component_Collider_Capsule") };
}

using namespace ProtoDesc;
