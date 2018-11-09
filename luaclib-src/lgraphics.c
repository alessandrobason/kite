#include "lgraphics.h"
#include "game.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <math.h>

/*
    default:    0,0, 0,1, 1,1, 1,0
    texcoord:   0,0, 0,1, 1,1, 1,0   

    1. width, height
    2. textureID
    3. texture_coord

*/
extern Game *G;

// layout (location = 0) in vec2 origin;
// layout (location = 1) in vec2 texcoord;
// layout (location = 2) in vec4 color;
// layout (location = 3) in vec2 position;
// layout (location = 4) in vec2 scale;
// layout (location = 5) in vec1 rotate;

// #define R(color) (float)(color>>24) & 0xFF 
// #define G(color) (float)(color>>16) & 0xFF 
// #define B(color) (float)(color>>8) & 0xFF 
// #define A(color) (float)(color) & 0xFF 

static int
lsprite(lua_State *L)
{
    uint32_t color;
    float x, y, w, h, r, g, b, a, xs, ys, ro;
    GLuint EBO, VAO, VBO;

    x = luaL_checknumber(L, 1);
    y = luaL_checknumber(L, 2);
    w = luaL_checknumber(L, 3);
    h = luaL_checknumber(L, 4);
    color = luaL_checkinteger(L, 5);
    xs = luaL_checknumber(L, 6);
    ys = luaL_checknumber(L, 7);
    ro = luaL_checknumber(L, 8) * (M_PI/180);

    r = (color>>24) & 0xFF;
    g = (color>>16) & 0xFF;
    b = (color>> 8) & 0xFF;
    a = (color>> 0) & 0xFF;

    float vertices[] = {
    //  direction     wh    texcoord    color     position  scale   rotate
        -1.0f,-1.0f,  w,h,  0.0f,0.0f,  r,g,b,a,  x,y,      xs,ys,  ro,
        -1.0f, 1.0f,  w,h,  0.0f,1.0f,  r,g,b,a,  x,y,      xs,ys,  ro,
         1.0f, 1.0f,  w,h,  1.0f,1.0f,  r,g,b,a,  x,y,      xs,ys,  ro,
         1.0f,-1.0f,  w,h,  1.0f,0.0f,  r,g,b,a,  x,y,      xs,ys,  ro
    };
    
    // float vertices[] = {
    // //  direction     texcoord    color     position  scale   rotate
    //     -1.0f,-1.0f,  0.0f,0.0f,  r,g,b,a,  x,y,      xs,ys,  ro,
    //     -1.0f, 1.0f,  0.0f,1.0f,  r,g,b,a,  x,y,      xs,ys,  ro,
    //     1.0f, 1.0f,  1.0f,1.0f,  r,g,b,a,  x,y,      xs,ys,  ro,
    //     1.0f, -1.0f,  1.0f,0.0f,  r,g,b,a,  x,y,      xs,ys,  ro
    // };

    GLuint indices[] = {
        0,1,2,
        2,3,0
    };

    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    static uint32_t step = 15 * sizeof(float);

    // direction
    glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,step,NULL);
    glEnableVertexAttribArray(0);

    // wh
    glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,step,(void*)(2*sizeof(float)));
    glEnableVertexAttribArray(1);

    // texcoord
    glVertexAttribPointer(2,2,GL_FLOAT,GL_FALSE,step,(void*)(4*sizeof(float)));
    glEnableVertexAttribArray(2);

    // color
    glVertexAttribPointer(3,4,GL_FLOAT,GL_FALSE,step,(void*)(6*sizeof(float)));
    glEnableVertexAttribArray(3);

    // position
    glVertexAttribPointer(4,2,GL_FLOAT,GL_FALSE,step,(void*)(10*sizeof(float)));
    glEnableVertexAttribArray(4);

    // scale
    glVertexAttribPointer(5,2,GL_FLOAT,GL_FALSE,step,(void*)(12*sizeof(float)));
    glEnableVertexAttribArray(5);

    // rotate
    glVertexAttribPointer(6,1,GL_FLOAT,GL_FALSE,step,(void*)(14*sizeof(float)));
    glEnableVertexAttribArray(6);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    lua_pushinteger(L, VAO);

    return 1;
}


static int
ltexture(lua_State *L)
{
    const char *imagename = luaL_checkstring(L, 1);
    GLuint tx = loadbmp(imagename);
    if (tx == 0 || tx == 0xFFFFFFFF) {
        return 0;
    }

    lua_pushinteger(L, tx);
    return 1;
}


static int
ltexture2(lua_State *L)
{
    const char *filename = luaL_checkstring(L, 1);
    stbi_set_flip_vertically_on_load(true);

    GLuint texture;
    int width, height, channel;
    unsigned char *data = stbi_load(filename, &width, &height, &channel, 0);

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    stbi_image_free(data);
    glBindTexture(GL_TEXTURE_2D, 0);

    lua_pushinteger(L, texture);
    return 1;
}


static float transform[] = {
    1, 0, 0, -0.5,
    0, 1, 0, -0.5,
    0, 0, 1, 0,
    0, 0, 0, 1,
};

static int
ldraw(lua_State *L) {
    GLuint vao, texture;
    vao = luaL_checkinteger(L, 1);
    texture = luaL_checkinteger(L, 2);

    glUniformMatrix4fv(G->transform, 1, GL_TRUE, transform);
    glBindVertexArray(vao);
    glBindTexture(GL_TEXTURE_2D, texture);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    return 0;
}


int
lib_graphics(lua_State *L)
{
	luaL_Reg l[] = {
		{"sprite", lsprite},
        {"texture2", ltexture},
        {"texture", ltexture2},
        {"draw", ldraw},
		{NULL, NULL}
	};
	luaL_newlib(L, l);
	return 1;
}