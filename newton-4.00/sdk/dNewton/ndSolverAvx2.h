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

#ifndef __D_SOLVER_AVX2__
#define __D_SOLVER_AVX2__

#include "dCoreStdafx.h"
#include "ndNewtonStdafx.h"

namespace ndAvx2
{
	#define D_SOA_WORD_GROUP_SIZE	8 

	#define D_RADIX_BITS	5
	#define D_RADIX_DIGIT	(1<<(D_RADIX_BITS + 1))

	D_MSV_NEWTON_ALIGN_32
	class ndSoaFloat
	{
		public:
		D_INLINE ndSoaFloat()
		{
		}

		D_INLINE ndSoaFloat(const dFloat32 val)
			:m_type(_mm256_set1_ps(val))
		{
		}

		D_INLINE ndSoaFloat(const __m256 type)
			: m_type(type)
		{
		}

		D_INLINE ndSoaFloat(const ndSoaFloat& copy)
			: m_type(copy.m_type)
		{
		}

		D_INLINE ndSoaFloat(const dVector& low, const dVector& high)
			: m_type(_mm256_set_m128(high.m_type, low.m_type))
		{
		}


		D_INLINE ndSoaFloat(const ndSoaFloat* const baseAddr, const ndSoaFloat& index)
			: m_type(_mm256_i32gather_ps(&(*baseAddr)[0], index.m_typeInt, 4))
		{
		}

		D_INLINE dFloat32& operator[] (dInt32 i)
		{
			dAssert(i < D_SOA_WORD_GROUP_SIZE);
			dAssert(i >= 0);
			//return m_f[i];
			dFloat32* const ptr = (dFloat32*)&m_type;
			return ptr[i];
		}

		D_INLINE const dFloat32& operator[] (dInt32 i) const
		{
			dAssert(i < D_SOA_WORD_GROUP_SIZE);
			dAssert(i >= 0);
			//return m_f[i];
			const dFloat32* const ptr = (dFloat32*)&m_type;
			return ptr[i];
		}

		D_INLINE ndSoaFloat operator+ (const ndSoaFloat& A) const
		{
			return _mm256_add_ps(m_type, A.m_type);
		}

		D_INLINE ndSoaFloat operator- (const ndSoaFloat& A) const
		{
			return _mm256_sub_ps(m_type, A.m_type);
		}

		D_INLINE ndSoaFloat operator* (const ndSoaFloat& A) const
		{
			return _mm256_mul_ps(m_type, A.m_type);
		}

		D_INLINE ndSoaFloat MulAdd(const ndSoaFloat& A, const ndSoaFloat& B) const
		{
			return _mm256_fmadd_ps(A.m_type, B.m_type, m_type);
		}

		D_INLINE ndSoaFloat MulSub(const ndSoaFloat& A, const ndSoaFloat& B) const
		{
			return _mm256_fnmadd_ps(A.m_type, B.m_type, m_type);
		}

		D_INLINE ndSoaFloat operator> (const ndSoaFloat& A) const
		{
			return _mm256_cmp_ps(m_type, A.m_type, _CMP_GT_OQ);
		}

		D_INLINE ndSoaFloat operator< (const ndSoaFloat& A) const
		{
			return _mm256_cmp_ps(m_type, A.m_type, _CMP_LT_OQ);
		}

		D_INLINE ndSoaFloat operator| (const ndSoaFloat& A) const
		{
			return _mm256_or_ps(m_type, A.m_type);
		}

		D_INLINE ndSoaFloat operator& (const ndSoaFloat& A) const
		{
			return _mm256_and_ps(m_type, A.m_type);
		}

		D_INLINE ndSoaFloat GetMin(const ndSoaFloat& A) const
		{
			return _mm256_min_ps(m_type, A.m_type);
		}

		D_INLINE ndSoaFloat GetMax(const ndSoaFloat& A) const
		{
			return _mm256_max_ps(m_type, A.m_type);
		}

		D_INLINE dFloat32 AddHorizontal() const
		{
			__m256 tmp0(_mm256_add_ps(m_type, _mm256_permute2f128_ps(m_type, m_type, 1)));
			__m256 tmp1(_mm256_hadd_ps(tmp0, tmp0));
			__m256 tmp2(_mm256_hadd_ps(tmp1, tmp1));
			//int xxx = _mm256_cvtsi256_si32 (tmp2);
			return *((dFloat32*)&tmp2);
		}

		static D_INLINE void FlushRegisters()
		{
			_mm256_zeroall();
		}

		union
		{
			__m256 m_type;
			__m256i m_typeInt;
		};
	} D_GCC_NEWTON_ALIGN_32;

	D_MSV_NEWTON_ALIGN_32
	class ndSoaVector3
	{
		public:
		ndSoaFloat m_x;
		ndSoaFloat m_y;
		ndSoaFloat m_z;
	} D_GCC_NEWTON_ALIGN_32;

	D_MSV_NEWTON_ALIGN_32
	class ndSoaVector6
	{
		public:
		ndSoaVector3 m_linear;
		ndSoaVector3 m_angular;
	} D_GCC_NEWTON_ALIGN_32;

	D_MSV_NEWTON_ALIGN_32
	class ndSoaJacobianPair
	{
		public:
		ndSoaVector6 m_jacobianM0;
		ndSoaVector6 m_jacobianM1;
	} D_GCC_NEWTON_ALIGN_32;


	D_MSV_NEWTON_ALIGN_32
	class ndSoaMatrixElement
	{
		public:
		ndSoaJacobianPair m_Jt;
		ndSoaJacobianPair m_JMinv;

		ndSoaFloat m_force;
		ndSoaFloat m_diagDamp;
		ndSoaFloat m_invJinvMJt;
		ndSoaFloat m_coordenateAccel;
		ndSoaFloat m_normalForceIndex;
		ndSoaFloat m_lowerBoundFrictionCoefficent;
		ndSoaFloat m_upperBoundFrictionCoefficent;
	} D_GCC_NEWTON_ALIGN_32;

	class ndSoaMassMatrixElement
	{
		public:
		ndSoaMatrixElement m_matrix[D_CONSTRAINT_MAX_ROWS];
	};
};

#endif
