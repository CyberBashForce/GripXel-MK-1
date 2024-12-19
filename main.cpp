#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_opengl3.h>
#include <windows.h>

#include <Custom/camera.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <Custom/shader_s.h>
#include <Custom/model.h>

#include <iostream>
#include <string>
#include <algorithm>

const unsigned int SCR_WIDTH = 1000;
const unsigned int SCR_HEIGHT = 800;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void fitToScreen();

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;
bool firstLoad = true;

glm::mat4 model = (1.0f);

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

Model* ourModel = nullptr;
Shader* globalShader = nullptr;
//Shader ourShader("I:/CodeX/OpenGL/Projects/Project8/model_loading_vs.glsl", "I:/CodeX/OpenGL/Projects/Project8/model_loading_fs.glsl");

// Function to open file dialog and get the file path
std::string OpenFileDialog() {
	// Define the file path buffer (wide characters)
	wchar_t filePath[MAX_PATH] = { 0 };

	OPENFILENAME ofn;
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = NULL; // No specific owner window
	ofn.lpstrFilter = L"3D Model Files\0*.obj;*.fbx;*.glb;*.gltf;*.stl\0All Files\0*.*\0"; // Wide string for filter
	ofn.lpstrFile = filePath; // Wide string buffer to hold selected file path
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;

	// Show the file open dialog
	if (GetOpenFileName(&ofn)) {
		// Convert the wide-character string to a narrow-character string (UTF-8)
		int bufferSize = WideCharToMultiByte(CP_UTF8, 0, filePath, -1, NULL, 0, NULL, NULL);
		char* buffer = new char[bufferSize];
		WideCharToMultiByte(CP_UTF8, 0, filePath, -1, buffer, bufferSize, NULL, NULL);

		std::string result = std::string(buffer);
		delete[] buffer; // Clean up the allocated memory

		std::replace(result.begin(), result.end(), '\\', '/');

		return result;
	}

	return "";
}

int main() {

	//Setup GLFW

	glfwInit();
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "GripXel MK 1", NULL, NULL);

	if (window == NULL) {
		std::cout << "Error Creating the Window!" << '\n';
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	//Load Glad

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Oops! Seems like GLAD failed to load" << '\n';
		glfwTerminate();
		return -1;
	}

	// tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
	stbi_set_flip_vertically_on_load(true);

	// configure global opengl state
	glEnable(GL_DEPTH_TEST);

	// Setup ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.FontGlobalScale = 1.5f;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330"); // GLSL version

	// Setup ImGui Style
	ImGui::StyleColorsDark();

	Shader ourShader("I:/CodeX/OpenGL/Projects/Project8/model_loading_vs.glsl", "I:/CodeX/OpenGL/Projects/Project8/model_loading_fs.glsl");
	//ourModel = new Model("I:/CodeX/OpenGL/resources/backpack/backpack.obj");
	globalShader = &ourShader;
	//Render Engine
	while (!glfwWindowShouldClose(window)) {

		float currentFrame = static_cast<float>(glfwGetTime());
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		processInput(window);

		glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// don't forget to enable shader before setting uniforms
		ourShader.use();

		// view/projection transformations
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		glm::mat4 view = camera.GetViewMatrix();
		ourShader.setMat4("projection", projection);
		ourShader.setMat4("view", view);

		ourShader.setMat4("model", model);

		// Render the loaded model (if it's loaded)
		if (ourModel != nullptr) {
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			ourModel->Draw(ourShader); // Draw the model
			//std::cout << "Model loaded with " << ourModel->meshes.size() << " meshes." << std::endl;

		}

		// Start the ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		// Menu Bar
		if (ImGui::BeginMainMenuBar()) {
			if (ImGui::BeginMenu("File")) {
				if (ImGui::MenuItem("Import")) {
					std::string selectedFile = OpenFileDialog();
					if (!selectedFile.empty()) {
						std::cout << "Selected File: " << selectedFile << std::endl;
						//selectedFile = "\"" + selectedFile + "\"";
						if (ourModel != nullptr) {
							delete ourModel; // Clean up the previous model if any
						}
						ourModel = new Model(selectedFile); // Load the new model
						fitToScreen();
					}
					else {
						std::cout << "No file selected." << std::endl;
					}
				}
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Edit")) {
				ImGui::MenuItem("Undo", "Ctrl+Z");
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Help")) {
				ImGui::MenuItem("About");
				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		
		}
		// Render ImGui
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(window);
		glfwPollEvents();

	}
	// Cleanup
	if (ourModel != nullptr) {
		delete ourModel; // Clean up the model
	}

	// Cleanup ImGui
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwTerminate();

	return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window) {
	if (GLFW_PRESS == glfwGetKey(window, GLFW_KEY_ESCAPE)) {
		glfwSetWindowShouldClose(window, true);
	}
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
		camera.ProcessKeyboard(UP, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
		camera.ProcessKeyboard(DOWN, deltaTime);
	
	if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
		fitToScreen();
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
	float xpos = static_cast<float>(xposIn);
	float ypos = static_cast<float>(yposIn);

	if (firstMouse) {
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
		cout << "#CLAED | " << xpos - lastX << '\n';
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos;
	lastX = xpos;
	lastY = ypos;
	model = glm::rotate(model, glm::radians(xoffset), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::rotate(model, glm::radians(yoffset), glm::vec3(0.0f, 1.0f, 0.0f));
	//camera.ProcessMouseMovement(-xoffset, -yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

void fitToScreen() {
	camera.Position = glm::vec3(model[0])+glm::vec3(0.0f,0.0f,(ourModel->center.z + 2.0f));
}