/*
 * string_table.hpp
 *
 *  Created on: 2017年11月14日
 *      Author: zhengxiaolin
 */

#ifndef INCLUDE_RUNTIME_JAVA_LANG_STRING_HPP_
#define INCLUDE_RUNTIME_JAVA_LANG_STRING_HPP_

#include <unordered_set>
#include <memory>
#include <string>
#include <sstream>
#include <iostream>
#include "utils/lock.hpp"

using std::wstring;
using std::shared_ptr;
using std::unordered_set;
using std::wstringstream;

// this code is from javaClasses.hpp, openjdk8 hotspot src code. T is jchar, (unsigned short).
//template <typename T> static unsigned int hash_code(T* s, int len) {
//    unsigned int h = 0;
//    while (len-- > 0) {
//      h = 31*h + (unsigned int) *s;
//      s++;
//    }
//    return h;
//  }

class Oop;
class InstanceOop;

struct java_string_hash	// Hash 用于决定此 T 变量放到哪个桶中，equator 才是判断两个 T 相不相同的重要因素！！
{
	size_t operator()(Oop* ptr) noexcept;
};

struct java_string_equal_to
{
	bool operator() (const Oop *lhs, const Oop *rhs) const;
};


class java_lang_string {
private:
	static Lock & getLock() {
		static Lock lock;
		return lock;
	}
	static auto & get_string_table() {
		static unordered_set<Oop *, java_string_hash, java_string_equal_to> string_table;
		return string_table;
	}
	static Oop *intern_to_oop(const wstring & str);
public:
	static wstring print_stringOop(InstanceOop *stringoop);
	static inline __attribute__((always_inline)) Oop *intern(const wstring & str) {		// intern 用到的次数非常多。建议内联。
		Oop *stringoop = java_lang_string::intern_to_oop(str);							// TODO: 注意！！这里也用了 new，但是没有放到 GC 堆当中............
		LockGuard lg(getLock());
		auto iter = java_lang_string::get_string_table().find(stringoop);
#ifndef DEBUG		// TODO: 1. DEBUG 宏到这里会消失？？
#define DEBUG		// TODO: 2. 同一个 string intern 之后出现多份？？？bug...
#endif
#ifdef DEBUG
	std::cout << "===-------------- string_table ---------------===" << std::endl;
	for(auto iter : get_string_table()) {
		std::wcout << java_lang_string::print_stringOop((InstanceOop *)iter) << std::endl;	// TODO: map 的 iter 是 pair... set 的 iter 就是元素自身..... 都忘光了......
	}
	std::cout << "===-------------------------------------------===" << std::endl;
#endif
		if (iter == java_lang_string::get_string_table().end()) {
			java_lang_string::get_string_table().insert(stringoop);
			return stringoop;
		} else {
			return *iter;
		}
	}
};

#endif /* INCLUDE_RUNTIME_JAVA_LANG_STRING_HPP_ */