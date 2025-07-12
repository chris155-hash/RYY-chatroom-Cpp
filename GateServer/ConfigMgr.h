#pragma once
#include "const.h"
//自动处理config的配置     一个map<string,SectionInfo>.
// SectionInfo就是map<string,string>这种结构里面的map的key就是port，value就是对应的值
//[GateServer]
//port = 8080
//[VarifyServer]
//port = 50051
struct SectionInfo {
	SectionInfo(){}
	~SectionInfo() { _sections_datas.clear(); }

	//拷贝构造和拷贝复制也写一下，后面这个_section_datas要移动
	SectionInfo(const SectionInfo& src) {
		_sections_datas = src._sections_datas;
	}

	SectionInfo& operator = (const SectionInfo& src) {
		if (this == &src) {
			return *this;
		}
		this->_sections_datas = src._sections_datas;
		return *this;
	}

	std::map<std::string, std::string> _sections_datas;

	std::string operator[](const std::string& key) {   //重载一下[],方便后面取数据
		if (_sections_datas.find(key) == _sections_datas.end()) {
			return "";
		}
		return _sections_datas[key];
	}
};

class ConfigMgr
{
public:
	~ConfigMgr() {
		_config_map.clear();
	}

	SectionInfo operator[](const std::string& section) {
		if (_config_map.find(section) == _config_map.end()) {
			return SectionInfo();
		}
		return _config_map[section];
	}

	ConfigMgr();
	
	ConfigMgr (const ConfigMgr& src) {
		_config_map = src._config_map;
	};
	ConfigMgr& operator =(const ConfigMgr& src) {
		if (this != &src) {
			this->_config_map = src._config_map;
		}
		return *this;
	};

private:
	std::map<std::string, SectionInfo> _config_map;
	
};

