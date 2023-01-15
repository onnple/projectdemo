#define _CRT_SECURE_NO_WARNINGS
#include "model.h"
#include <boost/json.hpp>
#include <cpprest/http_client.h>
#include <cpprest/uri.h>
#include <sstream>
#include <string>
#include "common.h"
#include <xlnt/xlnt.hpp>
#include "config.h"
#include <mysql/jdbc.h>
#include <pugixml.hpp>
#include <tidy.h>
#include <tidybuffio.h>
#include <boost/thread/thread.hpp>
#include <chrono>
#include <thread>

std::string tool::model::TranslationModel::translate(std::string text, int type) {
	if (type < 1 || type > 2) type = 3;
	web::http::client::http_client_config config;
	config.set_timeout(utility::seconds(10));
	web::http::client::http_client client(L"https://v.api.aa1.cn/api/api-fanyi-yd/index.php", config);
	web::uri_builder uri_builder;
	uri_builder.append_query(L"msg", tool::Common::string2Wstring(text));
	uri_builder.append_query(L"type", std::to_wstring(type));
	web::http::http_request request(web::http::methods::GET);
	request.set_request_uri(uri_builder.to_uri());
	web::http::http_response res = client.request(request).get();
	std::wstring json = res.extract_string().get();
	std::stringstream ss(tool::Common::wstring2String(json));

	boost::json::parse_options opt;
	opt.allow_comments = true;
	opt.allow_trailing_commas = true;
	opt.allow_invalid_utf8 = true;
	auto obj = boost::json::parse(ss, {}, opt);
	auto r = boost::json::value_to<tool::model::TranslationModel::Response>(obj);
	return r.text;
}

std::string tool::model::WeatherModel::search(std::string cityName) {
	web::http::client::http_client_config config;
	config.set_timeout(utility::seconds(10));
	web::http::client::http_client client(L"https://v.api.aa1.cn/api/api-tianqi-3/index.php", config);
	web::uri_builder uri_builder;
	uri_builder.append_query(L"msg", tool::Common::string2Wstring(cityName));
	uri_builder.append_query(L"type", std::to_wstring(1));
	web::http::http_request request(web::http::methods::GET);
	request.set_request_uri(uri_builder.to_uri());
	web::http::http_response res = client.request(request).get();
	std::wstring json = res.extract_string().get();
	std::stringstream ss(tool::Common::wstring2String(json));

	boost::json::parse_options opt;
	opt.allow_comments = true;
	opt.allow_trailing_commas = true;
	opt.allow_invalid_utf8 = true;
	auto obj = boost::json::parse(ss, {}, opt);
	auto r = boost::json::value_to<tool::model::WeatherModel::Response>(obj);
	if (r.code != "1" || r.data.empty()) return "";
	std::string w;
	for (auto d : r.data) {
		w.append(d.riqi).append("\t").append(d.wendu).append("\t").append(d.tianqi).append("\t").append(d.fengdu).append("\t").append(d.pm).append("\n");
	}
	return w;
}

void tool::model::tag_invoke(boost::json::value_from_tag, boost::json::value& jv, tool::model::TranslationModel::Response const& c) {
	auto& jo = jv.emplace_object();
	jo["type"] = c.type;
	jo["desc"] = c.desc;
	jo["text"] = c.text;
}

tool::model::TranslationModel::Response tool::model::tag_invoke(boost::json::value_to_tag<tool::model::TranslationModel::Response>, boost::json::value const& jv) {
	auto& jo = jv.as_object();
	return tool::model::TranslationModel::Response(jo.at("type").as_string().c_str(), jo.at("desc").as_string().c_str(), jo.at("text").as_string().c_str());
}

void tool::model::tag_invoke(boost::json::value_from_tag, boost::json::value& jv, tool::model::WeatherModel::Response const& r) {
	auto& jo = jv.emplace_object();
	jo["code"] = r.code;
	boost::json::array arr;
	for (auto i : r.data) {
		boost::json::value v;
		v = {
			{"riqi", i.riqi},
			{"wendu", i.wendu},
			{"tianqi", i.tianqi},
			{"fengdu", i.fengdu},
			{"pm", i.pm},
		};
		arr.push_back(v);
	}
	jo["data"] = arr;
}

tool::model::WeatherModel::Response tool::model::tag_invoke(boost::json::value_to_tag<tool::model::WeatherModel::Response>, boost::json::value const& jv) {
	auto& jo = jv.as_object();
	auto array = jo.at("data").as_array();
	tool::model::WeatherModel::Response res;
	res.code = jo.at("code").as_string().c_str();
	for (auto item : array) {
		tool::model::WeatherModel::WDay day;
		day.riqi = item.at("riqi").as_string().c_str();
		day.wendu = item.at("wendu").as_string().c_str();
		day.tianqi = item.at("tianqi").as_string().c_str();
		day.fengdu = item.at("fengdu").as_string().c_str();
		day.pm = item.at("pm").as_string().c_str();
		res.data.push_back(day);
	}
	return res;
}

void tool::model::tag_invoke(boost::json::value_from_tag, boost::json::value& jv, tool::model::Websites const& w) {
	auto& jo = jv.emplace_object();
	boost::json::array arr;
	for (auto i : w.data_list) {
		boost::json::value v;
		v = {
			{"link_base_url", i.link_base_url},
			{"link_paging_tag", i.link_paging_tag},
			{"links_xpath", i.links_xpath},
			{"content_title_xpath", i.content_title_xpath},
			{"content_cover_xpath", i.content_cover_xpath},
			{"content_price_xpath", i.content_price_xpath},
			{"content_views_xpath", i.content_views_xpath},
			{"content_update_time_xpath", i.content_update_time_xpath},
			{"content_xpath", i.content_xpath}
		};
		arr.push_back(v);
	}
	jo["data_list"] = arr;
}

tool::model::Websites tool::model::tag_invoke(boost::json::value_to_tag<tool::model::Websites>, boost::json::value const& jv) {
	auto& jo = jv.as_object();
	auto array = jo.at("data_list").as_array();
	tool::model::Websites websites;
	for (auto item : array) {
		tool::model::Website w;
		w.link_base_url = item.at("link_base_url").as_string().c_str();
		w.link_paging_tag = item.at("link_paging_tag").as_string().c_str();
		w.links_xpath = item.at("links_xpath").as_string().c_str();
		w.content_title_xpath = item.at("content_title_xpath").as_string().c_str();
		w.content_cover_xpath = item.at("content_cover_xpath").as_string().c_str();
		w.content_price_xpath = item.at("content_price_xpath").as_string().c_str();
		w.content_views_xpath = item.at("content_views_xpath").as_string().c_str();
		w.content_update_time_xpath = item.at("content_update_time_xpath").as_string().c_str();
		w.content_xpath = item.at("content_xpath").as_string().c_str();
		websites.data_list.push_back(w);
	}
	return websites;
}


/**
* 编号 原文 译文
**/
bool tool::model::ExcelModel::saveTranslation(std::string src, std::string dest) {
	xlnt::workbook book;
	std::string excelPath = tool::Common::GBK_2_UTF8(TRANSLATION_EXCEL_PATH);
	std::ifstream in(WEATHER_EXCEL_PATH);
	if (in.good()) {
		book.load(excelPath);
	}
	in.close();
	xlnt::worksheet ws = book.active_sheet();

	int next = ws.next_row();
	size_t rows = ws.rows().length();
	if (rows == next) {
		ws.cell("A1").value(tool::Common::GBK_2_UTF8("编号"));
		ws.cell("B1").value(tool::Common::GBK_2_UTF8("原文"));
		ws.cell("C1").value(tool::Common::GBK_2_UTF8("译文"));
		ws.cell("A2").value(1);
		ws.cell("B2").value(tool::Common::GBK_2_UTF8(src));
		ws.cell("C2").value(tool::Common::GBK_2_UTF8(dest));
	}
	else {
		ws.cell(xlnt::column_t(1), xlnt::row_t(next)).value(next - 1);
		ws.cell(xlnt::column_t(2), xlnt::row_t(next)).value(tool::Common::GBK_2_UTF8(src));
		ws.cell(xlnt::column_t(3), xlnt::row_t(next)).value(tool::Common::GBK_2_UTF8(dest));
	}

	book.save(excelPath);
	return true;
}

std::string tool::model::ExcelModel::getTranslation(std::string src) {
	xlnt::workbook book;
	std::string excelPath = tool::Common::GBK_2_UTF8(TRANSLATION_EXCEL_PATH);
	std::ifstream in(TRANSLATION_EXCEL_PATH);
	if (in.good()) {
		in.close();
		book.load(excelPath);
	}
	else {
		in.close();
		return "";
	}
	xlnt::worksheet ws = book.active_sheet();
	for (xlnt::worksheet::iterator i = ws.begin(); i != ws.end(); i++) {
		xlnt::cell_vector v = *i;
		if (v[1].value<std::string>() == tool::Common::GBK_2_UTF8(src))
			return v[2].value<std::string>();
	}
	return "";
}

/**
* 编号 城市名 天气信息
**/
bool tool::model::ExcelModel::saveWeather(std::string cityName, std::string cityWeather) {
	xlnt::workbook book;
	std::string excelPath = tool::Common::GBK_2_UTF8(WEATHER_EXCEL_PATH);
	std::ifstream in(WEATHER_EXCEL_PATH);
	if (in.good()) {
		book.load(excelPath);
	}
	in.close();
	xlnt::worksheet ws = book.active_sheet();

	int next = ws.next_row();
	size_t rows = ws.rows().length();
	if (rows == next) {
		ws.cell("A1").value(tool::Common::GBK_2_UTF8("编号"));
		ws.cell("B1").value(tool::Common::GBK_2_UTF8("城市"));
		ws.cell("C1").value(tool::Common::GBK_2_UTF8("天气信息"));
		ws.cell("A2").value(1);
		ws.cell("B2").value(tool::Common::GBK_2_UTF8(cityName));
		ws.cell("C2").value(tool::Common::GBK_2_UTF8(cityWeather));
	}
	else {
		for (xlnt::worksheet::iterator i = ws.begin(); i != ws.end(); i++) {
			xlnt::cell_vector v = *i;
			if (v[1].value<std::string>() == tool::Common::GBK_2_UTF8(cityName) &&
				v[2].value<std::string>() == tool::Common::GBK_2_UTF8(cityWeather))
				return false;
		}
		ws.cell(xlnt::column_t(1), xlnt::row_t(next)).value(next - 1);
		ws.cell(xlnt::column_t(2), xlnt::row_t(next)).value(tool::Common::GBK_2_UTF8(cityName));
		ws.cell(xlnt::column_t(3), xlnt::row_t(next)).value(tool::Common::GBK_2_UTF8(cityWeather));
	}

	book.save(excelPath);
	return true;
}


tool::model::SpiderModel::SpiderModel() {
	contentModel = new tool::model::ContentModel();
}

tool::model::SpiderModel::~SpiderModel() {
	if (contentModel != NULL) {
		delete contentModel;
		contentModel = NULL;
	}
}

bool tool::model::SpiderModel::print() {
	std::vector<Content> contents = contentModel->query(10);
	for (Content c : contents) {
		std::cout << c << std::endl;
		std::cout << std::endl;
	}
	return true;
}

bool tool::model::SpiderModel::clear() {
	contentModel->removeAll();
	return true;
}

tool::model::Websites tool::model::SpiderModel::fromLocalConfig() {
	std::string json = tool::Common::readText(WEBSITES_JSON_PATH);
	std::stringstream ss(json);
	auto value = boost::json::parse(ss);
	return boost::json::value_to<tool::model::Websites>(value);
}

std::vector<std::string> tool::model::SpiderModel::crawlLinks(tool::model::Website& website) {
	std::vector<std::string> links;
	int i = 1;
	while (true) {
		int tryNum = 10;
		while (tryNum--) {
			try {
				std::string url = website.link_base_url;
				if (!website.link_paging_tag.empty()) {
					size_t s = url.find(website.link_paging_tag);
					url = url.replace(s, website.link_paging_tag.size(), std::to_string(i));
				}
				std::cout << "- 配置第[" << i << "]页：" << url << std::endl;
				std::string html = tool::Common::get(url);
				if (html == "") return links;
				html = tool::Common::html2XML(html.c_str());
				pugi::xml_document doc;
				pugi::xml_parse_result r = doc.load_string(html.c_str());
				if (!r) {
					std::cout << "read html failed!" << std::endl;
					std::cout << r.description() << std::endl;
					break;
				}
				pugi::xpath_node_set nodes = doc.select_nodes(website.links_xpath.c_str());
				if (nodes.empty()) break;
				for (auto node : nodes) {
					std::string href = node.node().attribute("href").value();
					links.push_back(href);
					std::cout << "搜集链接：" << href << std::endl;
				}
				break;
			}
			catch (const std::exception& e) {
				std::cout << tool::Common::UTF8_2_GBK(e.what()) << std::endl;
			}
		}
		if (website.link_paging_tag.empty()) return links;  // 无分页
		i++;
	}
	return links;
}

tool::model::Content tool::model::SpiderModel::extractContent(std::string url, Website& website) {
	std::string html = tool::Common::get(url);
	Content content;
	content.url = "";
	if (html == "") return content;
	html = tool::Common::html2XML(html.c_str());

	pugi::xml_document doc;
	pugi::xml_parse_result r = doc.load_string(html.c_str());
	if (!r) {
		std::cout << "read html failed!" << std::endl;
		std::cout << r.description() << std::endl;
		return content;
	}
	pugi::xpath_node_set ps = doc.select_nodes(website.content_xpath.c_str());
	if (ps.empty()) return content;
	std::stringstream ss;
	for (auto p : ps) {
		p.node().print(ss);
	}
	std::string tempContent = ss.str();
	tempContent = tool::Common::xml2HTML(tempContent.c_str());
	content.content = tempContent.substr(tempContent.find("<body>") + 6, tempContent.find("</body>"));
	tool::Common::trim(content.content);

	content.id = 0;
	content.title = doc.select_node(website.content_title_xpath.c_str()).node().child_value();
	content.cover = doc.select_node(website.content_cover_xpath.c_str()).node().attribute("src").value();
	std::stringstream(doc.select_node(website.content_price_xpath.c_str()).node().child_value()) >> content.price;
	std::stringstream(doc.select_node(website.content_views_xpath.c_str()).node().value()) >> content.views;  // 无数据，JS加载

	std::string timeString = doc.select_node(website.content_update_time_xpath.c_str()).node().value();
	content.update_time = tool::Common::timeformat2Timestamp(timeString.c_str());
	content.url = url;
	content.date = time(NULL);

	return content;
}

bool tool::model::SpiderModel::spider() {
	Websites websites = fromLocalConfig();
	for (auto web : websites.data_list) {
		std::cout << "处理配置：" << web.link_base_url << std::endl;
		std::vector<std::string> links = crawlLinks(web);
		for (std::string link : links) {
			if (contentModel->contains(link)) continue;
			std::cout << std::endl;
			std::cout << "提取内容：" << link << std::endl;
			int tryNum = 10;
			while (tryNum--) {
				try {
					tool::model::Content c = extractContent(link, web);
					if (c.url == "") break;
					contentModel->add(c);
					std::cout << c << std::endl;
					std::cout << std::endl;
					break;
				}
				catch (const std::exception& e) {
					std::cout << tool::Common::UTF8_2_GBK(e.what()) << std::endl;
				}
			}
		}
	}
	std::cout << "爬虫任务完成！" << std::endl;
	return true;
}

std::ostream& tool::model::operator<<(std::ostream& out, Content& content) {
	out << content.url << std::endl;
	out << tool::Common::UTF8_2_GBK(content.title) << std::endl;
	out << content.price << std::endl;
	out << tool::Common::timestamp2Timeformat(content.update_time);
	return out;
}


bool tool::model::ContentModel::add(Content& content) {
	sql::mysql::MySQL_Driver* driver = sql::mysql::get_driver_instance();
	sql::Connection* connection = driver->connect(DB_URL, DB_USERNAME, DB_PASSWORD);
	sql::PreparedStatement* statement = connection->prepareStatement("insert into content values(?,?,?,?,?,?,?,?,?);");
	statement->setInt(1, 0);
	statement->setString(2, content.title);
	statement->setString(3, content.cover);
	statement->setInt(4, content.price);
	statement->setInt(5, content.views);
	statement->setInt64(6, content.update_time);
	statement->setString(7, content.content);
	statement->setString(8, content.url);
	statement->setInt64(9, content.date);

	int r = statement->executeUpdate();
	if (r) std::cout << "添加成功：" << content.url << std::endl;
	else {
		std::cout << "添加失败！" << content.url << std::endl;
		throw std::exception("添加数据失败！");
	}

	delete connection;
	delete statement;
	return true;
}

bool tool::model::ContentModel::removeAll() {
	sql::mysql::MySQL_Driver* driver = sql::mysql::get_driver_instance();
	sql::Connection* connection = driver->connect(DB_URL, DB_USERNAME, DB_PASSWORD);
	sql::Statement* statement = connection->createStatement();
	int r = statement->executeUpdate("delete from content;");
	if (!r) {
		std::cout << "清空表失败！" << std::endl;
		delete statement;
		delete connection;
		system("pause");
		return false;
	}
	std::cout << "清空表成功！" << std::endl;
	delete statement;
	delete connection;
	return true;
}

std::vector<tool::model::Content> tool::model::ContentModel::query(int number) {
	sql::mysql::MySQL_Driver* driver = sql::mysql::get_driver_instance();
	sql::Connection* connection = driver->connect(DB_URL, DB_USERNAME, DB_PASSWORD);
	sql::Statement* statement = connection->createStatement();
	std::string sql = "select * from content limit {number};";
	sql.replace(sql.find("{number}"), sql.size(), std::to_string(number));
	sql::ResultSet* set = statement->executeQuery(sql);

	std::vector<tool::model::Content> contents;
	if (!set) {
		std::cout << "查询失败！" << std::endl;
		delete statement;
		delete connection;
		system("pause");
		return contents;
	}
	while (set->next()) {
		tool::model::Content content;
		content.id = set->getInt("id");
		content.title = set->getString("title");
		content.cover = set->getString("cover");
		content.price = set->getInt("price");
		content.views = set->getInt("views");
		content.update_time = set->getInt64("update_time");
		content.content = set->getString("content");
		content.url = set->getString("url");
		content.date = set->getInt64("date");
		contents.push_back(content);
	}

	delete set;
	delete statement;
	delete connection;
	return contents;
}

bool tool::model::ContentModel::contains(std::string url) {
	sql::mysql::MySQL_Driver* driver = sql::mysql::get_driver_instance();
	sql::Connection* connection = driver->connect(DB_URL, DB_USERNAME, DB_PASSWORD);
	sql::Statement* statement = connection->createStatement();
	std::string sql = "select id from content where url='?';";
	sql = sql.replace(sql.find("?"), 1, url);
	sql::ResultSet* set = statement->executeQuery(sql);

	if (!set) {
		std::cout << "查询失败！" << std::endl;
		delete statement;
		delete connection;
		return false;
	}

	bool r = set->next();
	delete set;
	delete statement;
	delete connection;
	return r;
}