//
// 后端服务器选择器，这里采用userid hash做法
//
#ifndef _BACKEND_SELECTOR_H_
#define _BACKEND_SELECTOR_H_

#include "config.h"
#include "request.hpp"

#include <vector>

namespace http{
namespace server2 {
class backend_selector {
public:
	// 使用hash方式选择host
	backend_selector() : init_(false) {};
	void init(const config& c);
	// 得到当前请求的host
	int pick(const request& r, backend& b);	
private:
	// 后端的host信息
	std::vector<backend> backends_;
	// 需要做hash的字符串的标志  userid
	std::string hash_str_;
	bool init_;
};
};
};

#endif
