#ifndef DRAWING_H
#define DRAWING_H

#include "pch.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

class Drawing
{
private:
	static LPCSTR lpWindowName;
	static ImVec2 vWindowSize;
	static ImGuiWindowFlags WindowFlags;
	static bool bDraw;

public:
	static void Active();
	static bool isActive();
	static void Draw();
	static ImVec4 rgbaToDecimal(float r, float g, float b, float a);
	static void AddTaskToSchedule(std::string taskName, std::string taskDescription, std::string taskTime, std::string taskDate);
	static void LoadSchedule(std::string taskDate);
	static std::string GetCurrentDate();
	static void closeAllWindows();
	static std::string getCurrentHour();
	static int getHourIn24Hrs();
};

#endif
