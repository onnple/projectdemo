#include <iostream>
#include "app.h"
#include <boost/json.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/timer/progress_display.hpp>
#include <cpprest/http_client.h>  // cpprestsdk
#include <cpprest/uri.h>
#include "common.h"
#include <fstream>
#include <pugixml.hpp>
//#include <htmlcxx/ParserDom.h>
#include <tidy.h>
#include <tidybuffio.h>
#include <stdio.h>
#include <errno.h>
//#include <OpenXLSX.hpp>  // C++ 17
#include <xlnt/xlnt.hpp>
#include <mysql/jdbc.h>  // mysql connector c++ 需要改为Release运行


/**
* 需求：
* 小工具：翻译、天气查询、网络爬虫
* 1. 翻译：自定义输入中文翻译成英文，显示结果，保存到excel中
* 2. 天气查询：自定义查询某个城市的天气情况，显示结果，保存到excel中
* 3. 爬取网站内容：https://www.wusuo8.com/	获取所有栏目的内容，并保存到数据库
* 技术栈
* 1. http O - cpprestsdk
* 2. mysql O - mysql connector c++
* 3. excel O - OpenXLSX/xlnt
* 4. http内容解析 O - tidy+pugixml
* 5. json O - boost/json.hpp
* 6. urlencode/decode O - cpprestsdk
*
**/

static void test_boost() {
	boost::timer::progress_display pd(100);
	for (int i = 0; i < 100; i++) {
		++pd;
	}
}

/**
* cpprestsdk http请求
**/
static void test_http() {
	try
	{
		web::http::client::http_client client(U("https://v.api.aa1.cn/api/api-fanyi-yd/index.php?msg=%E6%88%91%E7%88%B1%E4%BD%A0&type=1"));
		auto response = client.request(web::http::methods::GET).get();
		std::wstring body = response.extract_string().get();
		std::wcout.imbue(std::locale("chs"));
		std::wcout << L"Status code: " << response.status_code() << std::endl;
		std::wcout << L"Content type: " << response.headers().content_type() << std::endl;
		std::wcout << L"Content length: " << response.headers().content_length() << L"bytes" << std::endl;
		std::wcout << body << std::endl;
	}
	catch (const std::exception& e)
	{
		std::cout << e.what() << std::endl;
	}
}

/**
* 中文URL编解码
**/
static void test_url() {
	std::wcout.imbue(std::locale("chs"));
	std::wcout << web::uri::encode_data_string(L"我爱你") << std::endl;
	std::wcout << web::uri::encode_data_string(L"深圳") << std::endl;
	std::wcout << web::uri::decode(U("%E6%B7%B1%E5%9C%B3")) << std::endl;
}


/**
* 2. mysql
* 3. excel
* 4. http内容解析
* 5. json
**/
static void test_json() {
	const int Debug = 0;
	std::string json_string, temp;
	std::ifstream in("./json.txt", std::ios::in);
	while (std::getline(in, temp)) {
		json_string.append(temp).append("\n");
	}
	in.close();
	if (Debug) {
		json_string = tool::Common::utf8ToGB(json_string.c_str());
		std::cout << json_string << std::endl;
	}
	std::stringstream ss(json_string);
	//boost::property_tree::ptree root;
	//boost::property_tree::read_json(ss, root);

	/**
	* 1. boost::property_tree::ptree get<>(key)，把当前节点作为一个对象进行获取，key是这个对象的属性名称，如 "key": {"k1": "v1", "k2": "v2", ...}
	* 2. boost::property_tree::ptree get_value<>(key)，把当前节点作为简单的键值对，key是键值，如 "key": "value"
	**/
	//for (auto i = root.begin(); i != root.end(); i++) {
	//	std::string key = i->first;
	//	auto value = i->second;
	//	std::cout << key << ": " << Common::utf8ToGB(value.get_value<std::string>(key).c_str()) << std::endl;
	//}
	//for (auto i = root.begin(); i != root.end(); i++) {
	//	std::string key = i->first;
	//	auto value = i->second;
	//	if (key == "code") {
	//		std::cout << key << ": " << value.get_value<std::string>(key) << std::endl;
	//	}
	//	else if (key == "data") {
	//		auto data = value.get_child("");
	//		for (auto i = data.begin(); i != data.end(); i++) {
	//			std::cout << Common::utf8ToGB(i->second.get<std::string>("fengdu").c_str()) << std::endl;
	//		}
	//	}
	//}

	// JSON字符串转为CXX对象
	auto obj = boost::json::parse(ss);
	auto r = boost::json::value_to<tool::model::TranslationModel::Response>(obj);
	std::cout << tool::Common::utf8ToGB(r.type.c_str()) << std::endl;
	std::cout << tool::Common::utf8ToGB(r.desc.c_str()) << std::endl;
	std::cout << r.text << std::endl;
	std::cout << "-----------------" << std::endl;

	// CXX对象转为JSON字符串
	tool::model::TranslationModel::Response res("英语转西语", "如何将英语翻译为西语", "hola");
	boost::json::value v = boost::json::value_from(res);
	std::cout << v << std::endl;
}

/**
* boost::json
**/
static void test_json2() {
	// 1. 序列化：构造JSON对象
	boost::json::object obj;
	obj["a_string"] = "Once";
	obj["a_number"] = 189;
	obj["a_null"] = NULL;
	obj["a_array"] = { 2, "3.14", boost::json::object({{"n1", "frida"}}) };
	obj["a_object"].emplace_object()["name"] = "xposed";
	//obj["a_object"].emplace_object()["age"] = 1213;
	obj["a_bool"] = true;
	std::cout << obj << std::endl;
	std::cout << "---------------------------" << std::endl;

	/**
	* 2. 序列化：快速构造JSON对象
	* {}表示一个对象，{{1}}中的{1}表示对象的一个属性键值对。如果第一层多于一个属性键值对，那么表示一个数组
	*
	* 1）{args}，如果{}中只有一个值或三个或三个以上，则{args}表示一个数组，如果是两个则表示一组属性的键值对：
	* 2）args中只有两个值：
	*	2.1）普通数据属性键值对：{k, v}，k和v都是一般数据类型的数据，则{k, v}表示一个属性键值对，k表示该属性的名称，v表示该属性的值
	*	2.2）数组：{k, {array}}，k为属性名称，array表示k属性保存的是一个数组，整个都是数组，无论array中有多少个值都是数组
	**/
	boost::json::value v = {
		{"string", "edxposed"},
		{"number", 3.1415926},
		{"null", NULL},
		{"array", {8, "frida", {{"name", "python"}, {"os", "linux"}}}},
		{"bool", true},
		{"data", {{{"n1", "1"}, {"n2", "2"}}, {{"n1", "5"}, {"n2", "4"}}}},
	};
	std::cout << v << std::endl;
	std::cout << "---------------------------" << std::endl;
	std::cout << boost::json::serialize(v) << std::endl;
	std::cout << "---------------------------" << std::endl;

	// 3. 反JSON序列化：将JSON字符串转为JSON对象，不支持中文
	std::string json = "{\"type\": \"AAA\",\"desc\": \"BBB\",\"text\": \"I love you\"}";
	boost::json::value value = boost::json::parse(json);
	std::cout << value.at("type").as_string() << std::endl;
	std::cout << value.at("desc").as_string() << std::endl;
	std::cout << value.at("text").as_string() << std::endl;
}

/**
* cpprest sdk
* get/post
* 1. 同步请求
* 2. 异步请求
*
* GET同步请求
**/
static void test_cpprest1() {
	// 1. 创建URI
	//std::wstring cityName = L"北京";
	//cityName = web::uri::encode_data_string(cityName);
	//std::wstring url = L"http://localhost:8080/api/test/option";
	//url.replace(url.find(L"{"), 10, cityName);
	//std::wcout << "Request: " << url << std::endl;
	//web::uri uri(url);

	// 2. 创建http客户端
	web::http::client::http_client_config config;
	config.set_proxy(web::web_proxy(web::uri(L"http://127.0.0.1:1080")));
	web::http::client::http_client client(L"http://localhost:8080/api/test/option", config);

	// 3. 添加查询参数
	web::uri_builder uri_builder;
	uri_builder.append_query(L"id", 2000);
	uri_builder.append_query(L"username", "Once");
	uri_builder.append_query(L"password", "YJUF1S5DF1JH4S86Z");

	// 4. 创建请求
	web::http::http_request request(web::http::methods::GET);
	request.set_request_uri(uri_builder.to_uri());
	//auto request = client.request(web::http::methods::GET, uri_builder.to_string());

	// 5. 添加请求头
	request.headers().add(L"Token", L"hg1j5s1d51saf54dgh2zs15x8r3a");
	request.headers().add(L"Cookie", L"key=5f8h415s41df8sd9;value=4g8hj10sza5dw;ck_XP=2fgthrt15h1yt5");

	// 6. 发送请求，接收请求
	web::http::http_response res = client.request(request).get();

	// 7. 获取返回的响应内容
	std::wstring json = res.extract_string().get();

	std::wcout << json << std::endl;
}

/**
* GET异步请求
**/
static void test_cpprest2() {
	// 1. 创建HTTP客户端
	web::http::client::http_client client(L"http://localhost:8080/api/test/option");

	// 2. 添加请求参数
	web::http::uri_builder uri_builder;
	uri_builder.append_query(L"id", L"1000");
	uri_builder.append_query(L"username", L"exile");
	uri_builder.append_query(L"password", L"a41s5fdadadwe8r4fwe8r5f1xcv2");

	// 3. 创建请求
	web::http::http_request request(web::http::methods::GET);
	request.set_request_uri(uri_builder.to_uri());

	// 4. 添加请求头
	request.headers().add(L"Token", L"*SAD145FGHIE415DS1^(^UKU21VY&");
	request.headers().add(L"Cookie", L"key=f45g1g5fd4g1d5sf4gf5d;ck_ptr=48kiu45484df4as154kuy;");

	// 5. 开始异步请求
	std::cout << "http request....." << std::endl;
	pplx::task<void> getTask = client.request(request).then([](web::http::http_response response) { return response.extract_string(); })
		.then([](std::wstring body) { std::wcout << body << std::endl; });

	// 6. 等待请求
	try {
		getTask.wait();
		std::cout << "http close....." << std::endl;
	}
	catch (const std::exception& e) {
		std::cout << e.what() << std::endl;
	}
}

/**
* POST同步请求
**/
static void test_cpprest3() {
	// 1. 创建HTTP客户端
	web::http::client::http_client client(L"http://localhost:8080/api/test/option");

	// 2. 创建请求
	web::http::http_request request(web::http::methods::POST);

	// 3. 添加请求头
	request.headers().add(L"Content-Type", L"application/json");

	// 3. 添加JSON请求体
	request.set_body(L"{\"username\": \"exile\",\"password\" : \"12s5fd1saf1ds56f41d4\"}");

	// 4. 开始请求
	web::http::http_response res = client.request(request).get();
	//pplx::task<void> myTask = client.request(request).then([](web::http::http_response res) {return res.extract_string(); }).then([](std::wstring body) {
	//	std::wcout.imbue(std::locale("chs"));
	//	std::wcout << body << std::endl;
	//	});

		// 5. 获取响应内容
	std::wstring result = res.extract_string().get();

	//try {
	//	myTask.wait();
	//}
	//catch (const std::exception& e) {
	//	std::cout << e.what() << std::endl;
	//}

	std::wcout.imbue(std::locale("chs"));
	std::wcout << result << std::endl;
}

/**
* POST异步请求
**/
static void test_cpprest4() {
	// 1. 创建http客户端
	web::http::client::http_client client(L"http://localhost:8080/api/test/option");

	// 2. 创建请求
	web::http::http_request request(web::http::methods::POST);
	web::http::uri_builder uri_builder;
	request.set_request_uri(uri_builder.to_uri());

	// 3. 添加请求头
	request.headers().add(L"Content-Type", L"application/json");

	// 4. 添加请求体
	request.set_body(L"{\"username\": \"sign of the times\",\"password\" : \"y52a0z98ref14qqwafh\"}");

	// 5. 创建异步请求和处理请求
	pplx::task<void> httpTask = client.request(request).then([](web::http::http_response response) { return response.extract_string(); })
		.then([](std::wstring body) { std::wcout.imbue(std::locale("chs")); std::wcout << body << std::endl; });

	// 6. 等待请求结果
	try {
		httpTask.wait();
	}
	catch (const std::exception& e) {
		std::cout << e.what() << std::endl;
	}
}

/**
* XML文本解析：pugixml
**/
static void test_http_resolver1() {
	std::string html = "<div><meta content=\"Content-Type\"/><span>sign of the times</span><a href=\"https://www.blog.csdn.net/\">访问主页</a></div>";
	// 1. 创建一个XML文档
	pugi::xml_document doc;
	//pugi::xml_parse_result result = doc.load_file("./index.xml");

	// 2. 加载内容到文档中并进行解析
	pugi::xml_parse_result result = doc.load_string(html.c_str());
	if (!result) std::cout << "HTML解析失败！" << std::endl;

	// 3. 文档处理：增删改查CRUD
	for (pugi::xml_node node : doc.child("div")) {
		std::cout << node.name() << " -> " << node.child_value() << std::endl;
		if (strcmp(node.name(), "a") == 0) {
			std::cout << "a[href]=" << node.attribute("href").value() << std::endl;
		}
	}
	doc.save(std::cout);
}

/**
* XML CRUD + 遍历 + 打印
**/
static void test_http_resolver2() {
	// 1. 读取XML文档
	pugi::xml_document doc;
	pugi::xml_parse_result r = doc.load_file("./index.html");
	if (!r) {
		std::cout << "read html failed!" << std::endl;
		std::cout << r.description() << std::endl;
	}
	auto metas = doc.child("html").child("head").children("meta");
	for (pugi::xml_node meta : metas) {
		std::cout << meta.attribute("name").value() << " -> " << meta.attribute("content").value() << std::endl;
	}
}

static void test_html1() {
	////待解析的html数据
	//std::string html = "<div><span>sign of the times</span><a href=\"https://blog.csdn.net/\">访问主页</a></div>";
	////有些含有中文的html解析可能报错，需要添加这句
	//setlocale(LC_ALL, ".OCP");
	//htmlcxx::HTML::ParserDom parser;
	//tree<htmlcxx::HTML::Node> dom = parser.parseTree(html);

	////遍历文档的迭代器
	//tree<htmlcxx::HTML::Node>::iterator it = dom.begin();
	//tree<htmlcxx::HTML::Node>::iterator end = dom.end();
	////遍历整个dom
	//for (; it != end; ++it) {
	//	//it->isTag()判断it指向的节点是否是标签
	//	if (it->isTag()) {
	//		//加载标签的属性
	//		it->parseAttributes();
	//		//it->tagName()获取标签的名字
	//		//it->attribute("href")返回的是pair<bool,object>，first表示的该标签是否含有该属性，second是属性值
	//		if (it->tagName() == "a" && it->attribute("href").first) {
	//			std::cout << "a[href]=" << it->attribute("href").second << std::endl;
	//		}
	//		else if (it->tagName() == "span") {
	//			//std::cout << it-> << std::endl;
	//		}
	//	}
	//	else {
	//		std::cout << it->text() << std::endl;
	//	}
	//}
}

static void test_html2() {
	const char* input = "<title>Foo</title><img src=\"abc\"><meta content=\"key\">";
	TidyBuffer output = { 0 };
	TidyBuffer errbuf = { 0 };
	int rc = -1;
	Bool ok;

	TidyDoc tdoc = tidyCreate();                     // Initialize "document"
	printf("Tidying:\t%s\n", input);

	ok = tidyOptSetBool(tdoc, TidyXhtmlOut, yes);  // Convert to XHTML
	if (ok)
		rc = tidySetErrorBuffer(tdoc, &errbuf);      // Capture diagnostics
	if (rc >= 0)
		rc = tidyParseString(tdoc, input);           // Parse the input
	if (rc >= 0)
		rc = tidyCleanAndRepair(tdoc);               // Tidy it up!
	if (rc >= 0)
		rc = tidyRunDiagnostics(tdoc);               // Kvetch
	if (rc > 1)                                    // If error, force output.
		rc = (tidyOptSetBool(tdoc, TidyForceOutput, yes) ? rc : -1);
	if (rc >= 0)
		rc = tidySaveBuffer(tdoc, &output);          // Pretty Print

	if (rc >= 0)
	{
		if (rc > 0)
			printf("\nDiagnostics:\n\n%s", errbuf.bp);
		printf("\nAnd here is the result:\n\n%s", output.bp);
	}
	else
		printf("A severe error (%d) occurred.\n", rc);

	tidyBufFree(&output);
	tidyBufFree(&errbuf);
	tidyRelease(tdoc);
}

// html to xml
static void test_html3() {
	std::string html = "<head><meta content=\"Content-Type\" value=\"UTF-8\"></head><img src=\"http://www.baidu.com\">";
	std::string xml = tool::Common::html2XML(html.c_str());
	std::cout << "HTML:\n" << html << std::endl;
	std::cout << std::endl;
	std::cout << "XML:\n" << xml << std::endl;
}

// xml to html
static void test_html4() {
	std::string xml = "<head><meta content=\"Content-Type\" value=\"UTF-8\"/></head><img src=\"http://www.baidu.com\"/>";
	std::string html = tool::Common::xml2HTML(xml.c_str());
	std::cout << "XML:\n" << xml << std::endl;
	std::cout << std::endl;
	std::cout << "HTML:\n" << html << std::endl;
}

/**
* 解析HTML
* 1. 获取HTML文本
* 2. 将HTML文本转为XML
* 3. 使用pugixml处理XML
* 4. 结果保留XML或将XML转为HTML
**/
static void test_tidy_pugixml1() {
	std::string htmlText = tool::Common::readText("./post.html");
	htmlText = tool::Common::html2XML(htmlText.c_str());
	//std::cout << htmlText << std::endl;
	pugi::xml_document doc;
	pugi::xml_parse_result r = doc.load_string(htmlText.c_str());
	if (!r) {
		std::cout << "read html failed!" << std::endl;
		std::cout << r.description() << std::endl;
	}
	//auto metas = doc.child("html").child("head").children("meta");
	//for (pugi::xml_node meta : metas) {
	//	if (strcmp(meta.attribute("name").value(), "") != 0)
	//		std::cout << meta.attribute("name").value() << " -> " << tool::Common::utf8ToGB(meta.attribute("content").value()) << std::endl;
	//}
	//std::cout << std::endl;

	pugi::xpath_node title = doc.select_node("/html/head/title");
	std::cout << "标题：\n" << tool::Common::utf8ToGB(title.node().child_value()) << std::endl;
	std::cout << std::endl;
	pugi::xpath_node description = doc.select_node("/html/head/meta[@name='description']");
	std::cout << "描述：\n" << tool::Common::utf8ToGB(description.node().attribute("content").value()) << std::endl;
	std::cout << std::endl;
	pugi::xpath_node keywords = doc.select_node("/html/head/meta[@name='keywords']");
	std::cout << "关键词：\n" << tool::Common::utf8ToGB(keywords.node().attribute("content").value()) << std::endl;
	std::cout << std::endl;
	pugi::xpath_node content = doc.select_node("/html/body/div[1]/main/article/section");
	std::stringstream ss;
	content.node().print(ss);
	std::string s = ss.str();
	std::cout << "内容：\n" << tool::Common::utf8ToGB(s.c_str()).substr(0, 300) << "......" << std::endl;
	std::cout << std::endl;

	std::string html = tool::Common::xml2HTML(s.c_str());
	std::cout << "HTML:\n" << tool::Common::utf8ToGB(html.substr(html.find("<img"), 300).c_str()) << std::endl;
	std::cout << std::endl;

	std::cout << "BEGIN:\n" << tool::Common::utf8ToGB(html.substr(0, 300).c_str()) << std::endl;
	std::cout << std::endl;

	std::cout << "CONTENT:\n" << tool::Common::utf8ToGB(html.substr(html.find("<body>") + 6, 300).c_str()) << std::endl;
	std::cout << "..............." << std::endl;
	std::cout << tool::Common::utf8ToGB(html.substr(html.find("</body>") - 300, 300).c_str()) << std::endl;
	std::cout << std::endl;
}

static void test_tidy_pugixml2() {
	std::string htmlText = tool::Common::readText("./body.html");
	htmlText = tool::Common::html2XML(htmlText.c_str());
	pugi::xml_document doc;
	pugi::xml_parse_result r = doc.load_string(htmlText.c_str());
	if (!r) {
		std::cout << "read html failed!" << std::endl;
		std::cout << r.description() << std::endl;
	}
	pugi::xpath_node content = doc.select_node("//*[@id='content']");
	std::stringstream ss;
	content.node().print(ss);
	std::string s = ss.str();
	std::cout << "内容：\n" << tool::Common::utf8ToGB(s.c_str()).substr(0, 500) << "......" << std::endl;
	std::cout << std::endl;

	std::string html = tool::Common::xml2HTML(s.c_str());
	std::cout << "CONTENT:\n" << tool::Common::utf8ToGB(html.substr(html.find("<body>") + 6, 300).c_str()) << std::endl;
	std::cout << "..............." << std::endl;
	std::cout << tool::Common::utf8ToGB(html.substr(html.find("</body>") - 300, 300).c_str()) << std::endl;
	std::cout << std::endl;
}

static void test_tidy_pugixml3() {
	pugi::xml_document doc;
	// 1. 添加节点
	pugi::xml_node root = doc.append_child("body");
	pugi::xml_node h1 = root.append_child("h1");
	h1.append_child(pugi::node_pcdata).set_value("A Root Of Tree");
	pugi::xml_node p = root.append_child("p");
	p.append_child(pugi::node_pcdata).set_value("As fast as possible....");
	pugi::xml_node img = root.append_child("img");
	img.append_attribute("src").set_value("https://www.google.com/");
	pugi::xml_node div = root.append_child("div");
	div.append_attribute("class").set_value("bg-normal on-green");
	pugi::xml_node p2 = root.append_child("p");
	p2.append_child(pugi::node_pcdata).set_value("Someone asked me once in an interview....");

	// 2. 删除节点
	root.remove_child("p");
	//root.remove_children();

	// 3. 修改节点
	pugi::xml_node h2 = root.insert_child_before("h2", img);
	h2.append_child(pugi::node_pcdata).set_value("I Really Want To Stay At Your House");

	// 4. 查找节点
	pugi::xml_node i = root.find_child([](pugi::xml_node node) { if (strcmp(node.name(), "img")) return true; else return false; });
	i.attribute("src").set_value("https://www.youtube.com/");

	std::stringstream ss;
	doc.print(ss);
	std::cout << ss.str() << std::endl;
}

/**
* Excel文件读写
**/
static void test_excel1() {
	xlnt::workbook book;
	xlnt::worksheet sheet = book.active_sheet();
	sheet.cell("A1").value(1314);
	sheet.cell("B2").value("frida and xposed");
	sheet.cell("C3").formula("=RAND()");
	sheet.merge_cells("C3:C4");
	sheet.freeze_panes("B2");
	book.save("book.xlsx");
}

/**
* 读取excel文件
**/
static void test_excel2() {
	xlnt::workbook book;
	book.load("ids.xlsx");
	xlnt::worksheet ws = book.active_sheet();
	for (int i = 2; i < 40; i++) {
		for (int j = 1; j < 8; j++) {
			xlnt::cell c = ws.cell(xlnt::column_t(j), xlnt::row_t(i));
			if (j == 1 && i > 2) std::cout << c.value<int>() << "\t";
			else std::cout << tool::Common::utf8ToGB(c.value<std::string>().c_str()) << "\t";
		}
		std::cout << std::endl;
	}
}

/**
* excel创建和写入
**/
static void test_excel3() {
	xlnt::workbook book;
	xlnt::worksheet ws = book.active_sheet();
	ws.cell("A1").number_format(xlnt::number_format::percentage());
	ws.cell("A1").value(0.31445926);
	ws.cell("A2").number_format(xlnt::number_format::number_00());
	ws.cell("A2").value(3.1415926);
	ws.cell("A3").number_format(xlnt::number_format::date_datetime());
	ws.cell("A3").value("2023/11/22 12:30:40");
	book.save("excel2.xlsx");
}

/**
* 遍历Excel
**/
static void test_excel4() {
	xlnt::workbook book;
	book.load("ids.xlsx");
	xlnt::worksheet sheet = book.active_sheet();
	for (xlnt::worksheet::iterator i = sheet.begin(); i != sheet.end(); i++) {
		xlnt::cell_vector v = *i;
		for (xlnt::cell_vector::iterator vi = v.begin(); vi != v.end(); vi++) {
			xlnt::cell c = *vi;
			if (c.data_type() == xlnt::cell_type::number)
				std::cout << c.value<int>() << "\t";
			else
				std::cout << tool::Common::utf8ToGB(c.value<std::string>().c_str()) << "\t";
		}
		std::cout << std::endl;
	}
}

/**
* mysql增删改查：使用本地mysql中的静态库和头文件
**/
static void test_mysql1() {
	try {
		// 1. 创建mysql数据库驱动
		sql::mysql::MySQL_Driver* driver = sql::mysql::get_driver_instance();
		sql::Connection* con;
		sql::Statement* stmt;
		sql::ResultSet* res;

		// 2. 创建数据库连接
		con = driver->connect("tcp://127.0.0.1:3306/test", "root", "123456");

		// 3. 创建语句
		stmt = con->createStatement();

		// 4. 执行数据库语句
		res = stmt->executeQuery("SELECT * from user;");

		// 5. 处理语句返回结果
		while (res && res->next()) {
			std::cout << res->getInt(1) << "\t" << res->getString("username") << "\t\t" << res->getString("password") << std::endl;
		}

		// 6. 释放数据库资源：数据库连接，语句，返回结果
		delete res;
		delete stmt;
		delete con;
	}

	// 7. 处理数据库处理异常
	catch (sql::SQLException& e) {
		std::cout << "# ERR: SQLException in " << __FILE__;
		std::cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << std::endl;
		std::cout << "# ERR: " << e.what();
		std::cout << " (MySQL error code: " << e.getErrorCode();
		std::cout << ", SQLState: " << e.getSQLState() << " )" << std::endl;
	}
}

/**
* 添加数据
**/
static void test_mysql2() {
	// 1. 创建mysql数据库驱动
	sql::mysql::MySQL_Driver* driver = sql::mysql::get_driver_instance();

	// 2. 连接数据库
	sql::Connection* connection = driver->connect("tcp://127.0.0.1:3306/test", "root", "123456");

	// 3. 创建语句
	sql::PreparedStatement* statement = connection->prepareStatement("insert into user values(?,?,?,?);");
	srand(time(NULL));
	for (int i = 0; i < 10; i++) {
		statement->setInt(1, 0);
		statement->setString(2, std::to_string(rand()));
		statement->setString(3, std::to_string(rand()));
		statement->setString(4, std::to_string(rand()));

		// 4. 执行语句
		int r = statement->executeUpdate();
		// 5. 处理执行结果
		if (r) std::cout << "添加成功：[" << r << "] " << std::endl;
		else std::cout << "添加失败！" << std::endl;
	}

	// 6. 释放资源：数据库连接，语句，查询结果
	delete connection;
	delete statement;
}

/**
* 删除数据
**/
static void test_mysql3() {
	try {
		// 1. 创建mysql数据库驱动
		sql::mysql::MySQL_Driver* driver = sql::mysql::get_driver_instance();

		// 2. 连接数据库
		sql::Connection* con = driver->connect("tcp://127.0.0.1:3306/test", "root", "123456");

		// 3. 创建语句
		sql::Statement* stat = con->createStatement();

		// 4. 执行语句
		int r = stat->executeUpdate("delete from user where id = 1");
		// 5. 处理返回结果
		if (!r) {
			std::cout << "删除失败！" << std::endl;
			delete stat;
			delete con;
			return;
		}
		std::cout << "删除成功！" << std::endl;
		// 6. 释放资源
		delete stat;
		delete con;
	}
	// 7. 处理异常
	catch (const std::exception& e) {
		std::cout << e.what() << std::endl;
	}
}

/**
* 更新数据
**/
static void test_mysql4() {
	try {
		// 1. 创建mysql数据驱动
		sql::mysql::MySQL_Driver* driver = sql::mysql::get_driver_instance();

		// 2. 创建数据库连接
		sql::Connection* con = driver->connect("tcp://127.0.0.1/test", "root", "123456");

		// 3. 创建语句
		sql::Statement* stat = con->createStatement();

		// 4. 执行语句
		int r = stat->executeUpdate("update user set username='food', password='funny', description='fuck' where id = 2");

		// 5. 处理语句执行结果
		if (!r) {
			std::cout << "更新数据失败！" << std::endl;
			delete stat;
			delete con;
			return;
		}

		// 6. 释放资源
		std::cout << "更新数据成功！" << std::endl;
		delete stat;
		delete con;

		// 7. 处理异常
	}
	catch (const std::exception& e) {
		std::cout << e.what() << std::endl;
	}
}

/**
* 查询数据
**/
#include "config.h"
static void test_mysql5() {
	try {
		// 1. 创建mysql数据库驱动
		sql::mysql::MySQL_Driver* driver = sql::mysql::get_driver_instance();

		// 2. 连接数据库
		sql::Connection* con = driver->connect("tcp://127.0.0.1/test", "root", "123456");

		// 3. 创建查询语句
		sql::Statement* stat = con->createStatement();

		// 4. 执行查询语句
		sql::ResultSet* set = stat->executeQuery("select * from user;");

		// 5. 处理查询结果
		if (!set) {
			std::cout << "查询失败！" << std::endl;
			delete stat;
			delete con;
			return;
		}
		while (set->next()) {
			std::cout << set->getInt("id") << "\t" << set->getString("username") << "\t\t" << set->getString("password") << std::endl;
		}

		// 6. 释放数据资源
		delete set;
		delete stat;
		delete con;
	}
	// 7. 处理异常
	catch (const std::exception& e) {
		std::cout << e.what() << std::endl;
	}
}

static void testcase1() {
	tool::model::TranslationModel* tModel = new tool::model::TranslationModel();
	std::string r1 = tModel->translate("我是谁？我从哪里来？我要走向何方？", 1);
	std::string r2 = tModel->translate("what would you do if you don't have much money?", 2);
	std::string r3 = tModel->translate("someone asked me once in an interview", 3);
	std::string r4 = tModel->translate("如何快速完成任务？", 0);
	std::string r5 = tModel->translate("I can see you standing honey, with his arms around you body.", 4);
	std::cout << "中英：" << r1 << std::endl;
	std::cout << "英中：" << r2 << std::endl;
	std::cout << "自动：" << r3 << std::endl;
	std::cout << "自动：" << r4 << std::endl;
	std::cout << "自动：" << r5 << std::endl;
}

static void testcase2() {
	tool::model::ExcelModel* eModel = new tool::model::ExcelModel();
	if (eModel->saveTranslation("how are you?", "你最近怎么样？")) {
		std::cout << "保存翻译到Excel成功！" << std::endl;
		std::cout << tool::Common::UTF8_2_GBK(eModel->getTranslation("how are you?")) << std::endl;
	}
	else std::cout << "保存翻译到Excel失败！" << std::endl;
}

static void testcase3() {
	tool::model::ExcelModel* eModel = new tool::model::ExcelModel();
	std::cout << tool::Common::UTF8_2_GBK(eModel->getTranslation("你现在在哪个城市工作？")) << std::endl;
}

static void testcase4() {
	tool::model::ExcelModel* eModel = new tool::model::ExcelModel();
	std::cout << "保存城市天气：" << eModel->saveWeather("广州", "blablablablablabla") << std::endl;
	delete eModel;
}

static void testcase51() {
	tool::model::WeatherModel::Response res;
	res.code = "1";
	for (int i = 0; i < 5; i++) {
		tool::model::WeatherModel::WDay day;
		day.riqi = tool::Common::GBK_2_UTF8("星期一");
		day.wendu = tool::Common::GBK_2_UTF8("暖和");
		day.tianqi = tool::Common::GBK_2_UTF8("晴朗");
		day.fengdu = tool::Common::GBK_2_UTF8("微风");
		day.pm = tool::Common::GBK_2_UTF8("优");
		res.data.push_back(day);
	}
	auto value = boost::json::value_from(res);
	std::stringstream s1;
	s1 << value;
	std::cout << tool::Common::UTF8_2_GBK(s1.str()) << std::endl;
	std::cout << std::endl;

	std::stringstream ss;
	ss << value;
	std::string json = ss.str();
	auto v = boost::json::parse(json);
	auto r = boost::json::value_to<tool::model::WeatherModel::Response>(v);
	std::cout << "code:   " << r.code << std::endl;
	std::cout << "tianqi: " << tool::Common::UTF8_2_GBK(r.data[3].tianqi) << std::endl;
}

static void testcase5() {
	tool::model::WeatherModel* sModel = new tool::model::WeatherModel();
	std::cout << "广州天气：\n" << sModel->search("广州") << std::endl;
	std::cout << "深圳天气：\n" << sModel->search("深圳") << std::endl;
	std::cout << "东莞天气：\n" << sModel->search("东莞") << std::endl;
	delete sModel;
}

static void test_html_encoding() {
	std::string htmlText = tool::Common::readText("./sample.html");
	htmlText = tool::Common::GBK_2_UTF8(htmlText);
	std::cout << "原始内容：\n" << tool::Common::UTF8_2_GBK(htmlText.substr(200, 500)) << std::endl;
	std::cout << std::endl;
	htmlText = tool::Common::html2XML(htmlText.c_str());
	std::cout << "转为XML后：\n" << tool::Common::UTF8_2_GBK(htmlText.substr(200, 500)) << std::endl;
	std::cout << std::endl;

	pugi::xml_document doc;
	pugi::xml_parse_result r = doc.load_string(htmlText.c_str());
	if (!r) {
		std::cout << "read html failed!" << std::endl;
		std::cout << r.description() << std::endl;
		return;
	}

	pugi::xpath_node title = doc.select_node("/html/head/title");
	std::cout << "标题：\n" << tool::Common::utf8ToGB(title.node().child_value()) << std::endl;
	std::cout << std::endl;
	pugi::xpath_node description = doc.select_node("/html/head/meta[@name='description']");
	std::cout << "描述：\n" << tool::Common::utf8ToGB(description.node().attribute("content").value()) << std::endl;
	std::cout << std::endl;
	pugi::xpath_node keywords = doc.select_node("/html/head/meta[@name='keywords']");
	std::cout << "关键词：\n" << tool::Common::utf8ToGB(keywords.node().attribute("content").value()) << std::endl;
	std::cout << std::endl;
	pugi::xpath_node_set content = doc.select_nodes("/html/body/div[1]/div[3]/article/p");
	std::stringstream ss;
	for (auto n : content) {
		n.node().print(ss);
	}
	std::string s = ss.str();
	std::cout << "内容1：\n" << tool::Common::utf8ToGB(s.substr(s.find("<body>") + 6, s.find("</body>")).c_str()) << std::endl;
	std::cout << std::endl;

	std::string html = tool::Common::xml2HTML(s.c_str());

	std::cout << "内容2：\n" << tool::Common::utf8ToGB(html.substr(html.find("<body>") + 6, html.find("</body>")).c_str()) << std::endl;
}

static void testcase6() {
	tool::model::SpiderModel* sModel = new tool::model::SpiderModel;
	sModel->spider();
	delete sModel;
}

static void testcase7() {
	tool::model::SpiderModel* sModel = new tool::model::SpiderModel;
	sModel->print();
	delete sModel;
}

static void testcase8() {
	tool::model::SpiderModel* sModel = new tool::model::SpiderModel;
	sModel->clear();
	delete sModel;
}

static void testcase9() {

}

static void testcase10() {

}

static void testcase11() {

}

static void testcase12() {

}

int main(int argc, char** args) {
	try {
		tool::app::App::getApp()->run();
		//test_http();
		//test_url();
		//test_json();
		//test_json2();
		//test_cpprest1();
		//test_cpprest2();
		//test_cpprest3();
		//test_cpprest4();
		//test_http_resolver1();
		//test_http_resolver2();
		//test_html1();
		//test_html2();
		//test_html3();
		//test_html4();
		//test_tidy_pugixml1();
		//test_tidy_pugixml2();
		//test_tidy_pugixml3();
		//test_excel1();
		//test_excel2();
		//test_excel3();
		//test_excel4();
		//test_mysql1();
		//test_mysql2();
		//test_mysql3();
		//test_mysql4();
		//test_mysql5();

		//testcase1();
		//testcase2();
		//testcase3();
		//testcase4();
		//testcase51();
		//testcase5();
		//test_html_encoding();
		//testcase6();
		//testcase7();
		//testcase8();
		testcase9();
		testcase10();
		testcase11();
		testcase12();
	}
	catch (const std::exception& e) {
		std::cout << tool::Common::UTF8_2_GBK(e.what()) << std::endl;
	}
	return 0;
}