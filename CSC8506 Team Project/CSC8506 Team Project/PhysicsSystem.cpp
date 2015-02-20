#include "PhysicsSystem.h"
#include "CollisionHelper.h"

PhysicsSystem* PhysicsSystem::instance = 0;
int PhysicsSystem::numUpdates = 0;
int PhysicsSystem::numUpdatesTotal = 0;
int PhysicsSystem::numNodes = 0;
int PhysicsSystem::numNodesRest = 0;
int PhysicsSystem::numCollisions = 0;
int PhysicsSystem::numUpdatesPerSec = 0;
Octree* _octree = NULL;

PhysicsSystem::PhysicsSystem(void)	{
	hm = new HeightMap(TEXTUREDIR"terrain.raw");
	
	
}

PhysicsSystem::~PhysicsSystem(void)	{
	for (unsigned int i = 0; i < allSprings.size(); i++) {
		delete allSprings[i];
	}
	//delete _octree;
}

void	PhysicsSystem::Update(float msec) {
	numUpdates++;
	numUpdatesTotal++;
	std::lock_guard<std::mutex> lock(node_lock);
	numNodes = allNodes.size();
	
	 narrowList.clear();

	for (vector<Constraint*>::iterator i = allSprings.begin(); i != allSprings.end(); ++i) {
		(*i)->Update(msec);
	}

	for (vector<PhysicsNode*>::iterator i = allNodes.begin(); i != allNodes.end(); ++i) {
		

		(*i)->Update(msec);
		narrowList.push_back(*i);
	}

	//NarrowPhaseCollisions();

	BroadPhaseCollisions();
	//NarrowPhaseCollisions_1();
}

void PhysicsSystem::SortSweep()
{
	if (narrowList.size() == 0)
		return ;

	sort(narrowList.begin(), narrowList.end(),[](PhysicsNode* leftx, PhysicsNode* rightx){return leftx->GetPosition().x < rightx->GetPosition().x;});
	
	for (vector<PhysicsNode*>::iterator i = narrowList.begin(); i < narrowList.end() ; ++i) {
		
		if ((*i)->GetCollisionVolume() && (*i)->GetCollisionVolume()->GetType() == COLLISION_SPHERE) 
			
			 CollisionHelper::HeightMapCollision(hm->GetVertices(), hm->GetNormals(), *(*i));

		
		for (vector<PhysicsNode*>::iterator j = i+1 ; j != narrowList.end(); ++j) {
	

				if((*i)->GetEndNode() > (*j)->GetBeginNode() || (*i)->GetBeginNode() < (*j)->GetEndNode())
				{	
				
	
					NarrowPhaseHelper((*i),(*j));

			
				}
			
		}

	}

}


void	PhysicsSystem::OctreeSort() {

	collisions_sphere.clear();
	
	_octree = new Octree(Vector3(0, 0, 0), Vector3(BOX_SIZE * 16, BOX_SIZE * 16, BOX_SIZE * 16), 1);

	for (vector<PhysicsNode*>::iterator i = allNodes.begin(); i != allNodes.end(); ++i)
	{
		if ((*i)->GetCollisionVolume()->GetType() == COLLISION_SPHERE)
		{
			CollisionHelper::HeightMapCollision(hm->GetVertices(), hm->GetNormals(), *(*i));
			_octree->updateOctree(*i);
			
		}
	}
	
	_octree->potentialBallBallCollisions(collisions_sphere);
	

//	cout << sizeof(collisions_sphere) << endl;
	

}

void	PhysicsSystem::BroadPhaseCollisions() {
	SortSweep();
	//OctreeSort();
	

}


void	PhysicsSystem::NarrowPhaseCollisions_1()
{
	
	//cout << collisions_sphere.size()<<endl;
	
	for (vector<NodePair>::iterator i = collisions_sphere.begin();
		i != collisions_sphere.end(); ++i){
		CollisionData data;
		if (CollisionHelper::SphereSphereCollision(*(i->node1), *(i->node2), &data)){
			CollisionHelper::AddCollisionImpulse(*(i->node1), *(i->node2), data);
		}
		
	}
	delete _octree;
}

void	PhysicsSystem::NarrowPhaseCollisions() {
	for (unsigned int i = 0; i < allNodes.size(); i++) {
		//left -> terrain collision
		if (allNodes[i]->GetCollisionVolume() && allNodes[i]->GetCollisionVolume()->GetType() == COLLISION_SPHERE) 
			CollisionHelper::HeightMapCollision(hm->GetVertices(), hm->GetNormals(), *allNodes[i]);

		//for all left children
		for (unsigned int j = 0; j < allNodes[i]->children.size(); ++j) {
			//against each right node
			for (unsigned int k = i + 1; k < allNodes.size(); ++k) {
				NarrowPhaseHelper(allNodes[i]->children[j], allNodes[k]);
				PhysicsSystem::numCollisions++;
				//against each right child
				for (unsigned int l = 0; l < allNodes[k]->children.size(); ++l) {
					NarrowPhaseHelper(allNodes[k]->children[l], allNodes[i]->children[j]);
					PhysicsSystem::numCollisions++;
				}
			}
			PhysicsSystem::numCollisions++;
		}

		//left -> right collision
		for (unsigned int j = i + 1; j < allNodes.size(); j++) {
			NarrowPhaseHelper(allNodes[i], allNodes[j]);
			PhysicsSystem::numCollisions++;
			//left -> right children collision
			for (unsigned int k = 0; k < allNodes[j]->children.size(); ++k) {
				NarrowPhaseHelper(allNodes[i], allNodes[j]->children[k]);
				PhysicsSystem::numCollisions++;
			}
		}

		
	}
}

/*Helper function to prevent NarrowPhaseCollisions turning into one huge ugly bastard function*/
void PhysicsSystem::NarrowPhaseHelper(PhysicsNode* left, PhysicsNode* right) {
	CollisionVolume* fv = left->GetCollisionVolume();
	CollisionVolume* sv = right->GetCollisionVolume();
	

	if (!fv || !sv) return;
	CollisionData data;

	if (fv->GetType() == COLLISION_SPHERE) {
		if (sv->GetType() == COLLISION_SPHERE) {
			if (CollisionHelper::SphereSphereCollision(*left, *right, &data)) {
				CollisionHelper::AddCollisionImpulse(*left, *right, data);
			}
		}
		if (sv->GetType() == COLLISION_CAPSULE) {
			if (CollisionHelper::SphereCapsuleCollision(*left, *right, &data)) {
				CollisionHelper::AddCollisionImpulse(*left, *right, data);
			}
		}
	}
	else if (fv->GetType() == COLLISION_PLANE) {
		if (sv->GetType() == COLLISION_SPHERE) {
			if (CollisionHelper::PlaneSphereCollision(*left, *right, &data)) {
				CollisionHelper::AddCollisionImpulse(*left, *right, data);
			}
		}
	}
	else if (fv->GetType() == COLLISION_CAPSULE) {
		if (sv->GetType() == COLLISION_SPHERE) {
			if (CollisionHelper::SphereCapsuleCollision(*right, *left, &data)) {
				CollisionHelper::AddCollisionImpulse(*right, *left, data);
			}
		}
	}
}

//mutex needed as threading was playing up without
void	PhysicsSystem::AddNode(PhysicsNode* n) {
	std::lock_guard<std::mutex> lock(node_lock);
	allNodes.push_back(n);
}

void	PhysicsSystem::RemoveNode(PhysicsNode* n) {
	std::lock_guard<std::mutex> lock(node_lock);
	for (vector<PhysicsNode*>::iterator i = allNodes.begin(); i != allNodes.end(); ++i) {
		if ((*i) == n) {
			allNodes.erase(i);
			narrowList.erase(i);
			if ((*i)->GetCollisionVolume()->GetType() == COLLISION_SPHERE)
			{
				balls.erase(i);
			}
			return;
		}
	}
}

void	PhysicsSystem::AddConstraint(Constraint* s) {
	allSprings.push_back(s);
}

void	PhysicsSystem::RemoveConstraint(Constraint* c) {
	for (vector<Constraint*>::iterator i = allSprings.begin(); i != allSprings.end(); ++i) {
		if ((*i) == c) {
			allSprings.erase(i);
			return;
		}
	}
}

void	PhysicsSystem::AddDebugDraw(DebugDrawer* d) {
	allDebug.push_back(d);
}

void	PhysicsSystem::RemoveDebugDraw(DebugDrawer* d) {
	for (vector<DebugDrawer*>::iterator i = allDebug.begin(); i != allDebug.end(); ++i) {
		if ((*i) == d) {
			allDebug.erase(i);
			return;
		}
	}
}

void    PhysicsSystem::DrawDebug() {
	for (vector<DebugDrawer*>::iterator i = allDebug.begin(); i != allDebug.end(); ++i) {
		(*i)->DebugDraw();
	}
}