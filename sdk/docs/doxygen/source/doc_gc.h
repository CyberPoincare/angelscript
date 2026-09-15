/**

\page doc_gc Garbage collection

Though AngelScript uses reference counting for memory management, there is still need for a garbage collector to 
take care of the few cases where circular referencing between objects prevents the reference counter from reaching zero.
By default AngelScript automatically runs a few incremental steps of the garbage collector every time a new garbage 
collected object is created, this allows the destruction of garbage in an automatic way, yet doesn't waste time or 
disrupts the responsiveness of the application. 

The automatic execution may not always be enough to clean up the garbage, for example, if the scripts create a lot of
garbage with circular references. In this case the application may need to manually invoke the garbage collector from
time to time. For this reason it is recommended that the application monitor the statistics for the garbage collector 
and adjust the frequency of the manual calls as necessary. The statistics is obtained through a call to 
\ref asIScriptEngine::GetGCStatistics "GetGCStatistics", which returns the number of objects currently known to the 
garbage collector as well as the number of objects that have been destroyed and the number of object that have been 
detected as garbage with circular references. 

The garbage collector implemented in AngelScript is incremental, so it can be executed for short periods of time without
requiring the entire application to halt. For this reason, it is recommended that one or more calls to \ref 
asIScriptEngine::GarbageCollect "GarbageCollect"(\ref asGC_ONE_STEP) is made at least once during the normal event processing.
The number of calls that should be made depends on how much garbage is created.

If the scripts produce a lot of garbage but only a low number of garbage with circular references, the application can make a 
call to \ref asIScriptEngine::GarbageCollect "GarbageCollect"(\ref asGC_FULL_CYCLE | \ref asGC_DESTROY_GARBAGE), which will 
only destroy the known garbage without trying to detect circular references. This call is relatively fast as the garbage 
collector only has to make a trivial local check to determine if an object is garbage without circular references.

Finally, if the application goes into a state where responsiveness is not so critical, it might be a good idea to do a full
cycle on the garbage collector, thus cleaning up all garbage at once. To do this, call \ref 
asIScriptEngine::GarbageCollect "GarbageCollect"(\ref asGC_FULL_CYCLE).

Should the automatic garbage collections not be desired, e.g. in critical inner loops where maximum performance is
needed, it can easily be turned off with a call to \ref asIScriptEngine::SetEngineProperty "SetEngineProperty"(\ref asEP_AUTO_GARBAGE_COLLECT, false).

\see \ref doc_memory

\section doc_gc_circcallback Callback for detected circular references

During the testing phase of your project it is wise to try to identify situations that create circular references as 
these have a potential of significantly impacting the performance. While the \ref asIScriptEngine::GetGCStatistics "GetGCStatistics"
will tell you if circular references have been detected and destroyed, it will not tell you much about the actual objects involved in 
the circular references. To aid in this the application can set a callback for when circular references is detected with
\ref asIScriptEngine::SetCircularRefDetectedCallback "SetCircularRefDetectedCallback". 

When this callback is invoked the objects in the circular reference have not yet been destroyed, so the application can inspect 
the content to get valuable hints on where and when they were created. The application must not however try to modify the 
objects at this time as would lead to undefined behaviour, and possibly even application crashes.

\section doc_gc_threads Garbage collection and multi-threading

The garbage collector itself is thread safe, but in order to be able to safely use the garbage collector in a multi-threaded
environment the application must make sure all the objects that may be in the garbage collector has thread safe implementations
of \ref doc_gc_object "the GC behaviours". 

\see \ref doc_reg_gcref_4

\section doc_gc_shared_java 本项目的 Java 对象桥接定制

ShareGarbageCollector(owner) 使接收引擎加入 owner 的引用图，须在接收引擎创建任何 GC 对象前调用。
对象只登记一次，行为编号和类型编号仍由该对象所属引擎解释；共享图不共享脚本类或模块。
GetGCStatistics、GetObjectInGC 和循环引用诊断回调的范围随之扩展为整个共享图。
每个引擎独立同步编译、注册和 GC 的元数据读取；原生 GC 行为仍须自行保证引用枚举与修改的安全。
关闭引擎只移除该引擎节点；GC 回调中发起的关闭请求在本次遍历完成后执行，调用者交出的引擎引用由队列接管。
各子引擎保留图所有者，图所有者应在全部子引擎关闭后再关闭。此定制不改变原 GC 的循环检测和引用计数算法。


*/
