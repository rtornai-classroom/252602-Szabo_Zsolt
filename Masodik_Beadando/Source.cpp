enum eVertexArrayObject {
	VAOVerticesData,
	VAOControlPointsData,
	VAOCount
};
enum eBufferObject {
	VBOVerticesData,
	VBOControlPointsData,
	BOCount
};
enum eProgram {
	QuadScreenProgram,
	ProgramCount
};
enum eTexture {
	NoTexture,
	TextureCount
};

#include <common.cpp>

GLchar		windowTitle[] = "Második Beadandó!";
GLfloat		aspectRatio;
GLuint		isLine;

static vector<vec2> myControlPoints;
static vector<vec2> pointToDraw;
static int draggedIndex = -1;

int NCR(int n, int r) {
	/* binomial coefficient */
	if (r == 0) return 1;

	/* Extra computation saving for large R,
	 using property:
	 N choose R = N choose (N - R) */
	if (r > n / 2) return NCR(n, n - r);

	long res = 1;

	for (int k = 1; k <= r; ++k) {
		res *= n - k + 1;
		res /= k;
	}

	return res;
}

/*
It will be the Bernstein basis polynomial of degree n.
*/
GLfloat blending(GLint n, GLint i, GLfloat t) {
	return NCR(n, i) * pow(t, i) * pow(1.0f - t, n - i);
}

/*
 * Feltölti a pointToDraw vektort a Bézier-görbe pontjaival.
 * Fills pointToDraw with the sampled Bezier curve points.
 */
void drawBezierCurve(const vector<vec2>& controlPoints) {
	pointToDraw.clear();

	glm::vec3	nextPoint;
	GLfloat		t = 0.0f, B;
	GLfloat		increment = 1.0f / 100.0f;

	while (t <= 1.0f + increment * 0.5f) {
		if (t > 1.0f) t = 1.0f;
		nextPoint = glm::vec3(0.0f, 0.0f, 0.0f);
		for (int i = 0; i < controlPoints.size(); i++) {
			B = blending(controlPoints.size() - 1, i, t);
			nextPoint.x += B * controlPoints.at(i).x;
			nextPoint.y += B * controlPoints.at(i).y;
		}

		pointToDraw.push_back(vec2(nextPoint.x, nextPoint.y));
		if (t == 1.0f) break;
		t += increment;
	}
}

void drawCircle(float cx, float cy, float r, int segments = 32) {
	vector<vec2> circle;
	for (int i = 0; i <= segments; i++) {
		float angle = 2.0f * M_PI * i / segments;
		circle.push_back(vec2(cx + r * cos(angle), cy + r * sin(angle)));
	}

	GLuint tmpVAO, tmpVBO;
	glGenVertexArrays(1, &tmpVAO);
	glGenBuffers(1, &tmpVBO);

	glBindVertexArray(tmpVAO);
	glBindBuffer(GL_ARRAY_BUFFER, tmpVBO);
	glBufferData(GL_ARRAY_BUFFER, circle.size() * sizeof(vec2), circle.data(), GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(vec2), 0);

	glDrawArrays(GL_LINE_LOOP, 0, (GLsizei)circle.size());

	glBindVertexArray(0);
	glDeleteBuffers(1, &tmpVBO);
	glDeleteVertexArrays(1, &tmpVAO);
}

void uploadData() {
	drawBezierCurve(myControlPoints);

	glBindVertexArray(VAO[VAOVerticesData]);
	glBindBuffer(GL_ARRAY_BUFFER, BO[VBOVerticesData]);
	glBufferData(GL_ARRAY_BUFFER, pointToDraw.size() * sizeof(vec2), pointToDraw.data(), GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(vec2), 0);

	glBindVertexArray(VAO[VAOControlPointsData]);
	glBindBuffer(GL_ARRAY_BUFFER, BO[VBOControlPointsData]);
	glBufferData(GL_ARRAY_BUFFER, myControlPoints.size() * sizeof(vec2), myControlPoints.data(), GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(vec2), 0);
}

vec2 screenToWorld(double xPos, double yPos) {
	float xNDC = (float)(xPos / windowWidth) * 2.0f - 1.0f;
	float yNDC = (float)(yPos / windowHeight) * 2.0f - 1.0f;
	yNDC = -yNDC;

	glm::mat4 invVP = glm::inverse(matProjection * matView);
	glm::vec4 worldPos = invVP * glm::vec4(xNDC, yNDC, 0.0f, 1.0f);
	return vec2(worldPos.x, worldPos.y);
}

int findNearestControlPoint(vec2 worldPos, float threshold = 0.07f) {
	int   nearest = -1;
	float minDist = threshold;
	for (int i = 0; i < (int)myControlPoints.size(); i++) {
		float d = glm::distance(worldPos, myControlPoints[i]);
		if (d < minDist) {
			minDist = d;
			nearest = i;
		}
	}
	return nearest;
}

void initShaderProgram() {
	ShaderInfo shader_info[ProgramCount][3] = { {
		{ GL_VERTEX_SHADER,   "./vertexShader.glsl"   },
		{ GL_FRAGMENT_SHADER, "./fragmentShader.glsl" },
		{ GL_NONE, nullptr } } };

	for (int programItem = 0; programItem < ProgramCount; programItem++) {
		program[programItem] = LoadShaders(shader_info[programItem]);
		locationMatModel = glGetUniformLocation(program[programItem], "matModel");
		locationMatView = glGetUniformLocation(program[programItem], "matView");
		locationMatProjection = glGetUniformLocation(program[programItem], "matProjection");
		isLine = glGetUniformLocation(program[programItem], "isLine");
	}

	/*

	drawBezierCurve(myControlPoints);


	glBindVertexArray(VAO[VAOVerticesData]);
	glBindBuffer(GL_ARRAY_BUFFER, BO[VBOVerticesData]);
	glBufferData(GL_ARRAY_BUFFER, pointToDraw.size() * sizeof(vec2), pointToDraw.data(), GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(vec2), 0);

	glBindVertexArray(VAO[VAOControlPointsData]);
	glBindBuffer(GL_ARRAY_BUFFER, BO[VBOControlPointsData]);
	glBufferData(GL_ARRAY_BUFFER, myControlPoints.size() * sizeof(vec2), myControlPoints.data(), GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(vec2), 0);

	*/

	glUseProgram(program[QuadScreenProgram]);

	matModel = mat4(1.0);
	matView = lookAt(
		vec3(0.0f, 0.0f, 9.0f),
		vec3(0.0f, 0.0f, 0.0f),
		vec3(0.0f, 1.0f, 0.0f));

	glUniformMatrix4fv(locationMatModel, 1, GL_FALSE, value_ptr(matModel));
	glUniformMatrix4fv(locationMatView, 1, GL_FALSE, value_ptr(matView));
	glUniformMatrix4fv(locationMatProjection, 1, GL_FALSE, value_ptr(matProjection));
	glUniform1i(isLine, 1);

	uploadData();
}

void display(GLFWwindow* window, double currentTime) {
	glClear(GL_COLOR_BUFFER_BIT);

	if (myControlPoints.size() >= 2){
		// Bezier-görbe
		glUniform1i(isLine, 1);
		glLineWidth(2.0f);
		glBindVertexArray(VAO[VAOVerticesData]);
		glDrawArrays(GL_LINE_STRIP, 0, (GLsizei)pointToDraw.size());

		// Kontrollpoligon
		glLineWidth(1.0f);
		glBindVertexArray(VAO[VAOControlPointsData]);
		glDrawArrays(GL_LINE_STRIP, 0, (GLsizei)myControlPoints.size());
	}

	// Kontrollpontok
	glUniform1i(isLine, 0);
	float radius = 0.05f;
	for (const auto& cp : myControlPoints) {
		drawCircle(cp.x, cp.y, radius);
	}
}

void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
	windowWidth = glm::max(width, 1);
	windowHeight = glm::max(height, 1);
	aspectRatio = (float)windowWidth / (float)windowHeight;

	glViewport(0, 0, windowWidth, windowHeight);

	if (windowWidth < windowHeight)
		matProjection = ortho(-worldSize, worldSize, -worldSize / aspectRatio, worldSize / aspectRatio, -100.0, 100.0);
	else
		matProjection = ortho(-worldSize * aspectRatio, worldSize * aspectRatio, -worldSize, worldSize, -100.0, 100.0);

	glUniformMatrix4fv(locationMatProjection, 1, GL_FALSE, value_ptr(matProjection));
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (action == GLFW_PRESS && key == GLFW_KEY_ESCAPE)
		glfwSetWindowShouldClose(window, GLFW_TRUE);

	if (action == GLFW_PRESS)  keyboard[key] = GL_TRUE;
	else if (action == GLFW_RELEASE) keyboard[key] = GL_FALSE;
}

void cursorPosCallback(GLFWwindow* window, double xPos, double yPos) {
	if (draggedIndex >= 0) {
		vec2 worldPos = screenToWorld(xPos, yPos);
		myControlPoints[draggedIndex] = worldPos;
		uploadData();
	}
}
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
	double xPos, yPos;
	glfwGetCursorPos(window, &xPos, &yPos);
	vec2 worldPos = screenToWorld(xPos, yPos);

	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		int idx = findNearestControlPoint(worldPos);
		if (idx >= 0) {
			draggedIndex = idx;
		}
		else {
			myControlPoints.push_back(worldPos);
			uploadData();
		}
	}

	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
		draggedIndex = -1;
	}

	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
		int idx = findNearestControlPoint(worldPos);
		if (idx >= 0) {
			myControlPoints.erase(myControlPoints.begin() + idx);
			uploadData();
		}
	}
}


int main(void) {
	init(3, 3, GLFW_OPENGL_COMPAT_PROFILE);
	initShaderProgram();
	framebufferSizeCallback(window, windowWidth, windowHeight);
	setlocale(LC_ALL, "");

	cout << "Bezier curve" << endl << endl;
	cout << "Bal klikk\t\tKontrollpont hozzaadas" << endl;
	cout << "Bal klikk + huzas\tKontrollpont mozgatasa" << endl;
	cout << "Jobb klikk\t\tKontrollpont torlese" << endl;
	cout << "ESC\t\tKilepes" << endl;

	while (!glfwWindowShouldClose(window)) {
		display(window, glfwGetTime());
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	cleanUpScene(EXIT_SUCCESS);
	return EXIT_SUCCESS;
}