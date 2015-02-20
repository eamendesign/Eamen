#pragma once
#include "DebugDrawer.h"
#include "Renderer.h"
/**
* Author: Callum Rhodes <c.g.rhodes@ncl.ac.uk>
* Edits: Callan White
*/
#include "../../nclgl/Vector3.h"

enum CollisionVolumeType {
	COLLISION_NONE,
	COLLISION_SPHERE,
	COLLISION_AABB,
	COLLISION_PLANE,
	COLLISION_CYLINDER,
	COLLISION_CAPSULE
};

class CollisionVolume : public DebugDrawer {
public:
	CollisionVolume(Vector3 pos, CollisionVolumeType t = COLLISION_NONE) : m_pos(pos), type(t) {}
	CollisionVolumeType GetType() const { return type; }
	Vector3 GetPos() const { return m_pos; }
	void SetPos(Vector3 v) { m_pos = v; }
protected:
	Vector3 m_pos;
	CollisionVolumeType type;
	virtual void DebugDraw() { return; }
};


class CollisionSphere : public CollisionVolume {
public:
	CollisionSphere(Vector3 p, float radius) : radius(radius), CollisionVolume(p, COLLISION_SPHERE) {}
	float GetRadius() const { return radius; }
	void SetRadius(float r) { radius = r; }
	virtual void DebugDraw() {
		Renderer::GetRenderer().DrawDebugCircle(DEBUGDRAW_PERSPECTIVE, m_pos, radius, Vector3(1, 0, 0));
	}
private:
	float radius;
};


class CollisionPlane : public CollisionVolume {
public:
	CollisionPlane(Vector3 pos, Vector3 normal, float distance) : distance(distance), normal(normal), CollisionVolume(pos, COLLISION_PLANE) { m_pos = Vector3(0, 0, 0); }
	Vector3 GetNormal() const { return normal; }
	float GetDistance() const { return distance; }
private:
	float distance;
	Vector3 normal;
};

class CollisionAABB : public CollisionVolume {
public:
	CollisionAABB(Vector3 pos, Vector3 halfDim) : halfDim(halfDim), CollisionVolume(pos, COLLISION_AABB) {}
	Vector3 getHalfDimensions() const { return halfDim; }
private:
	Vector3 halfDim;
};

class CollisionCapsule : public CollisionSphere {
public:
	CollisionCapsule(Vector3 p, float r, Vector3 ep) : ep(ep), CollisionSphere(p, r) {
		this->type = COLLISION_CAPSULE;
	}
	void SetEp(Vector3 ep) { this->ep = ep; this->height = Vector3::Distance(m_pos, ep); }
	Vector3 GetEp() { return ep; }
	void SetHeight(float h) { height = h; }
	float GetHeight() { return height; }
private:
	Vector3 ep;
	float height;
};

class CollisionData {
public:
	Vector3 m_point;
	Vector3 m_normal;
	float m_penetration;
};