/******************************************************************************
Class:Renderer		
Implements:OGLRenderer
Author:Rich Davison	<richard.davison4@newcastle.ac.uk> and YOU!
Description: For this module, you are being provided with a basic working
Renderer - to give you more time to work on your physics and AI!

It is basically the Renderer from the Graphics For Games Module as it was
by Tutorial 7 - Scene Management. It will split nodes up into those that are
opaque and transparent, and render accordingly. 

The only new bits are the ability to add and remove scene nodes, and set the
camera - as these are now to be controlled by your 'game'.

The renderer is one of those things that really there should be only one of.
I think it was a mistake to add support for multiple Windows / Renderers to
the code download, as it just overcomplicated things. To this end, The 
Renderer for this module is a singleton-like class, as with the 
PhysicsSystem. Thus, it is created, accessed, and destroyed via static 
functions. You don't have to do it this way if you don't want, and you are
of course free to add in the advanced graphical features from other tutorials
into this Renderer as you see fit. 


-_-_-_-_-_-_-_,------,   
_-_-_-_-_-_-_-|   /\_/\   NYANYANYAN
-_-_-_-_-_-_-~|__( ^ .^) /
_-_-_-_-_-_-_-""  ""   

*//////////////////////////////////////////////////////////////////////////////


#pragma once

#include "../../nclgl/OGLRenderer.h"
#include "../../nclgl/SceneNode.h"
#include "../../nclgl/Camera.h"
#include "../../nclgl/Frustum.h"
#include <algorithm>
#include "../../nclgl/HeightMap.h"
#include "../../nclgl/TextMesh.h"
#include "../../nclgl/Light.h"
#include "ParticleEmitter.h"

#define FOG_EQUATION_LINEAR 0 
#define FOG_EQUATION_EXP 1 
#define FOG_EQUATION_EXP2 2 





class Renderer : public OGLRenderer	{
public:
	virtual void RenderScene();
	virtual void UpdateScene(float msec);

	void	SetCamera(Camera*c);

	void	AddNode(SceneNode* n);

	void	RemoveNode(SceneNode* n);

	//Statics
	static bool Initialise() {
		instance = new Renderer(Window::GetWindow());
		return instance->HasInitialised();
	}

	static void Destroy() {
		delete instance;
	}
	
	static Renderer&GetRenderer() { return *instance;}
	static int physics_updates;
	static int elapsed_time;
	static int renderer_updates;
	static bool wireframe;
	Vector3*	GetHeightMapVertices() {
		return heightMap->GetVertices();
	}
protected:	
	Renderer(Window &parent);
	virtual ~Renderer(void);

	void SetShaderParticleSize(float f);
	void	drawEmitter_1();
	void	drawEmitter_2();
	void	BuildNodeLists(SceneNode* from);
	void	SortNodeLists();
	void	ClearNodeLists();
	void	DrawNodes();
	void	DrawNode(SceneNode*n);
	void	DrawText(const std::string &text, const Vector3 &position, const float size = 10.0f, const bool perspective = false);
	virtual void UpdateShaderMatrices();
	SceneNode*	root;
	Camera*		camera;
	Shader*		simpleShader;
	HeightMap* heightMap;
	GLuint cubemap_tex;
	Mesh* skybox_mesh;
	Shader* skybox_shader; 
	Shader* particleShader;
	Light* light;
	Frustum		frameFrustum;
	Shader* texture_shader;
	vector<SceneNode*> transparentNodeList;
	vector<SceneNode*> nodeList;
	vector<ParticleEmitter*> emitter;
	static Renderer*	instance;
	Font*	basicFont;	//A font! a basic one...
};

