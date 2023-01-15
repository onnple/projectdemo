#include "app.h"
#include <iostream>
#include <exception>
#include <sstream>
#include "common.h"

// class Activity
tool::app::Activity::Activity() {
	this->app = tool::app::App::getApp();
}

tool::app::Activity::~Activity() {

}

void tool::app::Activity::startActivity(tool::app::Activity* src, tool::app::Activity* dest) {
	App* app = App::getApp();
	if (src != app->getActivityStack().top())
		throw std::exception("invalid source activity");
	app->getActivityStack().push(dest);
}

int tool::app::Activity::getSelect() {
	std::string select;
	std::getline(std::cin, select);
	std::stringstream ss;
	ss << select;
	int n = 0;
	ss >> n;
	return n;
}


// class MainActivity
tool::app::MainActivity::MainActivity() : Activity() {
	translationModel = new tool::model::TranslationModel();
	weatherModel = new tool::model::WeatherModel();
	spiderModel = new tool::model::SpiderModel();
	excelModel = new tool::model::ExcelModel();
}

tool::app::MainActivity::~MainActivity() {
	if (translationModel != NULL) {
		delete translationModel;
		translationModel = NULL;
	}
	if (weatherModel != NULL) {
		delete weatherModel;
		weatherModel = NULL;
	}
	if (spiderModel != NULL) {
		delete spiderModel;
		spiderModel = NULL;
	}
	if (excelModel != NULL) {
		delete excelModel;
		excelModel = NULL;
	}
}

int tool::app::MainActivity::init() {
	system("cls");
	std::cout << " - 主页面 - " << std::endl;
	std::cout << "1. 翻译" << std::endl;
	std::cout << "2. 天气" << std::endl;
	std::cout << "3. 爬虫" << std::endl;
	std::cout << "4. 退出" << std::endl;
	std::cout << "输入您的选择：" << std::endl;
	int n = getSelect();
	system("cls");
	return n;
}

tool::app::Response tool::app::MainActivity::waitResponse() {
	try {
		int select = init();
		switch (select) {
		case 1:
			startActivity(this, new TranslationActivity(translationModel, excelModel));
			return BREED;
			break;
		case 2:
			startActivity(this, new WeatherActivity(weatherModel, excelModel));
			return BREED;
			break;
		case 3:
			startActivity(this, new SpiderActivity(spiderModel));
			return BREED;
			break;
		case 4:
			return EXIT;
			break;
		default:
			break;
		}
	}
	catch (const std::exception& e) {
		std::cerr << "发生异常：" << std::endl;
		std::cerr << tool::Common::utf8ToGB(e.what()) << std::endl;
		system("pause");
		return RUNNING;
	}
	return RUNNING;
}


// class TranslationActivity
tool::app::TranslationActivity::TranslationActivity(tool::model::TranslationModel* translationModel, tool::model::ExcelModel* excelModel) : Activity() {
	this->translationModel = translationModel;
	this->excelModel = excelModel;
}

tool::app::TranslationActivity::~TranslationActivity() {

}

int tool::app::TranslationActivity::init() {
	system("cls");
	std::cout << "- 快速翻译 - " << std::endl;
	std::cout << "输入翻译类型（0退出，1中英，2英中，其它数字中英自动检测翻译）：" << std::endl;
	int type = getSelect();
	if (type == 0) return -1;
	if (type < 1 || type > 2) type = 3;
	std::cout << "输入文本进行翻译（输入exit退出）：" << std::endl;
	std::string src;
	std::getline(std::cin, src);
	if (src == "exit") return -1;
	std::string dest = excelModel->getTranslation(src);
	if (dest == "") {
		dest = translationModel->translate(src, type);
		if (dest != "") excelModel->saveTranslation(src, dest);
		std::cout << "翻译结果：" << std::endl;
		std::cout << dest << std::endl;
	}
	else {
		std::cout << "翻译结果：" << std::endl;
		std::cout << tool::Common::UTF8_2_GBK(dest) << std::endl;
	}
	system("pause");
	system("cls");
	return 0;
}

tool::app::Response tool::app::TranslationActivity::waitResponse() {
	try {
		int select = init();
		switch (select) {
		case -1:
			return EXIT;
			break;
		case 0:
			break;
		default:
			break;
		}
	}
	catch (const std::exception& e) {
		std::cerr << "发生异常：" << std::endl;
		std::cerr << tool::Common::utf8ToGB(e.what()) << std::endl;
		system("pause");
		return RUNNING;
	}
	return RUNNING;
}


// class WeatherActivity
tool::app::WeatherActivity::WeatherActivity(tool::model::WeatherModel* weatherModel, tool::model::ExcelModel* excelModel) : Activity() {
	this->weatherModel = weatherModel;
	this->excelModel = excelModel;
}

tool::app::WeatherActivity::~WeatherActivity() {

}

int tool::app::WeatherActivity::init() {
	system("cls");
	std::cout << "- 查看天气预报 - " << std::endl;
	std::cout << "输入城市名称（输入exit退出）：" << std::endl;
	std::string cityName;
	std::getline(std::cin, cityName);
	if (cityName == "exit") return -1;
	std::string cityWeather = weatherModel->search(cityName);
	if (cityWeather != "") excelModel->saveWeather(cityName, cityWeather);
	std::cout << "天气查询结果[" << cityName << "]：" << std::endl;
	std::cout << cityWeather << std::endl;
	system("pause");
	system("cls");
	return 0;
}

tool::app::Response tool::app::WeatherActivity::waitResponse() {
	try {
		int select = init();
		switch (select) {
		case -1:
			return EXIT;
			break;
		case 0:
			break;
		default:
			break;
		}
	}
	catch (const std::exception& e) {
		std::cerr << "发生异常：" << std::endl;
		std::cerr << tool::Common::utf8ToGB(e.what()) << std::endl;
		system("pause");
		return RUNNING;
	}
	return RUNNING;
}


// class SpiderActivity
tool::app::SpiderActivity::SpiderActivity(tool::model::SpiderModel* spiderModel) : Activity() {
	this->spiderModel = spiderModel;
}

tool::app::SpiderActivity::~SpiderActivity() {

}

int tool::app::SpiderActivity::init() {
	system("cls");
	std::cout << " - 网站爬虫 - " << std::endl;
	std::cout << "1. 查看数据（前10条）" << std::endl;
	std::cout << "2. 清空数据库" << std::endl;
	std::cout << "3. 开始爬虫" << std::endl;
	std::cout << "4. 退出" << std::endl;
	std::cout << "输入您的选择：" << std::endl;
	int select = getSelect();
	switch (select) {
	case 1:
		spiderModel->print();
		break;
	case 2:
		spiderModel->clear();
		break;
	case 3:
		spiderModel->spider();
		break;
	case 4:
		return -1;
		break;
	default:
		break;
	}
	system("pause");
	system("cls");
	return select;
}

tool::app::Response tool::app::SpiderActivity::waitResponse() {
	try {
		int select = init();
		switch (select) {
		case -1:
			return EXIT;
			break;
		case 1:
		case 2:
		case 3:
			break;
		default:
			break;
		}
	}
	catch (const std::exception& e) {
		std::cerr << "发生异常：" << std::endl;
		std::cerr << tool::Common::utf8ToGB(e.what()) << std::endl;
		system("pause");
		return RUNNING;
	}
	return RUNNING;
}


// class App
tool::app::App* tool::app::App::app = NULL;
std::stack<tool::app::Activity*>* tool::app::App::activityStack = NULL;

tool::app::App::App() {

}

tool::app::App::~App() {
	if (app == NULL) {
		delete app;
		app = NULL;
	}
	if (activityStack == NULL) {
		delete activityStack;
		activityStack = NULL;
	}
}

tool::app::App* tool::app::App::getApp() {
	if (app == NULL) {
		app = new App();
		activityStack = new std::stack<Activity*>;
		activityStack->push(new MainActivity());
	}
	return app;
}

std::stack<tool::app::Activity*>& tool::app::App::getActivityStack() {
	return *activityStack;
}

void tool::app::App::run() {
	try {
		while (!activityStack->empty()) {
			Activity* act = activityStack->top();
			Response r = RUNNING;
			while ((r = act->waitResponse()) == RUNNING) {}
			if (r == EXIT) {
				delete act;
				activityStack->pop();
			}
		}
	}
	catch (const std::exception& e) {
		std::cout << tool::Common::UTF8_2_GBK(e.what()) << std::endl;
	}
}