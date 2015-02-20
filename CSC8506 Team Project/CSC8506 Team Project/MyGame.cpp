#include "MyGame.h"
#include "InertialMatrixHelper.h"
#include "Spring.h"
#include "../../nclgl/OBJMesh.h"
/*
Creates a really simple scene for our game - A cube robot standing on
a floor. As the module progresses you'll see how to get the robot moving
around in a physically accurate manner, and how to stop it falling
through the floor as gravity is added to the scene.

You can completely change all of this if you want, it's your game!

*/



MyGame::MyGame()	{
	gameCamera = new Camera(-30.0f, 0.0f, Vector3(0, 450, 850));
	Renderer::GetRenderer().SetCamera(gameCamera);

	cube = new OBJMesh(MESHDIR"cube.obj");
	cube->SetTexture(SOIL_load_OGL_texture(TEXTUREDIR"Barren Reds.jpg", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS));
	quad = Mesh::GenerateQuad();
	sphere = new OBJMesh(MESHDIR"sphere.obj");
	sphere->SetTexture(SOIL_load_OGL_texture(TEXTUREDIR"texture_c_z.jpg", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS));


	/*
	for (int i = 0; i < 10; i++)
	{
		for (int j = 0; j < 10; j++){
			allEntities.push_back(BuildSphereEntity(100, Vector3( i * 250,  500, j * 250), Vector3(0, 0, 0)));

		}
		
	}*/
	
	

}

MyGame::~MyGame(void)	{
	/*
	We're done with our assets now, so we can delete them
	*/
	delete cube;
	delete quad;
	delete sphere;
}

/*
Here's the base 'skeleton' of your game update loop! You will presumably
want your games to have some sort of internal logic to them, and this
logic will be added to this function.
*/
void MyGame::UpdateGame(float msec) {
	if (gameCamera) {
		gameCamera->UpdateCamera(msec);
	}

	for (vector<GameEntity*>::iterator i = allEntities.begin(); i != allEntities.end(); ++i) {
		(*i)->Update(msec);
	}

	//press 1, launch projectile
	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_1)) {
		Matrix4 t = gameCamera->BuildViewMatrix();
		Vector3 vel = Vector3(-t.values[2], -t.values[6], -t.values[10]);
		GameEntity* e = BuildSphereEntity(100.0f, gameCamera->GetPosition(), vel);
		e->GetPhysicsNode().SetLinearVelocity(vel);
		e->GetPhysicsNode().SetPosition(gameCamera->GetPosition());
		e->GetPhysicsNode().SetAngularVelocity(vel*0.005f);
		e->GetPhysicsNode().SetInverseMass(1.0f / 10.0f);
		e->GetPhysicsNode().SetInverseInertia(InertialMatrixHelper::createSphereInvInertial(1.0f / 10.0f, 100.0f));

		//PhysicsSystem::GetPhysicsSystem().GetOctree()->add(&(e->GetPhysicsNode()));
		//PhysicsSystem::GetPhysicsSystem().GetNarrowList().push_back(&(e->GetPhysicsNode()));
		//PhysicsSystem::GetPhysicsSystem().GetBalls().push_back(&(e->GetPhysicsNode()));

		allEntities.push_back(e);
	}

	//draw in wireframe mode
	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_F)) {
		Renderer::GetRenderer().wireframe = !Renderer::GetRenderer().wireframe;
	}

	PhysicsSystem::GetPhysicsSystem().DrawDebug();

}

/*
Makes a cube. Every game has a crate in it somewhere!
*/
GameEntity* MyGame::BuildCubeEntity(float size) {
	GameEntity*g = new GameEntity(new SceneNode(cube), new PhysicsNode());
	g->ConnectToSystems();

	SceneNode &test = g->GetRenderNode();

	test.SetModelScale(Vector3(size, size, size));
	test.SetBoundingRadius(size);
	PhysicsNode& p = g->GetPhysicsNode();
	p.SetInverseInertia(InertialMatrixHelper::createCuboidInvInertial(1.0f/10.0f, size, size, size));
	p.SetInverseMass(1.0f / 10.0f);
	p.SetCollisionVolume(new CollisionSphere(Vector3(), size));
	return g;
}
/*
Makes a sphere.
*/
GameEntity* MyGame::BuildSphereEntity(float radius, Vector3 pos, Vector3 vel) {
	SceneNode* s = new SceneNode(sphere);

	s->SetModelScale(Vector3(radius, radius, radius));
	s->SetBoundingRadius(radius);
	s->SetColour(Vector4(0.2f, 0.2f, 0.5f, 1.0f));
	PhysicsNode*p = new PhysicsNode();
	p->SetPosition(pos);
	p->SetLinearVelocity(vel);
	p->SetAngularVelocity(Vector3(0, 0, 0));
	p->SetInverseInertia(InertialMatrixHelper::createSphereInvInertial(1.0f, radius));
	p->SetInverseMass(1.0f);
	p->SetCollisionVolume(new CollisionSphere(pos, radius));
	p->SetRest(false);
	p->SetLinearVelocity(Vector3(0, -1, 0));
	/*
	CollisionSphere& cSphere = *(CollisionSphere*)p->GetCollisionVolume();
	float r = cSphere.GetRadius();
	float xPosition = p->GetPosition().x;

	p->SetBeginNode(xPosition - r);
	p->SetEndNode(xPosition + r);*/

	/*PhysicsSystem::GetPhysicsSystem().GetOctree()->add(p);
	PhysicsSystem::GetPhysicsSystem().GetNarrowList().push_back(p);
	PhysicsSystem::GetPhysicsSystem().GetBalls().push_back(p);*/

	GameEntity*g = new GameEntity(s, p);
	g->ConnectToSystems();
	//PhysicsSystem::GetPhysicsSystem().AddDebugDraw(p->GetCollisionVolume());
	return g;
}

/*
Makes a flat quad, initially oriented such that we can use it as a simple
floor.
*/
GameEntity* MyGame::BuildQuadEntity(float size) {
	SceneNode* s = new SceneNode(quad);

	s->SetModelScale(Vector3(size, size, size));
	//Oh if only we had a set texture function...we could make our brick floor again WINK WINK
	s->SetBoundingRadius(size);

	PhysicsNode*p = new PhysicsNode(Quaternion::AxisAngleToQuaterion(Vector3(1, 0, 0), 90.0f), Vector3());
	p->SetUseGravity(false);
	p->SetPosition(Vector3(0,0,0));
	p->SetInverseMass(0);
	p->SetInverseInertia(InertialMatrixHelper::createImmovableInvInertial());
	p->SetCollisionVolume(new CollisionPlane(Vector3(0,0,0), Vector3(0, 1, 0), 0));
	p->SetLinearVelocity(Vector3(0,0,0));
	GameEntity*g = new GameEntity(s, p);
	g->ConnectToSystems();
	return g;
}