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

#include "dCoreStdafx.h"
#include "ndNewtonStdafx.h"
#include "ndWorld.h"
#include "ndJointHinge.h"
#include "ndJointWheel.h"
#include "ndBodyDynamic.h"
#include "ndJointDoubleHinge.h"
#include "ndMultiBodyVehicle.h"

class ndMultiBodyVehicleMotor: public ndJointBilateralConstraint
{
	public:
	ndMultiBodyVehicleMotor(ndBodyKinematic* const motor, ndBodyKinematic* const chassis)
		:ndJointBilateralConstraint(3, motor, chassis, motor->GetMatrix())
	{
	}

	void AlignMatrix()
	{
		dMatrix matrix0;
		dMatrix matrix1;
		CalculateGlobalMatrix(matrix0, matrix1);

		//matrix1.m_posit += matrix1.m_up.Scale(1.0f);

		m_body0->SetMatrix(matrix1);
		m_body0->SetVelocity(m_body1->GetVelocity());

		dVector omega0(m_body0->GetOmega());
		dVector omega1(m_body1->GetOmega());
		dVector omega(
			matrix1.m_front.Scale(matrix1.m_front.DotProduct(omega0).GetScalar()) +
			matrix1.m_up.Scale(matrix1.m_up.DotProduct(omega1).GetScalar()) +
			matrix1.m_right.Scale(matrix1.m_right.DotProduct(omega1).GetScalar()));

		omega += matrix1.m_front.Scale(-40.0f) - matrix1.m_front.Scale(matrix1.m_front.DotProduct(omega0).GetScalar());
		//omega += matrix1.m_up.Scale(5.0f) - matrix1.m_up.Scale(matrix1.m_up.DotProduct(omega0).GetScalar());

		m_body0->SetOmega(omega);
	}

	void JacobianDerivative(ndConstraintDescritor& desc)
	{
		dMatrix matrix0;
		dMatrix matrix1;
		CalculateGlobalMatrix(matrix0, matrix1);
		
		//// save the current joint Omega
		dVector omega0(m_body0->GetOmega());
		dVector omega1(m_body1->GetOmega());

		//// the joint angle can be determined by getting the angle between any two non parallel vectors
		//const dFloat32 deltaAngle = AnglesAdd(-CalculateAngle(matrix0.m_up, matrix1.m_up, matrix1.m_front), -m_jointAngle);
		//m_jointAngle += deltaAngle;
		//m_jointSpeed = matrix1.m_front.DotProduct(omega0 - omega1).GetScalar();

		// two rows to restrict rotation around around the parent coordinate system
		const dFloat32 angleError = m_maxAngleError;
		const dFloat32 angle0 = CalculateAngle(matrix0.m_front, matrix1.m_front, matrix1.m_up);
		AddAngularRowJacobian(desc, matrix1.m_up, angle0);

		const dFloat32 angle1 = CalculateAngle(matrix0.m_front, matrix1.m_front, matrix1.m_right);
		AddAngularRowJacobian(desc, matrix1.m_right, angle1);
	}
};

class ndMultiBodyVehicleMotorGearBox : public ndJointBilateralConstraint
{
	public: 
	ndMultiBodyVehicleMotorGearBox(ndBodyKinematic* const motor, ndBodyKinematic* const differential)
		:ndJointBilateralConstraint(1, motor, differential, motor->GetMatrix())
	{
	}

	void JacobianDerivative(ndConstraintDescritor& desc)
	{

		dMatrix matrix0;
		dMatrix matrix1;
		CalculateGlobalMatrix(matrix0, matrix1);

		AddAngularRowJacobian(desc, matrix1.m_right, dFloat32(0.0f));

		ndJacobian& jacobian0 = desc.m_jacobian[desc.m_rowsCount - 1].m_jacobianM0;
		ndJacobian& jacobian1 = desc.m_jacobian[desc.m_rowsCount - 1].m_jacobianM1;

		jacobian0.m_angular = matrix0.m_front;
		jacobian1.m_angular = matrix1.m_front;

		const dVector& omega0 = m_body0->GetOmega();
		const dVector& omega1 = m_body1->GetOmega();

		const dVector relOmega(omega0 * jacobian0.m_angular + omega1 * jacobian1.m_angular);
		dFloat32 w = (relOmega.m_x + relOmega.m_y + relOmega.m_z) * dFloat32(0.5f);
		SetMotorAcceleration(desc, -w * desc.m_invTimestep);
	}
};

class ndDifferential: public ndJointBilateralConstraint
{
	public:
	ndDifferential (ndBodyKinematic* const differential, ndBodyKinematic* const chassis)
		:ndJointBilateralConstraint(2, differential, chassis, differential->GetMatrix())
	{
	}

	void AlignMatrix()
	{
		dMatrix matrix0;
		dMatrix matrix1;
		CalculateGlobalMatrix(matrix0, matrix1);

		//matrix1.m_posit += matrix1.m_up.Scale(1.0f);

		m_body0->SetMatrix(matrix1);
		m_body0->SetVelocity(m_body1->GetVelocity());

		dVector omega0(m_body0->GetOmega());
		dVector omega1(m_body1->GetOmega());
		dVector omega(
			matrix1.m_front.Scale(matrix1.m_front.DotProduct(omega0).GetScalar()) +
			matrix1.m_up.Scale(matrix1.m_up.DotProduct(omega0).GetScalar()) +
			matrix1.m_right.Scale(matrix1.m_right.DotProduct(omega1).GetScalar()));

		//omega += matrix1.m_front.Scale(5.0f) - matrix1.m_front.Scale(matrix1.m_front.DotProduct(omega0).GetScalar());
		//omega += matrix1.m_up.Scale(5.0f) - matrix1.m_up.Scale(matrix1.m_up.DotProduct(omega0).GetScalar());

		m_body0->SetOmega(omega);
	}

	void JacobianDerivative(ndConstraintDescritor& desc)
	{
		dMatrix matrix0;
		dMatrix matrix1;
		CalculateGlobalMatrix(matrix0, matrix1);

		// save the current joint Omega
		//dVector omega0(m_body0->GetOmega());
		//dVector omega1(m_body1->GetOmega());

		// only one rows to restrict rotation around around the parent coordinate system
		const dFloat32 angleError = m_maxAngleError;
		const dFloat32 angle = CalculateAngle(matrix0.m_front, matrix1.m_front, matrix1.m_right);
		AddAngularRowJacobian(desc, matrix1.m_right, angle);
	}
};

class ndDifferentialAxle : public ndJointBilateralConstraint
{
	public:
	ndDifferentialAxle(const dVector& pin0, const dVector& upPin, ndBodyKinematic* const differentialBody0,
					   const dVector& pin1, ndBodyKinematic* const body1)
		:ndJointBilateralConstraint(1, differentialBody0, body1, dGetIdentityMatrix())
	{
		dMatrix temp;
		dMatrix matrix0(pin0, upPin, pin0.CrossProduct(upPin), dVector::m_wOne);
		dMatrix matrix1(pin1);
		CalculateLocalMatrix(matrix0, m_localMatrix0, temp);
		CalculateLocalMatrix(matrix1, temp, m_localMatrix1);
	}

	void JacobianDerivative(ndConstraintDescritor& desc)
	{
		dMatrix matrix0;
		dMatrix matrix1;
		CalculateGlobalMatrix(matrix0, matrix1);

		AddAngularRowJacobian(desc, matrix1.m_right, dFloat32 (0.0f));
		
		ndJacobian& jacobian0 = desc.m_jacobian[desc.m_rowsCount - 1].m_jacobianM0;
		ndJacobian& jacobian1 = desc.m_jacobian[desc.m_rowsCount - 1].m_jacobianM1;

		jacobian0.m_angular = matrix0.m_front + matrix0.m_up;
		jacobian1.m_angular = matrix1.m_front;
		
		//if ((differential->m_mode == dMultiBodyVehicleDifferential::m_open) ||
		//	(differential->m_mode == dMultiBodyVehicleDifferential::m_slipLocked)) {
		//	jacobian1.m_angular = diffMatrix.m_front + diffMatrix.m_up.Scale(m_diffSign);
		//}
		
		const dVector& omega0 = m_body0->GetOmega();
		const dVector& omega1 = m_body1->GetOmega();
		
		const dVector relOmega(omega0 * jacobian0.m_angular + omega1 * jacobian1.m_angular);
		dFloat32 w = (relOmega.m_x + relOmega.m_y + relOmega.m_z) * dFloat32 (0.5f);
		SetMotorAcceleration(desc, -w * desc.m_invTimestep);
	}
};

ndMultiBodyVehicle::ndMultiBodyVehicle(const dVector& frontDir, const dVector& upDir)
	:ndModel()
	,m_localFrame(dGetIdentityMatrix())
	,m_chassis(nullptr)
	,m_motor(nullptr)
	,m_tireShape(new ndShapeChamferCylinder(dFloat32(0.75f), dFloat32(0.5f)))
	,m_gearBox(nullptr)
	,m_tiresList()
	,m_brakeTires()
	,m_handBrakeTires()
	,m_steeringTires()
	,m_differentials()
	,m_brakeTorque(dFloat32(0.0f))
	,m_steeringAngle(dFloat32 (0.0f))
	,m_handBrakeTorque(dFloat32(0.0f))
	,m_steeringAngleMemory(dFloat32(0.0f))
{
	m_tireShape->AddRef();
	m_localFrame.m_front = frontDir & dVector::m_triplexMask;
	m_localFrame.m_up = upDir & dVector::m_triplexMask;
	m_localFrame.m_right = m_localFrame.m_front.CrossProduct(m_localFrame.m_up).Normalize();
	m_localFrame.m_up = m_localFrame.m_right.CrossProduct(m_localFrame.m_front).Normalize();
}

ndMultiBodyVehicle::ndMultiBodyVehicle(const nd::TiXmlNode* const xmlNode)
	:ndModel(xmlNode)
	,m_localFrame(dGetIdentityMatrix())
	,m_chassis(nullptr)
	,m_tireShape(new ndShapeChamferCylinder(dFloat32(0.75f), dFloat32(0.5f)))
	,m_tiresList()
	,m_handBrakeTires()
	,m_steeringTires()
	,m_brakeTorque(dFloat32(0.0f))
	,m_steeringAngle(dFloat32(0.0f))
	,m_handBrakeTorque(dFloat32(0.0f))
	,m_steeringAngleMemory(dFloat32(0.0f))
{
	m_tireShape->AddRef();
}

ndMultiBodyVehicle::~ndMultiBodyVehicle()
{
	m_tireShape->Release();
}

void ndMultiBodyVehicle::AddChassis(ndBodyDynamic* const chassis)
{
	m_chassis = chassis;
}

void ndMultiBodyVehicle::SetBrakeTorque(dFloat32 brakeToqrue)
{
	m_brakeTorque = dAbs(brakeToqrue);
}

void ndMultiBodyVehicle::SetHandBrakeTorque(dFloat32 brakeToqrue)
{
	m_handBrakeTorque = dAbs(brakeToqrue);
}

void ndMultiBodyVehicle::SetSteeringAngle(dFloat32 angleInRadians)
{
	m_steeringAngle = angleInRadians;
}

ndJointWheel* ndMultiBodyVehicle::AddTire(ndWorld* const world, const ndJointWheel::ndWheelDescriptor& desc, ndBodyDynamic* const tire)
{
	dMatrix tireFrame(dGetIdentityMatrix());
	tireFrame.m_front = dVector(0.0f, 0.0f, 1.0f, 0.0f);
	tireFrame.m_up    = dVector(0.0f, 1.0f, 0.0f, 0.0f);
	tireFrame.m_right = dVector(-1.0f, 0.0f, 0.0f, 0.0f);
	dMatrix matrix (tireFrame * m_localFrame * m_chassis->GetMatrix());
	matrix.m_posit = tire->GetMatrix().m_posit;

	// make tire inertia spherical
	dVector inertia(tire->GetMassMatrix());
	dFloat32 maxInertia(dMax(dMax(inertia.m_x, inertia.m_y), inertia.m_z));
	inertia.m_x = maxInertia;
	inertia.m_y = maxInertia;
	inertia.m_z = maxInertia;
	tire->SetMassMatrix(inertia);

	ndJointWheel* const tireJoint = new ndJointWheel(matrix, tire, m_chassis, desc);
	m_tiresList.Append(tireJoint);
	world->AddJoint(tireJoint);
	return tireJoint;
}

ndBodyDynamic* ndMultiBodyVehicle::CreateInternalBodyPart(ndWorld* const world, dFloat32 mass, dFloat32 radius) const
{
	ndShapeInstance diffCollision(new ndShapeSphere(radius));
	diffCollision.SetCollisionMode(false);

	dAssert(m_chassis);
	ndBodyDynamic* const body = new ndBodyDynamic();
	body->SetMatrix(m_localFrame * m_chassis->GetMatrix());
	body->SetCollisionShape(diffCollision);
	body->SetMassMatrix(mass, diffCollision);
	body->SetGyroMode(false);
	world->AddBody(body);
	return body;
}

ndDifferential* ndMultiBodyVehicle::AddDifferential(ndWorld* const world, dFloat32 mass, dFloat32 radius, ndJointWheel* const leftTire, ndJointWheel* const rightTire)
{
	ndBodyDynamic* const differentialBody = CreateInternalBodyPart(world, mass, radius);

	ndDifferential* const differential = new ndDifferential(differentialBody, m_chassis);
	world->AddJoint(differential);
	m_differentials.Append(differential);

	dVector pin0(differentialBody->GetMatrix().RotateVector(differential->GetLocalMatrix0().m_front));
	dVector upPin(differentialBody->GetMatrix().RotateVector(differential->GetLocalMatrix0().m_up));
	dVector leftPin1(leftTire->GetBody0()->GetMatrix().RotateVector(leftTire->GetLocalMatrix0().m_front));

	ndDifferentialAxle* const leftAxle = new ndDifferentialAxle(pin0, upPin, differentialBody, leftPin1, leftTire->GetBody0());
	world->AddJoint(leftAxle);

	dVector rightPin1(rightTire->GetBody0()->GetMatrix().RotateVector(rightTire->GetLocalMatrix0().m_front));
	ndDifferentialAxle* const rightAxle = new ndDifferentialAxle(pin0, upPin.Scale (dFloat32 (-1.0f)), differentialBody, leftPin1, rightTire->GetBody0());
	world->AddJoint(rightAxle);

	return differential;
}

ndMultiBodyVehicleMotor* ndMultiBodyVehicle::AddMotor(ndWorld* const world, dFloat32 mass, dFloat32 radius, ndDifferential* const differential)
{
	//dAssert(0);
	ndBodyDynamic* const motorBody = CreateInternalBodyPart(world, mass, radius);

	m_motor = new ndMultiBodyVehicleMotor(motorBody, m_chassis);
	world->AddJoint(m_motor);

	m_gearBox = new ndMultiBodyVehicleMotorGearBox(motorBody, differential->GetBody0());
	world->AddJoint(m_gearBox);

	return m_motor;
}

ndShapeInstance ndMultiBodyVehicle::CreateTireShape(dFloat32 radius, dFloat32 width) const
{
	ndShapeInstance tireCollision(m_tireShape);
	dVector scale(2.0f * width, radius, radius, 0.0f);
	tireCollision.SetScale(scale);
	return tireCollision;
}

void ndMultiBodyVehicle::SetAsSteering(ndJointWheel* const tire)
{
	m_steeringTires.Append(tire);
}

void ndMultiBodyVehicle::SetAsBrake(ndJointWheel* const tire)
{
	m_brakeTires.Append(tire);
}

void ndMultiBodyVehicle::SetAsHandBrake(ndJointWheel* const tire)
{
	m_handBrakeTires.Append(tire);
}

void ndMultiBodyVehicle::Update(const ndWorld* const world, dFloat32 timestep)
{
	ApplyAligmentAndBalancing();
	ApplyBrakes();
	ApplySteering();
	ApplyTiremodel();
}

void ndMultiBodyVehicle::ApplyAligmentAndBalancing()
{
	const dVector chassisOmega(m_chassis->GetOmega());
	const dVector upDir(m_chassis->GetMatrix().RotateVector(m_localFrame.m_up));
	for (dList<ndJointWheel*>::dListNode* node = m_tiresList.GetFirst(); node; node = node->GetNext())
	{
		ndJointWheel* const tire = node->GetInfo();

		ndBodyDynamic* const tireBody = tire->GetBody0()->GetAsBodyDynamic();
		dAssert(tireBody != m_chassis);
		if (!tireBody->GetSleepState())
		{
			dMatrix tireMatrix;
			dMatrix chassisMatrix;
			tire->CalculateGlobalMatrix(tireMatrix, chassisMatrix);

			// align tire matrix 
			const dVector relPosit(tireMatrix.m_posit - chassisMatrix.m_posit);
			const dFloat32 distance = relPosit.DotProduct(upDir).GetScalar();
			const dFloat32 spinAngle = -tire->CalculateAngle(tireMatrix.m_up, chassisMatrix.m_up, chassisMatrix.m_front);

			dMatrix newTireMatrix(dPitchMatrix(spinAngle) * chassisMatrix);
			newTireMatrix.m_posit = chassisMatrix.m_posit + upDir.Scale(distance);

			dMatrix tireBodyMatrix(tire->GetLocalMatrix0().Inverse() * newTireMatrix);
			tireBody->SetMatrix(tireBodyMatrix);

			// align tire velocity
			const dVector chassiVelocity(m_chassis->GetVelocityAtPoint(tireBodyMatrix.m_posit));
			const dVector relVeloc(tireBody->GetVelocity() - chassiVelocity);
			const dFloat32 speed = relVeloc.DotProduct(upDir).GetScalar();
			const dVector tireVelocity(chassiVelocity + upDir.Scale(speed));
			tireBody->SetVelocity(tireVelocity);

			// align tire angular velocity
			const dVector relOmega(tireBody->GetOmega() - chassisOmega);
			const dFloat32 rpm = relOmega.DotProduct(chassisMatrix.m_front).GetScalar();
			const dVector tireOmega(chassisOmega + chassisMatrix.m_front.Scale(rpm));
			tireBody->SetOmega(tireOmega);
		}
	}

	for (dList<ndDifferential*>::dListNode* node = m_differentials.GetFirst(); node; node = node->GetNext())
	{
		ndDifferential* const diff = node->GetInfo();
		diff->AlignMatrix();
	}

	if (m_motor)
	{
		m_motor->AlignMatrix();
	}
}

void ndMultiBodyVehicle::ApplySteering()
{
	if (dAbs(m_steeringAngleMemory - m_steeringAngle) > dFloat32(1.0e-3f))
	{
		m_steeringAngleMemory = m_steeringAngle;
		for (dList<ndJointWheel*>::dListNode* node = m_steeringTires.GetFirst(); node; node = node->GetNext())
		{
			ndJointWheel* const tire = node->GetInfo();
			tire->SetSteeringAngle(m_steeringAngle);
		}
	}
}

void ndMultiBodyVehicle::Debug(ndConstraintDebugCallback& context) const
{
	// draw vehicle cordinade system;
	dMatrix chassisMatrix(m_chassis->GetMatrix());
	chassisMatrix.m_posit = chassisMatrix.TransformVector(m_chassis->GetCentreOfMass());
	context.DrawFrame(chassisMatrix);

	// draw velocity vector
	dVector veloc(m_chassis->GetVelocity());
	dVector p0(chassisMatrix.m_posit + m_localFrame.m_up.Scale(1.0f));
	dVector p1(p0 + veloc.Scale (0.25f));
	context.DrawLine(p0, p1, dVector(1.0f, 1.0f, 0.0f, 0.0f));

	// draw front direction for side slip angle reference
	dVector p2(p0 + chassisMatrix.RotateVector(m_localFrame.m_front).Scale(0.5f));
	context.DrawLine(p0, p2, dVector(1.0f, 1.0f, 1.0f, 0.0f));

	dVector weight(m_chassis->GetForce());
	dFloat32 scale = dSqrt(weight.DotProduct(weight).GetScalar());
	weight = weight.Normalize().Scale(-2.0f);

	// draw vehicle weight;
	dVector forceColor(dFloat32 (0.0f), dFloat32(0.0f), dFloat32(1.0f), dFloat32(0.0f));
	dVector lateralColor(dFloat32(1.0f), dFloat32(0.0f), dFloat32(0.0f), dFloat32(0.0f));
	dVector longitudinalColor(dFloat32(0.0f), dFloat32(1.0f), dFloat32(0.0f), dFloat32(0.0f));
	context.DrawLine(chassisMatrix.m_posit, chassisMatrix.m_posit + weight, forceColor);

	for (dList<ndJointWheel*>::dListNode* node = m_tiresList.GetFirst(); node; node = node->GetNext())
	{
		ndJointWheel* const tire = node->GetInfo();
		ndBodyDynamic* const tireBody = tire->GetBody0()->GetAsBodyDynamic();

		//tire->DebugJoint(context);
		//dVector color(1.0f, 1.0f, 1.0f, 0.0f);
		//context.DrawArrow(tireBody->GetMatrix(), color, -1.0f);

		// draw tire forces
		const ndBodyKinematic::ndContactMap& contactMap = tireBody->GetContactMap();
		ndBodyKinematic::ndContactMap::Iterator it(contactMap);
		for (it.Begin(); it; it++)
		{
			ndContact* const contact = *it;
			if (contact->IsActive())
			{
				const ndContactPointList& contactPoints = contact->GetContactPoints();
				for (ndContactPointList::dListNode* contactNode = contactPoints.GetFirst(); contactNode; contactNode = contactNode->GetNext())
				{
					const ndContactMaterial& contactPoint = contactNode->GetInfo();
					dMatrix frame(contactPoint.m_normal, contactPoint.m_dir0, contactPoint.m_dir1, contactPoint.m_point);

					dVector localPosit(m_localFrame.UntransformVector(chassisMatrix.UntransformVector(contactPoint.m_point)));
					dFloat32 offset = (localPosit.m_z > dFloat32(0.0f)) ? dFloat32(0.2f) : dFloat32(-0.2f);
					frame.m_posit += contactPoint.m_dir0.Scale(offset);
					frame.m_posit += contactPoint.m_normal.Scale(0.1f);
					//context.DrawFrame(frame);

					// normal force
					dFloat32 normalForce = dFloat32 (2.0f) * contactPoint.m_normal_Force.m_force / scale;
					context.DrawLine(frame.m_posit, frame.m_posit + contactPoint.m_normal.Scale (normalForce), forceColor);

					// lateral force
					dFloat32 lateralForce = dFloat32(2.0f) * contactPoint.m_dir0_Force.m_force / scale;
					context.DrawLine(frame.m_posit, frame.m_posit + contactPoint.m_dir0.Scale(lateralForce), lateralColor);

					// longitudinal force
					dFloat32 longitudinalForce = dFloat32(2.0f) * contactPoint.m_dir1_Force.m_force / scale;
					context.DrawLine(frame.m_posit, frame.m_posit + contactPoint.m_dir1.Scale(longitudinalForce), longitudinalColor);
				}
			}
		}
	}
}

void ndMultiBodyVehicle::ApplyBrakes()
{
	for (dList<ndJointWheel*>::dListNode* node = m_tiresList.GetFirst(); node; node = node->GetNext())
	{
		ndJointWheel* const tire = node->GetInfo();
		tire->SetBrakeTorque(dFloat32 (0.0f));
	}

	for (dList<ndJointWheel*>::dListNode* node = m_brakeTires.GetFirst(); node; node = node->GetNext())
	{
		ndJointWheel* const tire = node->GetInfo();
		tire->SetBrakeTorque(m_brakeTorque);
	}

	if (m_brakeTorque == dFloat32(0.0f))
	{
		for (dList<ndJointWheel*>::dListNode* node = m_handBrakeTires.GetFirst(); node; node = node->GetNext())
		{
			ndJointWheel* const tire = node->GetInfo();

			tire->SetBrakeTorque(m_handBrakeTorque);
		}
	}
}

void ndMultiBodyVehicle::BrushTireModel(const ndJointWheel* const tire, ndContactMaterial& contactPoint) const
{
	// calculate longitudinal slip ratio
	const ndBodyDynamic* const tireBody = tire->GetBody0()->GetAsBodyDynamic();
	const ndBodyDynamic* const otherBody = (contactPoint.m_body0 == tireBody) ? ((ndBodyKinematic*)contactPoint.m_body1)->GetAsBodyDynamic() : ((ndBodyKinematic*)contactPoint.m_body0)->GetAsBodyDynamic();
	dAssert(tireBody != otherBody);
	dAssert((tireBody == contactPoint.m_body0) || (tireBody == contactPoint.m_body1));

	const dVector tireVeloc(tireBody->GetVelocity());
	const dVector contactVeloc0(tireBody->GetVelocityAtPoint(contactPoint.m_point));
	const dVector contactVeloc1(otherBody->GetVelocityAtPoint(contactPoint.m_point));
	const dVector relVeloc(contactVeloc0 - contactVeloc1);

	const dFloat32 relSpeed = dAbs (relVeloc.DotProduct(contactPoint.m_dir1).GetScalar());
	const dFloat32 tireSpeed = dMax (dAbs (tireVeloc.DotProduct(contactPoint.m_dir1).GetScalar()), dFloat32 (0.1f));
	const dFloat32 longitudialSlip = relSpeed / tireSpeed;

	// calculate side slip ratio
	const dFloat32 sideSpeed = dAbs(relVeloc.DotProduct(contactPoint.m_dir0).GetScalar());
	const dFloat32 lateralSleep = sideSpeed / (relSpeed + dFloat32 (0.1f));

	const dFloat32 den = dFloat32(1.0f) / (dFloat32(1.0f) + longitudialSlip);
	const dFloat32 v = lateralSleep * den;
	const dFloat32 u = longitudialSlip * den;

	const ndJointWheel::ndWheelDescriptor& info = tire->GetInfo();
	const dFloat32 cz = info.m_laterialStiffeness * v;
	const dFloat32 cx = info.m_longitudinalStiffeness * u;
	const dFloat32 gamma = dSqrt(cx * cx + cz * cz) + dFloat32 (1.0e-3f);

	const dFloat32 frictionCoefficient = GetFrictionCoeficient(tire, contactPoint);
	// the code bellow not needed if we use a rigid body solver, 
	// since the solve will calculate the correct forces.
	//dAssert(gamma > dFloat32(0.0f));
	//const dFloat32 maxGamma = dFloat32(3.0f) * frictionCoefficient * contactPoint.m_normal_Force.m_force;
	//dFloat32 normalForce = frictionCoefficient * contactPoint.m_normal_Force.m_force;
	//if (gamma <= maxGamma)
	//{
	//	normalForce = gamma * (dFloat32(1.0f) - gamma / dFloat32 (3.0f) + gamma * gamma / dFloat32 (27.0f));
	//}

	const dFloat32 lateralFrictionCoefficient = frictionCoefficient * cz / gamma;
	const dFloat32 longitudinalFrictionCoefficient = frictionCoefficient * cx / gamma;

	contactPoint.m_material.m_staticFriction0 = lateralFrictionCoefficient;
	contactPoint.m_material.m_dynamicFriction0 = lateralFrictionCoefficient;
	contactPoint.m_material.m_staticFriction1 = longitudinalFrictionCoefficient;
	contactPoint.m_material.m_dynamicFriction1 = longitudinalFrictionCoefficient;
	//contactPoint.m_material.m_restitution = 0.0f;
}

void ndMultiBodyVehicle::ApplyTiremodel()
{
	for (dList<ndJointWheel*>::dListNode* node = m_tiresList.GetFirst(); node; node = node->GetNext())
	{
		ndJointWheel* const tire = node->GetInfo();

		const dMatrix tireMatrix (tire->GetLocalMatrix1() * tire->GetBody1()->GetMatrix());
		const ndBodyKinematic::ndContactMap& contactMap = tire->GetBody0()->GetContactMap();
		ndBodyKinematic::ndContactMap::Iterator it(contactMap);
		for (it.Begin(); it; it++)
		{
			ndContact* const contact = *it;
			if (contact->IsActive())
			{
				const ndContactPointList& contactPoints = contact->GetContactPoints();
				for (ndContactPointList::dListNode* contactNode = contactPoints.GetFirst(); contactNode; contactNode = contactNode->GetNext())
				{
					ndContactMaterial& contactPoint = contactNode->GetInfo();
					const dVector fronDir(contactPoint.m_normal.CrossProduct(tireMatrix.m_front));
					if (fronDir.DotProduct(fronDir).GetScalar() > dFloat32(1.0e-3f))
					{
						contactPoint.m_dir1 = fronDir.Normalize();
						contactPoint.m_dir0 = contactPoint.m_dir1.CrossProduct(contactPoint.m_normal);
						BrushTireModel(tire, contactPoint);
					}
				}
			}
		}
	}
}