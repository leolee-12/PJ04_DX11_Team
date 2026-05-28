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
	inline constexpr SHADER_DESC Shader_Trail = { ETOUI(LEVEL::STATIC), TEXT("Prototype_Component_Shader_Trail"), TEXT("../Bin/ShaderFiles/Shader_Trail.hlsl") };
	inline constexpr SHADER_DESC Shader_Slash = { ETOUI(LEVEL::STATIC), TEXT("Prototype_Component_Shader_Slash"), TEXT("../Bin/ShaderFiles/Shader_Slash.hlsl") };
	inline constexpr SHADER_DESC Shader_Sprite = { ETOUI(LEVEL::STATIC), TEXT("Prototype_Component_Shader_Sprite"), TEXT("../Bin/ShaderFiles/Shader_Sprite.hlsl") };
	inline constexpr SHADER_DESC Shader_SlashFire = { ETOUI(LEVEL::STATIC), TEXT("Prototype_Component_Shader_SlashFire"), TEXT("../Bin/ShaderFiles/Shader_SlashFire.hlsl") };
	inline constexpr SHADER_DESC Shader_SlashGeo = { ETOUI(LEVEL::STATIC), TEXT("Prototype_Component_Shader_SlashGeo"),     TEXT("../Bin/ShaderFiles/Shader_SlashGeo.hlsl") };
	inline constexpr SHADER_DESC Shader_SlashGeoFire = { ETOUI(LEVEL::STATIC),TEXT("Prototype_Component_Shader_SlashGeoFire"), TEXT("../Bin/ShaderFiles/Shader_SlashGeoFire.hlsl") };
	inline constexpr SHADER_DESC Shader_GroundDecal = { ETOUI(LEVEL::STATIC), TEXT("Prototype_Component_Shader_GroundDecal"), TEXT("../Bin/ShaderFiles/Shader_GroundDecal.hlsl") };
	inline constexpr SHADER_DESC Shader_MagicCircle = { ETOUI(LEVEL::STATIC), TEXT("Prototype_Component_Shader_MagicCircle"), TEXT("../Bin/ShaderFiles/Shader_MagicCircle.hlsl") };
	inline constexpr SHADER_DESC Shader_CrackBurst = { ETOUI(LEVEL::STATIC), TEXT("Prototype_Component_Shader_CrackBurst"), TEXT("../Bin/ShaderFiles/Shader_CrackBurst.hlsl") };
	inline constexpr SHADER_DESC Shader_Skill4Fan = { ETOUI(LEVEL::STATIC), TEXT("Prototype_Component_Shader_Skill4Fan"), TEXT("../Bin/ShaderFiles/Shader_Skill4Fan.hlsl") };
	inline constexpr SHADER_DESC  Shader_Skill4ChargeRing = { ETOUI(LEVEL::STATIC), TEXT("Prototype_Component_Shader_Skill4ChargeRing"), TEXT("../Bin/ShaderFiles/Shader_Skill4ChargeRing.hlsl") };

	inline constexpr TEXTURE_DESC Texture_LoadingUI = { ETOUI(LEVEL::LOADING), TEXT("Prototype_Component_Texture_LoadingUI"), TEXT("../../Resources/Textures/LoadingUI/LoadingUI_%d.png"), 3 };

	inline constexpr TEXTURE_DESC Texture_TitleUI = { ETOUI(LEVEL::LOGO), TEXT("Prototype_Component_Texture_TitleUI"), TEXT("../../Resources/Textures/Title/Title_Logo_%d.png"), 2 };

	inline constexpr TEXTURE_DESC Texture_LobbyBtn = { ETOUI(LEVEL::LOBBY), TEXT("Prototype_Component_Texture_LobbyBtn"), TEXT("../../Resources/Textures/LobbyUI/LobbyBtn.png"), 1 };
	inline constexpr TEXTURE_DESC Texture_GameStartBtn = { ETOUI(LEVEL::LOBBY), TEXT("Prototype_Component_Texture_GameStartBtn"), TEXT("../../Resources/Textures/LobbyUI/GameStartBtn.png"), 1 };
	inline constexpr TEXTURE_DESC Texture_GSBtn_Middle = { ETOUI(LEVEL::LOBBY), TEXT("Prototype_Component_Texture_GSBtn_Middle"), TEXT("../../Resources/Textures/LobbyUI/GSBtn_Middle.png"), 1 };

	inline constexpr TEXTURE_DESC Texture_Lobby_BackGround = { ETOUI(LEVEL::LOBBY), TEXT("Prototype_Component_Texture_Lobby_BackGround"), TEXT("../../Resources/Textures/LobbyUI/Lobby_BackGround_%d.png"), 2 };
	inline constexpr TEXTURE_DESC Texture_LobbyView_BackGround = { ETOUI(LEVEL::LOBBY), TEXT("Prototype_Component_Texture_LobbyView_BackGround"), TEXT("../../Resources/Textures/LobbyUI/View_BackGround.png"), 1 };

	inline constexpr TEXTURE_DESC Texture_ItemIcon = { ETOUI(LEVEL::LOBBY), TEXT("Prototype_Component_Texture_ItemIcon"), TEXT("../../Resources/Textures/ItemUI/Icon/ItemIcon_%d.png"), 88 };
	inline constexpr TEXTURE_DESC Texture_ItemSlot = { ETOUI(LEVEL::LOBBY), TEXT("Prototype_Component_Texture_ItemSlot"), TEXT("../../Resources/Textures/ItemUI/Slot/Item_Slot_%d.png"), 8 };
	inline constexpr TEXTURE_DESC Texture_CategoryIcon = { ETOUI(LEVEL::LOBBY), TEXT("Prototype_Component_Texture_CategoryIcon"), TEXT("../../Resources/Textures/ItemUI/Category/Category_%d.png"), 6 };

	inline constexpr TEXTURE_DESC Texture_SkinIcon_Back = { ETOUI(LEVEL::LOBBY), TEXT("Prototype_Component_Texture_SkinIcon_Back"), TEXT("../../Resources/Textures/CharactorUI/SkinUI/SkinSlot_%d.png"), 6 };
	inline constexpr TEXTURE_DESC Texture_SkinIcon_Skin = { ETOUI(LEVEL::LOBBY), TEXT("Prototype_Component_Texture_SkinIcon_Skin"), TEXT("../../Resources/Textures/CharactorUI/SkinUI/Lobby_Hisui_%d.png"), 3 };
	inline constexpr TEXTURE_DESC Texture_SkinIcon_Front = { ETOUI(LEVEL::LOBBY), TEXT("Prototype_Component_Texture_SkinIcon_Front"), TEXT("../../Resources/Textures/CharactorUI/SkinUI/SkinSlot_Frame_%d.png"), 3 };
	inline constexpr TEXTURE_DESC Texture_SkinIcon_Mask = { ETOUI(LEVEL::LOBBY), TEXT("Prototype_Component_Texture_SkinIcon_Mask"), TEXT("../../Resources/Textures/CharactorUI/SkinUI/SkinSlot_Mask.png"), 1 };

	inline constexpr TEXTURE_DESC Texture_SelectIcon_Back = { ETOUI(LEVEL::LOBBY), TEXT("Prototype_Component_Texture_SelectIcon_Back"), TEXT("../../Resources/Textures/CharactorUI/SelectUI/CharactorSlot_%d.png"), 3 };
	inline constexpr TEXTURE_DESC Texture_SelectIcon_Char = { ETOUI(LEVEL::LOBBY), TEXT("Prototype_Component_Texture_SelectIcon_Char"), TEXT("../../Resources/Textures/CharactorUI/SelectUI/Lobby_Hisui.png"), 1 };
	inline constexpr TEXTURE_DESC Texture_Skin2D = { ETOUI(LEVEL::LOBBY), TEXT("Prototype_Component_Texture_Skin2D"), TEXT("../../Resources/Textures/CharactorUI/SkinUI/Lobby_Full_Hisui_%d.png"), 3 };

	inline constexpr TEXTURE_DESC Texture_SelectGameStartBtn = { ETOUI(LEVEL::LOBBY), TEXT("Prototype_Component_Texture_SelectGameStartBtn"), TEXT("../../Resources/Textures/LobbyUI/Select_GameStart_Btn.png"), 1 };

	inline constexpr TEXTURE_DESC Texture_SkillSlot = { ETOUI(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Texture_SkillSlot"), TEXT("../../Resources/Textures/Hud/Skill/SkillSlot_%d.png"), 8 };
	inline constexpr TEXTURE_DESC Texture_SkillUpBtn = { ETOUI(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Texture_SkillUpBtn"), TEXT("../../Resources/Textures/Hud/Skill/LevelUPBtn_%d.png"), 3 };
	inline constexpr TEXTURE_DESC Texture_HisuiProfile = { ETOUI(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Texture_HisuiProfile"), TEXT("../../Resources/Textures/Hud/Hisui_Hud.png"), 1 };
	inline constexpr TEXTURE_DESC Texture_InvenSlot = { ETOUI(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Texture_InvenSlot"), TEXT("../../Resources/Textures/ItemUI/Slot/Ico_ItemGradebg_Empty.png"), 1 };
	inline constexpr TEXTURE_DESC Texture_HudBar = { ETOUI(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Texture_HudBar"), TEXT("../../Resources/Textures/Hud/Gage_%d.png"), 4 };
	inline constexpr TEXTURE_DESC Texture_SpecialStat = { ETOUI(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Texture_SpecialStat"), TEXT("../../Resources/Textures/Hud/SpecialStat_%d.png"), 2 };
	inline constexpr TEXTURE_DESC Texture_NamePlate = { ETOUI(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Texture_NamePlate"), TEXT("../../Resources/Textures/Hud/NamePlate.png"), 1 }; 

	inline constexpr TEXTURE_DESC Texture_Rio_Arrow = { ETOUI(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Texture_Rio_Arrow"), TEXT("../../Resources/Textures/Effect/Rio/FX_BI_HitGlow_01.png"), 1 };
	inline constexpr TEXTURE_DESC Texture_Rio_Arrow_Trail = { ETOUI(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Texture_Rio_Arrow_Trail"), TEXT("../../Resources/Textures/Effect/Rio/FX_BI_Trail_Line01.png"), 1 };
	inline constexpr TEXTURE_DESC Texture_Rio_Arrow_Hit = { ETOUI(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Texture_Rio_Arrow_Hit"), TEXT("../../Resources/Textures/Effect/Rio/FX_BI_Hit_06.png"), 1 };

	inline constexpr TEXTURE_DESC Texture_Hisui_Hit = { ETOUI(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Texture_Hisui_Hit"), TEXT("../../Resources/Textures/Effect/Hisui/FX_BI_Hisui_S002_Hit_01.png"), 1 };

	inline constexpr COMPONENT_DESC Model_Slash = { ETOUI(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Model_Slash") };
	inline constexpr TEXTURE_DESC Texture_Slash = { ETOUI(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Texture_Slash"), TEXT("../../Resources/Textures/Effect/Slash/FX_BI_Slash_02.png"), 1 };
	inline constexpr TEXTURE_DESC Texture_Slash_Flow = { ETOUI(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Texture_Slash_Flow"), TEXT("../../Resources/Textures/Effect/Slash/FX_BI_FlowMap_02.png"), 1 };

	inline constexpr COMPONENT_DESC Model_SlashFire = { ETOUI(LEVEL::GAMEPLAY),TEXT("Prototype_Component_Model_SlashFire") };
	inline constexpr TEXTURE_DESC Texture_Slash_Fire = { ETOUI(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Texture_Slash_Fire"), TEXT("../../Resources/Textures/Effect/Hisui/FX_BI_Hisui_S002_Fire_04_Targa.png"), 1 };
	inline constexpr TEXTURE_DESC Texture_Slash_Fire_OutLine = { ETOUI(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Texture_Slash_Fire_OutLine"), TEXT("../../Resources/Textures/Effect/Hisui/FX_BI_Hisui_S002_Fire_04_Dark.png"), 1 };

	inline constexpr COMPONENT_DESC Model_SlashDonut = { ETOUI(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Model_SlashDonut") };
	inline constexpr COMPONENT_DESC Model_SlashHalf = { ETOUI(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Model_SlashHalf") };

	inline constexpr TEXTURE_DESC Texture_GroundDecal_Base = { ETOUI(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Texture_GroundDecal_Base"), TEXT("../../Resources/Textures/Effect/Hisui/FX_BI_Hisui_Pattern_04_S002.png"), 1 };
	inline constexpr TEXTURE_DESC Texture_GroundDecal_Line = { ETOUI(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Texture_GroundDecal_Line"), TEXT("../../Resources/Textures/Effect/Hisui/FX_BI_Hisui_Pattern_04_S002_01.png"), 1 };
	inline constexpr TEXTURE_DESC Texture_GroundDecal_Mask = { ETOUI(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Texture_GroundDecal_Mask"), TEXT("../../Resources/Textures/Effect/Hisui/FX_BI_Hisui_Pattern_04_S002_Mask.png"), 1 };
	inline constexpr TEXTURE_DESC Texture_GroundDecal_Noise = {	ETOUI(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Texture_GroundDecal_Noise"), TEXT("../../Resources/Textures/Effect/Hisui/FX_BI_Noise_20.png"), 1 };
	inline constexpr TEXTURE_DESC Texture_GroundDecal_Track2_Base = { ETOUI(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Texture_GroundDecal_Track2_Base"), TEXT("../../Resources/Textures/Effect/Hisui/FX_BI_Hisui_S002_Pattern_05.png"), 1 };
	inline constexpr TEXTURE_DESC Texture_GroundDecal_Track2_Line = { ETOUI(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Texture_GroundDecal_Track2_Line"), TEXT("../../Resources/Textures/Effect/Hisui/FX_BI_Hisui_S002_Pattern_05_Line.png"), 1 };
	inline constexpr TEXTURE_DESC Texture_MagicCircle_Line = { ETOUI(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Texture_MagicCircle_Line"), TEXT("../../Resources/Textures/Effect/Hisui/FX_BI_Hisui_S002_Floor_01_Line.png"), 1 }; 
	inline constexpr TEXTURE_DESC Texture_MagicCircle_Tar = { ETOUI(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Texture_MagicCircle_Tar"), TEXT("../../Resources/Textures/Effect/Hisui/FX_BI_Hisui_S002_Floor_01_Tar.png"), 1 }; 

	inline constexpr TEXTURE_DESC Texture_Crack = { ETOUI(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Texture_Crack"), TEXT("../../Resources/Textures/Effect/Hisui/FX_BI_Hisui_S002_Crack_02.png"), 1 };

	inline constexpr TEXTURE_DESC Texture_Skill4Fan_Fan = { ETOUI(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Texture_Skill4Fan_Fan"), TEXT("../../Resources/Textures/Effect/Hisui/FX_BI_Hisui_S002_Pattern_03.png"), 1 };
	inline constexpr TEXTURE_DESC Texture_Skill4Fan_Wing = { ETOUI(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Texture_Skill4Fan_Wing"), TEXT("../../Resources/Textures/Effect/Hisui/FX_BI_Hisui_S002_Pattern_03_01.png"), 1 };
	inline constexpr TEXTURE_DESC Texture_Skill4Fan_Kanji = { ETOUI(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Texture_Skill4Fan_Kanji"), TEXT("../../Resources/Textures/Effect/Hisui/FX_BI_Hisui_S002_Pattern_03_1.png"), 1 };

	inline constexpr TEXTURE_DESC Texture_Skill4Ring_Shape = { ETOUI(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Texture_Skill4Ring_Shape"), TEXT("../../Resources/Textures/Effect/Hisui/FX_BI_Hisui_S002_Pattern_01.png"), 1 };
	inline constexpr TEXTURE_DESC Texture_Skill4Ring_Grad = { ETOUI(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Texture_Skill4Ring_Grad"), TEXT("../../Resources/Textures/Effect/Hisui/FX_BI_Hisui_S002_Pattern_02.png"), 1 };

}

using namespace ProtoDesc;