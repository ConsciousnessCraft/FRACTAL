/********************************************************/
/*                     CubeVBOShader.cpp                         */
/********************************************************/
/* Premiers pas avec OpenGL.                            */
/* Objectif : afficher a l'ecran uncube avec ou sans shader    */
/********************************************************/

/* inclusion des fichiers d'en-tete Glut */

#include <stdlib.h>
#include <stdio.h>

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <math.h>
#include "shader.hpp"
#include <string.h>
#include <GL/glut.h>
#include <cmath>
#include <cstdlib>

#include <iostream>
#include <stdlib.h>
#include <GL/glut.h>
#include <vector>
#include <sstream>
#include<string>
#include <math.h>
#include<cmath>
#include <random>
#include <armadillo>
// Include GLM
#include "../glm/glm.hpp"
#include "../glm/gtc/matrix_transform.hpp"




using namespace glm;
using namespace std;


#define P_SIZE 3
#define N_SIZE 3		// c'est forcement 3
#define C_SIZE 3

#define N_VERTS  8
#define N_VERTS_BY_FACE  3
#define N_FACES  12

#define NB_R 64
#define NB_r 64
#define BUFFER_OFFSET(i) ((char *)NULL + (i))




// initialisations

void genereVBO();
void deleteVBO();
void traceObjet();

// fonctions de rappel de glut
void affichage();
void clavier(unsigned char, int, int);
void mouse(int, int, int, int);
void mouseMotion(int, int);
void reshape(int, int);
// misc
void drawString(const char* str, int x, int y, float color[4], void* font);
void showInfo();
void* font = GLUT_BITMAP_8_BY_13; // pour afficher des textes 2D sur l'ecran
// variables globales pour OpenGL
bool mouseLeftDown;
bool mouseRightDown;
bool mouseMiddleDown;
float mouseX, mouseY;
float cameraAngleX;
float cameraAngleY;
float cameraDistance = 0.;

// variables Handle d'opengl 
//--------------------------
GLuint programID;
// handle pour le shader
GLuint MatrixIDMVP, MatrixIDView, MatrixIDModel, MatrixIDPerspective;    // handle pour la matrice MVP
GLuint VBO_sommets, VBO_normales, VBO_indices, VBO_UVtext, VBO_colors, VAO;
GLuint locCameraPosition;
GLuint locmaterialShininess;
GLuint locmaterialSpecularColor;
GLuint locmaterialDiffuseColor;
GLuint locmaterialAmbiantColor;
GLuint locLightPosition;
GLuint locSilhouette;
GLuint locShad;
GLuint locLightIntensities;//a.k.a the color of the light
GLuint locLightAttenuation;
GLuint locLightAmbientCoefficient;
GLuint locterraincolor;

// location des VBO
//------------------
GLuint indexVertex = 0, indexUVTexture = 2, indexNormale = 3, indexColor = 1;

//variable pour paramétrage eclairage
//--------------------------------------
vec3 cameraPosition(0., 0., 100.);
// le matériau
//---------------
GLfloat materialShininess = 30.;
vec3 materialSpecularColor(1, 1, 1);  // couleur du materiau
vec3 materialDiffuseColor(0.2, 0.2, 0.2);  // couleur du materiau
vec3 materialAmbiantColor(0.2, 0.2, 0.2);
vec3 terraincolor(0.0, 1.0, 0.0);

float silouette = 0.0;
float shadType = 0.0;
// la lumière
//-----------
vec3 LightPosition(10, 10, 5);
vec3 LightIntensities(1., 1., 1.);// couleur la lumiere
GLfloat LightAttenuation = 1;
GLfloat LightAmbientCoefficient = .1;

glm::mat4 MVP;      // justement la voilà
glm::mat4 Model, View, Projection;    // Matrices constituant MVP



int screenHeight = 500;
int screenWidth = 500;

// pour la texcture
//-------------------
GLuint image;
GLuint bufTexture0, bufTexture1, bufNormalMap;
GLuint locationTexture0, locationTexture1, locationNormalMap;
//-------------------------

//-----------------------------------------FRACTAL--------------------------------------------------
const int N = 6;
// const int tsize = (1 << N) + 1;  // work for linux/windows
const int tsize = pow(2, N) + 1; // only for linux
float terrain[tsize][tsize];
// higher value ----> less mountain height and visa versa
float roughness = 2;

//----------------------------------------------
GLfloat sommets[(tsize) * (tsize) * 3]; // x 3 coordonnées (+1 acr on double les dernierspoints pour avoir des coord de textures <> pour les points de jonctions)
GLuint indices[(tsize-1) * (tsize-1)* 6]; // x6 car pour chaque face quadrangulaire on a 6 indices (2 triangles=2x 3 indices)
GLfloat coordTexture[(tsize) * (tsize) * 2]; // x 2 car U+V par sommets,  not used for our fractal mountain 
GLfloat normales[(tsize) * (tsize) * 3];
GLfloat colors[(tsize) * (tsize) * 3];
//----------------------------------------------

float getRandomNumber(float min, float max) {
    srand(time(NULL));
    std::random_device rd;
    std::default_random_engine eng(rd());
    std::uniform_real_distribution<float> dist(min, max);
    return dist(eng);
}
void initialize() {
    // Set corner points to random 
    terrain[0][0] = rand() % 2;
    terrain[0][tsize - 1] = rand() % 2;
    terrain[tsize - 1][0] = rand() % 8;
    terrain[tsize - 1][tsize - 1] = rand() % 10;
}

void diamondSquare(int step, float roughness) {
    for (int s = step; s > 1; s /= 2) {
        // Square step


        for (int i = 0; i < tsize - 1; i += s) {
            for (int j = 0; j < tsize - 1; j += s) {
                float average = (terrain[i][j] + terrain[i + s][j] + terrain[i][j + s] + terrain[i + s][j + s]) / 4.0;
                float perturbation = getRandomNumber(-1, 1) * s * pow(2, -roughness);
                if (perturbation < -2) perturbation = -2;
                terrain[i + s / 2][j + s / 2] = average + perturbation;
            }
        }
        // Diamond step
        for (int i = 0; i < tsize - 1; i += s / 2) {
            for (int j = (i + (s / 2)) % s; j < tsize - 1; j += s) {
                float sum = 0;
                int count = 0;

                if (i >= s / 2) {
                    sum += terrain[i - s / 2][j];
                    count++;
                }
                if (i + s / 2 < tsize - 1) {
                    sum += terrain[i + s / 2][j];
                    count++;
                }
                if (j >= s / 2) {
                    sum += terrain[i][j - s / 2];
                    count++;
                }
                if (j + s / 2 < tsize - 1) {
                    sum += terrain[i][j + s / 2];
                    count++;
                }

                float average = sum / count;
                //perturbation
                float perturbation = getRandomNumber(-1, 1) * s * pow(2, -roughness);
                if (perturbation < -2) perturbation = -2;
                terrain[i][j] = average + perturbation;
            }
        }
        roughness = roughness / 2;
    }
}

float findMaxHeight(const float terrain[][tsize], int rows, int cols) {
    float maxHeight = -std::numeric_limits<float>::infinity(); 

    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            if (terrain[i][j] > maxHeight) {
                maxHeight = terrain[i][j];
            }
        }
    }

    return maxHeight;
}
float findMinHeight(const float terrain[][tsize], int rows, int cols) {
    float minHeight = std::numeric_limits<float>::infinity(); 

    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            if (terrain[i][j] < minHeight) {
                minHeight = terrain[i][j];
            }
        }
    }

    return minHeight;
}

struct Offset {
    int x;
    int y;
};

// Function to calculate normal
arma::vec calculateNormal(float terrain[][tsize], int i, int j) {

    arma::mat heightMap(tsize, tsize);
    for (int i = 0; i < tsize; ++i) {
        for (int j = 0; j < tsize; ++j) {
            heightMap(i, j) = terrain[i][j];
        }
    }
    // Define the neighbor offsets
    std::vector<Offset> offsets = {
        {-1, -1}, // Top-left
        {0, -1},  // Top
        {1, -1},  // Top-right
        {-1, 0},  // Left
        {1, 0},   // Right
        {-1, 1},  // Bottom-left
        {0, 1},   // Bottom
        {1, 1}    // Bottom-right
    };

    // Initialize variables for the sums of heights in the x and z directions
    double sumX = 0;
    double sumZ = 0;

    // Iterate through the neighboring points
    for (const auto& offset : offsets) {
        int ni = i + offset.x;
        int nj = j + offset.y;

        // Check if the indices are within bounds
        if (ni >= 0 && ni < heightMap.n_cols && nj >= 0 && nj < heightMap.n_rows) {
            // Get the height at the neighboring point
            double height = heightMap(nj, ni);

            // Update the sums of heights in the x and z directions
            sumX += offset.x * height;
            sumZ += offset.y * height;
        }
    }

    // Calculate the normal vector components
    double normalX = -sumX; // Negate for correct direction
    double normalY = 1.0;   // Vertical component
    double normalZ = -sumZ; // Negate for correct direction

    // Normalize the normal vector
    double length = std::sqrt(normalX * normalX + normalY * normalY + normalZ * normalZ);
    return { normalX / length, normalY / length, normalZ / length };
}
// calculate positions, normals, indices, colors for the shader ( ex; phong )
void generateMountainData() {

    float pasU = 1.0 / (tsize-1);
    float pasV = 1.0 /  (tsize-1);
    float snow_H = findMaxHeight(terrain, tsize, tsize);
    float water_H = findMinHeight(terrain, tsize, tsize);
    std::cout << "Maximum height: " << snow_H << std::endl;
    std::cout << "Min height: " << water_H << std::endl;

    for (int i = 0; i < tsize; i++) {
        for (int j = 0; j < tsize; j++) {

            // Calculate vertices

            sommets[(i * tsize + j) * 3] = j;
            sommets[(i * tsize + j) * 3 + 1] = terrain[i][j];
            sommets[(i * tsize + j) * 3 + 2] = i;

            // Calculate normals
            arma::vec normal = calculateNormal(terrain, j, i);
            // Assign the normal to the corresponding vertex
            normales[(i * tsize + j) * 3] = normal(0);
            normales[(i * tsize + j) * 3 + 1] = normal(1);
            normales[(i * tsize + j) * 3 + 2] = normal(2);

            // Calculate texture coordinates (not used for now)
            coordTexture[(i * tsize + j) * 2] = i * pasU;
            coordTexture[(i * tsize + j) * 2 + 1] = j * pasV;

            
            // 6 colors are used  ( white ,blue, 2 levels of green, 2 levels of brown )
            // water 
            if (terrain[i][j] < 0.3 && terrain[i][j + 1] < 0.3 && terrain[i + 1][j] < 0.3 && terrain[i + 1][j + 1] < 0.3) {
                colors[(i * tsize + j) * 3] = 0;
                colors[(i * tsize + j) * 3 + 1] = 0.51;
                colors[(i * tsize + j) * 3 + 2] = 0.89;
            }
            // snow
            else if (terrain[i][j] > snow_H - 8) {
                colors[(i * tsize + j) * 3] = 0.95;
                colors[(i * tsize + j) * 3 + 1] = 0.95;
                colors[(i * tsize + j) * 3 + 2] = 0.99;
            }
            // green ( color1 )
            else if (terrain[i][j] < snow_H - 8 && terrain[i][j]> 5) {
                colors[(i * tsize + j) * 3] = 0.15;
                colors[(i * tsize + j) * 3 + 1] = 0.66;
                colors[(i * tsize + j) * 3 + 2] = 0.03;
            }
            // green ( color2 )
            else if (terrain[i][j] < 5 && terrain[i][j]> 4.5) {
                colors[(i * tsize + j) * 3] = 0.09;
                colors[(i * tsize + j) * 3 + 1] = 0.99;
                colors[(i * tsize + j) * 3 + 2] = 0.25;
            }
            // brown ( color1 )
            else if (terrain[i][j] < 3 && terrain[i][j]> 1) {
                colors[(i * tsize + j) * 3] = 0.57;
                colors[(i * tsize + j) * 3 + 1] = 0.01;
                colors[(i * tsize + j) * 3 + 2] = 0.01;
            }
            // brown ( color2 )
            else {


                colors[(i * tsize + j) * 3] = 0.56;
                colors[(i * tsize + j) * 3 + 1] = 0.23;
                colors[(i * tsize + j) * 3 + 2] = 0.1;
            }

        }
    }

    // Generate indices
    for (int i = 0; i < tsize; i++) {
        for (int j = 0; j < tsize; j++) {
            int mx = tsize - 1;
            indices[(i * mx + j) * 6] = (i * tsize + j);
            indices[(i * mx + j) * 6 + 1] = ((i + 1) * tsize + j);
            indices[(i * mx + j) * 6 + 2] = ((i + 1) * tsize + (j + 1));
            indices[(i * mx + j) * 6 + 3] = (i * tsize + j);
            indices[(i * mx + j) * 6 + 4] = ((i + 1) * tsize + (j + 1));
            indices[(i * mx + j) * 6 + 5] = (i * tsize + (j + 1));

        }
    }

}






//----------------------------------------
GLubyte* glmReadPPM(char* filename, int* width, int* height)
//----------------------------------------
{
    FILE* fp;
    int i, w, h, d;
    unsigned char* image;
    char head[70];          /* max line <= 70 in PPM (per spec). */

    fp = fopen(filename, "rb");
    if (!fp) {
        perror(filename);
        return NULL;
    }

    /* grab first two chars of the file and make sure that it has the
       correct magic cookie for a raw PPM file. */
    fgets(head, 70, fp);
    if (strncmp(head, "P6", 2)) {
        fprintf(stderr, "%s: Not a raw PPM file\n", filename);
        return NULL;
    }

    /* grab the three elements in the header (width, height, maxval). */
    i = 0;
    while (i < 3) {
        fgets(head, 70, fp);
        if (head[0] == '#')     /* skip comments. */
            continue;
        if (i == 0)
            i += sscanf(head, "%d %d %d", &w, &h, &d);
        else if (i == 1)
            i += sscanf(head, "%d %d", &h, &d);
        else if (i == 2)
            i += sscanf(head, "%d", &d);
    }

    /* grab all the image data in one fell swoop. */
    image = new unsigned char[w * h * 3];
    fread(image, sizeof(unsigned char), w * h * 3, fp);
    fclose(fp);

    *width = w;
    *height = h;
    return image;
}

//----------------------------------------
void initTexture(void)
//-----------------------------------------
{
    int iwidth, iheight;
    GLubyte* image = NULL;


    glGenTextures(1, &bufTexture0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, bufTexture0);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);




    char filename6[] = "texture/MetalNRM.ppm";
    image = glmReadPPM(filename6, &iwidth, &iheight);
    glGenTextures(1, &bufTexture1);
    glBindTexture(GL_TEXTURE_2D, bufTexture1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexImage2D(GL_TEXTURE_2D, 0, 3, iwidth, iheight, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
    glGenerateMipmap(GL_TEXTURE_2D);

    locationTexture0 = glGetUniformLocation(programID, "ourTexture0");

    locationTexture1 = glGetUniformLocation(programID, "ourTexture1"); // et il y a la texture elle même  

}
//----------------------------------------
void initOpenGL(void)
//----------------------------------------
{
    glCullFace(GL_BACK); // on spécifie queil faut éliminer les face arriere
    glEnable(GL_CULL_FACE); // on active l'élimination des faces qui par défaut n'est pas active
    glEnable(GL_DEPTH_TEST);
    // le shader

    programID = LoadShaders("PhongShader.vert", "PhongShader.frag");

    // Get  handles for our matrix transformations "MVP" VIEW  MODELuniform
    MatrixIDMVP = glGetUniformLocation(programID, "MVP");
    MatrixIDView = glGetUniformLocation(programID, "VIEW");
    MatrixIDModel = glGetUniformLocation(programID, "MODEL");
    MatrixIDPerspective = glGetUniformLocation(programID, "PERSPECTIVE");

    // Projection matrix : 65 Field of View, 1:1 ratio, display range : 1 unit <-> 1000 units
    // ATTENTIOn l'angle est donné en radians si f GLM_FORCE_RADIANS est défini sinon en degré
    Projection = glm::perspective(glm::radians(60.f), 1.0f, 1.0f, 1000.0f);

    /* on recupere l'ID */
    locCameraPosition = glGetUniformLocation(programID, "cameraPosition");

    locmaterialShininess = glGetUniformLocation(programID, "materialShininess");
    locmaterialSpecularColor = glGetUniformLocation(programID, "materialSpecularColor");
    locmaterialDiffuseColor = glGetUniformLocation(programID, "materialDiffuseColor");
    locmaterialAmbiantColor = glGetUniformLocation(programID, "materialAmbiantColor");
    locLightPosition = glGetUniformLocation(programID, "light.position");
    locLightIntensities = glGetUniformLocation(programID, "light.intensities");//a.k.a the color of the light
    locLightAttenuation = glGetUniformLocation(programID, "light.attenuation");
    locLightAmbientCoefficient = glGetUniformLocation(programID, "light.ambientCoefficient");
    locSilhouette = glGetUniformLocation(programID, "silhouette");
    locShad = glGetUniformLocation(programID, "shadType");
    locterraincolor = glGetUniformLocation(programID, "terraincolor");


}
//----------------------------------------
int main(int argc, char** argv)
//----------------------------------------
{

    /* initialisation de glut et creation
       de la fenetre */

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowPosition(200, 200);
    glutInitWindowSize(screenWidth, screenHeight);
    glutCreateWindow("CUBE VBO SHADER ");


    // Initialize GLEW
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        return -1;
    }

    //info version GLSL
    std::cout << "***** Info GPU *****" << std::endl;
    std::cout << "Fabricant : " << glGetString(GL_VENDOR) << std::endl;
    std::cout << "Carte graphique: " << glGetString(GL_RENDERER) << std::endl;
    std::cout << "Version : " << glGetString(GL_VERSION) << std::endl;
    std::cout << "Version GLSL : " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl << std::endl;

    initOpenGL();


    //createTorus(1., .3);
    
    // generate the mountain
    diamondSquare(64, roughness);
    // generate the mountain's data ( colors, indices, positions, normals )
    generateMountainData();

    // construction des VBO a partir des tableaux du cube deja construit
    genereVBO();
    initTexture();


    /* enregistrement des fonctions de rappel */
    glutDisplayFunc(affichage);
    glutKeyboardFunc(clavier);
    glutReshapeFunc(reshape);
    glutMouseFunc(mouse);
    glutMotionFunc(mouseMotion);

    /* Entree dans la boucle principale glut */
    glutMainLoop();

    glDeleteProgram(programID);
    deleteVBO();
    return 0;
}

void genereVBO()
{


    if (glIsBuffer(VBO_sommets) == GL_TRUE) glDeleteBuffers(1, &VBO_sommets);
    glGenBuffers(1, &VBO_sommets);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_sommets);
    glBufferData(GL_ARRAY_BUFFER, sizeof(sommets), sommets, GL_STATIC_DRAW);

    if (glIsBuffer(VBO_normales) == GL_TRUE) glDeleteBuffers(1, &VBO_normales);
    glGenBuffers(1, &VBO_normales);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_normales);
    glBufferData(GL_ARRAY_BUFFER, sizeof(normales), normales, GL_STATIC_DRAW);
    /// 
    if (glIsBuffer(VBO_indices) == GL_TRUE) glDeleteBuffers(1, &VBO_indices);
    glGenBuffers(1, &VBO_indices); // ATTENTIOn IBO doit etre un GL_ELEMENT_ARRAY_BUFFER
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, VBO_indices);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    if (glIsBuffer(VBO_UVtext) == GL_TRUE) glDeleteBuffers(1, &VBO_UVtext);
    glGenBuffers(1, &VBO_UVtext);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_UVtext);
    glBufferData(GL_ARRAY_BUFFER, sizeof(coordTexture), coordTexture, GL_STATIC_DRAW);

    if (glIsBuffer(VBO_colors) == GL_TRUE) glDeleteBuffers(1, &VBO_colors);
    glGenBuffers(1, &VBO_colors);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_colors);
    glBufferData(GL_ARRAY_BUFFER, sizeof(colors), colors, GL_STATIC_DRAW);

    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO); // ici on bind le VAO , c'est lui qui recupèrera les configurations des VBO glVertexAttribPointer , glEnableVertexAttribArray...
    glEnableVertexAttribArray(indexVertex);
    glEnableVertexAttribArray(indexNormale);
    glEnableVertexAttribArray(indexColor);
    glEnableVertexAttribArray(indexUVTexture);

    glBindBuffer(GL_ARRAY_BUFFER, VBO_sommets);
    glVertexAttribPointer(indexVertex, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

    glBindBuffer(GL_ARRAY_BUFFER, VBO_normales);
    glVertexAttribPointer(indexNormale, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

    glBindBuffer(GL_ARRAY_BUFFER, VBO_colors);
    glVertexAttribPointer(indexColor, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

    glBindBuffer(GL_ARRAY_BUFFER, VBO_UVtext);
    glVertexAttribPointer(indexUVTexture, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, VBO_indices);

    // une fois la config terminée   
    // on désactive le dernier VBO et le VAO pour qu'ils ne soit pas accidentellement modifié 
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

}
//-----------------
void deleteVBO()
//-----------------
{
    glDeleteBuffers(1, &VBO_sommets);
    glDeleteBuffers(1, &VBO_normales);
    glDeleteBuffers(1, &VBO_indices);
    glDeleteBuffers(1, &VBO_UVtext);
    glDeleteBuffers(1, &VBO_colors);
    glDeleteBuffers(1, &VAO);
}



/* fonction d'affichage */
void affichage()
{

    /* effacement de l'image avec la couleur de fond */
   /* Initialisation d'OpenGL */
    glClearColor(0.8, 0.8, 0.8, 0.0);
    glClearDepth(10.0f);                         // 0 is near, >0 is far
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glColor3f(1.0, 1.0, 1);
    glPointSize(2.0);

    View = glm::lookAt(cameraPosition, // Camera is at (0,0,3), in World Space
        glm::vec3(0, 0, 0), // and looks at the origin
        glm::vec3(0, 1, 0)  // Head is up (set to 0,-1,0 to look upside-down)
    );
    Model = glm::mat4(1.0f);
    Model = glm::translate(Model, glm::vec3(0, 0, cameraDistance));
    Model = glm::rotate(Model, glm::radians(cameraAngleX), glm::vec3(1, 0, 0));
    Model = glm::rotate(Model, glm::radians(cameraAngleY), glm::vec3(0, 1, 0));
    Model = glm::scale(Model, glm::vec3(.8, .8, .8));
    MVP = Projection * View * Model;
    traceObjet();        // trace VBO avec ou sans shader

    /* on force l'affichage du resultat */
    glutPostRedisplay();
    glutSwapBuffers();
}




//-------------------------------------
//Trace le tore 2 via le VAO
void traceObjet()
//-------------------------------------
{
    // Use  shader & MVP matrix   MVP = Projection * View * Model;
    glUseProgram(programID);

    //on envoie les données necessaires aux shaders */
    glUniformMatrix4fv(MatrixIDMVP, 1, GL_FALSE, &MVP[0][0]);
    glUniformMatrix4fv(MatrixIDView, 1, GL_FALSE, &View[0][0]);
    glUniformMatrix4fv(MatrixIDModel, 1, GL_FALSE, &Model[0][0]);
    glUniformMatrix4fv(MatrixIDPerspective, 1, GL_FALSE, &Projection[0][0]);

    glUniform3f(locCameraPosition, cameraPosition.x, cameraPosition.y, cameraPosition.z);


    glUniform1f(locmaterialShininess, materialShininess);
    glUniform3f(locmaterialSpecularColor, materialSpecularColor.x, materialSpecularColor.y, materialSpecularColor.z);
    glUniform3f(locmaterialDiffuseColor, materialDiffuseColor.x, materialDiffuseColor.y, materialDiffuseColor.z);
    glUniform3f(locmaterialAmbiantColor, materialAmbiantColor.x, materialAmbiantColor.y, materialAmbiantColor.z);
    glUniform3f(locLightPosition, LightPosition.x, LightPosition.y, LightPosition.z);
    glUniform3f(locLightIntensities, LightIntensities.x, LightIntensities.y, LightIntensities.z);
    glUniform3f(locterraincolor, terraincolor.x, terraincolor.y, terraincolor.z);
    glUniform1f(locLightAttenuation, LightAttenuation);
    glUniform1f(locLightAmbientCoefficient, LightAmbientCoefficient);
    glUniform1f(locSilhouette, silouette);
    glUniform1f(locShad, shadType);
    glUniform1i(locationTexture0, 0);
    glUniform1i(locationTexture1, 1);
    //pour l'affichagetexture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, bufTexture0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, bufTexture1);
    glBindVertexArray(VAO); // on active le VAO
    glDrawElements(GL_TRIANGLES, sizeof(indices), GL_UNSIGNED_INT, 0);// on appelle la fonction dessin 
    glBindVertexArray(0);    // on desactive les VAO
    glUseProgram(0);         // et le pg

}

void reshape(int w, int h)
{
    // set viewport to be the entire window
    glViewport(0, 0, (GLsizei)w, (GLsizei)h);// ATTENTION GLsizei important - indique qu'il faut convertir en entier non négatif

    // set perspective viewing frustum
    float aspectRatio = (float)w / h;

    Projection = glm::perspective(glm::radians(60.0f), (float)(w) / (float)h, 1.0f, 1000.0f);
}


void clavier(unsigned char touche, int x, int y)
{
    switch (touche)
    {


    case 'b': /* for diff versions of mountain */
        diamondSquare(tsize-1, roughness);
        generateMountainData();
        genereVBO();
        glutPostRedisplay();
        break;
    
    case 'f': /* affichage du carre plein */
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glutPostRedisplay();
        break;
    case 'e': /* affichage en mode fil de fer */
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glutPostRedisplay();
        break;
    case 'v': /* Affichage en mode sommets seuls */
        glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
        glutPostRedisplay();
        break;
    case 's': /* Affichage en mode sommets seuls */
        materialShininess -= .5;
        glutPostRedisplay();
        break;
    case 'S': /* Affichage en mode sommets seuls */
        materialShininess += .5;
        glutPostRedisplay();
        break;
    case 'x': /* Affichage en mode sommets seuls */
        LightPosition.x -= .2;
        glutPostRedisplay();
        break;
    case 'X': /* Affichage en mode sommets seuls */
        LightPosition.x += .2;
        glutPostRedisplay();
        break;
    case 'y': /* Affichage en mode sommets seuls */
        LightPosition.y -= .2;
        glutPostRedisplay();
        break;
    case 'Y': /* Affichage en mode sommets seuls */
        LightPosition.y += .2;
        glutPostRedisplay();
        break;
    case 'z': /* Affichage en mode sommets seuls */
        LightPosition.z -= .2;
        glutPostRedisplay();
        break;
    case 'Z': /* Affichage en mode sommets seuls */
        LightPosition.z += .2;
        glutPostRedisplay();
        break;
    case 'a': /* Affichage en mode sommets seuls */
        LightAmbientCoefficient -= .1;
        glutPostRedisplay();
        break;
    case 'A': /* Affichage en mode sommets seuls */
        LightAmbientCoefficient += .1;
        glutPostRedisplay();
        break;


    case 'q': /*la touche 'q' permet de quitter le programme */
        exit(0);
    }
}



void mouse(int button, int state, int x, int y)
{
    mouseX = x;
    mouseY = y;

    if (button == GLUT_LEFT_BUTTON)
    {
        if (state == GLUT_DOWN)
        {
            mouseLeftDown = true;
        }
        else if (state == GLUT_UP)
            mouseLeftDown = false;
    }

    else if (button == GLUT_RIGHT_BUTTON)
    {
        if (state == GLUT_DOWN)
        {
            mouseRightDown = true;
        }
        else if (state == GLUT_UP)
            mouseRightDown = false;
    }

    else if (button == GLUT_MIDDLE_BUTTON)
    {
        if (state == GLUT_DOWN)
        {
            mouseMiddleDown = true;
        }
        else if (state == GLUT_UP)
            mouseMiddleDown = false;
    }
}


void mouseMotion(int x, int y)
{
    if (mouseLeftDown)
    {
        cameraAngleY += (x - mouseX);
        cameraAngleX += (y - mouseY);
        mouseX = x;
        mouseY = y;
    }
    if (mouseRightDown)
    {
        cameraDistance += (y - mouseY) * 0.2f;
        mouseY = y;
    }

    glutPostRedisplay();
}


