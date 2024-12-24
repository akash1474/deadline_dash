#pragma once
#include "chrono"

class Application
{
	enum class FormatOption {
	    HOURS_ONLY,
	    HOURS_FLOAT,
	    HH_MM_SS,
	    DAYS_HOURS_MINUTES_SECONDS
	};

	enum class Page{
		HomePage,
		Settings,
		Edit
	};

private:
	std::time_t mCurrentTime;
	tm mDeadline;
	bool mIsCountdownRunning = false;
	std::chrono::time_point<std::chrono::system_clock> mCurrentDeadlineTime;

	int mCurrentDisplayFormat=(int)FormatOption::HOURS_FLOAT;
	std::string mConfigPath;

	Page mPage;


public:
	Application(const Application&) = delete;
	~Application();

	static Application& Get()
	{
		static Application instance;
		return instance;
	}

	static void Init();

	static void Render();
	static void RenderEdit();
	static void RenderSettings();
	static void RenderHomePage();


	static void LoadConfig();
	static void SaveConfig();

	static std::string FormatCountdown(std::chrono::seconds remainingSeconds);
	static std::chrono::seconds CalculateRemainingTimeInSeconds();

private:
	Application() {};
};