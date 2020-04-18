#pragma once


#include <tuple>

// provides a module container

template<class ...ModuleCollection> 
class ModuleHub_M
{

	using items_t = std::tuple<ModuleCollection...>;
	items_t mItemModules;

public:

	template<typename T>
	T* get() {
		return &std::get<T>(mItemModules);
	}

	template<size_t I = 0>
	void init() {
		std::get<I>(mItemModules).init();
		if constexpr(I+1 != std::tuple_size<items_t>::value)
		    init<I+1>();
	}

	template<size_t I = 0>
	bool isExeReady() {
		if constexpr (I+1 != std::tuple_size<items_t>::value){
			if(std::get<I>(mItemModules).isExeReady()){
				return isExeReady<I+1>();
			}else{
				return false;
			}
		}else{
			return std::get<I>(mItemModules).isExeReady();
		}
	}
	
	template<size_t I = 0>
	bool isDelReady() {
		if constexpr (I+1 != std::tuple_size<items_t>::value){
			if(std::get<I>(mItemModules).isDelReady()){
				return isDelReady<I+1>();
			}else{
				return false;
			}
		}else{ 
			return std::get<I>(mItemModules).isDelReady();
		}
	}

	template<size_t I = 0>
	void makePreExe() {
		std::get<I>(mItemModules).makePreExe();
		if constexpr(I+1 != std::tuple_size<items_t>::value)
		    makePreExe<I+1>();
	}

	template<size_t I = 0>
	void makePostExe() {
		std::get<I>(mItemModules).makePostExe();
		if constexpr(I+1 != std::tuple_size<items_t>::value)
		    makePostExe<I+1>();
	}
	
	template<size_t I = 0>
	void makePreDel() {
		std::get<I>(mItemModules).makePreDel();
		if constexpr(I+1 != std::tuple_size<items_t>::value)
		    makePreDel<I+1>();
	}
};