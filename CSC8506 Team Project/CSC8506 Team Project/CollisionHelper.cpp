#include "CollisionHelper.h"
#include "PhysicsSystem.h"
bool CollisionHelper::SphereSphereCollision(PhysicsNode& p0, PhysicsNode& p1, CollisionData* data) {
	CollisionSphere& s0 = *(CollisionSphere*)p0.GetCollisionVolume();
	CollisionSphere& s1 = *(CollisionSphere*)p1.GetCollisionVolume();

	Vector3 normal = s0.GetPos() - s1.GetPos();
	const float distSq = LengthSq(normal);
	const float sumRadius = s0.GetRadius() + s1.GetRadius();

	if (distSq < sumRadius*sumRadius) {
		if (data) {
			data->m_penetration = sumRadius - sqrtf(distSq);
			normal.Normalise();
			data->m_normal = normal;
			data->m_point = s0.GetPos() - normal * (s0.GetRadius() - data->m_penetration * 0.5f);
		}
		//PhysicsSystem::numCollisions++;
		return true;
	}
	return false;
}

bool CollisionHelper::PlaneSphereCollision(PhysicsNode& p0, PhysicsNode& p1, CollisionData* data) {
	CollisionPlane& plane = *(CollisionPlane*)p0.GetCollisionVolume();
	CollisionSphere& sphere = *(CollisionSphere*)p1.GetCollisionVolume();

	float separation = Vector3::Dot(sphere.GetPos(), plane.GetNormal()) - plane.GetDistance();

	if (separation > sphere.GetRadius()) {
		return false;
	}
	if (data) {
		data->m_penetration = sphere.GetRadius() - separation;
		data->m_normal = -plane.GetNormal();
		data->m_point = sphere.GetPos() - plane.GetNormal() * separation;
	}
	PhysicsSystem::numCollisions++;
	return true;
}

//used for the branches of the l-system because screw cylinders
bool CollisionHelper::SphereCapsuleCollision(PhysicsNode& p0, PhysicsNode& p1, CollisionData* data) {
	CollisionSphere& sphere = *(CollisionSphere*)p0.GetCollisionVolume();
	CollisionCapsule& capsule = *(CollisionCapsule*)p1.GetCollisionVolume();

	//compute our point/line segment distance
	Vector3 diff = sphere.GetPos() - capsule.GetPos();
	Vector3 dir = capsule.GetEp() - capsule.GetPos();
	float t = Vector3::Dot(diff, dir) / Vector3::Dot(dir, dir);

	float dist = 0;
	//use t to interpolate along the line segment
	if (t < 0) dist = (sphere.GetPos() - capsule.GetPos()).Length();
	else if (t > 1) dist = (sphere.GetPos() - capsule.GetEp()).Length();
	else dist = (sphere.GetPos() - (capsule.GetPos() + (dir * t))).Length();

	if ((t < 0 || t > 1) && dist >= capsule.GetRadius()) return false; //are we on top/below the capsule? if so we dont use both radii
	if ((capsule.GetRadius() + sphere.GetRadius()) > dist) {
		if (data) {
			data->m_penetration = (sphere.GetRadius() + capsule.GetRadius()) - dist;
			data->m_normal = sphere.GetPos() - capsule.GetPos(); data->m_normal.Normalise();
			data->m_point = sphere.GetPos() - data->m_normal * (sphere.GetRadius() - data->m_penetration * 0.5f);
		}
		return true;
	}
	return false;
}

bool CollisionHelper::AABBCollision(PhysicsNode& p0, PhysicsNode& p1) {
	CollisionAABB& aabb0 = *(CollisionAABB*)p0.GetCollisionVolume();
	CollisionAABB& aabb1 = *(CollisionAABB*)p1.GetCollisionVolume();

	float dist = abs(p0.GetPosition().x - p1.GetPosition().x);
	float sum = aabb0.getHalfDimensions().x + aabb1.getHalfDimensions().x;
	if (dist <= sum) {
		dist = abs(p0.GetPosition().y - p1.GetPosition().y);
		sum = aabb0.getHalfDimensions().y + aabb1.getHalfDimensions().y;
		if (dist <= sum) {
			dist = abs(p0.GetPosition().z - p1.GetPosition().z);
			sum = aabb0.getHalfDimensions().z + aabb1.getHalfDimensions().z;
			if (dist <= sum) {
				return true;
			}
		}
	}
	return false;
}

void CollisionHelper::AddCollisionImpulse(PhysicsNode& p0, PhysicsNode& p1, CollisionData& data) {

	if (p0.GetInverseMass() != 0) p0.SetPosition(p0.GetPosition() + (data.m_normal * data.m_penetration));
	if (p1.GetInverseMass() != 0) p1.SetPosition(p1.GetPosition() - (data.m_normal * data.m_penetration));

	Vector3 vab = p0.GetLinearVelocity() - p1.GetLinearVelocity();
	float vn = Vector3::Dot(vab, data.m_normal);

	float impulse = (-(1.0f + 0.9f) * vn) / (Vector3::Dot(data.m_normal, data.m_normal * (p0.GetInverseMass() + p1.GetInverseMass())));

	p0.SetLinearVelocity(p0.GetLinearVelocity() + (data.m_normal * impulse * p0.GetInverseMass()));
	p1.SetLinearVelocity(p1.GetLinearVelocity() - (data.m_normal * impulse * p1.GetInverseMass()));

	Vector3 a_av = p0.GetAngularVelocity();
	Vector3 b_av = p1.GetAngularVelocity();

	a_av = a_av + (p0.GetInverseInertia() * Vector3::Cross(data.m_point - p0.GetPosition(), data.m_normal * impulse * 10));
	b_av = b_av - (p1.GetInverseInertia() * Vector3::Cross(data.m_point - p1.GetPosition(), data.m_normal * impulse * 10));

	p0.SetAngularVelocity(a_av);
	p1.SetAngularVelocity(b_av);

}

void CollisionHelper::potentialBallBallCollisions(vector<NodePair> &potentialCollisions, vector<PhysicsNode*> &balls, Octree* octree)
{
	octree->potentialBallBallCollisions(potentialCollisions);
}


void CollisionHelper::handleBallBallCollisions(vector<PhysicsNode*> &balls, Octree* octree, CollisionData& data)
{
	vector<NodePair> bps;
	potentialBallBallCollisions(bps, balls, octree);
	//cout << bps.size() << endl;
	for (unsigned int i = 0; i < bps.size(); i++) {
		NodePair bp = bps[i];

		PhysicsNode* b1 = bp.node1;
		PhysicsNode* b2 = bp.node2;

		if (CollisionHelper::SphereSphereCollision(*b1, *b2, &data)) {
			CollisionHelper::AddCollisionImpulse(*b1, *b2, data);
		//	cout << "done" << endl;
		}
		PhysicsSystem::numCollisions++;
	}
	bps.clear();
}

void CollisionHelper::HeightMapCollision(Vector3* v, Vector3* normals, PhysicsNode& p) {
	CollisionSphere& sphere = *(CollisionSphere*)p.GetCollisionVolume();
	Vector3 sphereP = p.GetPosition();
	//are we within the heightmap?
	if ((sphereP.x > 0 && sphereP.z > 0) && (sphereP.x < (RAW_WIDTH * 16.0f)) && (sphereP.z < (RAW_WIDTH * 16.0f)) && sphereP.y < 600) {
		Vector3 pos = sphere.GetPos(); pos.y -= sphere.GetRadius();
		//hash the position to find heightmap indices relevant to collision
		int x = (int)(pos.x / 16.0f);
		int z = (int)(pos.z / 16.0f);
		int ind0 = x * 257 + z;
		int ind1 = (x + 1) * 257 + z;
		int ind2 = x * 257 + (z + 1);
		int ind3 = (x + 1) * 257 + (z + 1);
		if (ind2 % 257 == 0 && ind3 % 257 == 0) return;
		if (ind0 > 0 && ind0 < 256 * 256 && ind2 > 0 && ind3 > 0 && ind2 < 256 * 256 && ind3 < 256 * 256) {
			//sanity check passed, get the points near and form a triangle
			Vector3 hpos = v[ind0];
			Vector3 hpos1 = v[ind1];
			Vector3 hpos2 = v[ind2];
			Vector3 hpos3 = v[ind3];

			Vector3 n0 = normals[ind0];
			Vector3 n1 = normals[ind1];
			Vector3 n2 = normals[ind2];
			Vector3 n3 = normals[ind3];

			float height = 0.0f;
			float sqx = (pos.x / 16.0f) - x;
			float sqz = (pos.z / 16.0f) - z;


			Vector3 n;
			float d;
			if ((sqx + sqz) < 1){
				height = hpos.y;
				height += (hpos1.y - hpos.y) * sqx;
				height += (hpos2.y - hpos.y) * sqz;
				n = n0;
				n += (n1 - n0) * sqx;
				n += (n2 - n0) * sqz;
				d = Vector3::Dot(hpos, n);

			}
			else {
				height = hpos3.y;
				height += (hpos1.y - hpos3.y) * (1.0f - sqz);
				height += (hpos2.y - hpos3.y) * (1.0f - sqx);
				n = n3;
				n += (n1 - n0) * (1.0f - sqz);
				n += (n2 - n3) * (1.0f - sqx);
				d = Vector3::Dot(hpos3, n);
			}

			//Used for debugging the heights of the heightmap vs the position of the ball
			//Renderer::DrawDebugCross(DEBUGDRAW_PERSPECTIVE, Vector3(sphereP.x, height, sphereP.z), Vector3(50,50,50));


			n.Normalise();
			float sep = Vector3::Dot(sphereP, n) - d;

			if (sphereP.y <= height || sep < sphere.GetRadius()){

				if (p.GetLinearVelocity().Length() > 0.0005f || p.GetAngularVelocity().Length() > 0.0005f)
				{

					p.SetPosition(p.GetPosition() + (n * (sphere.GetRadius() - sep)));
					float vn = Vector3::Dot(p.GetLinearVelocity(), n);
					float impulse = (-(1.0f + 0.5f) * vn) / (Vector3::Dot(n, n * p.GetInverseMass()));
					p.SetLinearVelocity(p.GetLinearVelocity() + (n * impulse * p.GetInverseMass()));
				}
				else
				{
					p.SetAngularVelocity(Vector3(0.f, 0.f, 0.f));
					p.SetLinearVelocity(Vector3(0.f, 0.f, 0.f));
				}

			}


		}
	}

}