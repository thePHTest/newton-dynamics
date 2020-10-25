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

/****************************************************************************
*
*  Visual C++ 6.0 created by: Julio Jerez
*
****************************************************************************/

#include "dCoreStdafx.h"
#include "dMemory.h"
#include "dPolygonSoupDatabase.h"

dPolygonSoupDatabase::dPolygonSoupDatabase(const char* const name)
{
	m_vertexCount = 0;
	m_strideInBytes = 0;
	m_localVertex = nullptr;
}

dPolygonSoupDatabase::~dPolygonSoupDatabase ()
{
	if (m_localVertex) 
	{
		dMemory::Free (m_localVertex);
	}
}

dUnsigned32 dPolygonSoupDatabase::GetTagId(const dInt32* const face, dInt32 indexCount) const
{
	return dUnsigned32 (face[indexCount]);
}

void dPolygonSoupDatabase::SetTagId(const dInt32* const facePtr, dInt32 indexCount, dUnsigned32 newID) const
{
	dUnsigned32* const face = (dUnsigned32*) facePtr;
	face[indexCount] = newID;
}

dInt32 dPolygonSoupDatabase::GetVertexCount()	const
{
	return m_vertexCount;
}

dFloat32* dPolygonSoupDatabase::GetLocalVertexPool() const
{
	return m_localVertex;
}

dInt32 dPolygonSoupDatabase::GetStrideInBytes() const
{
	return m_strideInBytes;
}

dFloat32 dPolygonSoupDatabase::GetRadius() const
{
	return 0.0f;
}


