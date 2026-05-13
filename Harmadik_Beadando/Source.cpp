enum eVertexArrayObject {
    VAOCube,
    VAOSphere,
    VAOCount
};
enum eBufferObject {
    VBOCube,
    VBOSphere,
    EBOSphere,
    BOCount
};
enum eProgram {
    MainShaderProgram,
    ProgramCount
};
enum eTexture {
    SunTexture,
    TextureCount
};

#include <common.cpp>

GLchar windowTitle[] = "Kockak Beadando";

// --- Globális állapot ---
float   r = 9.0f;
float   camAngle = 0.0f, camZ = 0.0f;
bool    lightingEnabled = true;

GLuint  sunTex;
int     sphereIndexCount = 0;

// Uniform locations
GLuint  locationModel;
GLuint  locationView;
GLuint  locationProjection;
GLuint  locationInverseTranspose;
GLuint  locationLightPos;
GLuint  locationLightColor;
GLuint  locationLightingEnabled;
GLuint  locationIsLightSource;

// --- Szilárd Gömb generálás textúrával (d=0.5) ---
void setupSphere() {
    std::vector<float>          vertices;
    std::vector<unsigned int>   indices;
    int     sectors = 40, stacks = 40;
    float   radius = 0.25f;

    for (int i = 0; i <= stacks; ++i) {
        float stackAngle = glm::pi<float>() / 2 - i * glm::pi<float>() / stacks;
        float xy = radius * cosf(stackAngle);
        float z = radius * sinf(stackAngle);

        for (int j = 0; j <= sectors; ++j) {
            float sectorAngle = j * 2 * glm::pi<float>() / sectors;
            float x = xy * cosf(sectorAngle);
            float y = xy * sinf(sectorAngle);
            vertices.push_back(x); vertices.push_back(y); vertices.push_back(z); // Pozíció
            vertices.push_back(x); vertices.push_back(y); vertices.push_back(z); // Normális
            vertices.push_back((float)j / sectors); vertices.push_back((float)i / stacks); // UV
        }
    }

    for (int i = 0; i < stacks; ++i) {
        int k1 = i * (sectors + 1), k2 = k1 + sectors + 1;
        for (int j = 0; j < sectors; ++j, ++k1, ++k2) {
            if (i != 0) { indices.push_back(k1); indices.push_back(k2); indices.push_back(k1 + 1); }
            if (i != (stacks - 1)) { indices.push_back(k1 + 1); indices.push_back(k2); indices.push_back(k2 + 1); }
        }
    }
    sphereIndexCount = (int)indices.size();

    glBindVertexArray(VAO[VAOSphere]);
    glBindBuffer(GL_ARRAY_BUFFER, BO[VBOSphere]);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, BO[EBOSphere]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
}

void setupCube() {
    float cube[] = {
        -0.5f,-0.5f,-0.5f, 0,0,-1,  0.5f,-0.5f,-0.5f, 0,0,-1,  0.5f, 0.5f,-0.5f, 0,0,-1,
         0.5f, 0.5f,-0.5f, 0,0,-1, -0.5f, 0.5f,-0.5f, 0,0,-1, -0.5f,-0.5f,-0.5f, 0,0,-1,
        -0.5f,-0.5f, 0.5f, 0,0, 1,  0.5f,-0.5f, 0.5f, 0,0, 1,  0.5f, 0.5f, 0.5f, 0,0, 1,
         0.5f, 0.5f, 0.5f, 0,0, 1, -0.5f, 0.5f, 0.5f, 0,0, 1, -0.5f,-0.5f, 0.5f, 0,0, 1,
        -0.5f, 0.5f, 0.5f,-1,0, 0, -0.5f, 0.5f,-0.5f,-1,0, 0, -0.5f,-0.5f,-0.5f,-1,0, 0,
        -0.5f,-0.5f,-0.5f,-1,0, 0, -0.5f,-0.5f, 0.5f,-1,0, 0, -0.5f, 0.5f, 0.5f,-1,0, 0,
         0.5f, 0.5f, 0.5f, 1,0, 0,  0.5f, 0.5f,-0.5f, 1,0, 0,  0.5f,-0.5f,-0.5f, 1,0, 0,
         0.5f,-0.5f,-0.5f, 1,0, 0,  0.5f,-0.5f, 0.5f, 1,0, 0,  0.5f, 0.5f, 0.5f, 1,0, 0,
        -0.5f,-0.5f,-0.5f, 0,-1,0,  0.5f,-0.5f,-0.5f, 0,-1,0,  0.5f,-0.5f, 0.5f, 0,-1,0,
         0.5f,-0.5f, 0.5f, 0,-1,0, -0.5f,-0.5f, 0.5f, 0,-1,0, -0.5f,-0.5f,-0.5f, 0,-1,0,
        -0.5f, 0.5f,-0.5f, 0, 1,0,  0.5f, 0.5f,-0.5f, 0, 1,0,  0.5f, 0.5f, 0.5f, 0, 1,0,
         0.5f, 0.5f, 0.5f, 0, 1,0, -0.5f, 0.5f, 0.5f, 0, 1,0, -0.5f, 0.5f,-0.5f, 0, 1,0
    };

    glBindVertexArray(VAO[VAOCube]);
    glBindBuffer(GL_ARRAY_BUFFER, BO[VBOCube]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube), cube, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
}

void initShaderProgram() {
    ShaderInfo shader_info[ProgramCount][3] = { {
        { GL_VERTEX_SHADER,   "./vertexShader.glsl"   },
        { GL_FRAGMENT_SHADER, "./fragmentShader.glsl" },
        { GL_NONE, nullptr } } };

    for (int programItem = 0; programItem < ProgramCount; programItem++) {
        program[programItem] = LoadShaders(shader_info[programItem]);
    }

    glUseProgram(program[MainShaderProgram]);

    locationModel = glGetUniformLocation(program[MainShaderProgram], "model");
    locationView = glGetUniformLocation(program[MainShaderProgram], "view");
    locationProjection = glGetUniformLocation(program[MainShaderProgram], "projection");
    locationInverseTranspose = glGetUniformLocation(program[MainShaderProgram], "inverseTransposeMatrix");
    locationLightPos = glGetUniformLocation(program[MainShaderProgram], "lightPos");
    locationLightColor = glGetUniformLocation(program[MainShaderProgram], "lightColor");
    locationLightingEnabled = glGetUniformLocation(program[MainShaderProgram], "lightingEnabled");
    locationIsLightSource = glGetUniformLocation(program[MainShaderProgram], "isLightSource");

    sunTex = SOIL_load_OGL_texture(
        "sun.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID,
        SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y);

    setupCube();
    setupSphere();
}

void display(GLFWwindow* window, double currentTime) {
    glClearColor(0.02f, 0.02f, 0.04f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::vec3 cameraPos = glm::vec3(r * cos(camAngle), r * sin(camAngle), camZ);
    glm::mat4 view = glm::lookAt(cameraPos, glm::vec3(0, 0, 0), glm::vec3(0, 0, 1));
    glm::mat4 projection = glm::perspective(glm::radians(55.0f), (float)windowWidth / (float)windowHeight, 0.1f, 100.0f);

    glUniformMatrix4fv(locationView, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(locationProjection, 1, GL_FALSE, glm::value_ptr(projection));

    float     time = (float)currentTime;
    glm::vec3 lightPos = glm::vec3(2.0f * r * cos(time), 2.0f * r * sin(time), 0.0f);
    glUniform3fv(locationLightPos, 1, glm::value_ptr(lightPos));
    glUniform3f(locationLightColor, 1.0f, 0.95f, 0.4f);
    glUniform1i(locationLightingEnabled, lightingEnabled);

    // Kockák
    glUniform1i(locationIsLightSource, 0);
    glBindVertexArray(VAO[VAOCube]);

    glm::vec3 cubePositions[] = {
        glm::vec3(0, 0,  0),
        glm::vec3(0, 0,  2),
        glm::vec3(0, 0, -2)
    };

    for (int i = 0; i < 3; i++) {
        glm::mat4 model = glm::translate(glm::mat4(1.0f), cubePositions[i]);
        glm::mat3 invTrans = glm::mat3(glm::transpose(glm::inverse(model)));
        glUniformMatrix4fv(locationModel, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix3fv(locationInverseTranspose, 1, GL_FALSE, glm::value_ptr(invTrans));
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }

    // Nap (Textúrázott szilárd gömb)
    glUniform1i(locationIsLightSource, 1);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, sunTex);
    glBindVertexArray(VAO[VAOSphere]);

    glm::mat4 modelSun = glm::translate(glm::mat4(1.0f), lightPos);
    glUniformMatrix4fv(locationModel, 1, GL_FALSE, glm::value_ptr(modelSun));
    glDrawElements(GL_TRIANGLES, sphereIndexCount, GL_UNSIGNED_INT, 0);
}

void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    windowWidth = glm::max(width, 1);
    windowHeight = glm::max(height, 1);
    glViewport(0, 0, windowWidth, windowHeight);
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS)        keyboard[key] = GL_TRUE;
    else if (action == GLFW_RELEASE) keyboard[key] = GL_FALSE;

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);

    if (key == GLFW_KEY_L && action == GLFW_PRESS)
        lightingEnabled = !lightingEnabled;
}

void cursorPosCallback(GLFWwindow* window, double xPos, double yPos) {}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {}

int main(void) {
    init(3, 3, GLFW_OPENGL_CORE_PROFILE);
    glEnable(GL_DEPTH_TEST);

    initShaderProgram();
    framebufferSizeCallback(window, windowWidth, windowHeight);
    setlocale(LC_ALL, "");

    cout << "Billentyuk:" << endl;
    cout << "ESC\t\tKilepes" << endl;
    cout << "L\t\tMegvilagitas be/ki" << endl;
    cout << "LEFT/RIGHT\tKamera forgatasa" << endl;
    cout << "UP/DOWN\t\tKamera fel/le" << endl;

    while (!glfwWindowShouldClose(window)) {
        if (keyboard[GLFW_KEY_LEFT])  camAngle -= 0.02f;
        if (keyboard[GLFW_KEY_RIGHT]) camAngle += 0.02f;
        if (keyboard[GLFW_KEY_UP])    camZ += 0.05f;
        if (keyboard[GLFW_KEY_DOWN])  camZ -= 0.05f;

        display(window, glfwGetTime());
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    cleanUpScene(EXIT_SUCCESS);
    return EXIT_SUCCESS;
}