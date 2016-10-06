#pragma once

struct RenderStringData
{
	uint32 SizeInPx;
	uint32 X, Y;
	char *Text;
};

inline void PushRenderString(cbArena* arena, uint32 fontSize, uint32 x, uint32 y, uint32 textLength, char* text)
{
	RenderCommand* command = PushStruct(arena, RenderCommand);
	RenderStringData* data = PushStruct(arena, RenderStringData);
	char *string = (char *)PushSize(arena, textLength);


	data->SizeInPx = fontSize;
	data->X = x;
	data->Y = y;
	data->Text = string;

	// Copy Text
	while(*text)
	{
		*(string++) = *(text++);
	}

	command->Action = RenderString;
	command->OffsetToNext = (data->Text + textLength) - (char *)command;
	command->IsValid = true;
	command->Data = data;

}