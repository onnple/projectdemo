#include <boost/thread/thread.hpp>
#include <chrono>
#include <thread>
#include "common.h"
#include <string>
#include <windows.h>
#include <tidy.h>
#include <tidybuffio.h>
#include <stdio.h>
#include <errno.h>
#include <iostream>
#include <comdef.h>
#include <cpprest/http_client.h>
#include <cpprest/uri.h>
#include <ctime>

std::string tool::Common::utf8ToGB(const char* str) {
	std::string result;
	WCHAR* strSrc;
	LPSTR szRes;

	//获得临时变量的大小
	int i = MultiByteToWideChar(CP_UTF8, 0, str, -1, NULL, 0);
	strSrc = new WCHAR[i + 1];
	MultiByteToWideChar(CP_UTF8, 0, str, -1, strSrc, i);

	//获得临时变量的大小
	i = WideCharToMultiByte(CP_ACP, 0, strSrc, -1, NULL, 0, NULL, NULL);
	szRes = new CHAR[i + 1];
	WideCharToMultiByte(CP_ACP, 0, strSrc, -1, szRes, i, NULL, NULL);

	result = szRes;
	delete[]strSrc;
	delete[]szRes;

	return result;
}

std::string tool::Common::xhtml(const char* html, TidyOptionId tidyOptionId) {
	TidyBuffer output = { 0 };
	TidyBuffer errbuf = { 0 };
	int rc = -1;
	Bool ok;

	TidyDoc tdoc = tidyCreate();

	ok = tidyOptSetBool(tdoc, tidyOptionId, yes);  // Convert to XHTML
	if (ok)
		rc = tidySetErrorBuffer(tdoc, &errbuf);      // Capture diagnostics
	if (rc >= 0)
		rc = tidyParseString(tdoc, html);           // Parse the input
	if (rc >= 0)
		rc = tidyCleanAndRepair(tdoc);               // Tidy it up!
	if (rc >= 0)
		rc = tidyRunDiagnostics(tdoc);               // Kvetch
	if (rc > 1)                                    // If error, force output.
		rc = (tidyOptSetBool(tdoc, TidyForceOutput, yes) ? rc : -1);
	if (rc >= 0)
		rc = tidySaveBuffer(tdoc, &output);          // Pretty Print

	std::string result;
	if (rc >= 0) {
		//if (rc > 0)
		//	printf("\nDiagnostics:\n\n%s", errbuf.bp);
		//printf("\nAnd here is the result:\n\n%s", output.bp);
		result.resize(output.size);
		for (size_t i = 0; i < output.size; i++) {
			result[i] = output.bp[i];
		}
	}
	else
		printf("A severe error (%d) occurred.\n", rc);

	tidyBufFree(&output);
	tidyBufFree(&errbuf);
	tidyRelease(tdoc);
	return result;
}

std::string tool::Common::html2XML(const char* html) {
	return xhtml(html, TidyXhtmlOut);
}

std::string tool::Common::xml2HTML(const char* html) {
	return xhtml(html, TidyHtmlOut);
}

std::string tool::Common::readText(std::string filePath) {
	std::ifstream in(filePath);
	if (!in.is_open()) {
		std::cerr << "open file failed: " << filePath << std::endl;
		return "";
	}
	std::string buffer, temp;
	while (std::getline(in, temp)) {
		buffer.append(temp).append("\n");
	}
	in.close();
	return buffer;
}

bool tool::Common::writeText(std::string text, std::string filePath) {
	std::ofstream out(filePath);
	out << text;
	out.close();
	return true;
}

std::string tool::Common::wstring2String(std::wstring wstr) {
	// support chinese
	std::string res;
	int len = WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), wstr.size(), nullptr, 0, nullptr, nullptr);
	if (len <= 0) {
		return res;
	}
	char* buffer = new char[len + 1];
	if (buffer == nullptr) {
		return res;
	}
	WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), wstr.size(), buffer, len, nullptr, nullptr);
	buffer[len] = '\0';
	res.append(buffer);
	delete[] buffer;
	return res;
}

std::wstring tool::Common::string2Wstring(std::string wstr) {
	std::wstring res;
	int len = MultiByteToWideChar(CP_ACP, 0, wstr.c_str(), wstr.size(), nullptr, 0);
	if (len < 0) {
		return res;
	}
	wchar_t* buffer = new wchar_t[len + 1];
	if (buffer == nullptr) {
		return res;
	}
	MultiByteToWideChar(CP_ACP, 0, wstr.c_str(), wstr.size(), buffer, len);
	buffer[len] = '\0';
	res.append(buffer);
	delete[] buffer;
	return res;
}


std::string tool::Common::GBK_2_UTF8(std::string gbkStr) {
	std::string outUtf8 = "";
	int n = MultiByteToWideChar(CP_ACP, 0, gbkStr.c_str(), -1, NULL, 0);
	WCHAR* str1 = new WCHAR[n];
	MultiByteToWideChar(CP_ACP, 0, gbkStr.c_str(), -1, str1, n);
	n = WideCharToMultiByte(CP_UTF8, 0, str1, -1, NULL, 0, NULL, NULL);
	char* str2 = new char[n];
	WideCharToMultiByte(CP_UTF8, 0, str1, -1, str2, n, NULL, NULL);
	outUtf8 = str2;
	delete[]str1;
	str1 = NULL;
	delete[]str2;
	str2 = NULL;
	return outUtf8;
}

std::string tool::Common::UTF8_2_GBK(std::string utf8Str) {
	std::string outGBK = "";
	int n = MultiByteToWideChar(CP_UTF8, 0, utf8Str.c_str(), -1, NULL, 0);
	WCHAR* str1 = new WCHAR[n];
	MultiByteToWideChar(CP_UTF8, 0, utf8Str.c_str(), -1, str1, n);
	n = WideCharToMultiByte(CP_ACP, 0, str1, -1, NULL, 0, NULL, NULL);
	char* str2 = new char[n];
	WideCharToMultiByte(CP_ACP, 0, str1, -1, str2, n, NULL, NULL);
	outGBK = str2;
	delete[] str1;
	str1 = NULL;
	delete[] str2;
	str2 = NULL;
	return outGBK;
}

std::string tool::Common::get(std::string url) {
	web::http::client::http_client_config config;
	config.set_timeout(utility::seconds(20));
	web::http::client::http_client client(string2Wstring(url), config);

	web::uri_builder uri_builder;
	//uri_builder.append_query(L"username", "Once");

	web::http::http_request request(web::http::methods::GET);
	//request.set_request_uri(uri_builder.to_uri());
	request.headers().add(L"User-Agent", L"Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/86.0.4240.198 Safari/537.36");

	web::http::http_response res = client.request(request).get();
	auto code = res.status_code();
	if (code == 404 || code == 403 || code == 443) return "";
	std::string html = wstring2String(res.extract_string().get());
	std::this_thread::sleep_for(std::chrono::seconds(10));
	return GBK_2_UTF8(html);
}


/**
* 将时间戳转为格式化时间字符串
**/
std::string tool::Common::timestamp2Timeformat(size_t t) {
	time_t tt = t;
	char tmp[32] = { NULL };
	strftime(tmp, sizeof(tmp), "%Y-%m-%d %H:%M:%S", localtime(&tt));
	return std::string(tmp);
}

/**
* 将格式化时间字符串转为时间戳
**/
size_t tool::Common::timeformat2Timestamp(const char* timeformat) {
	struct tm tm_;
	int year, month, day, hour, minute, second;
	sscanf(timeformat, "%d-%d-%d %d:%d:%d", &year, &month, &day, &hour, &minute, &second);
	tm_.tm_year = year - 1900;
	tm_.tm_mon = month - 1;
	tm_.tm_mday = day;
	tm_.tm_hour = hour;
	tm_.tm_min = minute;
	tm_.tm_sec = second;
	tm_.tm_isdst = 0;

	time_t timeStamp = mktime(&tm_);
	return timeStamp;
}

std::string& tool::Common::ltrim(std::string& ss) {
	std::string::iterator   p = find_if(ss.begin(), ss.end(), std::not1(std::ptr_fun(isspace)));
	ss.erase(ss.begin(), p);
	return  ss;
}

std::string& tool::Common::rtrim(std::string& ss) {
	std::string::reverse_iterator  p = find_if(ss.rbegin(), ss.rend(), std::not1(std::ptr_fun(isspace)));
	ss.erase(p.base(), ss.end());
	return   ss;
}

std::string& tool::Common::trim(std::string& st) {
	ltrim(rtrim(st));
	return   st;
}