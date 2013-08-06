#include "backend_selector.h"
#include "config.h"

#include <boost/functional/hash.hpp>
#include <string>

namespace http {
namespace server2 {


void backend_selector::init(const config& c) {
	hash_str_ = c.hash_str_;
	backends_ = c.backends_;
	init_ = true;
}

int backend_selector::pick(const request& r, backend& b) {
	if(init_ == false) {
		return 1;
	}
	std::string uri = r.uri;
	std::string hash_str;
	std::size_t start, end;
	start = uri.find(hash_str_);
	if(start == std::string::npos) {
		// 失败，不做继续的转发
		return 1;		
	}
	else {
		end = uri.find("&");
		if(end == std::string::npos) {
			// /query?q=as&userid=123
			hash_str = uri.substr(end + hash_str_.size() + 1);
			if(hash_str.size() <= 0) {
				return 1;
			}
		}
		else {
			// /query?userid=123&q=as
			hash_str = uri.substr( + hash_str_.size() + 1, (end - start));
			if(hash_str.size() <= 0) {
				return 1;
			}
		}

		boost::hash<std::string> hfunc;
		b = backends_[ hfunc(hash_str) % backends_.size() ];
		return 0;
	}
}
};
};
