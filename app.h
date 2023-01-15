#pragma once
#include <stack>
#include "model.h"

namespace tool {

	namespace app {

		enum Response {
			RUNNING, EXIT, BREED
		};

		/**
		* Main Translation Weather Spider
		**/
		class App;
		class Activity {

		protected:
			App* app;
			virtual int init() = 0;

		public:
			Activity();
			virtual ~Activity() = 0;
			virtual Response waitResponse() = 0;
			void startActivity(Activity* src, Activity* dest);
			int getSelect();
		};

		class MainActivity : public Activity {

		private:
			tool::model::TranslationModel* translationModel;
			tool::model::WeatherModel* weatherModel;
			tool::model::SpiderModel* spiderModel;
			tool::model::ExcelModel* excelModel;

		private:
			int init();

		public:
			MainActivity();
			~MainActivity();
			Response waitResponse();

		};

		class TranslationActivity : public Activity {

		private:
			tool::model::TranslationModel* translationModel;
			tool::model::ExcelModel* excelModel;

		private:
			int init();

		public:
			TranslationActivity(tool::model::TranslationModel* translationModel, tool::model::ExcelModel* excelModel);
			~TranslationActivity();
			Response waitResponse();

		};

		class WeatherActivity : public Activity {

		private:
			tool::model::WeatherModel* weatherModel;
			tool::model::ExcelModel* excelModel;

		private:
			int init();

		public:
			WeatherActivity(tool::model::WeatherModel* weatherModel, tool::model::ExcelModel* excelModel);
			~WeatherActivity();
			Response waitResponse();

		};

		class SpiderActivity : public Activity {

		private:
			tool::model::SpiderModel* spiderModel;

		private:
			int init();

		public:
			SpiderActivity(tool::model::SpiderModel* spiderModel);
			~SpiderActivity();
			Response waitResponse();

		};

		class App {

		private:
			static App* app;
			static std::stack<Activity*>* activityStack;  // 顶层activity为当前活动的activity

		private:
			App();

		public:
			~App();
			static App* getApp();
			static std::stack<Activity*>& getActivityStack();
			void run();

		};

	}

}