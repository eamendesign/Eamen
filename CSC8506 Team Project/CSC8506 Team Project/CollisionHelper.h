#pragma once
/**
* Author: Callum Rhodes <c.g.rhodes@ncl.ac.uk>
* Edits: Callan White
*/
#include "PhysicsNode.h"
#include "Octree.h"

class CollisionHelper {
public:

	static bool SphereSphereCollision(PhysicsNode& p0, PhysicsNode& p1, CollisionData* data = NULL);

	static bool PlaneSphereCollision(PhysicsNode& p0, PhysicsNode& p1, CollisionData* data = NULL);

	static bool SphereCapsuleCollision(PhysicsNode& p0, PhysicsNode& p1, CollisionData* data = NULL);

	static bool AABBCollision(PhysicsNode& p0, PhysicsNode& p1);

	static void AddCollisionImpulse(PhysicsNode& p0, PhysicsNode& p1, CollisionData& data);

	static void handleBallBallCollisions(vector<PhysicsNode*> &balls, Octree* octree, CollisionData& data);

	static void HeightMapCollision(Vector3* v, Vector3* normals, PhysicsNode& p);

	static void potentialBallBallCollisions(vector<NodePair> &potentialCollisions, vector<PhysicsNode*> &balls, Octree* octree);
};

inline float LengthSq(Vector3 v) {
	return Vector3::Dot(v, v);
}