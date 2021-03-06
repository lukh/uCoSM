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

#include <type_traits>
#include "uscosm-sys-data.h"
#include "utils.h"

//-------------------------------  Task Traits  -------------------------------

// µCoSm - Cooperative Scheduler Module


/*  Task modules :
 * 
 *  must contain a set of funtions :
 * 
 *	  - template<typename derived_t> void init()
 * 
 *	  - bool isExeReady const ()
 *	  - bool isDelReady const ()
 *
 *	  - void makePreExe()
 *	  - void makePreDel()
 *	  - void makePostExe()
 *	  	  
 * 
 */







template<class ...ModuleCollection> 
struct Modules : public ModuleCollection...
{

	Modules()
	{}

	void init()
	{
		uint8_t d[] = {(uint8_t)0, (ModuleCollection::template init<Modules<ModuleCollection...>>(), (uint8_t)0)...};
		static_cast<void>(d); // avoid warning for unused variable
	}

	bool isExeReady()
	{
		bool ready[] = {
			true, (ModuleCollection::isExeReady())...
		};
		for(index_t i=0 ; i<sizeof(ready) ; i++)
		{
			if(!ready[i]){ return false; }
		}
		return true;
	}
	
	bool isDelReady()
	{
		bool ready[] = {
			true, (ModuleCollection::isDelReady())...
		};
		for(index_t i=0 ; i<sizeof(ready) ; i++)
		{
			if(!ready[i]){ return false; }
		}
		return true;
	}

	void makePreExe()
	{
		uint8_t d[] = {(uint8_t)0, (ModuleCollection::makePreExe(), (uint8_t)0)...};
		static_cast<void>(d); // avoid warning for unused variable
	}

	void makePostExe()
	{
		uint8_t d[] = {(uint8_t)0, (ModuleCollection::makePostExe(), (uint8_t)0)...};
		static_cast<void>(d); // avoid warning for unused variable
	}
	 
	void makePreDel()
	{
		uint8_t d[] = {
			(uint8_t)0, (ModuleCollection::makePreDel(), (uint8_t)0)...
		};
		static_cast<void>(d); // avoid warning for unused variable
	}
	
private:
	
};


using no_module = Modules<>;





namespace ucosm_modules
{







	

// Allows to add basic task priority management :
// the priority goes from 1 to 255
// where 1 is the highest priority i.e. it will be executed on every mainloop cycles
// and 255 is the lowest, it will be executed once every 255 mainloop cycles

struct Prio // 1 byte
{

	void setPriority(const uint8_t inPrio)
	{
		// priority can't be inferior to 1
		mPriority = (inPrio)?inPrio:1; 
	}

protected:

	template<typename derived_t>
	 void init()
	{
		mPriority = 1;
	}
	 
    bool isExeReady() const 
	{   
		if(SysKernelData::sCnt)
		{
			return (!(SysKernelData::sCnt%mPriority));
		}else{
			return (!((SysKernelData::sCnt+1)%mPriority));
		}
	}
	 
	bool isDelReady() const {return true;}
	void makePreExe(){}
	void makePreDel(){}
	void makePostExe(){}
	
private:
	 
    uint8_t mPriority;
};












struct Status // 1 byte
{
 
	enum eStatus:uint8_t
	{
		eSuspended			= 0b00000100,
		eLocked				= 0b00001000,
		eStatusMask			= 0b00001111
	};

		
	bool isStatus(uint8_t s){ return ((mStatus&s) == s);}
		
	void setStatus(uint8_t s, bool state)
	{
		// task is locked : cancel operation
		if(isStatus(eLocked) && s!=eLocked){ return;}
		
		state ? mStatus|=s : mStatus&=~s;
	}

	void setStatus(eStatus s, bool state)
	{
		setStatus(static_cast<uint8_t>(s), state);
	}

	bool isRunning()
	{
		return (mStatus&eRunning);
	}

	bool isStarted()
	{
		return (mStatus&eStarted);
	}

protected:
	
	template<typename derived_t>
	void init()	{ mStatus = 0; }
	bool isExeReady() const { return !(mStatus&eSuspended) ;}
	bool isDelReady() const { return !(mStatus&eLocked);}
	void makePreExe(){  mStatus |= eRunning; }
	void makePreDel(){}
	void makePostExe()
	{
		mStatus &= ~eRunning; 
		mStatus |= eStarted;
	}
	
	enum eSystemStatus:uint8_t
	{
		eRunning			= 0b00000001,
		eStarted			= 0b00000010
	};
	
	uint8_t mStatus;
	
};






template<typename Callee_t>
struct StatusNotify : private Status // 1 byte
{
 
	enum eNotifyStatus
	{
		eNotifyStarted		= Status::eStarted	<<3,	// 0b00010000
		eNotifySuspended	= Status::eSuspended<<3,	// 0b00100000
		eNotifyLocked		= Status::eLocked	<<3,	// 0b01000000
		eNotifyDeleted		= 0b10000000,				// 0b10000000
		eNotifyMask			= ~Status::eStatusMask
    };
	
	bool isStatus(uint8_t s){ return ((mStatus&s) == s); }
		
	void setStatus(uint8_t s, bool state)
	{
		if(isStatus(Status::eLocked) && s!=Status::eLocked){ return;}

		uint8_t prevNotifStatus = (mStatus&Status::eStatusMask)<<3;
		
		state ? mStatus|=s : mStatus&=~s;

		// is the new status notifiable
		uint8_t newNotifStatus = ((s&Status::eStatusMask)<<3)&(mStatus&eNotifyMask);
		
		if(newNotifStatus)// status notifiable: notify callee
		{
			Callee_t::notifyStatusChange(prevNotifStatus, newNotifStatus);
		}
	}
	
	void setStatus(Status::eStatus s, bool state)
	{
		setStatus(static_cast<uint8_t>(s), state);
	}
	

protected:

	template<typename derived_t>
	void init()	{ mStatus = 0; }
	
	bool isExeReady() const { return !(mStatus&Status::eSuspended) ;}
	bool isDelReady() const { return !(mStatus&Status::eLocked);}

	void makePreExe()
	{  
		setStatus(Status::eRunning, true); 
	}
	
	void makePostExe()
	{
		setStatus(Status::eRunning, false); 
		setStatus(Status::eStarted, true);
	}
	
	void makePreDel()
	{
		if( isStatus(eNotifyDeleted) )
		{
			Callee_t::notifyStatusChange(mStatus, eNotifyDeleted);
		}
		mStatus = 0;
	}
	
};











struct Delay // 4 bytes
{

	void setDelay(tick_t inDelay)
	{
		mExecution_time_stamp = SysKernelData::sGetTick()+inDelay; 
	}
	
	tick_t getDelay()
	{	
		if(mExecution_time_stamp > SysKernelData::sGetTick()){
			return mExecution_time_stamp - SysKernelData::sGetTick(); 
		}else{
			return 0;
		}
	}

protected:

	template<typename derived_t>
	void init()
	{
		mExecution_time_stamp = SysKernelData::sGetTick();
	}
	
    bool isExeReady() const 
	{
		return (SysKernelData::sGetTick() >= mExecution_time_stamp);
	}
	
	bool isDelReady() const { return true; }

	void makePreExe(){}
	
	void makePreDel(){}
	void makePostExe(){}	
	
private:
	
	tick_t mExecution_time_stamp;
	
};






// periodic call of the function, guarantees a constant average execution rate
struct Periodic // 6 bytes
{

	using period_t = uint16_t;

	
	void setPeriod(period_t inPeriod)
	{
		mPeriod = inPeriod;
		mExecution_time_stamp += mPeriod;
	}

	period_t getPeriod()
	{
		return mPeriod;
	}

	void setDelay(tick_t inDelay)
	{	
		mExecution_time_stamp = SysKernelData::sGetTick()+inDelay; 
	}
	
	tick_t getDelay()
	{	if(mExecution_time_stamp > SysKernelData::sGetTick()){
			return mExecution_time_stamp - SysKernelData::sGetTick(); 
		}else{
			return 0;
		}
	}

protected:

	template<typename derived_t>
	void init()
	{
		mExecution_time_stamp = SysKernelData::sGetTick();
	}
	
	bool isExeReady() const {
		return (SysKernelData::sGetTick() >= mExecution_time_stamp);
	}
	bool isDelReady() { return true; }
	void makePreExe()
	{
		mExecution_time_stamp += mPeriod;
	}
	void makePreDel(){}
	void makePostExe(){}
		
private:
	
	tick_t mExecution_time_stamp;
	period_t mPeriod;
};








// Allow to send data to a specific task
// Data is unaccessible if the owner task is not currently running
template<typename T, uint16_t fifo_size>
struct Signal
{
	
	bool send(Signal *inReceiver, T inData)
	{
		if(!inReceiver){return false;}
		return (inReceiver->mRxData.push(inData));
	}

	T receive()
	{
		if(!reinterpret_cast<Status *>(this)->isRunning()){
			return T();
		}

		return mRxData.pop();
	}

	bool hasData()
	{
		return !mRxData.isEmpty();
	}

protected:

	template<typename derived_t>
	void init()
	{ 
		static_assert(std::is_base_of<Status, derived_t>::value, "Signal must implement Status");
	}
	bool isExeReady() { return true; }
	bool isDelReady() { return mRxData.isEmpty(); }
	void makePreExe() {}
	void makePreDel() {}
	void makePostExe(){}

private:
	
	Fifo<T, fifo_size> mRxData;
};






// contains an element of the specified type
template<typename T> 
struct Content
{	
	
	T& getContent()
	{
		return content;
	}
	
protected:

	template<typename derived_t>
	void init() {}
	bool isExeReady() const { return true; }
	bool isDelReady() const { return true; } 
	void makePreExe(){}
	void makePreDel(){}
	void makePostExe(){}

	T content;
};







// contains a buffer of the specified type and size
template<typename buffer_t, uint16_t size> 
struct Buffer
{

	void setData(buffer_t *inData, uint16_t inByteSize)
	{
		if(inByteSize > size*sizeof(buffer_t)) { return; }
		memcpy(mBuffer, inData, inByteSize);
	}

	void setDataAt(uint16_t inIdx, buffer_t inData)
	{
		if(inIdx >= size) { return; }
		mBuffer[inIdx] = inData;
	}
	
	buffer_t getData(uint16_t inIdx)
	{
		if(inIdx >= size) { return 0; }
		return mBuffer[inIdx];
	}
	
	buffer_t *getBuffer()
	{
		return mBuffer;
	}
	
protected:

	template<typename derived_t>
	void init() {}
	bool isExeReady() const { return true; }
	bool isDelReady() const { return true; } 
	void makePreExe(){}
	void makePreDel(){}
	void makePostExe(){}

	buffer_t mBuffer[size];
};

























struct ListItem
{
	ListItem* mPrev;
	ListItem* mNext;
};



// automatically updated linked list of tasks by chronology of execution
template<int listIndex>
struct LinkedList : public ListItem // 9 bytes
{

	ListItem *getNext() { return mNext; }
	ListItem *getPrev() { return mPrev; }
	
protected:

	template<typename derived_t>
	void init()
	{   
		static_assert(std::is_base_of<Status, derived_t>::value, "LinkedList must implement Status");
		mPrev = mNext = nullptr;
	}

	bool isExeReady() const { return true; }
	bool isDelReady() const { return true; } 
	void makePreExe()
	{
		if(reinterpret_cast<Status *>(this)->isStarted()){return;}

		if(sTopHandle)
		{
			sTopHandle->mNext = this;
			mPrev = sTopHandle;
		}
		sTopHandle = this;
	}
	void makePreDel()
	{
		if(sTopHandle == this)
		{
			if(mPrev){
				sTopHandle = mPrev;
			}else{
				sTopHandle = nullptr;
			}
		}
		if(mPrev && mNext)
		{
			mPrev->mNext = mNext;
			mNext->mPrev = mPrev;
		}else if(mPrev){
			mPrev->mNext = nullptr;
		}else if(mNext){
			mNext->mPrev = nullptr;
		}
	}
	void makePostExe(){}

private:

	static ListItem *sTopHandle;
	
};

template<int listIndex>
ListItem *LinkedList<listIndex>::sTopHandle = nullptr;










































// Fixed size dynamic memory allocation
// features :
//  - Allows to allocate and release a buffer of sizeof(elem_t) bytes
//  - Forbids task deletion if a buffer is allocated to avoid memory leakage
//  - The number of buffer per task types has a maximum value of 32

template<typename elem_t, uint16_t elem_count>
struct MemPool32
{
	static_assert(elem_count <= 32, "size of pool must not exceed 32");
	
	static_assert( (sizeof(elem_t) * sizeof(elem_count) ) > 4, 
	"Suboptimal implementation : Pool's size inferior to overhead's");

	template<typename T>
	T *allocate()
	{
		static_assert(sizeof(T) <= sizeof(elem_t), "Allocation error");
		elem_t *e = allocate();
		if(e == nullptr){ return nullptr; }
		return reinterpret_cast<T *>(e);
	}
		
	// could use "placement new" here
	elem_t *allocate()
	{
		// pool is full
		if(mMemoryMap == (1<<elem_count)-1){ return nullptr; }

		// task already has allocated memory
		if(mAllocIndex){ return nullptr; }

		
		uint8_t i=0;
		
		do{
			if(!mMemoryMap&(1<<i)) // slot free
			{
				mMemoryMap |= (1<<i); // take slot

				mAllocIndex = i; // stores the index for fast deletion
				
				// set task alloc active with a boolean,
				// allows to check allocation in case index is 0
				mAllocIndex |= kAllocBoolMask;
				
				return &mElems[i];
			}
		}while(++i<elem_count);
		
		// allocation error : should not happen
		return nullptr;
	}
		
	bool release(){

		// pool is empty, nothing to free
		if(!mMemoryMap){ return false; }

		// task has no allocated memory
		if(!mAllocIndex){ return false; }

		// remove boolean alloc state
		mAllocIndex &= ~kAllocBoolMask;

		// security check : verify that the memory has been allocated
		// critical error : map and index does not coincide
		if(!(mMemoryMap&(1<<mAllocIndex))){ return false; }

		// release memory
		mMemoryMap &= ~(1<<mAllocIndex);

		// delete index
		mAllocIndex = 0;
		return true;

	}

	template<typename T>
	T *getMemory()
	{
		static_assert(sizeof(T) <= sizeof(elem_t), "Allocation error");
		elem_t *e = getMemory();
		if(e == nullptr){ return nullptr; }
		return reinterpret_cast<T *>(e);
	}

	elem_t *getMemory()
	{
		if(!mAllocIndex){ return nullptr; }
		return &mElems[mAllocIndex&(~kAllocBoolMask)];
	}

protected:

	template<typename derived_t>
	void init() { mAllocIndex = 0; }
	bool isExeReady() const { return true; }
	
	// decide if deletion forbidden if allocated memory or auto release?
	bool isDelReady() const { return !mAllocIndex; } 
	void makePreExe(){}
	void makePreDel(){}
	void makePostExe(){}
	
private:

	uint8_t mAllocIndex;
	
	static elem_t mElems[elem_count];
	static uint32_t mMemoryMap;
	static const uint8_t kAllocBoolMask = 0b10000000;

};

template <typename elem_t, uint16_t elem_count>
elem_t MemPool32<elem_t, elem_count>::mElems[elem_count];

template <typename elem_t, uint16_t elem_count>
uint32_t MemPool32<elem_t, elem_count>::mMemoryMap = 0;













// allows to set a Parent/Child relation between two tasks,
// it forbids the deletion of the parent task if the child is alive
struct Parent
{

	void setChild(Parent *inChild)
	{
		inChild->mParent = this;
		mChild = inChild;
		mIsParent = true;
	}
	
protected:

	template<typename derived_t>
	void init()
	{ 
		mIsParent = false;
		mChild = nullptr; 
	}
	bool isExeReady() const { return true; }
	bool isDelReady()
	{
		if(!mIsParent) { return true; }
		
		if(!mChild)	{ return true; }
	
		return false; 
	} 
	void makePreExe(){}
	void makePreDel()
	{
		if(!mIsParent && mParent)
		{
			mParent->mChild = nullptr;
		}
	}
	void makePostExe(){}

private:

	union{
		Parent *mChild;
		Parent *mParent;
	};
	
	bool mIsParent;
};









// Be careful with this one!
// It should not be employed in an infinite loop
// It will mess up thisTaskHandle()
// You'll have to store the current task value
struct Coroutine
{

	void waitFor(tick_t inDuration)
	{
		if(SysKernelData::sMaster)
		{	
			SysKernelData::sMaster->schedule(inDuration);	
		}
	}
						

protected:

	template<typename derived_t>
	void init()
	{ 
		static_assert(std::is_base_of<Status, derived_t>::value, "Coroutine must implement Status");
	}
	
	bool isExeReady() 
	{ 
		return !(reinterpret_cast<Status *>(this)->isRunning());
	}
	
	bool isDelReady()
	{
		return true; 
	}
	
	void makePreExe(){}
	void makePreDel(){}
	void makePostExe(){}

private:
	
};






// macro based coroutine
// inspired by protothread
template<uint16_t max_context_size = 0> 
struct Coroutine2
{

#define CR_CTX_START			struct Ctx_def{

#define CR_CTX_END(label)		};Ctx_def *label = thisTaskHandle()->getContext<Ctx_def>();

#define CR_START				switch(thisTaskHandle()->line){case 0:

#define CR_YIELD				thisTaskHandle()->line = __LINE__;return;case  __LINE__  :

#define CR_WAIT_UNTIL(cond)		thisTaskHandle()->line = __LINE__;case __LINE__ :  if(!(cond)){return;}

#define CR_WAIT_FOR(timestamp)	thisTaskHandle()->line = __LINE__;case __LINE__ :  if(timestamp < SysKernelData::sGetTick()){return;}

#define CR_RESET         		thisTaskHandle()->line = 0;
		
#define CR_END         			}deleteTask(thisTaskHandle());


	
	template<typename T>
	T *getContext()
	{
		static_assert(sizeof(T) <= sizeof(mContext), "Coroutine context size error");
		if(!line){
			// instantiate T inside context buffer
			T temp;
			uint8_t *dest = mContext;
			uint8_t *src = reinterpret_cast<uint8_t *>(&temp);
			const uint8_t *end = src+sizeof(T);
			while(src != end){
				*dest++ = *src++;
			}
		}
		return reinterpret_cast<T *>(mContext);
	}

	uint16_t line;
	
protected:

	template<typename derived_t>
	void init() { line = 0;}
	bool isExeReady() const { return true; }
	bool isDelReady() const { return true; } 
	void makePreExe(){}
	void makePreDel(){}
	void makePostExe(){}

private:
	
	uint8_t mContext[max_context_size];
	
	
};









} // end of task_traits namespace 











