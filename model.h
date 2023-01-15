#pragma once
#include <boost/json.hpp>
#include <string>
#include <vector>

namespace tool {
	namespace model {

		struct Config {
			std::string link_index_url;
			std::string link_paging_url;
			std::string link_xpath_selector;
			std::string post_xpath_selector;
		};

		/**
	* 有道翻译：https://v.api.aa1.cn/api/api-fanyi-yd/index.php?msg=我爱你&type=1
	* GET请求：
	* msg	string	翻译内容
	* type	string	请输入翻译类型（1代表中-英，2代表英-中，3或其他数字代表中<=>英【自动检测翻译】）
	* 响应：
	*
	{
		"type": "中英互译",
		 "desc": "中文翻译英文",
		 "text": "I love you"
	}
	**/
		class TranslationModel {

		public:
			struct Response {
				std::string type;
				std::string desc;
				std::string text;
				Response(std::string type, std::string desc, std::string text) : type(type), desc(desc), text(text) {

				}

			};
			std::string translate(std::string text, int type);
		};

		class WeatherModel {

		public:
			struct WDay {
				std::string riqi;
				std::string wendu;
				std::string tianqi;
				std::string fengdu;
				std::string pm;
			};
			struct Response {
				std::string code;
				std::vector<WDay> data;
			};
			std::string search(std::string cityName);

		};

		class ExcelModel {

		public:
			bool saveTranslation(std::string src, std::string dest);
			std::string getTranslation(std::string src);
			bool saveWeather(std::string cityName, std::string cityWeather);

		};

		/**
					"link_base_url": "https://www.wusuo8.com/list-28-{paging}.html",
					"link_paging_tag": "{paging}",
					"links_xpath": "/html/body/div[1]/div[3]/ul/li/a",
					"content_title_xpath": "/html/body/div[1]/div[3]/div[2]/h1",
					"content_cover_xpath": "/html/body/div[1]/div[3]/div[1]/img",
					"content_price_xpath": "/html/body/div[1]/div[3]/div[2]/ul/li[1]/b[1]/span",
					"content_views_xpath": "/html/body/div[1]/div[3]/div[2]/ul/li[2]/span",
					"content_update_time_xpath": "/html/body/div[1]/div[3]/div[2]/ul/li[3]/text()",
					"content_xpath": "/html/body/div[1]/div[3]/article/p"
				**/
		struct Website {
			std::string link_base_url;
			std::string link_paging_tag;
			std::string links_xpath;
			std::string content_title_xpath;
			std::string content_cover_xpath;
			std::string content_price_xpath;
			std::string content_views_xpath;
			std::string content_update_time_xpath;
			std::string content_xpath;
		};

		struct Websites {
			std::vector<Website> data_list;
		};

		struct Content {
			int id;
			std::string title;
			std::string cover;
			int price;
			int views;
			size_t update_time;
			std::string content;
			std::string url;
			size_t date;
		};

		class ContentModel;
		class SpiderModel {

		private:
			ContentModel* contentModel;

		private:
			Websites fromLocalConfig();
			std::vector<std::string> crawlLinks(Website& website);
			Content extractContent(std::string url, Website& website);

		public:
			SpiderModel();
			~SpiderModel();
			bool print();
			bool clear();
			bool spider();

		};

		class ContentModel {

		public:
			bool add(Content& content);
			bool removeAll();
			std::vector<Content> query(int number);
			bool contains(std::string url);

		};

		// 1. JSON序列化对象
		void tag_invoke(boost::json::value_from_tag, boost::json::value& jv, TranslationModel::Response const& c);
		// 2. JSON反序列化对象
		TranslationModel::Response tag_invoke(boost::json::value_to_tag<TranslationModel::Response>, boost::json::value const& jv);

		void tag_invoke(boost::json::value_from_tag, boost::json::value& jv, WeatherModel::Response const& c);
		WeatherModel::Response tag_invoke(boost::json::value_to_tag<WeatherModel::Response>, boost::json::value const& jv);

		void tag_invoke(boost::json::value_from_tag, boost::json::value& jv, Websites const& w);
		Websites tag_invoke(boost::json::value_to_tag<Websites>, boost::json::value const& jv);

		std::ostream& operator<<(std::ostream& out, Content& content);

	}
}