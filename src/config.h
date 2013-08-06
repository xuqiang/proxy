// 
// 全局配置信息，backends, host, port
//
#ifndef _CONFIG_H_
#define _CONFIG_H_

#include <iostream>
#include <string>
#include <vector>

namespace http {
namespace server2 {

struct backend {
	std::string address;
	int port;
	// query:port
	std::string query_str;
};

class config {
public:
	config(const std::string& f);
	int parse();

	// proxy启动的ip地址
	std::string address_;
	// 监听端口 string类型，server启动需要
	std::string port_;
	// 线程数
	int thread_num_;
	// 后端服务器数量
	std::vector<backend> backends_;
	// 分流字段 userid
	std::string hash_str_;
private:
	std::string conf_fname_;
};
};
};

#endif
