#pragma once


// simple order process
struct SimpleQueue_M
{

	void executeAfter(Order_M *inPrev){
		mPrev = inPrev;
	}

	void init(){
		mPrev = nullptr;
		mIsStarted = false;
	}

    bool isExeReady() const {
		if(mPrev){
			return mPrev->mIsStarted;
		}else{
			return true;
		}
	}

	bool isDelReady() const {return true;}

	void makePreExe(){}

	void makePreDel(){}

	void makePostExe(){
		mIsStarted = true;	
	}

private:

	Order_M *mPrev;
	bool mIsStarted;

};



// automatically updates the order of process
struct AutoQueue_M
{

	void executeAfter(Order_M *inPrev){
		mPrev = inPrev;
		if(inPrev){
			inPrev->mNext = this;
		}
	}

	void init(){
		mPrev = nullptr;
		mNext = nullptr;
		mIsStarted = false;
	}

    bool isExeReady() const {
		if(mPrev){
			return mPrev->mIsStarted;
		}else{
			return true;
		}
	}

	bool isDelReady() const {return true;}

	void makePreExe(){}

	void makePreDel(){
		if(mPrev && mNext){
			mPrev->mNext = mNext;
			mNext->mPrev = mPrev;
		}else if(mPrev){
			mPrev->mNext = nullptr;
		}else if(mNext){
			mNext->mPrev = nullptr;
		}
	}

	void makePostExe(){
		mIsStarted = true;	
	}

private:

	Order_M *mPrev, mNext;
	bool mIsStarted;

};

