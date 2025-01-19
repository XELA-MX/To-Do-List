#include "Drawing.h"

LPCSTR Drawing::lpWindowName = "Your Tasks For today!";
ImVec2 Drawing::vWindowSize = { 500, 400 };
ImGuiWindowFlags Drawing::WindowFlags = 0;
using json = nlohmann::json;
bool Drawing::bDraw = true;
// Matriz de tareas con su nombre, descripcion y hora
std::string tasks[100][3];

short tries = 0;

// VARS
bool addATask = false;
bool windows[1] = { addATask };
ImVec2 mainWindowPos;
ImVec2 mainWindowSize;
static char taskName[128];
static char taskDescription[128];
static char taskTime[128];
static char addTaskStatus[128];
bool NoTasks = false;
bool windowTopMost = false;
std::string taskDate;

void ToggleButton(const char* str_id, bool* v)
{
	ImVec2 p = ImGui::GetCursorScreenPos();
	ImDrawList* draw_list = ImGui::GetWindowDrawList();

	float height = ImGui::GetFrameHeight();
	float width = height * 1.55f;
	float radius = height * 0.50f;

	if (ImGui::InvisibleButton(str_id, ImVec2(width, height)))
		*v = !*v;
	ImU32 col_bg;
	if (ImGui::IsItemHovered())
		col_bg = *v ? IM_COL32(145 + 20, 211, 68 + 20, 255) : IM_COL32(218 - 20, 218 - 20, 218 - 20, 255);
	else
		col_bg = *v ? IM_COL32(145, 211, 68, 255) : IM_COL32(218, 218, 218, 255);

	draw_list->AddRectFilled(p, ImVec2(p.x + width, p.y + height), col_bg, height * 0.5f);
	draw_list->AddCircleFilled(ImVec2(*v ? (p.x + width - radius) : (p.x + radius), p.y + radius), radius - 1.5f, IM_COL32(255, 255, 255, 255));
}

void Drawing::Active()
{
	bDraw = true;
}

bool Drawing::isActive()
{
	return bDraw == true;
}


void Drawing::Draw()
{
	if (isActive())
	{
		// TopMost
		ImGui::SetNextWindowSize(vWindowSize, ImGuiCond_Once);
		ImGui::SetNextWindowBgAlpha(1.0f);
		ImGui::Begin(lpWindowName, &bDraw, WindowFlags);
		{

			mainWindowPos = ImGui::GetWindowPos();
			mainWindowSize = ImGui::GetWindowSize();

				Drawing::LoadSchedule(GetCurrentDate() + ".json");


			// Add A Task window
			if (addATask) {

				ImGui::SetNextWindowPos(ImVec2((mainWindowPos.x + mainWindowSize.x) + 20, mainWindowPos.y), ImGuiCond_Always);
				ImGui::SetNextWindowSize(ImVec2(400, 200), ImGuiCond_Always);
				if (ImGui::Begin("Add A Task Window!")) {

					bool empty = false;
					taskDate = GetCurrentDate();
					ImGui::InputText("Task Name", taskName, IM_ARRAYSIZE(taskName));
					ImGui::InputText("Task Description", taskDescription, IM_ARRAYSIZE(taskDescription));

					// Variables locales para horas y minutos
					static int hour = 0;
					static int minute = 0;

					// Controles para horas y minutos
					ImGui::SliderInt("Hour (0-24)", &hour, 0, 23);
					ImGui::SliderInt("Minute (0-59)", &minute, 0, 59);

					// Mostrar el tiempo seleccionado en un formato legible
					sprintf_s(taskTime, "%02d:%02d", hour, minute);

					// VERIFICAR SI HAY CONTENIDO EN LOS CAMPOS DE TEXTO
					if (strlen(taskName) == 0 || strlen(taskDescription) == 0 || strlen(taskTime) == 0) {
						strcpy_s(addTaskStatus, "Please fill in all fields.");
						empty = true;
					}
					else {
						strcpy_s(addTaskStatus, "");
					}

					// VERIFICAR SI LA TAREA YA PASÓ
					if (getHourIn24Hrs() > hour) {
						strcpy_s(addTaskStatus, "Task time has already passed.");
						empty = true;
					}
					else if (getHourIn24Hrs() == hour) {
						if (std::stoi(getCurrentHour().substr(3, 2)) > minute) {
							strcpy_s(addTaskStatus, "Task time has already passed.");
							empty = true;
						}
					}

					// Mostrar el estado de adición
					ImGui::Text(addTaskStatus);


					if (ImGui::Button("Add Task") && !empty) {
						AddTaskToSchedule(taskName, taskDescription, taskTime, taskDate);
						// Reset the input fields
						strcpy_s(taskName, "");
						strcpy_s(taskDescription, "");
						strcpy_s(taskTime, "");
						strcpy_s(addTaskStatus, "Task Added Successfully!");
					}
					ImGui::SameLine();
					if (ImGui::Button("Close")) {
						addATask = false;
					}
				}ImGui::End();
			}

			// Top menu , add tasks , save schedule , load schedule , exit , delete a task , settings
			if (ImGui::Button("Add a Task")) {
				Drawing::closeAllWindows();
				addATask = true;
			}
			ImGui::Dummy(ImVec2(0, 3));
			ImGui::Separator();

			ImGui::Dummy(ImVec2(0, 10));

			// check if there are tasks
			if (tasks[0][0] == "" && NoTasks) {
				ImGui::Text("No tasks for today!");
			}
			else {
				// Draw the tasks
				for (int i = 0; i < 100; i++) {
					if (tasks[i][0] == "") {
						break;
					}
					else {
						// ADD
						if ((std::string)tasks[i][3] == "false") {
							ImGui::Text("Task Name: %s", tasks[i][0].c_str());
							ImGui::Text("Task Description: %s", tasks[i][1].c_str());
							ImGui::Text("Task Time: %s", tasks[i][2].c_str());
							ImGui::Dummy(ImVec2(0, 5));
							ImGui::Separator();
						}
						else if((std::string) tasks[i][3] == "true") {
							ImGui::Text("Task Name: %s", tasks[i][0].c_str());
							ImGui::Text("Task Description: %s", tasks[i][1].c_str());
							ImGui::Text("Task Time: %s", tasks[i][2].c_str());
							ImGui::TextColored(rgbaToDecimal(255,0,0,1), "Task Done!");
							ImGui::Dummy(ImVec2(0, 5));
							ImGui::Separator();
						}
					}
				}
			}

		}
		ImGui::End();
	}
}

ImVec4 Drawing::rgbaToDecimal(float r, float g, float b, float a)
{
	return ImVec4((float) r/255 , (float) g/255 ,(float) b/255 , (float)a);
}

void Drawing::AddTaskToSchedule(std::string taskName, std::string taskDescription, std::string taskTime, std::string taskDate)
{
	json jsonData;
	std::string fileName = taskDate + ".json";
	std::ifstream inputFile(fileName);

	// Cargando los datos del archivo
	if (inputFile.is_open()) {
		inputFile >> jsonData;
		inputFile.close();
	}// Si no se encuentra crear uno nuevo con filename
	else {
		jsonData["tasks"] = json::array();
	}

	// Agregando la tarea
	jsonData["tasks"].push_back({
		{"name", taskName},
		{"description", taskDescription},
		{"time", taskTime},
		{"done" , "false"}
		});

	// Guardando los datos con reintentos
	bool success = false;
	for (int attempt = 0; attempt < 3; ++attempt) {
		std::ofstream outputFile(fileName);
		if (outputFile.is_open()) {
			outputFile << jsonData.dump(4);
			outputFile.close();
			success = true;
			break;
		}
		else {
			std::cerr << "Intento fallido para guardar tareas (" << attempt + 1 << "/3)." << std::endl;
		}
	}

	if (!success) {
		std::cerr << "No se pudo guardar las tareas después de 3 intentos. Por favor, inténtelo más tarde." << std::endl;
	}
}


void Drawing::LoadSchedule(std::string taskDate) {
	json jsonData;
	std::string fileName = taskDate;
	std::ifstream inputFile(fileName);

	if (inputFile.is_open()) {
		inputFile >> jsonData;
		inputFile.close();
		NoTasks = false;  // Se encontró el archivo
	}
	else if (!inputFile.is_open()) {
		std::cerr << "No se pudo abrir el archivo: " << fileName << std::endl;
		NoTasks = true;
		return;
	}

	// Cargando las tareas
	for (int i = 0; i < jsonData["tasks"].size(); i++) {
		tasks[i][0] = jsonData["tasks"][i]["name"];
		tasks[i][1] = jsonData["tasks"][i]["description"];
		tasks[i][2] = jsonData["tasks"][i]["time"];
		tasks[i][3] = jsonData["tasks"][i]["done"];
	}
}


std::string Drawing::GetCurrentDate()
{
	// FECHA Y HORA ACTUALES DEL SISTEMA
	auto now = std::chrono::system_clock::now();
	auto in_time_t = std::chrono::system_clock::to_time_t(now);

	// A estructura tm
	std::tm buf;
	#if defined(_WIN32) || defined(WIN64) 
		localtime_s(&buf, &in_time_t);
	#else
		localtime_r(&in_time_t, &buf); // Linux/Unix
	#endif // defined(_WIN32) || defined(WIN64) 

		// FORMATEAR A MM/DD/YYYY
		std::ostringstream dateStream;
		dateStream << std::setw(2) << std::setfill('0') << (buf.tm_mon + 1) << "-"
			<< std::setw(2) << std::setfill('0') << buf.tm_mday << "-"
			<< (buf.tm_year + 1900);

		return dateStream.str();

}

void Drawing::closeAllWindows()
{
	for (bool x : windows) {
		x = false;
	}
}

std::string Drawing::getCurrentHour()
{
	// DE 24 HORAS OSEA 0-23 - retornar 14:00
	auto now = std::chrono::system_clock::now();
	auto in_time_t = std::chrono::system_clock::to_time_t(now);

	// A estructura tm
	std::tm buf;
#if defined(_WIN32) || defined(WIN64) 
	localtime_s(&buf, &in_time_t);
#else
	localtime_r(&in_time_t, &buf); // Linux/Unix
#endif // defined(_WIN32) || defined(WIN64)
	// FORMATEAR A MM/DD/YYYY
	std::ostringstream dateStream;
	dateStream << std::setw(2) << std::setfill('0') << buf.tm_hour << ":"
		<< std::setw(2) << std::setfill('0') << buf.tm_min;
	return dateStream.str();
}

int Drawing::getHourIn24Hrs()
{
	auto now = std::chrono::system_clock::now();
	auto in_time_t = std::chrono::system_clock::to_time_t(now);
	// A estructura tm
	std::tm buf;

#if defined(_WIN32) || defined(WIN64)
	localtime_s(&buf, &in_time_t);
#else
	localtime_r(&in_time_t, &buf); // Linux/Unix
#endif // defined(_WIN32) || defined(WIN64)

	return buf.tm_hour;
}