#pragma once
#include <string>
#include <windows.h>
#include <tidy.h>
#include <fstream>

namespace tool {

	class Common {

	private:
		static std::string xhtml(const char* html, TidyOptionId tidyOptionId);

	public:
		static std::string utf8ToGB(const char* str);
		static std::string html2XML(const char* html);
		static std::string xml2HTML(const char* html);
		static std::string readText(std::string filePath);
		static bool writeText(std::string text, std::string filePath);
		static std::string wstring2String(std::wstring wstr);
		static std::wstring string2Wstring(std::string wstr);
		static std::string GBK_2_UTF8(std::string gbkStr);
		static std::string UTF8_2_GBK(std::string utf8Str);
		static std::string get(std::string url);
		static std::string timestamp2Timeformat(size_t t);
		static size_t timeformat2Timestamp(const char* timeformat);
		static std::string& ltrim(std::string& st);
		static std::string& rtrim(std::string& st);
		static std::string& trim(std::string& st);

	};

}