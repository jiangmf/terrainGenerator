/*
    COMPSCI 3GC3 Assignment 2
    Name: Mingfei Jiang
    Student Number: 1320376
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h> 
#include <iostream>
#include <vector>
#include <algorithm>
using namespace std;

#ifdef __APPLE__
#  include <OpenGL/gl.h>
#  include <OpenGL/glu.h>
#  include <GLUT/glut.h>
#else
#  include <GL/gl.h>
#  include <GL/glu.h>
#  include <GL/freeglut.h>
#endif

struct Vertex{
    float x;
    float y;
    float z;
};

/************************
     GLOBAL VARIABLES
************************/

// size of the heightmap
int matrixSize = 0;
// matrixSize by matrixSize 2D array of height values;
vector<vector<float> > matrix;
// matrixSize by matrixSize 2D array of normal vectors;
vector<vector<Vertex> > normals;
// camera angleA, used to rotate the camera about the y axis
int angleA = 45;
// camera angleB, used to rotate the camera about the x-z plane
int angleB = 45;

// user toggles
bool lightEnabled = true;
bool smoothShading = true;
char drawMethod = 't'; // toggles between quads and triangles
char displayMode = 'w'; // toggles between wireframe, solid, or both
// toggles between circles algorithm, fault algorithm, or particle diposition
char generationAlgorithm = 'c'; 

// light position
float lightPos1[] = {0,0,0};
float lightPos2[] = {0,0,0};

int simulationSpeed = 100;
int mainWindow, overviewWindow;

// algorithm variables
float disp = 2;
float faultDisp = 0.2;
float particalDisp = 2;

// min/max heights
float minHeight = 0;
float maxHeight = 0;

/**********************
    HELPER FUNCTIONS
***********************/

// calculates the minimum height in the height map 
float getMinHeight(){
    vector<float> mins;
    for (int x=0; x<matrixSize; x++){
        mins.push_back(*min_element(matrix[x].begin(),matrix[x].end()));
    }
    return *min_element(mins.begin(), mins.end());
}

float getMaxHeight(){
    vector<float> maxs;
    for (int x=0; x<matrixSize; x++){
        maxs.push_back(*max_element(matrix[x].begin(),matrix[x].end()));
    }
    return *max_element(maxs.begin(), maxs.end());
}

Vertex reduceToUnitVector(Vertex v){
    // calculate the length of the vector
    float len = sqrt(pow(v.x,2) + pow(v.y,2) + pow(v.z,2));

    if (len == 0.0f)
        len = 1.0f;
 
    // reduce to unit size
    v.x /= len;
    v.y /= len;
    v.z /= len;

    return v;
}

// calculate the normal given 3 vertexs;
Vertex calculateNormal(Vertex p1, Vertex p2, Vertex p3){
    Vertex v1, v2, normal;

    // get the vectors between the points
    v1.x = p2.x - p1.x;
    v1.y = p2.y - p1.y;
    v1.z = p2.z - p1.z;
    v2.x = p3.x - p1.x;
    v2.y = p3.y - p1.y;
    v2.z = p3.z - p1.z;

    // get the cross product of the 2 vectors
    normal.x = ((v1.y * v2.z) - (v1.z * v2.y));
    normal.y = ((v1.z * v2.x) - (v1.x * v2.z));
    normal.z = ((v1.x * v2.y) - (v1.y * v2.x));

    // return the normal reduced to a unit vector
    return reduceToUnitVector(normal);
}

// adds up a list of normals and reduces it to unit size
Vertex sumNormals(vector<Vertex> normals){
    Vertex normal = {0,0,0};

    // add up each compent
    for (int i=0; i<normals.size(); i++){
        normal.x += normals[i].x;
        normal.y += normals[i].y;
        normal.z += normals[i].z;
    }

    // return the sum reduced to a unit vector
    return reduceToUnitVector(normal);
}

void calculateNormals(void){
    // initialize the matrix variable
    normals.clear();

    printf("Calculating Normals...\n");
    // calculate the normals for each vertex
    for(int x=0; x<matrixSize; x++){
        vector<Vertex> _normals;
        for(int z=0; z<matrixSize; z++){
            vector<Vertex> __normals;
            Vertex v1 = {x, matrix[x][z], z};
            // get adjacent faces and calculate normals for them
            if(drawMethod == 't'){
                // adjacent faces using triangles
                if(z+1<matrixSize and x+1<matrixSize){Vertex v2 = {x,   matrix[x][z+1],   z+1}; Vertex v3 = {x+1, matrix[x+1][z],   z  }; __normals.push_back(calculateNormal(v1, v2, v3));}
                if(x+1<matrixSize and z-1>=0        ){Vertex v2 = {x+1, matrix[x+1][z],   z  }; Vertex v3 = {x+1, matrix[x+1][z-1], z-1}; __normals.push_back(calculateNormal(v1, v2, v3));}
                if(x+1<matrixSize and z-1>=0        ){Vertex v2 = {x+1, matrix[x+1][z-1], z-1}; Vertex v3 = {x,   matrix[x][z-1],   z-1}; __normals.push_back(calculateNormal(v1, v2, v3));}
                if(z-1>=0 and x-1>=0                ){Vertex v2 = {x,   matrix[x][z-1],   z-1}; Vertex v3 = {x-1, matrix[x-1][z],   z  }; __normals.push_back(calculateNormal(v1, v2, v3));}
                if(x-1>=0 and z+1<matrixSize        ){Vertex v2 = {x-1, matrix[x-1][z],   z  }; Vertex v3 = {x-1, matrix[x-1][z+1], z+1}; __normals.push_back(calculateNormal(v1, v2, v3));}
                if(x-1>=0 and z+1<matrixSize        ){Vertex v2 = {x-1, matrix[x-1][z+1], z+1}; Vertex v3 = {x,   matrix[x][z+1],   z+1}; __normals.push_back(calculateNormal(v1, v2, v3));}            
            }
            if(drawMethod == 'q'){
                // adjacent faces using quads
                if(z+1<matrixSize and x+1<matrixSize){Vertex v2 = {x,   matrix[x][z+1],   z+1}; Vertex v3 = {x+1, matrix[x+1][z], z  }; __normals.push_back(calculateNormal(v1, v2, v3));}
                if(x+1<matrixSize and z-1>=0        ){Vertex v2 = {x+1, matrix[x+1][z],   z  }; Vertex v3 = {x  , matrix[x][z-1], z-1}; __normals.push_back(calculateNormal(v1, v2, v3));}
                if(z-1>=0 and x-1>=0                ){Vertex v2 = {x,   matrix[x][z-1],   z-1}; Vertex v3 = {x-1, matrix[x-1][z], z  }; __normals.push_back(calculateNormal(v1, v2, v3));}
                if(x-1>=0 and z+1<matrixSize        ){Vertex v2 = {x-1, matrix[x-1][z],   z  }; Vertex v3 = {x  , matrix[x][z+1], z+1}; __normals.push_back(calculateNormal(v1, v2, v3));}
            }
            // printf("vertex %d,%d has %d faces\n", x, z, __normals.size());
            Vertex normal = sumNormals(__normals);
            _normals.push_back(normal);
        }
        normals.push_back(_normals);
    }
}

void generateHeightMap(){
    // initialize the matrix to a matrixSize by matrixSize 2D array of 0s
    matrix.clear();
    for(int x=0; x<matrixSize; x++){
        vector<float> v(matrixSize, 0);
        matrix.push_back(v);
    }
    printf("Generating Heightmap...\n");

    // Generate terrain using the circle algorithm
    if(generationAlgorithm == 'c'){
        // run the algorithm matrixSize*10 times
        for(int i=0; i<matrixSize*10; i++){
            int circleX = rand() % matrixSize;
            int circleZ = rand() % matrixSize;
            int circleSize = rand() % (matrixSize/5);

            for(int x=0; x<matrixSize; x++){
                for(int z=0; z<matrixSize; z++){
                    float pd = sqrt(pow(circleX-x, 2)+pow(circleZ-z, 2))*2/circleSize;
                    if(fabs(pd) <= 1){
                        matrix[x][z] += disp/2 + cos(pd*3.14)*disp/2;
                    }
                }
            }
        }
    // Generate terrain using the fault algorithm
    } else if(generationAlgorithm == 'f'){
        for(int i=0; i<matrixSize*10; i++){
            int v = rand();
            float a = sin(v);
            float b = cos(v);
            float d = sqrt(matrixSize*matrixSize + matrixSize*matrixSize);
            float c = ((float) rand() / (RAND_MAX)) * d - d/2;
            for(int x=0; x<matrixSize; x++){
                for(int z=0; z<matrixSize; z++){
                    if (a*x + b*z - c > 0) 
                        matrix[x][z] += faultDisp;
                    else
                        matrix[x][z] -= faultDisp;
                }
            }
        }
    // Generate terrain using the particle deposition algorithm
    } else if(generationAlgorithm == 'p'){
        srand(time(0));
        float disp = particalDisp*matrixSize/100;
        int x = 0;
        int z = 0;
        for(long i=0; i<pow(matrixSize, 2.5); i++){
            int v = rand() % 7;
            switch(v) {
                case 0: x++;break;
                case 1: x--;break;
                case 2: z++;break;
                case 3: z--;break;
                case 4: x++;z++;break;
                case 5: x++;z--;break;
                case 6: x--;z++;break;
                case 7: x--;z--;break;
            }
            matrix[fabs(x%matrixSize)][fabs(z%matrixSize)] += disp;
            disp *= 0.999985;
        }
    }



    // set the min/max height
    minHeight = getMinHeight();
    maxHeight = getMaxHeight();

    calculateNormals();
}

void drawStrokeText(string s, int x, int y) {
    float mat_ambient[] ={0,0,0,0};
    float mat_diffuse[] ={0,0,0,0};
    float mat_specular[] ={0,0,0,0};
    float mat_emission[] = {0,1,1,0};
    float shine = 51.2f;
    glColor3f(0,1,1);
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, mat_ambient);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat_diffuse);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat_specular);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shine);
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, mat_emission);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
        glLoadIdentity();
        // glutSetWindow(mainWindow);
        gluOrtho2D(0.0, glutGet(GLUT_WINDOW_WIDTH), 0.0, glutGet(GLUT_WINDOW_HEIGHT));
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
            glLoadIdentity();
            glColor3f(0,1,1);
            glRasterPos2i(x, y);
            void * font = GLUT_BITMAP_9_BY_15;

            for (std::string::iterator i = s.begin(); i != s.end(); ++i)
            {
                char c = *i;
                glutBitmapCharacter(font, c);
            }
            glMatrixMode(GL_MODELVIEW);
        glPopMatrix();
        glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glEnable(GL_TEXTURE_2D);
}

//OpenGL functions
void keyboard(unsigned char key, int xIn, int yIn) {
    int mod = glutGetModifiers();
    switch (key)
    {
        case 'q':
        case 27:
            // exit the application
            exit(0);
            break;
        case 'l':
            // toggle lighting
            lightEnabled = !lightEnabled;
            break;
        case 's':
            // toggle between smooth and flat shading
            smoothShading = !smoothShading;
            break;
        case 'r':
            // generate new terrain
            generateHeightMap();
            break;
        case 'w':
            // toggle the display mode between wire frame, solid, or both
            if (displayMode == 'w')
                displayMode = 's';
            else if (displayMode == 's')
                displayMode = 'b';
            else if (displayMode == 'b')
                displayMode = 'w';
            break;
        case 'a':
            // toggle the display mode between wire frame, solid, or both
            if (generationAlgorithm == 'c')
                generationAlgorithm = 'f';
            else if (generationAlgorithm == 'f')
                generationAlgorithm = 'p';
            else if (generationAlgorithm == 'p')
                generationAlgorithm = 'c';
            generateHeightMap();
            break;
        case 't':
            // set drawMethod to triangles
            drawMethod = 't';
            // update normals to reflect change in geometry
            calculateNormals();
            break;
        case 'y':
            // set drawMethod to quads
            drawMethod = 'q';
            // update normals to reflect change in geometry
            calculateNormals();
            break;
        // change Light Position
        case 'u':
            if(mod == GLUT_ACTIVE_ALT){
                lightPos2[2] -=1;
            } else {
                lightPos1[2] -=1;
            }
            break;
        case 'j':
            if(mod == GLUT_ACTIVE_ALT){
                lightPos2[2] +=1;
            } else {
                lightPos1[2] +=1;
            }
            break;
        case 'h':
            if(mod == GLUT_ACTIVE_ALT){
                lightPos2[0] -=1;
            } else {
                lightPos1[0] -=1;
            }
            break;
        case 'k':
            if(mod == GLUT_ACTIVE_ALT){
                lightPos2[0] +=1;
            } else {
                lightPos1[0] +=1;
            }
            break;
        case 'n':
            if(mod == GLUT_ACTIVE_ALT){
                lightPos2[1] -=1;
            } else {
                lightPos1[1] -=1;
            }
            break;
        case 'm':
            if(mod == GLUT_ACTIVE_ALT){
                lightPos2[1] +=1;
            } else {
                lightPos1[1] +=1;
            }
            break;
        case 'p':
            if(mod == GLUT_ACTIVE_ALT){
                int i, j;
                for (i = 0; i < matrixSize; ++i){
                    for (j = 0; j < matrixSize; ++j){
                        printf("%d,%.3f,%d ", i, matrix[i][j], j);
                    }
                    printf("\n");
                }
            } else {
                int i, j;
                for (i = 0; i < matrixSize; ++i){
                    for (j = 0; j < matrixSize; ++j){
                        printf("%.3f,%.3f,%.3f ", normals[i][j].x, normals[i][j].y, normals[i][j].z);
                    }
                    printf("\n");
                }
            }
            break;
    }
}

// Camera controls
void special(int key, int x, int y) {
    switch (key) {
        // rotate camera about the y axis
        case GLUT_KEY_RIGHT:
            angleA--;
            break;
        case GLUT_KEY_LEFT:
            angleA++;
            break;

        // rotate camera about the x-z plane
        case GLUT_KEY_UP:
            if(angleB <90){
                angleB++;
            }
            break;
        case GLUT_KEY_DOWN:
            if(angleB > 0){
                angleB--;
            }
            break;
    }
}

void init(void) {
    glClearColor(0, 0, 0, 0);
    glColor3f(1, 1, 1);

    glEnable(GL_DEPTH_TEST);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45, 1, 1, 1000);
}

void drawTerrain(void) {
    glPushMatrix();
        for (int x=0; x<matrixSize-1; x++){
            // draws the matrix using triangle strips
            if(drawMethod == 't'){
                glBegin(GL_TRIANGLE_STRIP);
            // draws the matrix using quad strips
            } else if (drawMethod == 'q'){
                glBegin(GL_QUAD_STRIP);
            }
            for (int z=0; z<matrixSize; z++){
                float color = (matrix[x][z]-minHeight)/(maxHeight-minHeight);
                glColor3f(color, color, color);
                glNormal3f(normals[x][z].x, normals[x][z].y, normals[x][z].z);
                glVertex3f(x,matrix[x][z],z);

                color = (matrix[x+1][z]-minHeight)/(maxHeight-minHeight);
                glColor3f(color, color, color);
                glNormal3f(normals[x+1][z].x, normals[x+1][z].y, normals[x+1][z].z);
                glVertex3f(x+1,matrix[x+1][z],z);
            }
            glEnd();
        }
    glPopMatrix();
}

void display(void) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    //calculate the camera position

    // camera is positioned matrixSize * 1.5 from where the eye is 
    // (a good number that keeps everything in view)
    float camDistance = matrixSize * 1.5;

    // calculate where the camera looks at
    // it looks at the the center of the matrix at the lowest point
    float lookX = matrixSize/2;
    float lookY = minHeight;
    float lookZ = matrixSize/2;

    // caluclate where the camera is based on the 2 angles and the camera distance
    float eyeX = lookX + camDistance*cos(angleA*3.14/180)*cos(angleB*3.14/180);
    float eyeY = lookY + camDistance*sin(angleB*3.14/180);
    float eyeZ = lookZ + camDistance*sin(angleA*3.14/180)*cos(angleB*3.14/180);

    // add lighting at the specified position if it's enabled
    if(lightEnabled){
        glEnable(GL_LIGHTING);
        glEnable(GL_LIGHT0);
        glEnable(GL_LIGHT1);
        glLightfv(GL_LIGHT0, GL_POSITION, lightPos1);
        glLightfv(GL_LIGHT1, GL_POSITION, lightPos2);
    } else {
        glDisable(GL_LIGHTING);
    }

    // use smooth lighting if it's enabled
    if(smoothShading){
        glShadeModel(GL_SMOOTH);
    } else {
        glShadeModel(GL_FLAT);
    }

    // set up the camera
    gluLookAt(eyeX, eyeY, eyeZ, lookX, lookY, lookZ, 0, 1, 0);
    
    // draw the wire mesh terrain
    if(displayMode == 'w'){
        // a green color with a material with green emision and no ambient/diffuse/specular
        float mat_ambient[] ={0,0,0,0};
        float mat_diffuse[] ={0,0,0,0};
        float mat_specular[] ={0,0,0,0};
        float mat_emission[] = {0,1,0,0};
        float shine = 51.2f;
        glColor3f(0,1,0);
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, mat_ambient);
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat_diffuse);
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat_specular);
        glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shine);
        glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, mat_emission);
        // set polygon mode to line
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        drawTerrain();
    // draw the solid terrain
    } else if (displayMode == 's'){
        // a grey color with a silver like material
        float mat_ambient[] ={ 0.19225f, 0.19225f, 0.19225f, 1.0f };
        float mat_diffuse[] ={ 0.50754f, 0.50754f, 0.50754f, 1.0f};
        float mat_specular[] ={0.508273f, 0.508273f, 0.508273f, 1.0f };
        float mat_emission[] = {0,0,0,0};
        float shine = 51.2f;
        glColor3f(0.5,0.5,0.5);
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, mat_ambient);
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat_diffuse);
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat_specular);
        glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shine);
        glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, mat_emission);
        // set polygon mode to fill
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        drawTerrain();
    // draw both the mesh and the solid terrain
    } else if (displayMode == 'b'){
        // a grey color with a silver like material
        float mat_ambient[] ={ 0.19225f, 0.19225f, 0.19225f, 1.0f };
        float mat_diffuse[] ={ 0.50754f, 0.50754f, 0.50754f, 1.0f};
        float mat_specular[] ={0.508273f, 0.508273f, 0.508273f, 1.0f };
        float mat_emission[] = {0,0,0,0};
        float shine = 51.2f;
        glColor3f(0.5,0.5,0.5);
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, mat_ambient);
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat_diffuse);
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat_specular);
        glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shine);
        glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, mat_emission);
        // set polygon mode to fill
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        drawTerrain();
        // a green color with a material with green emision and no ambient/diffuse/specular
        float mat_ambient2[] ={0,0,0,0};
        float mat_diffuse2[] ={0,0,0,0};
        float mat_specular2[] ={0,0,0,0};
        float mat_emission2[] = {0,1,0,0};
        float shine2 = 51.2f;
        glColor3f(0,1,0);
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, mat_ambient2);
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat_diffuse2);
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat_specular2);
        glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shine2);
        glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, mat_emission2);
        // set polygon mode to line
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        drawTerrain();
    }

    // write the various information on screen
    int windowHeight = glutGet(GLUT_WINDOW_HEIGHT);
    drawStrokeText("Light 1 Position: " + 
        to_string(lightPos1[0]) + ", " +
        to_string(lightPos1[1]) + ", " +
        to_string(lightPos1[2])
        , 10, windowHeight-20);
    drawStrokeText("Light 2 Position: " + 
        to_string(lightPos2[0]) + ", " +
        to_string(lightPos2[1]) + ", " +
        to_string(lightPos2[2])
        , 10, windowHeight-40);
    drawStrokeText("Camera Position: " + 
        to_string(eyeX) + ", " +
        to_string(eyeY) + ", " +
        to_string(eyeZ)
        , 10, windowHeight-60);

    if(lightEnabled){
        drawStrokeText("Light Enabled", 10, 10);
    } else {
        drawStrokeText("Light Disabled", 10, 10);
    }

    if(smoothShading){
        drawStrokeText("Smooth Shading", 10, 30);
    } else {
        drawStrokeText("Flat Shading", 10, 30);
    }

    if(drawMethod == 't'){
        drawStrokeText("Draw Method: Triangles", 10, 50);
    } else {
        drawStrokeText("Draw Method: Quads", 10, 50);
    }

    if(displayMode == 't'){
        drawStrokeText("Drawing: Mesh", 10, 70);
    } else if (displayMode == 's') {
        drawStrokeText("Drawing: Solid", 10, 70);
    } else {
        drawStrokeText("Drawing: Mesh and Solid", 10, 70);
    }

    if(generationAlgorithm == 'c'){
        drawStrokeText("Generated using: Circles", 10, 90);
    } else if (generationAlgorithm == 'f') {
        drawStrokeText("Generated using: Fault", 10, 90);
    } else {
        drawStrokeText("Generated using: Particle Deposition", 10, 90);
    }


    //switch our buffers for a smooth animation
    glutSwapBuffers();
}

void FPS(int val) {
    glutPostRedisplay();
    glutTimerFunc(simulationSpeed, FPS, 0);
}

void reshape(int w, int h) {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45, (float)((w+0.0f)/h), 1, 1000);
    glMatrixMode(GL_MODELVIEW);
    glViewport(0, 0, w, h);
}

void callBackInit() {
    glutDisplayFunc(display);   //registers "display" as the display callback function
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(special);
    glutReshapeFunc(reshape);
    glutTimerFunc(0, FPS, 0);
}

/* main function - program entry point */
int main(int argc, char** argv)
{
    srand(time(0));
    printf("\n\n======================== TERRAIN GENERATER INSTRUCTIONS ========================\n\n");
    printf("  Use the UP and DOWN arrow keys to rotate the camera about the x-axis.\n");
    printf("  Use the LEFT and RIGHT arrow keys to rotate the camera about the y-axis.\n");
    printf("  Use the u and j keys to move LIGHT0 paralell to the x-axis\n");
    printf("  Use the h and k keys to move LIGHT0 paralell to the z-axis\n");
    printf("  Use the n and m keys to move LIGHT0 paralell to the y-axis\n");
    printf("  Press ALT with the u, j, h, k, n, m keys to move LIGHT1 instead of LIGHT0\n");
    printf("  Press a to switch the generation algorithm (This will generate a new terrain).\n");
    printf("  Press r to generate a new random terrain.\n");
    printf("  Press w to toggle between wireframe, solid, or both display modes.\n");
    printf("  Press l to toggle lighting on and off.\n");
    printf("  Press s to toggle between smooth and flat shading.\n");
    printf("  Press t to draw the terrain using triangles.\n");
    printf("  Press y to draw the terrain using quads.\n");
    printf("  Press q or esc to exit the program.\n\n");
    printf("================================================================================\n\n");

    // Get user's input, make sure it's between 50 and 300
    while(!(matrixSize>=50 && matrixSize<=300)){
        printf("Please enter your grid size (min:50, max:300):");
        cin >> matrixSize;
    }

    // Generate the terrain and initialize light positions
    generateHeightMap();
    lightPos1[0] = matrixSize;
    lightPos1[1] = matrixSize;
    lightPos1[2] = matrixSize;
    lightPos2[1] = matrixSize;

    glutInit(&argc, argv);      //starts up GLUT
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);

    glutInitWindowSize(600, 600);
    glutInitWindowPosition(50, 50);
    glutCreateWindow("Terrain");  //creates the main window
    callBackInit();
    init();

    glutMainLoop();             //starts the event glutMainLoop
    return(0);                  //return may not be necessary on all compilers
}