/**
* Author: Yujia Hou
* Assignment: Simple 2D Scene
* Date due: 2025-02-15, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/
#define GL_SILENCE_DEPRECATION
#define STB_IMAGE_IMPLEMENTATION
#define LOG(argument) std::cout << argument << '\n'
#define GL_GLEXT_PROTOTYPES 1

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "stb_image.h"

enum AppStatus { RUNNING, TERMINATED };

constexpr int WINDOW_WIDTH  = 640,
              WINDOW_HEIGHT = 480;


constexpr int VIEWPORT_X      = 0,
              VIEWPORT_Y      = 0,
              VIEWPORT_WIDTH  = WINDOW_WIDTH,
              VIEWPORT_HEIGHT = WINDOW_HEIGHT;

constexpr char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
               F_SHADER_PATH[] = "shaders/fragment_textured.glsl";


constexpr GLint NUMBER_OF_TEXTURES = 1,
                LEVEL_OF_DETAIL    = 0,
                TEXTURE_BORDER     = 0;

// source1: https://pngimg.com/image/36307
// source2: https://www.pngmart.com/image/tag/minions
constexpr char SPRITE_FILEPATH_1[] = "minions1.png",
               SPRITE_FILEPATH_2[] = "minions2.png";

constexpr glm::vec3 INIT_SCALE = glm::vec3(4.0f, 4.5f, 4.0f);
constexpr glm::vec3 INIT_POS_1 = glm::vec3(-5.0f, -8.0f, 0.0f);

constexpr float ROT_INCREMENT = 1.0f;

SDL_Window* g_display_window;
AppStatus g_app_status = RUNNING;
ShaderProgram g_shader_program = ShaderProgram();

glm::mat4 g_view_matrix,
          g_matrix_1,
          g_matrix_2,
          g_projection_matrix;

float g_previous_ticks = 0.0f;

glm::vec3 g_rotation_1 = glm::vec3(0.0f, 0.0f, 0.0f),
          g_rotation_2 = glm::vec3(0.0f, 0.0f, 0.0f);

GLuint g_texture_id_1,
       g_texture_id_2;

glm::vec3 g_position_1 = INIT_POS_1;
//glm::vec3 g_position_2 = g_position_1 + glm::vec3(-1.0f, 1.0f, 0.0f);
glm::vec3 g_position_2 = glm::vec3(0.0f, 0.0f, 0.0f);

constexpr glm::vec3 INIT_POS_2 = glm::vec3(-1.0f, 1.0f, 0.0f);

float scalingFactor = 1.0f;



GLuint load_texture(const char* filepath)
{
    int width, height, number_of_components;
    unsigned char* image = stbi_load(filepath, &width, &height, &number_of_components, STBI_rgb_alpha);

    if (image == NULL)
    {
        LOG("Unable to load image. Make sure the path is correct.");
        assert(false);
    }

    GLuint textureID;
    glGenTextures(NUMBER_OF_TEXTURES, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, LEVEL_OF_DETAIL, GL_RGBA, width, height, TEXTURE_BORDER, GL_RGBA, GL_UNSIGNED_BYTE, image);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    stbi_image_free(image);

    return textureID;
}


void initialise()
{
    SDL_Init(SDL_INIT_VIDEO);

    g_display_window = SDL_CreateWindow("Hello, Minions!", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_OPENGL);

    SDL_GLContext context = SDL_GL_CreateContext(g_display_window);
    SDL_GL_MakeCurrent(g_display_window, context);

    if (g_display_window == nullptr)
    {
        std::cerr << "Error: SDL window could not be created.\n";
        SDL_Quit();
        exit(1);
    }

#ifdef _WINDOWS
    glewInit();
#endif

    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);

    g_shader_program.load(V_SHADER_PATH, F_SHADER_PATH);

    g_matrix_1 = glm::mat4(1.0f);
    g_matrix_2 = glm::mat4(1.0f);
    g_view_matrix = glm::mat4(1.0f);
    g_projection_matrix = glm::ortho(-5.0f * 3, 5.0f * 3, -3.75f * 3, 3.75f * 3, -1.0f, 1.0f);

    g_shader_program.set_projection_matrix(g_projection_matrix);
    g_shader_program.set_view_matrix(g_view_matrix);

    glUseProgram(g_shader_program.get_program_id());

    glClearColor(0.5f, 0.5f, 1.0f, 1.0f);

    g_texture_id_1 = load_texture(SPRITE_FILEPATH_1);
    g_texture_id_2 = load_texture(SPRITE_FILEPATH_2);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}


void process_input()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE)
        {
            g_app_status = TERMINATED;
        }
    }
}


void update()
{
    float ticks = (float) SDL_GetTicks() / 1000.0f;
    float delta_time = ticks - g_previous_ticks;
    g_previous_ticks = ticks;
    

    g_rotation_2.y += ROT_INCREMENT * delta_time;
    g_position_1.x += cos(ticks) * delta_time + 0.01;
    g_position_1.y += sin(ticks) * delta_time;
    g_position_2.x += cos(ticks) * delta_time;
    g_position_2.y += sin(ticks) * delta_time;

    g_matrix_1 = glm::mat4(1.0f);
    g_matrix_2 = glm::mat4(1.0f);
    
    scalingFactor += 0.01f;

    g_matrix_1 = glm::translate(g_matrix_1, g_position_1);
    g_matrix_1 = glm::scale(g_matrix_1, INIT_SCALE+scalingFactor);

    g_matrix_2 = glm::translate(g_matrix_1, INIT_POS_2);
    g_matrix_2 = glm::translate(g_matrix_1, g_position_2);
    g_matrix_2 = glm::rotate(g_matrix_2, g_rotation_2.y, glm::vec3(0.0f, 1.0f, 0.0f));
    g_matrix_2 = glm::scale(g_matrix_2, 0.1f * INIT_SCALE);
    
}


void draw_object(glm::mat4 &object_g_model_matrix, GLuint &object_texture_id)
{
    g_shader_program.set_model_matrix(object_g_model_matrix);
    glBindTexture(GL_TEXTURE_2D, object_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}


void render()
{
    glClear(GL_COLOR_BUFFER_BIT);

    float vertices[] =
    {
        -0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f,
        -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f
    };

    float texture_coordinates[] =
    {
        0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f
    };



    glVertexAttribPointer(g_shader_program.get_position_attribute(), 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(g_shader_program.get_position_attribute());

    glVertexAttribPointer(g_shader_program.get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, texture_coordinates);
    glEnableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());

    draw_object(g_matrix_1, g_texture_id_1);
    draw_object(g_matrix_2, g_texture_id_2);

    glDisableVertexAttribArray(g_shader_program.get_position_attribute());
    glDisableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());

    SDL_GL_SwapWindow(g_display_window);
}


void shutdown() { SDL_Quit(); }


int main(int argc, char* argv[])
{
    initialise();

    while (g_app_status == RUNNING)
    {
        process_input();
        update();
        render();
    }

    shutdown();
    return 0;
}
