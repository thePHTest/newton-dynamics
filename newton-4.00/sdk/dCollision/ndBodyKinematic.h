/* Copyright (c) <2003-2019> <Julio Jerez, Newton Game Dynamics>
* 
* This software is provided 'as-is', without any express or implied
* warranty. In no event will the authors be held liable for any damages
* arising from the use of this software.
* 
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it
* freely, subject to the following restrictions:
* 
* 1. The origin of this software must not be misrepresented; you must not
* claim that you wrote the original software. If you use this software
* in a product, an acknowledgment in the product documentation would be
* appreciated but is not required.
* 
* 2. Altered source versions must be plainly marked as such, and must not be
* misrepresented as being the original software.
* 
* 3. This notice may not be removed or altered from any source distribution.
*/

#ifndef __D_BODY_KINEMATIC_H__
#define __D_BODY_KINEMATIC_H__

#include "ndCollisionStdafx.h"
#include "ndBody.h"

class ndSceneBodyNode;
class ndSceneAggregate;

D_MSV_NEWTON_ALIGN_32
class ndBodyKinematic: public ndBody
{
	class ndContactkey;
	class ndContactMap: public dTree<ndContact*, ndContactkey, dContainersFreeListAlloc<ndContact*>>
	{
		public:
		ndContactMap();
		ndContact* FindContact(const ndBody* const body0, const ndBody* const body1) const;
		void AttachContact(ndContact* const contact);
		void DetachContact(ndContact* const contact);

		mutable dSpinLock m_lock;
	};

	public:
	ND_COLLISION_API ndBodyKinematic();
	ND_COLLISION_API virtual ~ndBodyKinematic();

	ndScene* GetBroadPhase() const;

	dFloat32 GetInvMass() const;
	dVector GetMassMatrix() const;
	void SetMassMatrix(const dVector& massMatrix);

	ND_COLLISION_API ndShapeInstance& GetCollisionShape();
	ND_COLLISION_API const ndShapeInstance& GetCollisionShape() const;
	ND_COLLISION_API void SetCollisionShape(const ndShapeInstance& shapeInstance);

	ND_COLLISION_API virtual dFloat32 RayCast(ndRayCastNotify& callback, const dFastRayTest& ray, const dFloat32 maxT) const;

	void SetMassMatrix(dFloat32 mass, const ndShapeInstance& shapeInstance);
	void SetMassMatrix(dFloat32 Ixx, dFloat32 Iyy, dFloat32 Izz, dFloat32 mass);
	void GetMassMatrix(dFloat32& Ixx, dFloat32& Iyy, dFloat32& Izz, dFloat32& mass);

	ND_COLLISION_API virtual void AttachContact(ndContact* const contact);
	ND_COLLISION_API virtual void DetachContact(ndContact* const contact);
	ND_COLLISION_API virtual ndContact* FindContact(const ndBody* const otherBody) const;
	ND_COLLISION_API static void ReleaseMemory();

	virtual ndBodyKinematic* GetAsBodyKinematic() { return this; }

	private:
	void SetMassMatrix(dFloat32 mass, const dMatrix& inertia);

	protected:
	void UpdateCollisionMatrix();
	void SetBroadPhase(ndScene* const broadPhase, dList<ndBodyKinematic*>::dListNode* const node);

	ndSceneBodyNode* GetBroadPhaseBodyNode() const;
	void SetBroadPhaseBodyNode(ndSceneBodyNode* const node);

	ndSceneAggregate* GetBroadPhaseAggregate() const;
	void SetBroadPhaseAggregate(ndSceneAggregate* const node);

	ndShapeInstance m_shapeInstance;
	dVector m_mass;
	dVector m_invMass;
	ndContactMap m_contactList;

	ndScene* m_broadPhase;
	dList<ndBodyKinematic*>::dListNode* m_broadPhaseNode;
	ndSceneBodyNode* m_broadPhaseBodyNode;
	ndSceneAggregate* m_broadPhaseAggregateNode;

	friend class ndScene;
	friend class ndSceneMixed;
	friend class ndSceneBodyNode;
} D_GCC_NEWTON_ALIGN_32;

inline dFloat32 ndBodyKinematic::GetInvMass() const
{
	return m_invMass.m_w;
}

inline dVector ndBodyKinematic::GetMassMatrix() const
{
	return m_mass;
}

inline void ndBodyKinematic::GetMassMatrix(dFloat32& Ixx, dFloat32& Iyy, dFloat32& Izz, dFloat32& mass)
{
	Ixx = m_mass.m_x;
	Iyy = m_mass.m_y;
	Izz = m_mass.m_z;
	mass = m_mass.m_w;
}

inline void ndBodyKinematic::SetMassMatrix(const dVector& massMatrix)
{
	dMatrix inertia(dGetZeroMatrix());
	inertia[0][0] = massMatrix.m_x;
	inertia[1][1] = massMatrix.m_y;
	inertia[2][2] = massMatrix.m_z;
	SetMassMatrix(massMatrix.m_w, inertia);
}

inline void ndBodyKinematic::SetMassMatrix(dFloat32 Ixx, dFloat32 Iyy, dFloat32 Izz, dFloat32 mass)
{
	SetMassMatrix(dVector(Ixx, Iyy, Izz, mass));
}

inline void ndBodyKinematic::SetMassMatrix(dFloat32 mass, const ndShapeInstance& shapeInstance)
{
	dMatrix inertia(shapeInstance.CalculateInertia());

	dVector origin(inertia.m_posit);
	for (dInt32 i = 0; i < 3; i++) {
		inertia[i] = inertia[i].Scale(mass);
		//inertia[i][i] = (inertia[i][i] + origin[i] * origin[i]) * mass;
		//for (dInt32 j = i + 1; j < 3; j ++) {
		//	dgFloat32 crossIJ = origin[i] * origin[j];
		//	inertia[i][j] = (inertia[i][j] + crossIJ) * mass;
		//	inertia[j][i] = (inertia[j][i] + crossIJ) * mass;
		//}
	}

	// although the engine fully supports asymmetric inertia, I will ignore cross inertia for now
	SetCentreOfMass(origin);
	SetMassMatrix(mass, inertia);
}

inline ndScene* ndBodyKinematic::GetBroadPhase() const
{
	return m_broadPhase;
}

inline void ndBodyKinematic::SetBroadPhase(ndScene* const broadPhase, dList<ndBodyKinematic*>::dListNode* const node)
{
	m_broadPhaseNode = node;
	m_broadPhase = broadPhase;
}

inline ndSceneBodyNode* ndBodyKinematic::GetBroadPhaseBodyNode() const
{
	return m_broadPhaseBodyNode;
}

inline void ndBodyKinematic::SetBroadPhaseBodyNode(ndSceneBodyNode* const node)
{
	m_broadPhaseBodyNode = node;
}


#endif 
