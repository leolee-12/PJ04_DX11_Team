#include "Singleton_Destroyer.h"
#include "GameObject_Factory.h"
#include "Effect_Loader.h"
#include "Projectile_Manager.h"
#include "Mission_Manager.h"
#include "Bubble_Manager.h"
#include "DropStar_Manager.h"


void CSingleton_Destroyer::Destroy_GameContent_Singletons()
{
	CGameObject_Factory::DestroyInstance();
	CEffect_Loader::DestroyInstance();
	CProjectile_Manager::DestroyInstance();
	CMissionManager::DestroyInstance();
	CBubble_Manager::DestroyInstance();
	CDropStar_Manager::DestroyInstance();
}
