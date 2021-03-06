/* Copyright (c) <2003-2019> <Newton Game Dynamics>
* 
* This software is provided 'as-is', without any express or implied
* warranty. In no event will the authors be held liable for any damages
* arising from the use of this software.
* 
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it
* freely
*/

#include "ndSandboxStdafx.h"
#include "ndSkyBox.h"
#include "ndTargaToOpenGl.h"
#include "ndDemoMesh.h"
#include "ndDemoCamera.h"
#include "ndPhysicsUtils.h"
#include "ndPhysicsWorld.h"
#include "ndMakeStaticMap.h"
#include "ndDemoEntityManager.h"

static ndBodyDynamic* MakePrimitive(ndDemoEntityManager* const scene, const dMatrix& matrix, const ndShapeInstance& capsule, ndDemoMesh* const mesh, dFloat32 mass)
{
	ndPhysicsWorld* const world = scene->GetWorld();
	ndDemoEntity* const entity = new ndDemoEntity(matrix, nullptr);
	entity->SetMesh(mesh, dGetIdentityMatrix());
	ndBodyDynamic* const body = new ndBodyDynamic();
	body->SetNotifyCallback(new ndDemoEntityNotify(scene, entity));
	body->SetMatrix(matrix);
	body->SetCollisionShape(capsule);
	body->SetMassMatrix(mass, capsule);
	world->AddBody(body);
	scene->AddEntity(entity);
	return body;
}

static void BuildBallSocket(ndDemoEntityManager* const scene, const dVector& origin)
{
	dFloat32 mass = 1.0f;
	dFloat32 diameter = 0.5f;
	ndShapeInstance shape(new ndShapeCapsule(diameter * 0.5f, diameter * 0.5f, diameter * 1.0f));
	ndDemoMesh* const mesh = new ndDemoMesh("shape", scene->GetShaderCache(), &shape, "marble.tga", "marble.tga", "marble.tga");

	//dMatrix matrix(dGetIdentityMatrix());
	dMatrix matrix(dRollMatrix(90.0f * dDegreeToRad));
	matrix.m_posit = origin;
	matrix.m_posit.m_w = 1.0f;

	ndPhysicsWorld* const world = scene->GetWorld();

	dVector floor(FindFloor(*world, matrix.m_posit + dVector(0.0f, 100.0f, 0.0f, 0.0f), 200.0f));
	matrix.m_posit.m_y = floor.m_y;

	matrix.m_posit.m_y += 1.0f;
	ndBodyDynamic* const body0 = MakePrimitive(scene, matrix, shape, mesh, mass);
	matrix.m_posit.m_y += 1.0f;
	ndBodyDynamic* const body1 = MakePrimitive(scene, matrix, shape, mesh, mass);

	dMatrix bodyMatrix0(body0->GetMatrix());
	dMatrix bodyMatrix1(body1->GetMatrix());
	dMatrix pinMatrix(bodyMatrix0);
	pinMatrix.m_posit = (bodyMatrix0.m_posit + bodyMatrix1.m_posit).Scale(0.5f);
	ndJointBallAndSocket* const joint0 = new ndJointBallAndSocket(pinMatrix, body0, body1);
	world->AddJoint(joint0);
	
	bodyMatrix1.m_posit.m_y += 0.5f;
	ndBodyDynamic* const fixBody = world->GetSentinelBody();
	ndJointBallAndSocket* const joint1 = new ndJointBallAndSocket(bodyMatrix1, body1, fixBody);
	world->AddJoint(joint1);

	mesh->Release();
}

static void BuildSlider(ndDemoEntityManager* const scene, const dVector& origin, dFloat32 mass, dFloat32 diameter)
{
	//ndShapeInstance shape(new ndShapeCapsule(diameter * 0.5f, diameter * 0.5f, diameter * 1.0f));
	ndShapeInstance shape(new ndShapeBox(diameter, diameter, diameter));
	ndDemoMesh* const mesh = new ndDemoMesh("shape", scene->GetShaderCache(), &shape, "marble.tga", "marble.tga", "marble.tga");

	dMatrix matrix(dRollMatrix(90.0f * dDegreeToRad));
	matrix.m_posit = origin;
	matrix.m_posit.m_w = 1.0f;

	ndPhysicsWorld* const world = scene->GetWorld();

	dVector floor(FindFloor(*world, matrix.m_posit + dVector(0.0f, 100.0f, 0.0f, 0.0f), 200.0f));
	matrix.m_posit.m_y = floor.m_y;

	matrix.m_posit.m_y += 2.0f;

	ndBodyDynamic* const fixBody = world->GetSentinelBody();
	ndBodyDynamic* const body = MakePrimitive(scene, matrix, shape, mesh, mass);
	
	ndJointSlider* const joint = new ndJointSlider(matrix, body, fixBody);
	joint->SetAsSpringDamper(true, 500.0f, 5.0f);
	//joint->SetFriction(mass * 10.0f * 2.0f);
	joint->EnableLimits(true, -1.0f, 1.0f);
	world->AddJoint(joint);

	mesh->Release();
}

static void BuildHinge(ndDemoEntityManager* const scene, const dVector& origin, dFloat32 mass, dFloat32 diameter)
{
	ndShapeInstance shape(new ndShapeBox(diameter, diameter, diameter));
	ndDemoMesh* const mesh = new ndDemoMesh("shape", scene->GetShaderCache(), &shape, "marble.tga", "marble.tga", "marble.tga");

	dMatrix matrix(dRollMatrix(90.0f * dDegreeToRad));
	matrix.m_posit = origin;
	matrix.m_posit.m_w = 1.0f;

	ndPhysicsWorld* const world = scene->GetWorld();

	dVector floor(FindFloor(*world, matrix.m_posit + dVector(0.0f, 100.0f, 0.0f, 0.0f), 200.0f));
	matrix.m_posit.m_y = floor.m_y;

	matrix.m_posit.m_y += 2.0f;

	ndBodyDynamic* const fixBody = world->GetSentinelBody();
	ndBodyDynamic* const body = MakePrimitive(scene, matrix, shape, mesh, mass);

	ndJointHinge* const joint = new ndJointHinge(matrix, body, fixBody);
	//joint->SetAsSpringDamper(true, 500.0f, 5.0f);
	//joint->SetFriction(mass * 10.0f * 2.0f);
	//joint->EnableLimits(true, -1.0f, 1.0f);
	world->AddJoint(joint);

	mesh->Release();
}


void ndBasicJoints (ndDemoEntityManager* const scene)
{
	// build a floor
	BuildFloorBox(scene);

	BuildBallSocket(scene, dVector(0.0f, 0.0f, 0.0f, 0.0f));
	BuildHinge(scene, dVector(0.0f, 0.0f, -2.0f, 0.0f), 10.0f, 0.5f);
	BuildSlider(scene, dVector(0.0f, 0.0f, 2.0f, 0.0f), 10.0f, 0.5f);
	BuildSlider(scene, dVector(0.0f, 0.0f, 4.0f, 0.0f), 100.0f, 0.75f);

	dQuaternion rot;
	dVector origin(-10.0f, 2.0f, 0.0f, 0.0f);
	scene->SetCameraMatrix(rot, origin);
}
