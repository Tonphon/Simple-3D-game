#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/shader_m.h>
#include <learnopengl/model.h>

#include <iostream>
#include <vector>
#include <random>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

static const unsigned int SCR_WIDTH = 1280;
static const unsigned int SCR_HEIGHT = 720;


struct Player {
    glm::vec3 pos{ 0.0f, 30.0f, 0.0f };
    glm::vec3 vel{ 0.0f };
    float speed = 10.0f;
    float yawDeg = 0.0f;
    float scale = 0.15f;
    float radius = 0.6f;
} player;

float deltaTime = 0.0f, lastFrame = 0.0f;
bool prevA = false, prevD = false;

unsigned int planeVAO = 0, planeVBO = 0;
void renderPlane() {
    if (!planeVAO) {
        const float s = 50.0f;
        float v[] = {
            // pos      // normal  // uv
            -s,0,-s,    0,1,0,     0,0,
             s,0,-s,    0,1,0,     1,0,
             s,0, s,    0,1,0,     1,1,
             s,0, s,    0,1,0,     1,1,
            -s,0, s,    0,1,0,     0,1,
            -s,0,-s,    0,1,0,     0,0
        };
        glGenVertexArrays(1, &planeVAO);
        glGenBuffers(1, &planeVBO);
        glBindVertexArray(planeVAO);
        glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(v), v, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);                  glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float))); glEnableVertexAttribArray(1);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float))); glEnableVertexAttribArray(2);
    }
    glBindVertexArray(planeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}


struct SphereMesh {
    unsigned int vao = 0, vbo = 0, ebo = 0;
    GLsizei indexCount = 0;
} gSphere;

void buildSphere(SphereMesh& s, int stacks = 16, int slices = 24) {
    std::vector<float> v;
    std::vector<unsigned int> idx;

    for (int i = 0;i <= stacks;i++) {
        float vT = (float)i / stacks;
        float phi = vT * glm::pi<float>();
        float y = cos(phi);
        float r = sin(phi);
        for (int j = 0;j <= slices;j++) {
            float uT = (float)j / slices;
            float theta = uT * glm::two_pi<float>();
            float x = r * cos(theta);
            float z = r * sin(theta);
            // pos
            v.push_back(x); v.push_back(y); v.push_back(z);
            // normal
            v.push_back(x); v.push_back(y); v.push_back(z);
            // uv
            v.push_back(uT); v.push_back(1.0f - vT);
        }
    }
    for (int i = 0;i < stacks;i++) {
        for (int j = 0;j < slices;j++) {
            int a = i * (slices + 1) + j;
            int b = (i + 1) * (slices + 1) + j;
            idx.push_back(a); idx.push_back(b);     idx.push_back(a + 1);
            idx.push_back(b); idx.push_back(b + 1);   idx.push_back(a + 1);
        }
    }
    s.indexCount = (GLsizei)idx.size();

    glGenVertexArrays(1, &s.vao);
    glGenBuffers(1, &s.vbo);
    glGenBuffers(1, &s.ebo);
    glBindVertexArray(s.vao);
    glBindBuffer(GL_ARRAY_BUFFER, s.vbo);
    glBufferData(GL_ARRAY_BUFFER, v.size() * sizeof(float), v.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, s.ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, idx.size() * sizeof(unsigned int), idx.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);                  glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));  glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));  glEnableVertexAttribArray(2);
}

void renderSphere(const SphereMesh& s) {
    glBindVertexArray(s.vao);
    glDrawElements(GL_TRIANGLES, s.indexCount, GL_UNSIGNED_INT, 0);
}

// --------------------------------------------------------------------------------------
// Input
// --------------------------------------------------------------------------------------
void processInput(GLFWwindow* w) {
    if (glfwGetKey(w, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(w, true);
    bool a = glfwGetKey(w, GLFW_KEY_A) == GLFW_PRESS;
    bool d = glfwGetKey(w, GLFW_KEY_D) == GLFW_PRESS;
    if (a && !prevA) player.yawDeg -= 90.0f;
    if (d && !prevD) player.yawDeg += 90.0f;
    prevA = a; prevD = d;
}
void framebuffer_size_callback(GLFWwindow*, int w, int h) { glViewport(0, 0, w, h); }

// --------------------------------------------------------------------------------------
// Texture helper
// --------------------------------------------------------------------------------------
unsigned int loadTexture(const char* path, bool flip = false) {
    stbi_set_flip_vertically_on_load(flip);
    int w, h, n; unsigned char* data = stbi_load(path, &w, &h, &n, 0);
    if (!data) { std::cout << "Failed to load texture: " << path << "\n"; return 0; }
    GLenum fmt = (n == 4) ? GL_RGBA : GL_RGB;
    unsigned int tex; glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, fmt, w, h, 0, fmt, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(data);
    return tex;
}
unsigned int makeWhiteTexture() {
    unsigned int tex; glGenTextures(1, &tex);
    unsigned char px[4] = { 255,255,255,255 };
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, px);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    return tex;
}

// --------------------------------------------------------------------------------------
// spheres
// --------------------------------------------------------------------------------------
struct Coin {
    glm::vec3 pos;
    float     radius;
    glm::vec3 color;
    bool      active = true;
};
std::vector<Coin> coins;

// --------------------------------------------------------------------------------------
// Main
// --------------------------------------------------------------------------------------
int main() {
    // Window / GL
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "TempleRun-Min", nullptr, nullptr);
    if (!window) { std::cout << "GLFW window failed\n"; return -1; }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) { std::cout << "GLAD failed\n"; return -1; }

    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    Shader shader("shaders/templerun.vert", "shaders/templerun.frag");

    unsigned int floorTex = loadTexture("C:/Users/User/source/repos/TempleRun/textures/marble.jpg");
    unsigned int playerTex = loadTexture("C:/Users/User/source/repos/TempleRun/models/player/average_explorer.png");
    Model        playerModel("C:/Users/User/source/repos/TempleRun/models/player/average_explorer.fbx");

    unsigned int whiteTex = makeWhiteTexture();

    buildSphere(gSphere, 16, 24);

    // Spawn 5 random coins above the floor
    std::mt19937 rng(1337);
    std::uniform_real_distribution<float> dPos(-40.0f, 40.0f);
    std::uniform_real_distribution<float> dHue(0.3f, 1.0f);
    for (int i = 0;i < 5;i++) {
        Coin c;
        c.pos = glm::vec3(dPos(rng), 2.0f + (i * 0.2f), dPos(rng));
        c.radius = 2.0f;
        c.color = glm::vec3(dHue(rng), dHue(rng), dHue(rng));
        coins.push_back(c);
    }

    glm::vec3 lightDir = glm::normalize(glm::vec3(0.3f, -1.0f, -0.2f));
    glm::vec3 lightCol(1.0f);

    const float gravity = -20.0f;
    const float floorY = 0.0f;

    while (!glfwWindowShouldClose(window)) {
        float t = (float)glfwGetTime();
        deltaTime = t - lastFrame; lastFrame = t;

        processInput(window);

        glm::vec3 forward = glm::normalize(glm::vec3(
            sin(glm::radians(player.yawDeg)), 0.0f, -cos(glm::radians(player.yawDeg))
        ));

        player.pos += forward * player.speed * deltaTime;

        player.vel.y += gravity * deltaTime;
        player.pos.y += player.vel.y * deltaTime;

        float minY = floorY + player.radius;
        if (player.pos.y < minY) {
            player.pos.y = minY;
            if (player.vel.y < 0.0f) player.vel.y = 0.0f;
        }

        glm::vec3 camPos = player.pos - forward * 8.0f + glm::vec3(0, 6.0f, 0);
        glm::mat4 view = glm::lookAt(camPos, player.pos + forward * 5.0f, glm::vec3(0, 1, 0));
        glm::mat4 proj = glm::perspective(glm::radians(60.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 2000.0f);

        glClearColor(0.10f, 0.12f, 0.16f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.use();
        shader.setMat4("projection", proj);
        shader.setMat4("view", view);
        shader.setVec3("uLightDir", lightDir);
        shader.setVec3("uLightColor", lightCol);
        shader.setVec3("uViewPos", camPos);

        // ----- Floor
        shader.setBool("useOverride", false);
        shader.setFloat("tiling", 30.0f);
        shader.setVec3("uTint", glm::vec3(1.0f));
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, floorTex);
        shader.setInt("texture_diffuse1", 0);
        glm::mat4 M(1.0f);
        shader.setMat4("model", M);
        renderPlane();

        // ----- Player
        shader.setBool("useOverride", true);
        shader.setFloat("tiling", 1.0f);
        shader.setVec3("uTint", glm::vec3(1.0f));
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, playerTex);
        shader.setInt("overrideTex", 1);

        M = glm::mat4(1.0f);
        M = glm::translate(M, player.pos);
        M = glm::rotate(M, glm::radians(player.yawDeg), glm::vec3(0, 1, 0));
        M = glm::scale(M, glm::vec3(player.scale));
        shader.setMat4("model", M);
        playerModel.Draw(shader);

        // ----- Coins
        for (auto& c : coins) {
            if (!c.active) continue;

            shader.setBool("useOverride", true);
            shader.setFloat("tiling", 1.0f);
            shader.setVec3("uTint", c.color);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, whiteTex);
            shader.setInt("overrideTex", 1);

            glm::mat4 S = glm::scale(glm::mat4(1.0f), glm::vec3(c.radius));
            glm::mat4 T = glm::translate(glm::mat4(1.0f), c.pos);
            shader.setMat4("model", T * S);
            renderSphere(gSphere);

            float rSum = player.radius + c.radius;
            if (glm::length(player.pos - c.pos) <= rSum) {
                c.active = false;
            }
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
