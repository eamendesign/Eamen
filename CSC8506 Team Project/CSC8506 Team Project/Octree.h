
#pragma once
#include "../../nclgl/Vector3.h"
#include <vector>
#include "PhysicsNode.h"

const int MAX_OCTREE_DEPTH = 2;
const int MIN_BALLS_PER_OCTREE = 3;
const int MAX_BALLS_PER_OCTREE = 6;
const float BOX_SIZE = 257.0f;


//Stores a pair of balls
struct NodePair {
	PhysicsNode* node1;
	PhysicsNode* node2;
};


class Octree {
private:
	Vector3 corner1; //(minX, minY, minZ)
	Vector3 corner2; //(maxX, maxY, maxZ)
	Vector3 center;//((minX + maxX) / 2, (minY + maxY) / 2, (minZ + maxZ) / 2)

	/* The children of this, if this has any.  children[0][*][*] are the
	* children with x coordinates ranging from minX to centerX.
	* children[1][*][*] are the children with x coordinates ranging from
	* centerX to maxX.  Similarly for the other two dimensions of the
	* children array.
	*/
	Octree *children[2][2][2];
	//Whether this has children
	bool hasChildren;
	//The balls in this, if this doesn't have any children
	vector<PhysicsNode*> allNodes;
	//The depth of this in the tree
	int depth;
	//The number of balls in this, including those stored in its children
	int numNodes;

	//Adds a ball to or removes one from the children of this
	//void fileBall(Ball* ball, Vec3f pos, bool addBall) {
	void modifyNode(PhysicsNode* node, Vector3 pos, bool addNode) {
		//Figure out in which child(ren) the ball belongs
		switch (node->GetCollisionVolume()->GetType()){
		case COLLISION_SPHERE:
		{
								 CollisionSphere& cSphere = *(CollisionSphere*)node->GetCollisionVolume();
								 for (int x = 0; x < 2; x++) {
									 if (x == 0) {
										 if (pos.x - cSphere.GetRadius() < center.x) {
											 continue;
										 }
									 }
									 else
									 {
										 if (pos.x + cSphere.GetRadius() > center.x) {
											 continue;
										 }
									 }

									 for (int y = 0; y < 2; y++) {
										 if (y == 0) {
											 if (pos.y - cSphere.GetRadius() < center.y) {
												 continue;
											 }
										 }
										 else
										 {
											 if (pos.y + cSphere.GetRadius() > center.y) {
												 continue;
											 }
										 }
										 

										 for (int z = 0; z < 2; z++) {
											 if (z == 0) {
												 if (pos.z - cSphere.GetRadius() < center.z) {
													 continue;
												 }
											 }
											 else
											 {
												 if (pos.z + cSphere.GetRadius() > center.z) {
													 continue;
												 }
											 }
											 

											 //Add or remove the ball
											 if (addNode) {
												 children[x][y][z]->add(node);
												
												
											 }
											 else {
												 children[x][y][z]->remove(node, pos);
												
											 }
										 }
									 }
								 }

		}
			//	break;

		}

	}

	//Creates children of this, and moves the balls in this to the children
	void haveChildren() {
		for (int x = 0; x < 2; x++) {
			float minX;
			float maxX;
			if (x == 0) {
				minX = corner1.x;
				maxX = center.x;
			}
			else {
				minX = center.x;
				maxX = corner2.x;
			}

			for (int y = 0; y < 2; y++) {
				float minY;
				float maxY;
				if (y == 0) {
					minY = corner1.y;
					maxY = center.y;
				}
				else {
					minY = center.y;
					maxY = corner2.y;
				}

				for (int z = 0; z < 2; z++) {
					float minZ;
					float maxZ;
					if (z == 0) {
						minZ = corner1.z;
						maxZ = center.z;
					}
					else {
						minZ = center.z;
						maxZ = corner2.z;
					}

					children[x][y][z] = new Octree(Vector3(minX, minY, minZ), Vector3(maxX, maxY, maxZ), depth + 1);

					//cout << "========= child ======" << endl;
					
				}
			}
		}

		//Remove all balls from "balls" and add them to the new children
		for (vector<PhysicsNode*>::iterator it = allNodes.begin(); it != allNodes.end();
			it++) {
			PhysicsNode* node = *it;
			modifyNode(node, node->GetPosition(), true);
		}
		allNodes.clear();

		hasChildren = true;
	}

	//Adds all balls in this or one of its descendants to the specified set
	void collectNodes(vector<PhysicsNode*> &ns) {
		if (hasChildren) {
			for (int x = 0; x < 2; x++) {
				for (int y = 0; y < 2; y++) {
					for (int z = 0; z < 2; z++) {
						children[x][y][z]->collectNodes(ns);
					}
				}
			}
		}
		else {
			for (vector<PhysicsNode*>::iterator it = allNodes.begin(); it != allNodes.end();
				it++) {
				PhysicsNode* node = *it;
				ns.push_back(node);
			}
		}
	}

	//Destroys the children of this, and moves all balls in its descendants
	//to the "balls" set
	void destroyChildren() {
		//Move all balls in descendants of this to the "balls" set
		collectNodes(allNodes);

		for (int x = 0; x < 2; x++) {
			for (int y = 0; y < 2; y++) {
				for (int z = 0; z < 2; z++) {
					delete children[x][y][z];
				}
			}
		}

		hasChildren = false;
	}

	//Removes the specified ball at the indicated position
	void remove(PhysicsNode* node, Vector3 pos) {
		
		numNodes--;

		if (hasChildren && numNodes < MIN_BALLS_PER_OCTREE) {
			destroyChildren();
		}

		if (hasChildren) {
			modifyNode(node, pos, false);
		}
		else {
			for (auto i = allNodes.begin(); i != allNodes.end(); ++i) {
				if ((*i) == node) {
					i = allNodes.erase(i);
					break;
				}
			}
		}
	}


public:
	//Constructs a new Octree.  c1 is (minX, minY, minZ), c2 is (maxX, maxY,
	//maxZ), and d is the depth, which starts at 1.
	Octree(Vector3 c1, Vector3 c2, int d) {
		corner1 = c1;
		corner2 = c2;
		center = (c1 + c2) / 2;
		
		depth = d;
		numNodes = 0;
		hasChildren = false;
		
	}

	//Destructor
	~Octree() {
		if (hasChildren) {
			destroyChildren();
		}
	}

	//Adds a ball to this
	void add(PhysicsNode* node) {
	
		
		if (numNodes <= MAX_BALLS_PER_OCTREE){
			allNodes.push_back(node);
			numNodes++;
			
		}
		else{
			if (depth == MAX_OCTREE_DEPTH){
				allNodes.push_back(node);
				numNodes++;
			}
			else{
				if (hasChildren)
					modifyNode(node, node->GetPosition(), true);
				else{
					haveChildren();
				}
			}
				
		}
		

	}

	//Removes a ball from this
	void remove(PhysicsNode* node) {
		remove(node, node->GetPosition());
	}

	//Changes the position of a ball in this from oldPos to ball->pos
	void nodeMoved(PhysicsNode* node, Vector3 oldPos) {
		remove(node, oldPos);
		add(node);
	}

	//Adds potential ball-ball collisions to the specified set
	void potentialBallBallCollisions(vector<NodePair> &collisions) {
		if (hasChildren) {
			for (int x = 0; x < 2; x++) {
				for (int y = 0; y < 2; y++) {
					for (int z = 0; z < 2; z++) {
						children[x][y][z]->
							potentialBallBallCollisions(collisions);
					}
				}
			}
		}
		else {

			if (numNodes < 2)
				return;
			for (vector<PhysicsNode*>::iterator it = allNodes.begin(); it != allNodes.end();
				it++) {
				PhysicsNode* node1 = *it;
				for (vector<PhysicsNode*>::iterator it2 = allNodes.begin(); it2 != allNodes.end(); it2++) {
					PhysicsNode* node2 = *it2;
					//This test makes sure that we only add each pair once
					if (node1 != node2) {
						NodePair bp;
						bp.node1 = node1;
						bp.node2 = node2;
						collisions.push_back(bp);
					}
				}
			}

		}
	}

	void Octree::updateOctree(PhysicsNode* node) {
		//instead of re-genreate the octree,we update every sphere
		//is it stupid?
		
		Renderer::DrawDebugBox(DEBUGDRAW_PERSPECTIVE, center, corner2);
		
		//cout << depth;
		//remove(sphere, sphere->GetPreviousPosition());

		if (hasChildren) {
			for (int x = 0; x < 2; x++) {
				for (int y = 0; y < 2; y++) {
					for (int z = 0; z < 2; z++) {
						children[x][y][z]->updateOctree(node);
						
					}
				}
			}
		}
		else
		{
			//remove(node, node->GetPreviousPosition());
			add(node);
			
		}
		

		
	}

};

