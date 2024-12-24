#include "pch.h"
#include "FontAwesome6.h"
#include "imgui.h"
#include "ImGuiDatePicker.hpp"
#include "Application.h"

Application::~Application(){};

void Application::Init(){
	Application& app=Application::Get();
	std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
	app.mCurrentTime = std::chrono::system_clock::to_time_t(now);
	app.mDeadline = *std::gmtime(&app.mCurrentTime);
	app.mIsCountdownRunning = false;
	app.mConfigPath = "deadline.dat";
	app.mPage=Page::HomePage;
	LoadConfig();
}


void Application::Render()
{
	switch(Get().mPage){
	case Application::Page::HomePage:
		RenderHomePage();
		break;
	case Application::Page::Edit:
		RenderEdit();
		break;
	case Application::Page::Settings:
		RenderSettings();
		break;
	default:
		RenderHomePage();
	}
}


void Application::RenderSettings()
{

}


void Application::RenderHomePage()
{
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
    ImGui::SetNextWindowPos({0,0});
    ImGui::Begin("##application",0, ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDecoration |  ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
    // Display the countdown timer if it's running
    if (Get().mIsCountdownRunning) {
        auto remainingSeconds = CalculateRemainingTimeInSeconds();

            std::string formattedCountdown = FormatCountdown(remainingSeconds);
            ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);
            const float textWidth=ImGui::CalcTextSize(formattedCountdown.c_str()).x;
            ImGui::SetCursorPosX((ImGui::GetWindowWidth()-textWidth)*0.5f);
            ImGui::Text("%s", formattedCountdown.c_str());
            ImGui::PopFont();
    }


    ImGui::SetCursorPosX((ImGui::GetWindowWidth()-ImGui::CalcTextSize("Remaining...").x)*0.5f);
    ImGui::TextColored(ImColor(100, 100, 100, 255),"Remaining...");


    const float center=ImGui::GetWindowWidth()*0.5f;
    const float spacingBetween=10.0f;
    const float buttonWidth=110.0f;

    ImGui::SetCursorPosX(center-(buttonWidth+spacingBetween));
    ImGui::PushStyleColor(ImGuiCol_Button,Get().mIsCountdownRunning ? IM_COL32(202,5,64,255) : IM_COL32(24, 98, 218, 255) );
    if (ImGui::Button(Get().mIsCountdownRunning ? "Clear" : "Start",{buttonWidth,25.0f})) {}
    if(ImGui::IsItemHovered()) ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
    ImGui::PopStyleColor();


	ImGui::SameLine();


    ImGui::SetCursorPosX(center+spacingBetween);
    if (ImGui::Button(ICON_FA_PENCIL" Edit",{buttonWidth,25.0f})) Get().mPage=Application::Page::Edit;
    if(ImGui::IsItemHovered()) ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
    ImGui::End();

}


void Application::RenderEdit() 
{
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
    ImGui::SetNextWindowPos({0,0});
    ImGui::Begin("Configure Deadline",0, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

    ImGui::Text("Select Date:");
    ImGui::PushItemWidth(160.0f);
    ImGui::DatePicker("##datepicker", Get().mDeadline,false,85.0f);

    static int hour = Get().mDeadline.tm_hour, minute = Get().mDeadline.tm_min;
    ImGui::Text("Select Time (24Hrs Format):");
    ImGui::PushItemWidth(100.0f);
    if(ImGui::InputInt("##Hour", &hour))
    {
        if(hour>23) hour=23;
        if(hour<0) hour=0;
    }
    ImGui::SameLine();
    ImGui::Text(":");ImGui::SameLine();
    if(ImGui::InputInt("##Minute", &minute,1,1))
    {
        if(minute>59) minute=59;
        if(minute<0) minute=0;
    }



    ImGui::Dummy({2.0f,5.0f});
    ImGui::Separator();
    ImGui::Dummy({2.0f,5.0f});

    ImGui::Text("Display Format:");
    ImGui::PushItemWidth(180.0f);
    static const char* formatOptions[] = {"Hours","Hours (float)", "HH:MM:SS", "Days, Hrs, Mins, Secs"};
    if(ImGui::Combo("##displayformat", &Get().mCurrentDisplayFormat, formatOptions, IM_ARRAYSIZE(formatOptions))){
        SaveConfig();
    }
    ImGui::PopItemWidth();


    ImGui::Dummy({2.0f,5.0f});
    ImGui::Separator();
    ImGui::Dummy({2.0f,5.0f});

    ImGui::SetCursorPosX((ImGui::GetWindowWidth()-150.0f)*0.5f);
    ImGui::PushStyleColor(ImGuiCol_Button,IM_COL32(105, 42, 241, 255));
    if (ImGui::Button("Start Countdown",{150.0f,30.0f})) {
        if (hour >= 0 && hour <= 23 && minute >= 0 && minute <= 59) {
            Get().mDeadline.tm_hour = hour;
            Get().mDeadline.tm_min = minute;

            std::time_t deadline_time = std::mktime(&Get().mDeadline);
            if (deadline_time != -1) {
                Get().mCurrentDeadlineTime = std::chrono::system_clock::from_time_t(deadline_time);
                Get().mIsCountdownRunning = true;
                SaveConfig();
                Get().mPage=Application::Page::HomePage;
            } else {
                ImGui::Text("Invalid date/time! Please check your input.");
            }
        } else {
            ImGui::Text("Invalid time! Please check your input.");
        }
    }
    ImGui::PopStyleColor();
    if(ImGui::IsItemHovered()) ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);



    ImGui::End();
}



void Application::SaveConfig() {
    GL_INFO("SaveConfig");
    auto deadlineTimeT = std::chrono::system_clock::to_time_t(Get().mCurrentDeadlineTime);

    std::ofstream outFile(Get().mConfigPath, std::ios::binary);
    if (outFile.is_open()) {
        outFile.write(reinterpret_cast<const char*>(&deadlineTimeT), sizeof(deadlineTimeT));
        outFile.write(reinterpret_cast<const char*>(&Get().mCurrentDisplayFormat),sizeof(Get().mCurrentDisplayFormat));
        outFile.close();
    } else {
        std::cerr << "Error: Could not open file to save deadline." << std::endl;
    }
}




void Application::LoadConfig() 
{
    std::ifstream inFile(Get().mConfigPath, std::ios::binary);
    if (inFile.is_open()) {
        std::time_t deadlineTimeT;
        inFile.read(reinterpret_cast<char*>(&deadlineTimeT), sizeof(deadlineTimeT));
        inFile.read(reinterpret_cast<char*>(&Get().mCurrentDisplayFormat), sizeof(mCurrentDisplayFormat));
        inFile.close();

        if (deadlineTimeT > 0) {
            Get().mCurrentDeadlineTime = std::chrono::system_clock::from_time_t(deadlineTimeT);
            Get().mDeadline=*std::localtime(&deadlineTimeT);
            Get().mIsCountdownRunning = true;
        }
    }
}




std::string Application::FormatCountdown(std::chrono::seconds remainingSeconds) 
{
    switch (Get().mCurrentDisplayFormat) {
        case (int)FormatOption::HOURS_ONLY: {
            float hours = std::chrono::duration<float, std::chrono::hours::period>(remainingSeconds).count();
            std::string type="Hours";
            if(hours<=1){
                char buffer[20];
                int minutes = (remainingSeconds.count() % 3600) / 60;
                int seconds = remainingSeconds.count() % 60;
                snprintf(buffer, sizeof(buffer), "%02d Mins %02d Secs", minutes,seconds);
                return std::string(buffer);
            }
            return std::to_string((int)std::floor(hours)) + "Hours";
        }
        case (int)FormatOption::HOURS_FLOAT: {
            float hours = std::chrono::duration<float, std::chrono::hours::period>(remainingSeconds).count();
            char buffer[20];
            snprintf(buffer, sizeof(buffer), "%02f %s", hours,hours>1 ? "Hours": "Hour");
            return std::string(buffer);
        }
        case (int)FormatOption::HH_MM_SS: {
            int totalSeconds = remainingSeconds.count();
            int hours = totalSeconds / 3600;
            int minutes = (totalSeconds % 3600) / 60;
            int seconds = totalSeconds % 60;
            char buffer[20];
            snprintf(buffer, sizeof(buffer), "%02d:%02d:%02d", hours, minutes, seconds);
            return std::string(buffer);
        }
        case (int)FormatOption::DAYS_HOURS_MINUTES_SECONDS: {
            int totalSeconds = remainingSeconds.count();
            int days = totalSeconds / (3600 * 24);
            int hours = (totalSeconds % (3600 * 24)) / 3600;
            int minutes = (totalSeconds % 3600) / 60;
            int seconds = totalSeconds % 60;
            char buffer[64];
            snprintf(buffer, sizeof(buffer), "%d Days, %02d Hrs, %02d Mins, %02d Secs", days, hours, minutes, seconds);
            return std::string(buffer);
        }
        default:
            return "Invalid format";
    }
}



std::chrono::seconds Application::CalculateRemainingTimeInSeconds() 
{
    auto now = std::chrono::system_clock::now();
    return std::chrono::duration_cast<std::chrono::seconds>(Get().mCurrentDeadlineTime - now);
}