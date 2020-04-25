/*
 * Copyright (C) 2020 Thomas AUBERT <aubert.thms@gmail.com>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name ``Thomas AUBERT'' nor the name of any other
 *    contributor may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 * 
 * uCosmDev IS PROVIDED BY Thomas AUBERT ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL Thomas AUBERT OR ANY OTHER CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


#pragma once

#include <limits>
#include "IScheduler.h"



template<typename caller_t, typename task_module, size_t task_count>
class TaskHandler : public IScheduler
{

	using task_index_t = uint8_t;
	
	static_assert(task_count < std::numeric_limits<task_index_t>::max()-1 , "Task count too high");	
	
	using task_function_t = void (caller_t::*)();
	
public:

	using TaskID = uint8_t;
	
	TaskHandler() : mCurrTaskID(0)
 	{}
	
	bool schedule() final {

		bool hasExe = false;
		
		task_index_t i=0;
		
		do{
			if( mFunctions[i] && mModules[i].isExeReady() ){
				mCurrTaskID = i+1;
				mModules[i].makePreExe();
				(static_cast<caller_t *>(this)->*mFunctions[i])();
				mModules[i].makePostExe();
				mCurrTaskID = 0;
				hasExe = true;
			}
		}while(++i < task_count);
		
		return hasExe;
	}
	
	task_module* thisTask(){
		if(!mCurrTaskID){
			HandlerException("illegal call");
			return nullptr;
		}
		return &mModules[mCurrTaskID-1];
	}

	TaskID thisTaskID(){
		if(!mCurrTaskID){
			HandlerException("illegal call");
			return 0;
		}
		return mCurrTaskID;
	}

	task_module* getTask(TaskID inID){
		if(!inID || inID > task_count){
			HandlerException("Task does not exist");
			return nullptr;
		}
		return &mModules[inID-1];
	}
	
	bool createTask(task_function_t inFunc, TaskID *ioID = nullptr){
		
		// allocation
		task_index_t i=0;
		do{
			if(!mFunctions[i]){
				
				mFunctions[i] = inFunc;
				
				if(ioID){
					*ioID = i+1;
					mIDPtr[i] = ioID;
				}else{
					mIDPtr[i] = nullptr;
				}
				mModules[i].init();
				return true;
			}
		}while(++i < task_count);

		HandlerException("No more slots available");
		
		return false;
	}

	bool deleteTask(TaskID inID){
		if(!inID){ return false; }
		
		task_index_t i = inID-1;

		if(mModules[i].isDelReady()){
			
			mModules[i].makePreDel();
			mFunctions[i] = nullptr;
			
			if(mIDPtr[i] && ( *mIDPtr[i] == inID )){
				*mIDPtr[i] = 0;
			}
			
			mIDPtr[i] = nullptr;
		}
	}

protected:

	virtual void HandlerException(const char *inErrMsg){}
 
private:

	task_function_t mFunctions[task_count];
	task_module mModules[task_count];
	
	TaskID *mIDPtr[task_count];

	TaskID mCurrTaskID;
	
};




/*
template<typename caller_t, typename task_module, size_t task_count>
class TaskHandler : public IScheduler
{

	using task_index_t = uint8_t;
	
	static const task_index_t max_index = std::numeric_limits<task_index_t>::max();
	
	static_assert(task_count < max_index-1 , "Task count too high");	
	

	struct TaskItem : public task_module{
		
		constexpr TaskItem(): index(sCounterIndex++) {}
		const task_index_t index;
		
		private:
			
		static size_t sCounterIndex;
	};

	using task_t = Task<caller_t, TaskItem>;

	using task_function_t = void (caller_t::*)();
	 
public:
	
	using TaskHandle = TaskItem*;
	 
	TaskHandler() : mCurrHandleIndex(max_index)
 	{
		// safety check
		for(task_index_t i=0 ; i<task_count ; i++){
			if(mTasks[i].mAttributes.index != i){
				HandlerException("Critical declaration error");
				while(1){}
			}
		}
	}
	
	bool schedule() final {

		bool hasExe = false;
		
		task_index_t i=0;
		
		do{
			if( mTasks[i].mFunction && mTasks[i].mAttributes.isExeReady() ){
				mCurrHandleIndex = i;
				mTasks[i].mAttributes.makePreExe();
				(static_cast<caller_t *>(this)->*mTasks[i].mFunction)();
				mTasks[i].mAttributes.makePostExe();
				mCurrHandleIndex = max_index;
				hasExe = true;
			}
		}while(++i < task_count);
		
		return hasExe;
	}
	
	TaskHandle thisTaskHandle(){
		if(mCurrHandleIndex == max_index){
			HandlerException("thisTask() not allowed in this context");
			return 0;
		}
		return &mTasks[mCurrHandleIndex].mAttributes;
	}
	
	bool createTask(task_function_t inFunc, TaskHandle *ioHandle = nullptr){
				
		// allocation
		task_index_t i=0;
		do{
			if(!mTasks[i].mFunction){
				
				mTasks[i].mFunction = inFunc;
				
				if(ioHandle != nullptr){
					*ioHandle = &mTasks[i].mAttributes;
					mHandlePtr[i] = ioHandle;
				}else{
					mHandlePtr[i] = nullptr;
				}
				mTasks[i].mAttributes.init();
				return true;
			}
		}while(++i < task_count);

		HandlerException("No more slots available");
		
		return false;
	}

	bool deleteTask(TaskHandle inHandle){
		if(!inHandle){ return false; }
		
		task_index_t i = inHandle->index;

		if(mTasks[i].mAttributes.isDelReady()){
			
			mTasks[i].mAttributes.makePreDel();
			mTasks[i].mFunction = nullptr;
			
			if(&mHandlePtr[i] && ( *mHandlePtr[i] == &mTasks[i].mAttributes )){
				*mHandlePtr[i] = nullptr;
			}
			
			mHandlePtr[i] = nullptr;
		}
	}

protected:

	virtual void HandlerException(const char *inErrMsg){}

	template<typename T>
	bool contains(T *t, TaskHandle h){
		for(task_index_t i=0 ; i < task_count ; i++){
			if(mTasks[i].mAttributes.contains(t)){
				h = &mTasks[i].mAttributes;
				return true;
			}
		}
		return false;
	}
 
private:

	task_t mTasks[task_count];
	
	TaskHandle *mHandlePtr[task_count];

	task_index_t mCurrHandleIndex;
	
};


template<typename Caller_t, typename task_traits, size_t task_count>
size_t TaskHandler<Caller_t, task_traits, task_count>::TaskItem::sCounterIndex = 0;
*/
