#pragma once
//------------------------------------------------------------------------------
/**
	A shader read-write buffer can be both read and written from within a shader.

	(C)2017-2020 Individual contributors, see AUTHORS file
*/
//------------------------------------------------------------------------------
#include "ids/id.h"
#include "ids/idpool.h"
#include "util/stringatom.h"
#include "coregraphics/config.h"

namespace CoreGraphics
{
ID_24_8_TYPE(ShaderRWBufferId);

enum ShaderRWBufferUsage
{
	TransferSource			= (1 << 0),
	TransferDestination		= (1 << 1),
	ShaderMutable			= (1 << 2)
};
__ImplementEnumBitOperators(ShaderRWBufferUsage);

struct ShaderRWBufferCreateInfo
{
	Util::StringAtom name;
	SizeT size;
	BufferUpdateMode mode;
	ShaderRWBufferUsage usage;	// when set, buffer will only be used for uploading
	bool screenRelative : 1;	// when set, size is bytes per pixel
};

/// create shader RW buffer
const ShaderRWBufferId CreateShaderRWBuffer(const ShaderRWBufferCreateInfo& info);
/// destroy shader RW buffer
void DestroyShaderRWBuffer(const ShaderRWBufferId id);

/// update data in buffer
void ShaderRWBufferUpdate(const ShaderRWBufferId id, void* data, SizeT bytes);

} // CoreGraphics
