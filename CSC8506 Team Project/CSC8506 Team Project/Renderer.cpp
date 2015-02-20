#include "Renderer.h"
#include <sstream>
#include <iomanip>
#include "PhysicsSystem.h"
#include "MyGame.h"



namespace FogParameters
{
	float fDensity = 0.04f;
	float fStart = 10.0f;
	float fEnd = 75.0f;
	Vector4 vFogColor = Vector4(0.5f, 0.5f, 0.5f, 1.0f);
	int iFogEquation = FOG_EQUATION_EXP; // 0 = linear, 1 = exp, 2 = exp2
	int addFog = 0;
};

Renderer* Renderer::instance = NULL;
int Renderer::physics_updates = 0;
int Renderer::renderer_updates = 0;
int Renderer::elapsed_time = 0;
bool Renderer::wireframe = false;
Renderer::Renderer(Window &parent) : OGLRenderer(parent)	{	
	camera			= NULL;
	root			= new SceneNode();

	bool addRain = 0;
	for (int i = 0; i<4; i++)
	{
		emitter.push_back(new ParticleEmitter());
	}

	//simpleShader	= new Shader(SHADERDIR"TechVertex.glsl", SHADERDIR"TechFragment.glsl");
	simpleShader = new Shader(SHADERDIR"LightVertex.glsl", SHADERDIR"LightFragment.glsl");

	if(!simpleShader->LinkProgram() ){
		return;
	}

	texture_shader = new Shader(SHADERDIR"TexturedVertex.glsl", SHADERDIR"TexturedFragment.glsl");
	if (!texture_shader->LinkProgram()) return;

	light = new Light(Vector3(RAW_HEIGHT * HEIGHTMAP_X / 2.0f, 500.0f, (RAW_HEIGHT*HEIGHTMAP_Z / 2.0f)), Vector4(1, 1, 1, 1), 10000.0f);

	heightMap = new HeightMap(TEXTUREDIR"terrain.raw");
	heightMap->SetTexture(SOIL_load_OGL_texture(TEXTUREDIR"Barren Reds.jpg", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS));
	glBindTexture(GL_TEXTURE_2D, heightMap->GetTexture());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);

	basicFont = new Font(SOIL_load_OGL_texture(TEXTUREDIR"tahoma.tga", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_COMPRESS_TO_DXT), 16, 16);

	skybox_shader = new Shader(SHADERDIR"SkyboxVertex.glsl", SHADERDIR"SkyboxFragment.glsl");

	if (addRain == 1)
	particleShader = new Shader(SHADERDIR"vertex.glsl", SHADERDIR"RainFragment.glsl", SHADERDIR"RainGeometry.glsl");
	else
	particleShader = new Shader(SHADERDIR"vertex.glsl", SHADERDIR"fragment.glsl", SHADERDIR"geometry.glsl");

	if (!skybox_shader->LinkProgram() || !particleShader->LinkProgram())
		return;
	skybox_mesh = Mesh::GenerateQuad();
	cubemap_tex = SOIL_load_OGL_cubemap(TEXTUREDIR"rusted_west.jpg", TEXTUREDIR"rusted_east.jpg", TEXTUREDIR"rusted_up.jpg", TEXTUREDIR"rusted_down.jpg", TEXTUREDIR"rusted_south.jpg", TEXTUREDIR"rusted_north.jpg", SOIL_LOAD_RGB, SOIL_CREATE_NEW_ID, 0);
	skybox_mesh->SetTexture(cubemap_tex);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);



	instance		= this;

	init			= true;
}

Renderer::~Renderer(void)	{
	delete root;
	delete simpleShader;
	delete particleShader;
	emitter.clear();
	delete texture_shader;
	currentShader = NULL;
}

void Renderer::UpdateScene(float msec)	{
	renderer_updates++;
	if(camera) {
		camera->UpdateCamera(msec); 
	}

	root->Update(msec);
	emitter[0]->Update(msec);
	

	
	
}

void Renderer::RenderScene()	{
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);

	glPolygonMode(GL_FRONT_AND_BACK, wireframe ? GL_LINE : GL_FILL);


	if(camera) {
		

		viewMatrix = camera->BuildViewMatrix();
		projMatrix = Matrix4::Perspective(1.0f, 10000.0f, (float)width / (float)height, 45.0f);
	
		SetCurrentShader(skybox_shader); // skybox shader
		glDepthMask(GL_FALSE);
		UpdateShaderMatrices();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap_tex);
		glDisable(GL_CULL_FACE);
		skybox_mesh->Draw(false);
		glEnable(GL_CULL_FACE);
		glDepthMask(GL_TRUE);

		SetCurrentShader(simpleShader);


		glUniform1i(glGetUniformLocation(currentShader->GetProgram(), "diffuseTex"), 0);
		glDisable(GL_BLEND);
		textureMatrix.ToIdentity();
		modelMatrix.ToIdentity();
		viewMatrix		= camera->BuildViewMatrix();
		projMatrix		= Matrix4::Perspective(1.0f,10000.0f,(float)width / (float) height, 45.0f);
		frameFrustum.FromMatrix(projMatrix * viewMatrix);
		UpdateShaderMatrices();
		heightMap->Draw();

		//Return to default 'usable' state every frame!
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glDisable(GL_STENCIL_TEST);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		BuildNodeLists(root);
		SortNodeLists();
		DrawNodes();
		ClearNodeLists();
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		SetCurrentShader(texture_shader);
		UpdateShaderMatrices();

		std::stringstream ss;
		//ss << "Physics updates: " << (PhysicsSystem::numUpdates / Window::GetWindow().GetTimer()->GetMS()) * 1000.0f;
		ss << "Physics rate cur: " << PhysicsSystem::numUpdatesPerSec << " avg: " << (PhysicsSystem::numUpdatesTotal / Window::GetWindow().GetTimer()->GetMS()) * 1000.0f;
		DrawText(ss.str() , Vector3(0, 0, 0), 16.0f);
		ss.clear(); ss.str(std::string());

		ss << "Renderer updates: " << (renderer_updates / Window::GetWindow().GetTimer()->GetMS())* 1000.0f;
		DrawText(ss.str(), Vector3(0, 16.0f, 0), 16.0f);
		ss.clear(); ss.str(std::string());

		ss << "Elapsed time: " << (int)(Window::GetWindow().GetTimer()->GetMS() * 0.001f);
		DrawText(ss.str(), Vector3(0, 32.0f, 0), 16.0f);
		ss.clear(); ss.str(std::string());

		ss << "Physics nodes: " << PhysicsSystem::numNodes;
		DrawText(ss.str(), Vector3(0, 48.0f, 0), 16.0f);
		ss.clear(); ss.str(std::string());

		ss << "Collisions detected: " << PhysicsSystem::numCollisions;
		DrawText(ss.str(), Vector3(0, 80.0f, 0), 16.0f);
		ss.clear(); ss.str(std::string());

		Vector3 pos = camera->GetPosition();
		ss << "Camera position: <" << std::setprecision(1) << (int)pos.x << "," << (int)pos.y << "," << (int)pos.z << ">";
		DrawText(ss.str(), Vector3(0, 96.0f, 0), 16.0f);
		ss.clear(); ss.str(std::string());

		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		projMatrix = Matrix4::Perspective(1.0f, 10000.0f, (float)width / (float)height, 45.0f);
		viewMatrix = camera->BuildViewMatrix();

		//glDisable(GL_BLEND);
		drawEmitter_1();
		//drawEmitter_2();

		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	}

	

	glUseProgram(0);
	SwapBuffers();
}

void	Renderer::UpdateShaderMatrices() {
	if (currentShader && light) {
		glUniform3fv(glGetUniformLocation(currentShader->GetProgram(), "lightPos"), 1, (float*)&light->GetPosition());
		glUniform4fv(glGetUniformLocation(currentShader->GetProgram(), "lightColour"), 1, (float*)&light->GetColour());
		glUniform1f(glGetUniformLocation(currentShader->GetProgram(), "lightRadius"), light->GetRadius());
		glUniform3fv(glGetUniformLocation(currentShader->GetProgram(), "cameraPos"), 1, (float*)&camera->GetPosition());
		
		//glUniform1f(glGetUniformLocation(currentShader->GetProgram(), "fogParams.iEquation"), FogParameters::iFogEquation);
		glUniform4fv(glGetUniformLocation(currentShader->GetProgram(), "FogColour"), 1, (GLfloat*)&FogParameters::vFogColor);
		glUniform1i(glGetUniformLocation(currentShader->GetProgram(), "addFog"), FogParameters::addFog);

	/*	if (FogParameters::iFogEquation == FOG_EQUATION_LINEAR)
		{
			glUniform1f(glGetUniformLocation(currentShader->GetProgram(), "fogParams.fStart"), FogParameters::fStart);
			glUniform1f(glGetUniformLocation(currentShader->GetProgram(), "fogParams.fEnd"), FogParameters::fEnd);

		}
		else
			glUniform1f(glGetUniformLocation(currentShader->GetProgram(), "fogParams.fDensity"), FogParameters::fDensity);*/

	}
	OGLRenderer::UpdateShaderMatrices();
}

void	Renderer::DrawNode(SceneNode*n)	{
	if(n->GetMesh()) {
		glUniformMatrix4fv(glGetUniformLocation(currentShader->GetProgram(), "modelMatrix"),	1,false, (float*)&(n->GetWorldTransform()*Matrix4::Scale(n->GetModelScale())));
		glUniform4fv(glGetUniformLocation(currentShader->GetProgram(), "nodeColour"),1,(float*)&n->GetColour());

		n->Draw(*this);
	}
}

void	Renderer::BuildNodeLists(SceneNode* from)	{
	Vector3 direction = from->GetWorldTransform().GetPositionVector() - camera->GetPosition();
	from->SetCameraDistance(Vector3::Dot(direction,direction));

	if(frameFrustum.InsideFrustum(*from)) {
		if(from->GetColour().w < 1.0f) {
			transparentNodeList.push_back(from);
		}
		else{
			nodeList.push_back(from);
		}
	}

	for(vector<SceneNode*>::const_iterator i = from->GetChildIteratorStart(); i != from->GetChildIteratorEnd(); ++i) {
		BuildNodeLists((*i));
	}
}

void	Renderer::DrawNodes()	 {
	for(vector<SceneNode*>::const_iterator i = nodeList.begin(); i != nodeList.end(); ++i ) {
		DrawNode((*i));
	}

	for(vector<SceneNode*>::const_reverse_iterator i = transparentNodeList.rbegin(); i != transparentNodeList.rend(); ++i ) {
		DrawNode((*i));
	}
}

void	Renderer::SortNodeLists()	{
	std::sort(transparentNodeList.begin(),	transparentNodeList.end(),	SceneNode::CompareByCameraDistance);
	std::sort(nodeList.begin(),				nodeList.end(),				SceneNode::CompareByCameraDistance);
}

void	Renderer::ClearNodeLists()	{
	transparentNodeList.clear();
	nodeList.clear();
}

void	Renderer::SetCamera(Camera*c) {
	camera = c;
}

void	Renderer::AddNode(SceneNode* n) {
	root->AddChild(n);
}

void	Renderer::RemoveNode(SceneNode* n) {
	root->RemoveChild(n);
}

/*
Draw a line of text on screen. If we were to have a 'static' line of text, we'd
probably want to keep the TextMesh around to save processing it every frame,
but for a simple demonstration, this is fine...
*/
void Renderer::DrawText(const std::string &text, const Vector3 &position, const float size, const bool perspective)	{
	//Create a new temporary TextMesh, using our line of text and our font
	TextMesh* mesh = new TextMesh(text, *basicFont);

	//This just does simple matrix setup to render in either perspective or
	//orthographic mode, there's nothing here that's particularly tricky.
	if (perspective) {
		modelMatrix = Matrix4::Translation(position) * Matrix4::Scale(Vector3(size, size, 1));
		viewMatrix = camera->BuildViewMatrix();
		projMatrix = Matrix4::Perspective(1.0f, 10000.0f, (float)width / (float)height, 45.0f);
	}
	else{
		//In ortho mode, we subtract the y from the height, so that a height of 0
		//is at the top left of the screen, which is more intuitive
		//(for me anyway...)
		modelMatrix = Matrix4::Translation(Vector3(position.x, height - position.y, position.z)) * Matrix4::Scale(Vector3(size, size, 1));
		viewMatrix.ToIdentity();
		projMatrix = Matrix4::Orthographic(-1.0f, 1.0f, (float)width, 0.0f, (float)height, 0.0f);
	}
	//Either way, we update the matrices, and draw the mesh
	UpdateShaderMatrices();
	mesh->Draw();

	delete mesh; //Once it's drawn, we don't need it anymore!
}

void Renderer::drawEmitter_1()
{
	SetCurrentShader(particleShader);
	glUseProgram(currentShader->GetProgram());
	glUniform1i(glGetUniformLocation(currentShader->GetProgram(), "diffuseTex"), 0);
	SetShaderParticleSize(emitter[0]->GetParticleSize());
	//emitter[0]->SetParticleSize(18.0f);
	//emitter[0]->SetParticleVariance(1.0f);
	//emitter[0]->SetLaunchParticles(16.0f);
	emitter[0]->SetParticleLifetime(20000.0f);
	
	//emitter[0]->SetParticleSpeed(0.05f);

	modelMatrix =
		Matrix4::Translation(Vector3(2000, 1000, 2000)) *
		Matrix4::Scale(Vector3(10, 10, 10)) *
		Matrix4::Rotation(0, Vector3(1.0f, 0.0f, 0.0f));

	UpdateShaderMatrices();

	emitter[0]->Draw();

	glUseProgram(0);
}

void Renderer::SetShaderParticleSize(float f) {
	glUniform1f(glGetUniformLocation(currentShader->GetProgram(), "particleSize"), f);
}

void Renderer::drawEmitter_2()
{


	SetCurrentShader(particleShader);
	glUseProgram(currentShader->GetProgram());
	glUniform1i(glGetUniformLocation(currentShader->GetProgram(), "diffuseTex"), 0);
	SetShaderParticleSize(emitter[1]->GetParticleSize());
	emitter[1]->SetParticleLifetime(20000.0f);
	emitter[1]->SetFire(0);
	emitter[1]->SetSnow(1);

	modelMatrix =
		Matrix4::Translation(Vector3(2000, 4000, 2000)) *
		Matrix4::Scale(Vector3(10, 10, 10)) *
		Matrix4::Rotation(0, Vector3(1.0f, 0.0f, 0.0f));

	UpdateShaderMatrices();

	emitter[1]->Draw();


	glUseProgram(0);
}