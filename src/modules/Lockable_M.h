#pragma once



struct Lockable_M
{

	void setLock(bool state){
		if(state && mLock < 255){
			mLock++;
		}else if(!state && mLock){
			mLock--;
		}
	}

	void init(){
		mLock = 0;
	}

	bool isExeReady() const { return true; }

	bool isDelReady() const { return (mLock == 0); }

	void makePreExe(){}

	void makePreDel(){}

	void makePostExe(){}

private :

	uint8_t mLock;
};
