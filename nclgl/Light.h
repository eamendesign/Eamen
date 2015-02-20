#pragma once

#include "../nclgl/Vector4.h"
#include "../nclgl/Vector3.h"

class Light {
public:
	Light(Vector3 position = Vector3(0, 0, 0), Vector4 colour = Vector4(1, 1, 1, 1), float radius = 10.0f) {
		this->position = position;
		this->colour = colour;
		this->radius = radius;
	}

	~Light(void){}

	Vector3 GetPosition() const { return position; }
	Vector4 GetColour() const { return colour; }
	float GetRadius() const { return radius; }

	void SetPosition(Vector3 v) { position = v; }
	void SetColour(Vector4 v) { colour = v; }
	void SetRadius(float v) { radius = v; }
protected:
	Vector3 position;
	Vector4 colour;
	float radius;
};