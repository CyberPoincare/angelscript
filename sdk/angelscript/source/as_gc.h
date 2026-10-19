/*
   AngelCode Scripting Library
   Copyright (c) 2003-2018 Andreas Jonsson

   This software is provided 'as-is', without any express or implied 
   warranty. In no event will the authors be held liable for any 
   damages arising from the use of this software.

   Permission is granted to anyone to use this software for any 
   purpose, including commercial applications, and to alter it and 
   redistribute it freely, subject to the following restrictions:

   1. The origin of this software must not be misrepresented; you 
      must not claim that you wrote the original software. If you use
      this software in a product, an acknowledgment in the product 
      documentation would be appreciated but is not required.

   2. Altered source versions must be plainly marked as such, and 
      must not be misrepresented as being the original software.

   3. This notice may not be removed or altered from any source 
      distribution.

   The original version of this library can be located at:
   http://www.angelcode.com/angelscript/

   Andreas Jonsson
   andreas@angelcode.com
*/


//
// as_gc.h
//
// The garbage collector is used to resolve cyclic references
//



#ifndef AS_GC_H
#define AS_GC_H

#include "as_config.h"
#include "as_array.h"
#include "as_map.h"
#include "as_thread.h"
#include <mutex>
#include <unordered_map>

BEGIN_AS_NAMESPACE

class asCScriptEngine;
class asCObjectType;

class asCGarbageCollector
{
public:
	asCGarbageCollector();
	~asCGarbageCollector();

	int    GarbageCollect(asDWORD flags, asUINT iterations);
	void   GetStatistics(asUINT *currentSize, asUINT *totalDestroyed, asUINT *totalDetected, asUINT *newObjects, asUINT *totalNewDestroyed) const;
	void   GCEnumCallback(void *reference);
	void   SetObjectAlias(void *alias, void *object);
	int    AddScriptObjectToGC(void *obj, asCObjectType *objType);
	int    GetObjectInGC(asUINT idx, asUINT *seqNbr, void **obj, asITypeInfo **type);

	int    ReportAndReleaseUndestroyedObjects(asCScriptEngine *closingEngine);
	bool   EnterEngineShutdown(asCScriptEngine *closingEngine);
	void   LeaveEngineShutdown();
	void   DrainDeferredShutdowns();

	asCScriptEngine *engine;

	// Callback for when circular reference are detected
	asCIRCULARREFFUNC_t circularRefDetectCallbackFunc;
	void *              circularRefDetectCallbackParam;

protected:
	struct asSObjTypePair {void *obj; asCObjectType *type; asUINT seqNbr;};
	struct asSIntTypePair {int i; asCObjectType *type;};
	typedef asSMapNode<void*, asSIntTypePair> asSMapNode_t;

	enum egcDestroyState
	{
		destroyGarbage_init = 0,
		destroyGarbage_loop,
		destroyGarbage_haveMore
	};

	enum egcDetectState
	{
		clearCounters_init = 0,
		clearCounters_loop,
		buildMap_init,
		buildMap_loop,
		countReferences_init,
		countReferences_loop,
		detectGarbage_init,
		detectGarbage_loop1,
		detectGarbage_loop2,
		verifyUnmarked_init,
		verifyUnmarked_loop,
		breakCircles_init,
		breakCircles_loop,
		breakCircles_haveGarbage
	};

	int            DestroyNewGarbage();
	int            DestroyOldGarbage();
	int            IdentifyGarbageWithCyclicRefs();
	asSObjTypePair GetNewObjectAtIdx(int idx);
	asSObjTypePair GetOldObjectAtIdx(int idx);
	void           RemoveNewObjectAtIdx(int idx);
	void           RemoveOldObjectAtIdx(int idx);
	void           RemoveObjectAliases(void *object);
	void           RemoveObjectAlias(void *alias);
	void           MoveObjectToOldList(int idx);
	void           MoveAllObjectsToOldList();

	// Holds all the objects known by the garbage collector
	asCArray<asSObjTypePair>           gcNewObjects;
	asCArray<asSObjTypePair>           gcOldObjects;

	// This array temporarily holds references to objects known to be live objects
	asCArray<void*>                    liveObjects;

	// This map holds objects currently being searched for cyclic references, it also holds a 
	// counter that gives the number of references to the object that the GC can't reach
	asCMap<void*, asSIntTypePair>      gcMap;
	// 仅转换接口地址，不持有对象；实际节点退出共享 GC 图时删除所有对应别名。
	std::unordered_map<void*, void*>   objectAliases;
	// 反向索引也只保存地址；回收节点时只处理该节点的接口别名。
	std::unordered_multimap<void*, void*> aliasesByObject;

	// State variables
	egcDestroyState                    destroyNewState;
	egcDestroyState                    destroyOldState;
	asUINT                             destroyNewIdx;
	asUINT                             destroyOldIdx;
	asUINT                             numDestroyed;
	asUINT                             numNewDestroyed;
	egcDetectState                     detectState;
	asUINT                             detectIdx;
	asUINT                             numDetected;
	asUINT                             numAdded;
	asUINT                             seqAtSweepStart[3];
	asSMapNode_t                      *gcMapCursor;
	bool                               isProcessing;

	// We'll keep a pool of nodes to avoid allocating memory all the time
	asSMapNode_t            *GetNode(void *obj, asSIntTypePair it);
	void                     ReturnNode(asSMapNode_t *node);
	asCArray<asSMapNode_t*>  freeNodes;
	// GC 回调可释放最后一个运行时；本次遍历退出后才允许卸载它的类型。
	asCArray<asCScriptEngine*> deferredShutdowns;

	// Critical section for multithreaded access
	DECLARECRITICALSECTION(gcCritical)   // Used for adding/removing objects
	// Java 桥：关闭阶段在同一线程再次执行 GC；跨线程仍只能尝试进入，不能等待原生字段锁。
	std::recursive_mutex gcCollecting;
};

END_AS_NAMESPACE

#endif
