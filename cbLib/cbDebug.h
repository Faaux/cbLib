#pragma once
#include "cbInclude.h"
enum debug_type : uint8
{
	FrameStart,
	FrameEnd,
	BeginBlock,
	EndBlock
};

struct event_result
{
	uint64 ElapsedCylces;
	uint32 Level;
	uint32 Index;
	char *GUID;
};


struct debug_event
{
	uint64 Clock;
	char *GUID;
	uint8 Type;

	union
	{
		uint32 Value_U;
		int32 Value_S;
		float Value_F;
		bool Value_B;
	};
};

struct debug_table
{
	uint64 CurrentIndex;
	uint64 NextIndex;
	uint64 volatile NextEventIndex;
	debug_event Events[65536];
};

extern debug_table *GlobalDebugTable;

#define UniqueFileCounterString__(A, B, C, D) A "|" #B "|" #C "|" D
#define UniqueFileCounterString_(A, B, C, D) UniqueFileCounterString__(A, B, C, D)
#define DEBUG_NAME(Name) UniqueFileCounterString_(__FILE__, __LINE__, __COUNTER__, Name)

#define RecordDebugEvent(EventType, GUIDInit)	uint64 totalEventIndex = GlobalDebugTable->NextEventIndex++;		\
												uint32 eventIndex = totalEventIndex & 0xFFFF;					\
												Assert(eventIndex < ArrayCount(GlobalDebugTable->Events));			\
												debug_event *Event = &GlobalDebugTable->Events[totalEventIndex >> 32] + eventIndex;			\
												Event->Clock = __rdtsc();											\
												Event->Type = EventType;											\
												Event->GUID = GUIDInit;												\
												if(EventType == FrameStart) {										\
													GlobalDebugTable->CurrentIndex = GlobalDebugTable->NextIndex;	\
													GlobalDebugTable->NextIndex = (totalEventIndex >> 32) + eventIndex; }						\

#define TIMED_BLOCK__(GUID, Number, ...) timed_block TimedBlock_##Number(GUID, ## __VA_ARGS__)
#define TIMED_BLOCK_(GUID, Number, ...) TIMED_BLOCK__(GUID, Number, ## __VA_ARGS__)
#define TIMED_BLOCK(Name, ...) TIMED_BLOCK_(DEBUG_NAME(Name), __COUNTER__, ## __VA_ARGS__)
#define TIMED_FUNCTION(...) TIMED_BLOCK_(DEBUG_NAME(__FUNCTION__), ## __VA_ARGS__)

#define BEGIN_BLOCK_(GUID) {RecordDebugEvent(BeginBlock, GUID);}
#define END_BLOCK_(GUID) {RecordDebugEvent(EndBlock, GUID);}

#define BEGIN_BLOCK(Name) BEGIN_BLOCK_(DEBUG_NAME(Name))
#define END_BLOCK() END_BLOCK_(DEBUG_NAME("END_BLOCK_"))

#define FRAME_START() {RecordDebugEvent(FrameStart, DEBUG_NAME("Frame Start"));}
#define FRAME_END() {RecordDebugEvent(FrameEnd, DEBUG_NAME("Frame End"));}

#include <intrin.h>
struct timed_block
{
	timed_block(char *GUID)
	{
		BEGIN_BLOCK_(GUID);
	}

	~timed_block()
	{
		END_BLOCK();
	}
};