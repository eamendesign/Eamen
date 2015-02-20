#include "GameClass.h"

GameClass* GameClass::instance = NULL;

GameClass::GameClass()	{
	renderCounter = 0.0f;
	physicsCounter = 0.0f;
	elapsed = 0.0f;
	instance = this;
}

GameClass::~GameClass(void)	{
	//for (vector<GameEntity*>::iterator i = allEntities.begin(); i != allEntities.end(); ++i) {
	//	delete (*i);
	//} //just let the OS tear the memory down...
	delete gameCamera;
}

void GameClass::UpdatePhysics(float msec) {
	physicsCounter += msec;
	elapsed += msec; //for our framerate
	while (physicsCounter >= 0.0f) {
		physicsCounter -= PHYSICS_TIMESTEP;
		PhysicsSystem::GetPhysicsSystem().Update(PHYSICS_TIMESTEP);
	}
	//how many updates did we manage in the last 1sec? ONE HUNDRED AND TWENTY I HOPE
	if (elapsed > 1000.0f) {
		PhysicsSystem::numUpdatesPerSec = PhysicsSystem::numUpdates;
		PhysicsSystem::numUpdates = 0;
		elapsed = 0.0f;
	}
}

void GameClass::UpdateRendering(float msec) {
	renderCounter -= msec;

	if (renderCounter <= 0.0f) {	//Update our rendering logic
		Renderer::GetRenderer().UpdateScene(1000.0f / (float)RENDER_HZ);
		Renderer::GetRenderer().RenderScene();
		renderCounter += (1000.0f / (float)RENDER_HZ);
	}
}