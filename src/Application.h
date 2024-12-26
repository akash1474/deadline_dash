#pragma once
#include "GLFW/glfw3.h"
#include "chrono"
#include <chrono>

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
		Edit,
		Settings,
		MiniMode
	};

	struct Config{
		int uiFontSize;
		int tiFontSize;
		int icFontSize;
		std::string uiFontPath;
		std::string tiFontPath;
		int timerFormat;
		std::time_t deadline;
		bool floatingInNormalMode;
		bool floatingInFocusMode;
		std::string headerText;
		std::string footerText;
	};

private:
	std::time_t mCurrentTime;
	tm mDeadline;
	bool mIsCountdownRunning = false;

	bool mIsMiniMode=false;
	bool mIsSwitchingModes=false;

	std::chrono::time_point<std::chrono::system_clock> mCurrentDeadlineTime;

	Page mPage;
	GLFWwindow* mWindow{0};

	Config mConfig;
	std::string mConfigPath;
	std::string mConfigFolder;

	bool mUpdateFont;

public:
	Application(const Application&) = delete;
	~Application();

	static Application& Get()
	{
		static Application instance;
		return instance;
	}

	static void Init(GLFWwindow* glfwWindow);

	static void Render();
	static void RenderEdit();
	static void RenderSettings();
	static void RenderHomePage();


	static void LoadConfig();
	static void SaveConfig();

	static void SwitchDisplayModeIfNeeded(int width,int height);
	static void UpdateFontsIfNeeded();
	static void ResetFonts();
	static void LoadFonts();

	static void LoadDefaultUIFont();
	static void LoadDefaultTimerFont();

	static void LoadUserUIFont();
	static void LoadUserTimerFont();

	static void HandleFontFile(std::string& aConfigFontPath);

	static std::string FormatCountdown(std::chrono::seconds remainingSeconds);
	static std::chrono::seconds CalculateRemainingTimeInSeconds();

private:
	Application() {};
};