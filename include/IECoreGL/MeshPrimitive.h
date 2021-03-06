//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2013, Image Engine Design Inc. All rights reserved.
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions are
//  met:
//
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//
//     * Neither the name of Image Engine Design nor the names of any
//       other contributors to this software may be used to endorse or
//       promote products derived from this software without specific prior
//       written permission.
//
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
//  IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
//  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
//  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
//  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
//  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
//  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
//  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
//  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
//  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
//  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//////////////////////////////////////////////////////////////////////////

#ifndef IECOREGL_MESHPRIMITIVE_H
#define IECOREGL_MESHPRIMITIVE_H

#include "IECoreGL/Export.h"
#include "IECoreGL/Primitive.h"

#include "IECore/VectorTypedData.h"

namespace IECoreGL
{

/// \todo Fast drawing, uvs etc. Consider using NVIDIA tristrip library? something else? GLU?
class IECOREGL_API MeshPrimitive : public Primitive
{

	public :

		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( IECoreGL::MeshPrimitive, MeshPrimitiveTypeId, Primitive );

		MeshPrimitive( unsigned numTriangles );
		~MeshPrimitive() override;

		Imath::Box3f bound() const override;

		void addPrimitiveVariable( const std::string &name, const IECoreScene::PrimitiveVariable &primVar ) override;

		void renderInstances( size_t numInstances = 1 ) const override;

	private :

		IE_CORE_FORWARDDECLARE( MemberData );
		MemberDataPtr m_memberData;

		/// So Font can use the render( state, style ) method.
		friend class Font;

};

IE_CORE_DECLAREPTR( MeshPrimitive );

} // namespace IECoreGL

#endif // IECOREGL_MESHPRIMITIVE_H
