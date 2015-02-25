#include "ParticleEmitter.h"
#include <string>
#include <io.h>
#include <stdio.h>
#include <algorithm>    // std::sort

//Cheap random number generator, will generate
//numbers between 0.0 and 1.0 to 2 DP
#define RAND() ((rand()%101)/100.0f)

/*
Constructor, which sets everything to some 'sensible' defaults.
*/

enum TYPE{
	SNOW, FIRE

};
ParticleEmitter::ParticleEmitter(int type)	{
	
	
	/*
	Each particle is a white dot, which has an alpha fade on it,
	so the edges fade to 0.0 alpha.
	*/
	//texture = SOIL_load_OGL_texture("../Textures/particle.tga",
	//SOIL_LOAD_AUTO,SOIL_CREATE_NEW_ID,SOIL_FLAG_COMPRESS_TO_DXT);

	addFire = 0; addSnow = 0;

	//type = FIRE;

	switch (type)
	{
	case FIRE:
		texture = SOIL_load_OGL_texture("../../Textures/fire.jpg",
			SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_COMPRESS_TO_DXT);

		particleRate = 0.05f;
		particleLifetime = 500.0f;
		particleSize = frand(40.f, 60.f);
		particleVariance = 0.0f;
		nextParticleTime = 1000.0f;
		particleSpeed = 0.1f;
		numLaunchParticles = 1;
		largestSize = 10;
		cameraDistance = Vector3(0, 0, 0);
		rotation = Vector3(0, 1, 0);
		addFire = 1;
		break;
	case SNOW:
		texture = SOIL_load_OGL_texture("../../Textures/particle.tga",
			SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_COMPRESS_TO_DXT);

		particleRate = 10.0f;
		particleLifetime = 500.0f;
		particleSize = 24.0f;
		particleVariance = 0.08f;
		nextParticleTime = 10.0f;
		particleSpeed = 0.2f;
		numLaunchParticles = 5;
		largestSize = 10;
		addSnow = 1;
		break;

	}

	
}

/*
In our destructor, we must make sure we delete the particles held in BOTH
of our lists!
*/
ParticleEmitter::~ParticleEmitter(void)	{
	for (std::vector<Particle *>::iterator i = particles.begin(); i != particles.end(); ++i) {
		delete (*i);
	}
	for (std::vector<Particle *>::iterator i = freeList.begin(); i != freeList.end(); ++i) {
		delete (*i);
	}
}

/*
Every frame, we're going to call this Update function. In a 'real' game, this maybe wouldn't
be the Renderer's job, but maybe part of the gameplay 'entity' system. Depends on what
the game components can 'see' (i.e are we hiding what type of API we're using for rendering
from the rest of the game, etc).

It goes through every particle of this emitter, and updates it. if it has no 'life' left,
it is removed from the particle list. If it's time to generate some new particles, we do
that in here, too. Finally, this function resizes our VBOs if necessary.
*/
void ParticleEmitter::Update(float msec)	{


	

	nextParticleTime -= msec;	//some time has passed!

	/*
	Enough time might have passed that more than 1 'launch' of particles is necessary...
	*/
	while (nextParticleTime <= 0) {
		nextParticleTime += particleRate;
		//Add a number of particles to the vector, obtained from the free list.
		for (int i = 0; i < numLaunchParticles; ++i) {
			particles.push_back(GetFreeParticle());
		}
	}

	//Now for the particle 'think' function. Particles are so 'lightweight' there's not
	//much point putting this as a member variable of the Particle struct...


	for (std::vector<Particle *>::iterator i = particles.begin(); i != particles.end();/*No I++ here!!! */) {
		Particle *p = (*i);

		//We're keeping the particles 'life' in the alpha channel of its colour. 
		//This means it'll also fade out as it loses energy. Kinda cool?

		if (addFire)
		p->colour.w -= (msec * 2 / particleLifetime);

		if (addSnow)
		p->colour.w -= (msec  / particleLifetime);
		//If this particle has ran out of life, remove it from the 'active' list,
		//and put it on the 'free' list for reuse later.
		if (p->colour.w <= 0.0f) {
			freeList.push_back(p);
			i = particles.erase(i);
		}
		else{
			//Otherwise, this particle must be still 'alive'. Update its
			//position by multiplying its normalised direction by the
			//particle speed, and adding the result to the position. Easy!

			p->position += p->direction * ( msec * particleSpeed);
			
			// Rotation around X
			Rotate(rotation.x * msec / 1000.0f, p->position.y, p->position.z);
			// Rotation around Y
			Rotate(rotation.y * msec / 1000.0f, p->position.x, p->position.z);
			// Rotation around Z
			Rotate(rotation.z * msec / 1000.0f, p->position.x, p->position.y);


			++i;	//Only update our iterator value here.
			//If we didn't do this, we'd skip over particles when removing
			//particles from the list...
		}
	}

	//If we now have more particles than we have graphics memory for, we
	//must allocate some new space for them, using ResizeArrays.
	if (particles.size() > largestSize || !vertices || !colours) {
		ResizeArrays();
	}

	//cout << cameraDistance << endl;

	for (unsigned int i = 0; i < particles.size(); ++i) {

		Vector3 dir = particles[i]->position - cameraDistance;
		particles[i]->distanceFromCamera = Vector3::Dot(dir, dir);

	}


	std::sort(particles.begin(), particles.end(), ParticleEmitter::SortParticles);

}

/*
This function gets a pointer to an unused particle - either from the freelist,
or from a newly created particle on the heap.
*/
Particle* ParticleEmitter::GetFreeParticle()	{
	Particle * p = NULL;

	//If we have a spare particle on the freelist, pop it off!
	if (freeList.size() > 0) {
		p = freeList.back();
		freeList.pop_back();
	}
	else{
		//no spare particles, so we need a new one
		p = new Particle();
	}

	//Now we have to reset its values - if it was popped off the
	//free list, it'll still have the values of its 'previous life'

	if (addFire )
	{
		p->colour = Vector4(0.5, 0.1, RAND(), 0.5);
		p->direction = initialDirection;
		p->direction.x += frand(-1, 1);
		p->direction.y += frand(3, 7);
		p->direction.z += frand(-1, 1);
		//p->position = Vector3(0, 0, 0);
	}

	int a;

	if (addSnow)
	{
		p->colour = Vector4(1, 1, 1, 1);
		p->direction = initialDirection;
		p->direction.x += ((RAND() - RAND()) * particleVariance);
		p->direction.y += ((RAND() - RAND()) * particleVariance);
		p->direction.z += ((RAND() - RAND()) * particleVariance);
		//p->position = Vector3(0, 0, 0);
	}


	p->position = Vector3(0,-100,0);

	p->direction.Normalise();	//Keep its direction normalised!
	//p->position.ToZero();

	return p;	//return the new particle :-)
}

/*
If we end up with more particles than space in graphics memory, we must
allocate some new graphics memory!
*/
void	ParticleEmitter::ResizeArrays() {
	//Delete the system memory, as it is of the wrong size now...
	//yes, this does mean we're keeping particle info in THREE places...
	delete[] vertices;
	delete[] colours;

	//Tell OpenGL that we can delete our old VBOs
	glDeleteBuffers(1, &bufferObject[VERTEX_BUFFER]);
	glDeleteBuffers(1, &bufferObject[COLOUR_BUFFER]);




	//Make some new system memory
	vertices = new Vector3[particles.size()];
	colours = new Vector4[particles.size()];


	//Make some new VBO space in memory. Note how we don't actually upload any
	//data to the graphics card here! We just 'new' the memory for now.
	glGenBuffers(1, &bufferObject[VERTEX_BUFFER]);
	glBindBuffer(GL_ARRAY_BUFFER, bufferObject[VERTEX_BUFFER]);
	glBufferData(GL_ARRAY_BUFFER, particles.size()*sizeof(Vector3), 0, GL_DYNAMIC_DRAW);


	glGenBuffers(1, &bufferObject[COLOUR_BUFFER]);
	glBindBuffer(GL_ARRAY_BUFFER, bufferObject[COLOUR_BUFFER]);
	glBufferData(GL_ARRAY_BUFFER, particles.size()*sizeof(Vector4), 0, GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);	//Done with our buffers...

	largestSize = particles.size();		//Update our largest particles size
}

/*
Now for the overloaded Draw function. It's a little bit messy, as we have to copy all of the
Particle data structs into one contiguous lump of memory to send using the OpenGL command
glBufferSubData - which you should remember from the skeletal animation tutorial.

Why don't we use glMapBuffer? Just uploading a whole load of new data is probably easier
and quicker than faffing with mapping and unmapping buffers!
*/
void ParticleEmitter::Draw()	{

	

	//Get 2 contiguous sections of memory full of our particle info
	for (unsigned int i = 0; i < particles.size(); ++i) {
		vertices[i] = particles.at(i)->position;
		colours[i] = particles.at(i)->colour;
		//cout << i<< ": " <<particles[i]->distanceFromCamera << endl;
	}

	glBindVertexArray(arrayObject);

	//Bind our vertex data, and update its data with that of the vertices array
	glBindBuffer(GL_ARRAY_BUFFER, bufferObject[VERTEX_BUFFER]);
	glBufferSubData(GL_ARRAY_BUFFER, 0, particles.size()*sizeof(Vector3), (void*)vertices);
	glVertexAttribPointer(VERTEX_BUFFER, 3, GL_FLOAT, GL_FALSE, sizeof(Vector3), 0);	//Tell the VAO we have positions...
	glEnableVertexAttribArray(VERTEX_BUFFER);	//Binds this buffer to the VAO

	//And now do the same for colours...
	glBindBuffer(GL_ARRAY_BUFFER, bufferObject[COLOUR_BUFFER]);
	glBufferSubData(GL_ARRAY_BUFFER, 0, particles.size()*sizeof(Vector4), (void*)colours);
	glVertexAttribPointer(COLOUR_BUFFER, 4, GL_FLOAT, GL_FALSE, sizeof(Vector4), 0);
	glEnableVertexAttribArray(COLOUR_BUFFER);

	//We're going to use a type of alpha blending known as additive blending here. Overlapping 
	//particles will have their colours added together on screen, giving us a cool effect when
	//particles are near each other. Fire a lot of particles at once with slow speed to get a
	//'plasma ball' type effect!

	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	//glBlendFunc(GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA);
	
	//And now do our usual Drawing stuff...
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	glDrawArrays(GL_POINTS, 0, particles.size());	// draw ordered list of vertices
	glBindTexture(GL_TEXTURE_2D, 0);

	glBindVertexArray(0); //Remember to turn off our VAO ;)

	glEnable(GL_DEPTH_TEST);
};

float ParticleEmitter::frand(float start, float end)
{
	float num = (float)rand() / (float)RAND_MAX;
	return (start + (end - start) * num);
}

bool ParticleEmitter::SortParticles(Particle* a, Particle* b)
{
	return (a->distanceFromCamera > b->distanceFromCamera) ? true : false;
}

void ParticleEmitter::Rotate(float angle, float &x, float &y)
{
	float finalX = x * cos(angle) - y * sin(angle);
	float finalY = y * cos(angle) + x * sin(angle);

	x = finalX;
	y = finalY;
}