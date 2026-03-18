enum eVertexArrayObject {
    VAOQuad,
    VAOCount
};
enum eBufferObject {
    VBOQuad,
    BOCount
};
enum eProgram {
    ShaderProgram,
    ProgramCount
};
enum eTexture {
    NoTexture,
    TextureCount
};
enum eAnimationType {
    RotateFirst,
    TranslateFirst,
    Bouncing
};


#include <common.cpp>

using namespace std;

GLchar windowTitle[] = "Első Beadandó!";

static array<vec2, 6> quad = {
    vec2(-1.0f, -1.0f),
    vec2(1.0f, -1.0f),
    vec2(1.0f, 1.0f),
    vec2(-1.0f, -1.0f),
    vec2(1.0f, 1.0f),
    vec2(-1.0f, 1.0f)
};

GLfloat circleRadius = 50.0f;

GLuint centerLoc = 0;
GLuint radiusLoc = 0;
GLuint resolutionLoc = 0;
GLuint locationWindowSize = 0;
GLuint XoffsetLoc = 0;
GLuint YoffsetLoc = 0;
GLuint YLineLoc = 0;

GLfloat			x = 0.00f;
GLfloat			y = 0.00f;
GLfloat			increment = 2.0f;
GLboolean		xDir = GL_TRUE;
GLboolean		yDir = GL_FALSE;
mat4			translateMatrix = translate(mat4(1.0f), vec3(-0.5, 0.5, 0.0));
mat4			rotateMatrix = rotate(mat4(1.0f), (GLfloat)glfwGetTime(), vec3(0.0f, 0.0f, 1.0f));
eAnimationType	animationType = Bouncing;

GLfloat segmentYOffset = 0.0f;
GLfloat moveStep = 20.0f;

GLboolean moving = GL_FALSE;
GLfloat vx = 0.0f;
GLfloat vy = 0.0f;
GLfloat stepLength = 10.0f;

void initShaderProgram()
{
    ShaderInfo shader_info[] = {
        {GL_FRAGMENT_SHADER, "./fragmentShader.glsl"},
        {GL_VERTEX_SHADER, "./vertexShader.glsl"},
        {GL_NONE, nullptr} };

    /** A vertex-fragment program elkészítése. */
    program[ShaderProgram] = LoadShaders(shader_info);

    glBindVertexArray(VAO[VAOQuad]);
    glBindBuffer(GL_ARRAY_BUFFER, BO[VBOQuad]);
    glBufferData(GL_ARRAY_BUFFER, quad.size() * sizeof(vec2), quad.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(vec2), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    centerLoc = glGetUniformLocation(program[ShaderProgram], "center");
    radiusLoc = glGetUniformLocation(program[ShaderProgram], "radius");
    locationWindowSize = glGetUniformLocation(program[ShaderProgram], "windowSize");
    XoffsetLoc = glGetUniformLocation(program[ShaderProgram], "offsetX");
    YoffsetLoc = glGetUniformLocation(program[ShaderProgram], "offsetY");
    YLineLoc = glGetUniformLocation(program[ShaderProgram], "lineY");

    glUseProgram(program[ShaderProgram]);
}

/** A kód, amellyel rajzolni tudunk a GLFWwindow objektumunkba. */
void display(GLFWwindow* window, double currentTime)
{
    /** Töröljük le a színbuffert! */
    glClear(GL_COLOR_BUFFER_BIT);
    glUniform2f(locationWindowSize, windowWidth, windowHeight);

    glProgramUniform1f(program[ShaderProgram], YLineLoc, segmentYOffset); // Szakasz mozgatása

    float limit = windowWidth / 2.0f - circleRadius;
    if (moving) {
        x += vx;
        y += vy;

        if (x > limit || x < -limit) {
            vx = -vx; 
        }

        if (y > limit || y < -limit) {
            vy = -vy; 
        }

        glProgramUniform1f(program[ShaderProgram], XoffsetLoc, x);
        glProgramUniform1f(program[ShaderProgram], YoffsetLoc, y);
    }

    
    switch (animationType) {
    case RotateFirst:
        glProgramUniform1f(program[ShaderProgram], XoffsetLoc, 0.0f);
        glProgramUniform1f(program[ShaderProgram], YoffsetLoc, 0.0f);

        rotateMatrix = rotate(glm::mat4(1.0f), (GLfloat)currentTime, vec3(0.0f, 0.0f, 1.0f));
        matModel = translateMatrix * rotateMatrix;
        break;
        
    case Bouncing:
        matModel = mat4(1.0);
        if (xDir) {
            x += increment;

            if (x > limit || x < -limit)
                increment = -increment;
            glProgramUniform1f(program[ShaderProgram], XoffsetLoc, x);
            glProgramUniform1f(program[ShaderProgram], YoffsetLoc, y);
            
        }
        else if (yDir) {
            y += increment;

            if (y > limit || y < -limit)
                increment = -increment;
            glProgramUniform1f(program[ShaderProgram], XoffsetLoc, x);
            glProgramUniform1f(program[ShaderProgram], YoffsetLoc, y);
        }

        break;
    }
    

    glUniform2f(centerLoc, windowWidth / 2.0f, windowHeight / 2.0f);
    glUniform1f(radiusLoc, circleRadius);

    glBindVertexArray(VAO[VAOQuad]);
    matModelView = matView * matModel;
    glUniformMatrix4fv(locationMatModelView, 1, GL_FALSE, value_ptr(matModelView));

    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void framebufferSizeCallback(GLFWwindow* window, int width, int height)
{
    windowWidth = glm::max(width, 1);
    windowHeight = glm::max(height, 1);

    glViewport(0, 0, windowWidth, windowHeight);

    matModel = mat4(1.0);
    matView = lookAt(
        vec3(0.0f, 0.0f, 9.0f),		// the position of your camera, in world space
        vec3(0.0f, 0.0f, 0.0f),		// where you want to look at, in world space
        vec3(0.0f, 1.0f, 0.0f));	// upVector, probably glm::vec3(0,1,0), but (0,-1,0) would make you looking upside-down, which can be great too
    matModelView = matView * matModel;
    glUniformMatrix4fv(locationMatModelView, 1, GL_FALSE, value_ptr(matModelView));
    glUniformMatrix4fv(locationMatProjection, 1, GL_FALSE, value_ptr(matProjection));
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    /** ESC billentyûre kilépés. */
    if ((action == GLFW_PRESS) && (key == GLFW_KEY_ESCAPE))
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    /** A billentyûk lenyomásának és felengedésének regisztrálása. Lehetővé teszi gombkombinációk használatát. */
    if (action == GLFW_PRESS)
        keyboard[key] = GL_TRUE;
    else if (action == GLFW_RELEASE)
        keyboard[key] = GL_FALSE;
    if (key == GLFW_KEY_H && action == GLFW_PRESS) {
        xDir = true;
        yDir = false;
    }
    if (key == GLFW_KEY_V && action == GLFW_PRESS) {
        xDir = false;
        yDir = true;
    }
    if (key == GLFW_KEY_UP && action == GLFW_PRESS) {
        segmentYOffset += moveStep;
    }
    if (key == GLFW_KEY_DOWN && action == GLFW_PRESS) {
        segmentYOffset -= moveStep;
    }
    if (key == GLFW_KEY_S && action == GLFW_PRESS) {
        moving = true;
        float angleRad = glm::radians(25.0f);
        vx = stepLength * cos(angleRad);
        vy = stepLength * sin(angleRad);
    }
}

void cursorPosCallback(GLFWwindow* window, double xPos, double yPos) {}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {}

int main(void)
{
    init(3, 3, GLFW_OPENGL_COMPAT_PROFILE);

	glfwSetWindowAspectRatio(window, 1, 1);
	glfwSetWindowSizeLimits(window, 600, 600, 600, 600);

    initShaderProgram();

    framebufferSizeCallback(window, windowWidth, windowHeight);

    /** Karakterkódolás a szövegekhez. */
    setlocale(LC_ALL, "");
    cout << "Billentyuk:" << endl;
    cout << "ESC\texit" << endl;
    cout << "H\tX tengelyen mozog" << endl;
    cout << "V\tY tengelyen mozog" << endl;
	cout << "UP\tszakasz mozgatas felfele" << endl;
	cout << "DOWN\tszakasz mozgatas lefele" << endl;
	cout << "S\t25 fokos, 10 px iranyvektor inditas" << endl;

    /** A megadott window struktúra "close flag" vizsgálata. */
    while (!glfwWindowShouldClose(window))
    {
        /** A kód, amellyel rajzolni tudunk a GLFWwindow objektumunkba. */
        display(window, glfwGetTime());
        /** Double buffered mûködés. */
        glfwSwapBuffers(window);
        /** Események kezelése az ablakunkkal kapcsolatban, pl. gombnyomás. */
        glfwPollEvents();
    }
    /** Felesleges objektumok törlése. */
    cleanUpScene(EXIT_SUCCESS);
    /** Kilépés EXIT_SUCCESS kóddal. */
    return EXIT_SUCCESS;
}