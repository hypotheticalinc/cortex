//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008-2012, Image Engine Design Inc. All rights reserved.
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

#include <cassert>

#include "boost/format.hpp"

#include "IECore/MeshPrimitive.h"
#include "IECore/TriangulateOp.h"
#include "IECore/MeshNormalsOp.h"
#include "IECore/DespatchTypedData.h"
#include "IECore/MessageHandler.h"
#include "IECore/FaceVaryingPromotionOp.h"

#include "IECoreGL/ToGLMeshConverter.h"
#include "IECoreGL/MeshPrimitive.h"

using namespace IECoreGL;

IE_CORE_DEFINERUNTIMETYPED( ToGLMeshConverter );

ToGLConverter::ConverterDescription<ToGLMeshConverter> ToGLMeshConverter::g_description;

ToGLMeshConverter::ToGLMeshConverter( IECore::ConstMeshPrimitivePtr toConvert )
	:	ToGLConverter( "Converts IECore::MeshPrimitive objects to IECoreGL::MeshPrimitive objects.", IECore::MeshPrimitiveTypeId )
{
	srcParameter()->setValue( boost::const_pointer_cast<IECore::MeshPrimitive>( toConvert ) );
}

ToGLMeshConverter::~ToGLMeshConverter()
{
}

IECore::RunTimeTypedPtr ToGLMeshConverter::doConversion( IECore::ConstObjectPtr src, IECore::ConstCompoundObjectPtr operands ) const
{
	IECore::MeshPrimitivePtr mesh = boost::static_pointer_cast<IECore::MeshPrimitive>( src->copy() ); // safe because the parameter validated it for us
	
	if( !mesh->variableData<IECore::V3fVectorData>( "P", IECore::PrimitiveVariable::Vertex ) )
	{
		throw IECore::Exception( "Must specify primitive variable \"P\", of type V3fVectorData and interpolation type Vertex." );
	}

	if( mesh->variables.find( "N" )==mesh->variables.end() )
	{
		// the mesh has no normals - we need to explicitly add some. if it's a polygon
		// mesh (interpolation==linear) then we add per-face normals for a faceted look
		// and if it's a subdivision mesh we add smooth per-vertex normals.
		IECore::MeshNormalsOpPtr normalOp = new IECore::MeshNormalsOp();
		normalOp->inputParameter()->setValue( mesh );
		normalOp->copyParameter()->setTypedValue( false );
		normalOp->interpolationParameter()->setNumericValue(
			mesh->interpolation() == "linear" ? IECore::PrimitiveVariable::Uniform : IECore::PrimitiveVariable::Vertex
		);
		normalOp->operate();
	}
	
	IECore::TriangulateOpPtr op = new IECore::TriangulateOp();
	op->inputParameter()->setValue( mesh );
	op->throwExceptionsParameter()->setTypedValue( false ); // it's better to see something than nothing
	op->copyParameter()->setTypedValue( false );
	op->operate();

	IECore::FaceVaryingPromotionOpPtr faceVaryingOp = new IECore::FaceVaryingPromotionOp;
	faceVaryingOp->inputParameter()->setValue( mesh );
	faceVaryingOp->copyParameter()->setTypedValue( false );
	faceVaryingOp->operate();

	MeshPrimitivePtr glMesh = new MeshPrimitive( mesh->numFaces() );

	for ( IECore::PrimitiveVariableMap::iterator pIt = mesh->variables.begin(); pIt != mesh->variables.end(); ++pIt )
	{
		if( pIt->first == "uv" )
		{
			if( IECore::V2fVectorDataPtr uvData = mesh->expandedVariableData<IECore::V2fVectorData>( "uv", IECore::PrimitiveVariable::FaceVarying ) )
			{
				std::vector<Imath::V2f> &uvs = uvData->writable();
				for( unsigned i = 0; i < uvs.size(); ++i )
				{
					// as of Cortex 10, we take a UDIM centric approach
					// to UVs, which clashes with OpenGL, so we must flip
					// the v values during conversion.
					uvs[i][1] = 1.0 - uvs[i][1];
				}

				glMesh->addPrimitiveVariable( "uv", IECore::PrimitiveVariable( IECore::PrimitiveVariable::FaceVarying, uvData ) );
			}
			else
			{
				IECore::msg( IECore::Msg::Warning, "ToGLMeshConverter", "If specified, primitive variable \"uv\" must be of type V2fVectorData." );
			}
		}
		else if( pIt->second.data )
		{
			glMesh->addPrimitiveVariable( pIt->first, pIt->second );
		}
		else
		{
			IECore::msg( IECore::Msg::Warning, "ToGLMeshConverter", boost::format( "No data given for primvar \"%s\"" ) % pIt->first );
		}
	}

	return glMesh;
}
