#include "GLFW/glfw3.h"
#include "Timer.h"
#include "pch.h"
#include "FontAwesome6.h"
#include "imgui.h"
#include "ImGuiDatePicker.hpp"
#include "Application.h"
#include "./resources/ProtestStrike.embed"
#include "./resources/FontAwesomeSolid.embed"
#include "./resources/RecursiveLinearMedium.embed"

#include "Utils.h"
#include <filesystem>
#include <shellapi.h>

Application::~Application(){};

void Application::Init(GLFWwindow* win){
	Application& app=Application::Get();
	std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
    app.mWindow=win;
	app.mCurrentTime = std::chrono::system_clock::to_time_t(now);
	app.mDeadline = *std::localtime(&app.mCurrentTime);
	app.mIsCountdownRunning = false;
	app.mConfigPath = "deadline.dat";
    app.mConfigFolder = GetUserDirectory("deadline_dash")+"\\";
    app.mConfigPath=app.mConfigFolder+Get().mConfigPath;
	app.mPage=Page::HomePage;

    app.mConfig.uiFontSize=16;
    app.mConfig.tiFontSize=32;
    app.mConfig.icFontSize=20;
    app.mConfig.floatingInFocusMode=true;
    app.mConfig.floatingInNormalMode=false;
    app.mConfig.timerFormat=(int)FormatOption::HH_MM_SS;
    app.mConfig.footerText="The sooner you start working on something, the sooner you'll see results";
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


void Application::ResetFonts(){
    Get().mConfig.uiFontSize=16;
    Get().mConfig.tiFontSize=32;
    Get().mConfig.icFontSize=20;
    Get().mConfig.tiFontPath.clear();
    Get().mConfig.uiFontPath.clear();
    Get().SaveConfig();
}

void Application::LoadFonts(){
    Get().mUpdateFont=true;
    UpdateFontsIfNeeded();
}


void Application::LoadDefaultUIFont(){
    ImGuiIO& io=ImGui::GetIO();
    static const ImWchar icons_ranges[] = {ICON_MIN_FA,ICON_MAX_FA,0};
    ImFontConfig icon_config;
    icon_config.MergeMode = true;
    icon_config.PixelSnapH = true;
    icon_config.FontDataOwnedByAtlas=false;

    ImFontConfig font_config;
    font_config.FontDataOwnedByAtlas=false;
    io.Fonts->AddFontFromMemoryTTF((void*)RecursiveLinearMedium, IM_ARRAYSIZE(RecursiveLinearMedium),Get().mConfig.uiFontSize,&font_config);
    io.Fonts->AddFontFromMemoryTTF((void*)FontAwesomeSolid, IM_ARRAYSIZE(FontAwesomeSolid),Get().mConfig.icFontSize*2.0f/3.0f,&icon_config,icons_ranges);
}

void Application::LoadDefaultTimerFont(){
    ImFontConfig font_config;
    font_config.FontDataOwnedByAtlas=false;
    ImGui::GetIO().Fonts->AddFontFromMemoryTTF((void*)ProtestStrike,IM_ARRAYSIZE(ProtestStrike),Get().mConfig.tiFontSize,&font_config);
}


void Application::LoadUserUIFont(){
    ImGuiIO& io=ImGui::GetIO();
    static const ImWchar icons_ranges[] = {ICON_MIN_FA,ICON_MAX_FA,0};
    ImFontConfig icon_config;
    icon_config.MergeMode = true;
    icon_config.PixelSnapH = true;
    icon_config.FontDataOwnedByAtlas=false;

    ImFontConfig font_config;
    font_config.FontDataOwnedByAtlas=false;
    io.Fonts->AddFontFromFileTTF(Get().mConfig.uiFontPath.c_str(),Get().mConfig.uiFontSize,&font_config);
    io.Fonts->AddFontFromMemoryTTF((void*)FontAwesomeSolid, IM_ARRAYSIZE(FontAwesomeSolid),Get().mConfig.icFontSize*2.0f/3.0f,&icon_config,icons_ranges);
}

void Application::LoadUserTimerFont(){
    ImFontConfig font_config;
    font_config.FontDataOwnedByAtlas=false;
    ImGui::GetIO().Fonts->AddFontFromFileTTF(Get().mConfig.tiFontPath.c_str(),Get().mConfig.tiFontSize,&font_config);
}


void Application::UpdateFontsIfNeeded(){
    if(!Get().mUpdateFont) return;

    ImGui::GetIO().Fonts->Clear();
    Config& config=Get().mConfig;
    bool hasUserFontUI=config.uiFontPath.size()>0;
    bool hasUserFontTimer=config.tiFontPath.size()>0;
    
    if(hasUserFontUI) LoadUserUIFont();
    else LoadDefaultUIFont();

    if(hasUserFontTimer) LoadUserTimerFont();
    else LoadDefaultTimerFont();

    ImGui_ImplOpenGL3_CreateFontsTexture();
    Get().mUpdateFont=false;
}

void Application::HandleFontFile(std::string& aConfigFontPath){
    std::string path=SelectFile();
    std::filesystem::path fontPath(path);
    if(std::filesystem::exists(path)){
        aConfigFontPath=Get().mConfigFolder+fontPath.filename().string();
        if(!std::filesystem::exists(aConfigFontPath)) std::filesystem::copy_file(path,aConfigFontPath);
        Get().mUpdateFont=true;
        GL_INFO("from:{}  To:{}",path,aConfigFontPath);
        SaveConfig();
    }

}

void CenteredText(const char* label)
{
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);
    ImGui::SetCursorPos({(window->Size.x-label_size.x)*0.5f,ImGui::GetCursorPos().y-(ImGui::GetStyle().FramePadding.y*1.5f)});
    ImGui::Text("%s", label);
}

int CenterWrapped(const char* text,float max_width)
{
    float textSize = ImGui::CalcTextSize(text).x;
    float char_width = ImGui::CalcTextSize("A").x;
    int max_len=ImGui::GetWindowWidth()>max_width ? int(max_width/char_width) : int(ImGui::GetWindowWidth()/char_width); 
    int nLines=0;

    if (textSize > max_width) {
        std::string s(text);
        float allowedWidth=char_width*max_len;
        while(textSize>allowedWidth){
            size_t x = s.substr(0, max_len).find_last_of(' ');
            if(x==std::string::npos) x=max_len-1;
            nLines++;
            CenteredText(s.substr(0, x).c_str());
            s = s.substr(x + 1, s.size() - 1);
            textSize = ImGui::CalcTextSize(s.c_str()).x;
        }
        CenteredText(s.c_str());
    } else {
        CenteredText(text);
    }
    return nLines;
}


void Application::RenderSettings()
{
    static char buff[512]="";
    static bool shouldInit=true;
    if(shouldInit){
        strcpy(buff,Get().mConfig.footerText.c_str());
        shouldInit=false;
    }

    if(ImGui::IsKeyPressed(ImGuiKey_Escape)) Get().mPage=Application::Page::HomePage;

    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
    ImGui::SetNextWindowPos({0,0});
    ImGui::Begin("Configure Deadline",0, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar);

        if (ImGui::CollapsingHeader("General")){
            ImGui::Text("Display Format:");
            ImGui::PushItemWidth(180.0f);
            static const char* formatOptions[] = {"Hours","Hours (float)", "HH:MM:SS", "Days, Hrs, Mins, Secs"};
            if(ImGui::Combo("##displayformat", &Get().mConfig.timerFormat, formatOptions, IM_ARRAYSIZE(formatOptions))){
                SaveConfig();
            }
            ImGui::PopItemWidth();

            ImGui::Text("Footer Text:"); ImGui::SameLine();
            if(ImGui::InputText("##ftext",buff,IM_ARRAYSIZE(buff),ImGuiInputTextFlags_EnterReturnsTrue)){
                SaveConfig();
                Get().mConfig.footerText=buff;
            }


            ImGui::Text("Enable Always in Focus Mode:");ImGui::SameLine();
            if(ImGui::Checkbox("##floating", &Get().mConfig.floatingInFocusMode)){
                SaveConfig();
                Get().mIsSwitchingModes=true;
            }
            ImGui::Text("Enable Always in Normal Mode:");ImGui::SameLine();
            if(ImGui::Checkbox("##floatingN", &Get().mConfig.floatingInNormalMode)){
                SaveConfig();
                Get().mIsSwitchingModes=true;
            }
        }
        if (ImGui::CollapsingHeader("Font")){
            //UI Font
            ImGui::Text("Choose UI Font:"); ImGui::SameLine(); 
            if(ImGui::Button("Select Font UI")) HandleFontFile(Get().mConfig.uiFontPath);

            ImGui::Text("Font Size:"); 
            ImGui::SameLine();
            if(ImGui::InputInt("##fsui", &Get().mConfig.uiFontSize)){
                SaveConfig();
                Get().mUpdateFont=true;
            }

            ImGui::Text("Choose Timer Font:"); ImGui::SameLine(); 
            if(ImGui::Button("Select Font")) HandleFontFile(Get().mConfig.tiFontPath);

            //Timer Font
            ImGui::Text("Font Size:"); ImGui::SameLine();
            if(ImGui::InputInt("##fscd", &Get().mConfig.tiFontSize)){
                SaveConfig();
                Get().mUpdateFont=true;
            }
            ImGui::PushStyleColor(ImGuiCol_Button,IM_COL32(202,5,64,255));
            if(ImGui::Button("Reset Fonts")){
                ResetFonts();
                Get().mUpdateFont=true;
            }
            ImGui::PopStyleColor();
        }

        if (ImGui::CollapsingHeader("About")){
            CenteredText("Deadline Dash");
            CenteredText("v1.0.0");
            CenteredText("Developed by: Akash Pandit");
            ImGui::Separator();
            ImGui::Spacing();
            CenterWrapped("Deadline Dash is a countdown timer application built using GLFW, OpenGL, and ImGui.It helps you track deadlines and stay motivated by providing a customizable and intuitive user interface.",500.0f);
            ImGui::Spacing();
            ImGui::Text("Credits:");
            ImGui::BulletText("GLFW - Window and input handling");
            ImGui::BulletText("OpenGL - Rendering graphics");
            ImGui::BulletText("Dear ImGui - User interface library");
            ImGui::Spacing();
            if (ImGui::Button("GitHub Repository")) {
                ShellExecuteA(NULL, "open", "https://github.com/akash1474/deadline_dash", NULL, NULL, SW_SHOWNORMAL);
            }
        }
    ImGui::End();

}




void Application::SwitchDisplayModeIfNeeded(int width,int height){
    if(Get().mIsSwitchingModes){
        if(Get().mIsMiniMode){
            glfwSetWindowAttrib(Get().mWindow,GLFW_DECORATED,GL_FALSE);

            if(Get().mConfig.floatingInFocusMode) glfwSetWindowAttrib(Get().mWindow, GLFW_FLOATING, GL_TRUE);
            else glfwSetWindowAttrib(Get().mWindow, GLFW_FLOATING, GL_FALSE);
        }else{
            glfwSetWindowAttrib(Get().mWindow,GLFW_DECORATED,GL_TRUE);

            if(Get().mConfig.floatingInNormalMode) glfwSetWindowAttrib(Get().mWindow, GLFW_FLOATING, GL_TRUE);
            else glfwSetWindowAttrib(Get().mWindow, GLFW_FLOATING, GL_FALSE);
        }

        Get().mIsSwitchingModes=false;
    }
}



void Application::RenderHomePage()
{

    static float contentHeight=0;
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
    ImGui::SetNextWindowPos({0,0});
    ImGui::Begin("##application",0, ImGuiWindowFlags_NoDecoration |  ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

    const ImVec2 winSize=ImGui::GetWindowSize();
    ImGui::SetCursorPos({winSize.x-50.0f,20.f});
    if(ImGui::Button(Get().mIsMiniMode ? ICON_FA_UP_RIGHT_AND_DOWN_LEFT_FROM_CENTER :ICON_FA_DOWN_LEFT_AND_UP_RIGHT_TO_CENTER,{30,30})){
        Get().mIsMiniMode=!Get().mIsMiniMode;
        Get().mIsSwitchingModes=true;
    }
    if(ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal) && !Get().mIsMiniMode) ImGui::SetTooltip("Focus Mode");

    ImGui::SetCursorPos({winSize.x-90.0f,20.f});
    if(ImGui::Button(ICON_FA_GEAR,{30,30})){
        Get().mPage=Page::Settings;
    }
    if(ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal) && !Get().mIsMiniMode) ImGui::SetTooltip("Settings");


#ifdef GL_DEBUG
    static int displayedFPS = 0;       // Store the last displayed FPS value
    static double lastUpdateTime = 0; // Track the last update time

    double currentTime = ImGui::GetTime(); // Get the current time (in seconds)

    // Update FPS every 0.5 seconds
    if (currentTime - lastUpdateTime > 0.2) {
        displayedFPS = static_cast<int>(ImGui::GetIO().Framerate);
        lastUpdateTime = currentTime;
    }

    ImGui::SetCursorPos({10.0f,20.f});
    std::string fps=std::to_string(displayedFPS)+" FPS";
    ImGui::Button(fps.c_str(),{90,30});
#endif

    // Display the countdown timer if it's running
    float startY = (ImGui::GetWindowHeight() - contentHeight) / 2.0f;
    ImGui::SetCursorPosY(startY);
    contentHeight=0;

    //Header Text
    const ImVec2 headerSize=ImGui::CalcTextSize("DeadlineIn");
    contentHeight+=headerSize.y+ ImGui::GetStyle().FramePadding.y*2;
    ImGui::SetCursorPosX((ImGui::GetWindowWidth()-headerSize.x)*0.5f);
    ImGui::TextColored(ImColor(100, 100, 100, 255),"DeadlineIn");

    
    //Timer
    auto remainingSeconds = CalculateRemainingTimeInSeconds();

    std::string formattedCountdown = FormatCountdown(remainingSeconds);
    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);
    const ImVec2 textSize=ImGui::CalcTextSize(formattedCountdown.c_str());
    contentHeight+=textSize.y+ ImGui::GetStyle().FramePadding.y*2;

    ImGui::SetCursorPosX((ImGui::GetWindowWidth()-textSize.x)*0.5f);
    ImGui::Text("%s", formattedCountdown.c_str());
    ImGui::PopFont();


    //Footer Section
    const ImVec2 footerSize=ImGui::CalcTextSize(Get().mConfig.footerText.c_str());
    ImGui::SetCursorPos({(ImGui::GetWindowWidth()-footerSize.x)*0.5f,ImGui::GetCursorPos().y});
    ImGui::PushStyleColor(ImGuiCol_Text,IM_COL32(150, 150, 150, 255));
    int nLines=CenterWrapped(Get().mConfig.footerText.c_str(),ImGui::GetWindowWidth()-20.0f);
    contentHeight+=(footerSize.y * nLines) + (ImGui::GetStyle().FramePadding.y*2*nLines);
    ImGui::PopStyleColor();
    // ImGui::TextColored(,"%s",footer);

    if(!Get().mIsMiniMode){

        const float center=ImGui::GetWindowWidth()*0.5f;
        const float spacingBetween=10.0f;
        const float buttonWidth=110.0f;
        contentHeight+=25.0f + ImGui::GetStyle().FramePadding.y*2;

        // Buttons
        ImGui::SetCursorPosX(center-(buttonWidth+spacingBetween));
        ImGui::PushStyleColor(ImGuiCol_Button,Get().mIsCountdownRunning ? IM_COL32(202,5,64,255) : IM_COL32(24, 98, 218, 255) );
        if (ImGui::Button(Get().mIsCountdownRunning ? "Clear" : "Start",{buttonWidth,25.0f})) {
            if(Get().mIsCountdownRunning){
                Get().mIsCountdownRunning=false;
                SaveConfig();
            }
            else Get().mPage=Application::Page::Edit;
        }
        if(ImGui::IsItemHovered()) ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
        ImGui::PopStyleColor();


    	ImGui::SameLine();


        ImGui::SetCursorPosX(center+spacingBetween);
        if (ImGui::Button(ICON_FA_PENCIL" Edit",{buttonWidth,25.0f})) Get().mPage=Application::Page::Edit;
        if(ImGui::IsItemHovered()) ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);


    }

    ImGui::End();

}


void Application::RenderEdit() 
{
    if(ImGui::IsKeyPressed(ImGuiKey_Escape)) Get().mPage=Application::Page::HomePage;

    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
    ImGui::SetNextWindowPos({0,0});
    ImGui::Begin("Configure Deadline",0, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

    ImGui::Text("Select Date:");
    ImGui::PushItemWidth(160.0f);
    ImGui::DatePicker("##datepicker", Get().mDeadline,false,85.0f);

    ImGui::Separator();
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

    // ImGui::Separator();
    ImGui::Dummy({2.0f,15.0f});

    ImGui::SetCursorPosX((ImGui::GetWindowWidth()-160.0f)*0.5f);
    ImGui::PushStyleColor(ImGuiCol_Button,IM_COL32(105, 42, 241, 255));
    if (ImGui::Button("Start Countdown",{160.0f,25.0f})) {
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

    ImGui::SetCursorPosX((ImGui::GetWindowWidth()-160.0f)*0.5f);
    if (ImGui::Button("Cancel",{160.0f,25.0f})) Get().mPage=Page::HomePage;
    if(ImGui::IsItemHovered()) ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);


    ImGui::End();
}



void Application::SaveConfig() {
    GL_INFO("SaveConfig");
    if(Get().mIsCountdownRunning) Get().mConfig.deadline = std::chrono::system_clock::to_time_t(Get().mCurrentDeadlineTime);
    else Get().mConfig.deadline=0;

    std::ofstream outFile(Get().mConfigPath, std::ios::binary);
    if (outFile.is_open()) {
        // Serialize the Config struct
        size_t uiFontPathSize = Get().mConfig.uiFontPath.size();
        size_t tiFontPathSize = Get().mConfig.tiFontPath.size();
        size_t headerTextSize = Get().mConfig.headerText.size();
        size_t footerTextSize = Get().mConfig.footerText.size();

        // Write static members
        outFile.write(reinterpret_cast<const char*>(&Get().mConfig.uiFontSize), sizeof(Get().mConfig.uiFontSize));
        outFile.write(reinterpret_cast<const char*>(&Get().mConfig.tiFontSize), sizeof(Get().mConfig.tiFontSize));
        outFile.write(reinterpret_cast<const char*>(&Get().mConfig.icFontSize), sizeof(Get().mConfig.icFontSize));
        outFile.write(reinterpret_cast<const char*>(&Get().mConfig.timerFormat), sizeof(Get().mConfig.timerFormat));
        outFile.write(reinterpret_cast<const char*>(&Get().mConfig.deadline), sizeof(Get().mConfig.deadline));
        outFile.write(reinterpret_cast<const char*>(&Get().mConfig.floatingInNormalMode), sizeof(Get().mConfig.floatingInNormalMode));
        outFile.write(reinterpret_cast<const char*>(&Get().mConfig.floatingInFocusMode), sizeof(Get().mConfig.floatingInFocusMode));

        // Write string sizes and their content
        outFile.write(reinterpret_cast<const char*>(&uiFontPathSize), sizeof(uiFontPathSize));
        outFile.write(Get().mConfig.uiFontPath.c_str(), uiFontPathSize);

        outFile.write(reinterpret_cast<const char*>(&tiFontPathSize), sizeof(tiFontPathSize));
        outFile.write(Get().mConfig.tiFontPath.c_str(), tiFontPathSize);

        outFile.write(reinterpret_cast<const char*>(&headerTextSize), sizeof(headerTextSize));
        outFile.write(Get().mConfig.headerText.c_str(), headerTextSize);

        outFile.write(reinterpret_cast<const char*>(&footerTextSize), sizeof(footerTextSize));
        outFile.write(Get().mConfig.footerText.c_str(), footerTextSize);

        outFile.close();
    } else {
        std::cerr << "Error: Could not open file to save config." << std::endl;
    }
}

void Application::LoadConfig() {
    std::ifstream inFile(Get().mConfigPath, std::ios::binary);
    if (inFile.is_open()) {
        // Deserialize the Config struct
        size_t uiFontPathSize = 0, tiFontPathSize = 0;
        size_t headerTextSize = 0, footerTextSize = 0;

        // Read static members
        inFile.read(reinterpret_cast<char*>(&Get().mConfig.uiFontSize), sizeof(Get().mConfig.uiFontSize));
        inFile.read(reinterpret_cast<char*>(&Get().mConfig.tiFontSize), sizeof(Get().mConfig.tiFontSize));
        inFile.read(reinterpret_cast<char*>(&Get().mConfig.icFontSize), sizeof(Get().mConfig.icFontSize));
        inFile.read(reinterpret_cast<char*>(&Get().mConfig.timerFormat), sizeof(Get().mConfig.timerFormat));
        inFile.read(reinterpret_cast<char*>(&Get().mConfig.deadline), sizeof(Get().mConfig.deadline));
        inFile.read(reinterpret_cast<char*>(&Get().mConfig.floatingInNormalMode), sizeof(Get().mConfig.floatingInNormalMode));
        inFile.read(reinterpret_cast<char*>(&Get().mConfig.floatingInFocusMode), sizeof(Get().mConfig.floatingInFocusMode));

        // Read string sizes and their content
        inFile.read(reinterpret_cast<char*>(&uiFontPathSize), sizeof(uiFontPathSize));
        if(uiFontPathSize>0)
        {
            Get().mConfig.uiFontPath.resize(uiFontPathSize);
            inFile.read(&Get().mConfig.uiFontPath[0], uiFontPathSize);
        }

        inFile.read(reinterpret_cast<char*>(&tiFontPathSize), sizeof(tiFontPathSize));
        if(tiFontPathSize>0){
            Get().mConfig.tiFontPath.resize(tiFontPathSize);
            inFile.read(&Get().mConfig.tiFontPath[0], tiFontPathSize);
        }

        inFile.read(reinterpret_cast<char*>(&headerTextSize), sizeof(headerTextSize));
        Get().mConfig.headerText.resize(headerTextSize);
        inFile.read(&Get().mConfig.headerText[0], headerTextSize);

        inFile.read(reinterpret_cast<char*>(&footerTextSize), sizeof(footerTextSize));
        if(footerTextSize>0){
            Get().mConfig.footerText.resize(footerTextSize);
            inFile.read(&Get().mConfig.footerText[0], footerTextSize);
        }

        inFile.close();

        // Post-processing
        if (Get().mConfig.deadline > 0) {
            Get().mCurrentDeadlineTime = std::chrono::system_clock::from_time_t(Get().mConfig.deadline);
            Get().mDeadline = *std::localtime(&Get().mConfig.deadline);
            Get().mIsCountdownRunning = true;
        }else Get().mIsCountdownRunning=false;
    } else {
        std::cerr << "Error: Could not open file to load config." << std::endl;
        SaveConfig();
    }
}


std::string Application::FormatCountdown(std::chrono::seconds remainingSeconds) 
{
    switch (Get().mConfig.timerFormat) {
        case (int)FormatOption::HOURS_ONLY: {
            if(!Get().mIsCountdownRunning) return "00 Hours";
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
            if(!Get().mIsCountdownRunning) return "00.00 Hours";
            float hours = std::chrono::duration<float, std::chrono::hours::period>(remainingSeconds).count();
            char buffer[20];
            snprintf(buffer, sizeof(buffer), "%02f %s", hours,hours>1 ? "Hours": "Hour");
            return std::string(buffer);
        }
        case (int)FormatOption::HH_MM_SS: {
            if(!Get().mIsCountdownRunning) return "00:00:00";
            int totalSeconds = remainingSeconds.count();
            int hours = totalSeconds / 3600;
            int minutes = (totalSeconds % 3600) / 60;
            int seconds = totalSeconds % 60;
            char buffer[20];
            snprintf(buffer, sizeof(buffer), "%02d:%02d:%02d", hours, minutes, seconds);
            return std::string(buffer);
        }
        case (int)FormatOption::DAYS_HOURS_MINUTES_SECONDS: {
            if(!Get().mIsCountdownRunning) return "00 Days, 00 Hrs, 00 Mins, 00 Secs";
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