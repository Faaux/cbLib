#pragma once
#include <cbMemory.h>

struct RenderCommandGroup
{
	uint32 Width, Height;

	mem_size BufferSize;
	uint8 *BufferBase;
	uint8 *BufferDataAt;
};

#define RenderCommandStruct(MaxPushBufferSize, PushBuffer, Width, Height) \
{Width, Height, MaxPushBufferSize, (uint8 *)PushBuffer, ((uint8 *)PushBuffer) + MaxPushBufferSize};

enum RenderActionType
{
	RenderActionType_RenderStringData
};

struct RenderCommandHeader
{
	RenderActionType Action;
};

struct RenderStringData
{
	uint32 Size;
	uint32 X, Y;
	uint32 CurrentLength;
	char Text[256];
};

#define PushRenderElement(Group, type) (type *)PushRenderElement_(Group, sizeof(type), RenderActionType_##type)
inline void* PushRenderElement_(RenderCommandGroup* renderGroup, uint32 dataSize, RenderActionType action)
{
	uint32 totalSize = dataSize + sizeof(RenderCommandHeader);

	Assert(renderGroup->BufferDataAt - totalSize >= renderGroup->BufferBase)
	renderGroup->BufferDataAt -= totalSize;
	RenderCommandHeader *header = (RenderCommandHeader *)renderGroup->BufferDataAt;
	header->Action = action;


	return (uint8 *)header + sizeof(*header);
}

inline void PushRenderString(RenderCommandGroup* renderGroup, uint32 fontSize, uint32 x, uint32 y, char* text)
{
	RenderStringData* data = PushRenderElement(renderGroup, RenderStringData);
	
	data->Size = fontSize;
	data->X = x;
	data->Y = y;
	
	ZeroArray(ArrayCount(data->Text), &data->Text[0]);

	char *target = &data->Text[0];
	int length = 0;
	// Copy Text
	while(*text)
	{
		*(target++) = *(text++);
		++length;
	}
	data->CurrentLength = length;
}
