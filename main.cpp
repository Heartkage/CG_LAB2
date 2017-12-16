#pragma warning(disable:4996)

#include "mesh.h"
#include "FreeImage.h"
#include "glew.h"
#include "glut.h"
#include "environment.h"
#include<string.h>
#include<math.h>

#define NAMELEN 40
#define TEXTURE_AMOUNT 1
#define OBJECT_AMOUNT 20
#define PI 3.14159265

struct texture_obj {
	string name[TEXTURE_AMOUNT];
	GLuint number[TEXTURE_AMOUNT];
}texture_map;

int check_object_type(char *);
void bind_texture_map(int, int);
void light();
void LoadTexture(char *filename, int);
void LoadCubeMap(char *file1, char *file2, char *file3, char *file4, char *file5, char *file6, int id);
void TextureInit(void);
void display();
void reshape(GLsizei, GLsizei);
void keyboard(unsigned char, int, int);
void draw_image(void);

mesh *object[OBJECT_AMOUNT];
viewing *view;
lighting *lights;
scene *scenes;

int winh, winw;
int Gargc, object_count = 0;
int current_key = 0;
float obj[3];
char file_name[OBJECT_AMOUNT][NAMELEN];

GLfloat zoomfactor = 1.0;
GLfloat rotatefactor = 0.0;

int main(int argc, char *argv[]) {

	Gargc = argc - 1;
	//Handling argv data
	if (argc > OBJECT_AMOUNT) {
		cerr << "Too many Objects" << endl;
		exit(1);
	}	
	else if (argc < 2) {
		cerr << "No input Objects" << endl;
		exit(2);
	}	
	else {
		for (int i = 0; i < argc - 1; i++) {
			int check = check_object_type(argv[i + 1]);

			//0 = obj, 1 = view, 2 = light, 3 = scene, -1 = unknown
			if (check == 0) {
				memset(file_name[object_count], 0, NAMELEN);
				object[object_count] = new mesh(argv[i + 1]);
				strcpy(file_name[object_count], argv[i + 1]);
				object_count++;
			}		
			else if (check == 1) {
				view = new viewing(argv[i + 1]);
				cout << "[Completed] view file -> ok" << endl;
			}		
			else if (check == 2) {
				lights = new lighting(argv[i + 1]);
				cout << "[Completed] light file -> ok" << endl;
			}		
			else if (check == 3) {
				scenes = new scene(argv[i + 1]);
				cout << "[Completed] scene file -> ok" << endl;
			}	
			else {
				cerr << "Unknown object type" << endl;
				exit(3);
			}
				
		}
		cout << "Total objects read = " << object_count << endl << endl;
	}

	//OpenGL 
	cout << "[OpenGL] Start generating objects..." << endl;

	glutInit(&argc, argv);
	glutInitWindowSize(view->viewport[2], view->viewport[3]);
	glutInitWindowPosition(view->viewport[0], view->viewport[1]);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_STENCIL);
	
	glutCreateWindow("Project3_0416250");

	glewInit();
	FreeImage_Initialise();
	TextureInit();
	FreeImage_DeInitialise();

	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);

	glutMainLoop();

	return 0;
}

int check_object_type(char *words) {
	
	const char codeword0[10] = "obj";
	const char codeword1[10] = "view";
	const char codeword2[10] = "light";
	const char codeword3[10] = "scene";

	char code[10];
	char *temp = strtok(words, ".");
	temp = strtok(NULL, "\0");

	if (temp != NULL)
		strcpy(code, temp);
	else {
		cerr << "Object type undefine" << endl;
		exit(4);
	}
		

	if (!strcmp(code, codeword0)) {
		strcat(words, ".obj\0");
		return 0;
	}	
	else if (!strcmp(code, codeword1)) {
		strcat(words, ".view\0");
		return 1;
	}
	else if (!strcmp(code, codeword2)) {
		strcat(words, ".light\0");
		return 2;
	}	
	else if (!strcmp(code, codeword3)) {
		strcat(words, ".scene\0");
		return 3;
	}	
	else
		return -1;
}

void TextureInit(void) {

}

void bind_texture_map(int index, int layer) {
	int i;

	for (i = 0; i < TEXTURE_AMOUNT; i++) {
		if (texture_map.name[i].compare(scenes->obj_lists[index].texture_name[layer]) == 0) {
			cout << "[Binding] " << scenes->obj_lists[index].texture_name[layer] << endl;
			glBindTexture(GL_TEXTURE_2D, texture_map.number[i]);
			break;
		}
	}
	if (i == TEXTURE_AMOUNT) {
		cout << "Error in binding texture map" << endl;
		int temp;
		cin >> temp;
		exit(3);
	}
}

void LoadTexture(char *filename, int num) {
	FIBITMAP *pImage = FreeImage_Load(FreeImage_GetFileType(filename, 0), filename);
	FIBITMAP *p32BitsImage = FreeImage_ConvertTo32Bits(pImage);

	int width = FreeImage_GetWidth(p32BitsImage);
	int height = FreeImage_GetHeight(p32BitsImage);

	texture_map.name[num] = filename;
	glBindTexture(GL_TEXTURE_2D, texture_map.number[num]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height,
		0, GL_BGRA, GL_UNSIGNED_BYTE, (void*)FreeImage_GetBits(p32BitsImage));
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	FreeImage_Unload(p32BitsImage);
	FreeImage_Unload(pImage);

	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 50);

	glBindTexture(GL_TEXTURE_2D, 0);
}

void LoadCubeMap(char *file1, char *file2, char *file3, char *file4, char *file5, char *file6, int id) {
	FIBITMAP *pImage = FreeImage_Load(FreeImage_GetFileType(file1, 0), file1);
	FIBITMAP *p32BitsImage = FreeImage_ConvertTo32Bits(pImage);

	FIBITMAP *pImage2 = FreeImage_Load(FreeImage_GetFileType(file2, 0), file2);
	FIBITMAP *p32BitsImage2 = FreeImage_ConvertTo32Bits(pImage2);

	FIBITMAP *pImage3 = FreeImage_Load(FreeImage_GetFileType(file3, 0), file3);
	FIBITMAP *p32BitsImage3 = FreeImage_ConvertTo32Bits(pImage3);

	FIBITMAP *pImage4 = FreeImage_Load(FreeImage_GetFileType(file4, 0), file4);
	FIBITMAP *p32BitsImage4 = FreeImage_ConvertTo32Bits(pImage4);

	FIBITMAP *pImage5 = FreeImage_Load(FreeImage_GetFileType(file5, 0), file5);
	FIBITMAP *p32BitsImage5 = FreeImage_ConvertTo32Bits(pImage5);

	FIBITMAP *pImage6 = FreeImage_Load(FreeImage_GetFileType(file6, 0), file6);
	FIBITMAP *p32BitsImage6 = FreeImage_ConvertTo32Bits(pImage6);

	int width;
	int height;

	texture_map.name[id] = file1;
	glBindTexture(GL_TEXTURE_CUBE_MAP, texture_map.number[id]);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_LINEAR);

	//image 1
	width = FreeImage_GetWidth(p32BitsImage); height = FreeImage_GetHeight(p32BitsImage);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGBA8, width, height,
		0, GL_BGRA, GL_UNSIGNED_BYTE, (void*)FreeImage_GetBits(p32BitsImage));

	//image 2
	width = FreeImage_GetWidth(p32BitsImage2); height = FreeImage_GetHeight(p32BitsImage2);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGBA8, width, height,
		0, GL_BGRA, GL_UNSIGNED_BYTE, (void*)FreeImage_GetBits(p32BitsImage2));

	//image 3
	width = FreeImage_GetWidth(p32BitsImage3); height = FreeImage_GetHeight(p32BitsImage3);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGBA8, width, height,
		0, GL_BGRA, GL_UNSIGNED_BYTE, (void*)FreeImage_GetBits(p32BitsImage3));

	//image 4
	width = FreeImage_GetWidth(p32BitsImage4); height = FreeImage_GetHeight(p32BitsImage4);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGBA8, width, height,
		0, GL_BGRA, GL_UNSIGNED_BYTE, (void*)FreeImage_GetBits(p32BitsImage4));

	//image 5
	width = FreeImage_GetWidth(p32BitsImage5); height = FreeImage_GetHeight(p32BitsImage5);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGBA8, width, height,
		0, GL_BGRA, GL_UNSIGNED_BYTE, (void*)FreeImage_GetBits(p32BitsImage5));

	//image 6
	width = FreeImage_GetWidth(p32BitsImage6); height = FreeImage_GetHeight(p32BitsImage6);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGBA8, width, height,
		0, GL_BGRA, GL_UNSIGNED_BYTE, (void*)FreeImage_GetBits(p32BitsImage6));

	FreeImage_Unload(p32BitsImage6);
	FreeImage_Unload(pImage6);

	FreeImage_Unload(p32BitsImage5);
	FreeImage_Unload(pImage5);

	FreeImage_Unload(p32BitsImage4);
	FreeImage_Unload(pImage4);

	FreeImage_Unload(p32BitsImage3);
	FreeImage_Unload(pImage3);

	FreeImage_Unload(p32BitsImage2);
	FreeImage_Unload(pImage2);

	FreeImage_Unload(p32BitsImage);
	FreeImage_Unload(pImage);

	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LEVEL, 50);

	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

void keyboard(unsigned char key, int x, int y) {
	switch (key) {
	case 'w': zoomfactor -= 0.1; break;
	case 'W': zoomfactor -= 0.1; break;
	case 's': zoomfactor += 0.1; break;
	case 'S': zoomfactor += 0.1; break;
	case 'd': rotatefactor += 1; if (rotatefactor == 360) rotatefactor = 0; break;
	case 'D': rotatefactor += 1; if (rotatefactor == 360) rotatefactor = 0; break;
	case 'a': rotatefactor -= 1; if (rotatefactor < 0) rotatefactor = 359; break;
	case 'A': rotatefactor -= 1; if (rotatefactor < 0) rotatefactor = 359;  break;
	case 'r': rotatefactor = 0; zoomfactor = 1.0; current_key = 0; break;
	case 'R': rotatefactor = 0; zoomfactor = 1.0; current_key = 0; break;
	case 'b': rotatefactor = 180; current_key = 0; break;
	case 'B': rotatefactor = 180; current_key = 0; break;

	case '1': {
		if (((rotatefactor <= 90) && (rotatefactor >= 0)) || ((rotatefactor >= 270) && (rotatefactor <= 359))) {
			obj[0] = -10; obj[1] = 12; obj[2] = 0; current_key = 1; break;
		}
		else {
			obj[0] = 10; obj[1] = 12; obj[2] = 0; current_key = 1; break;
		}
	}	
	case '2': obj[0] = -70; obj[1] = 12; obj[2] = 0; current_key = 2; break;
	case '3': obj[0] = -170; obj[1] = 12; obj[2] = 0; current_key = 3; break;
	case '0': current_key = 0; break;

	case 27: exit(0); break;
	}

	glutPostRedisplay();
}

void light() {
	glShadeModel(GL_SMOOTH);
	// z buffer enable
	glEnable(GL_DEPTH_TEST);
	// enable lighting
	glEnable(GL_LIGHTING);


	for (int i = 0; i < lights->count; i++) {
		GLfloat light_position[] = { lights->lightsource[i].x, lights->lightsource[i].y, lights->lightsource[i].z, 1.0 };
		GLfloat light_ambient[] = { lights->lightsource[i].ar, lights->lightsource[i].ag, lights->lightsource[i].ab, 1.0 };
		GLfloat light_diffuse[] = { lights->lightsource[i].dr, lights->lightsource[i].dg, lights->lightsource[i].db, 1.0 };
		GLfloat light_specular[] = { lights->lightsource[i].sr, lights->lightsource[i].sg, lights->lightsource[i].sb, 1.0 };

		glEnable(GL_LIGHT0+i);
		glLightfv(GL_LIGHT0+i, GL_POSITION, light_position);
		glLightfv(GL_LIGHT0+i, GL_DIFFUSE, light_diffuse);
		glLightfv(GL_LIGHT0+i, GL_SPECULAR, light_specular);
		glLightfv(GL_LIGHT0+i, GL_AMBIENT, light_ambient);
			
	}

	//ambient
	GLfloat ambient[] = { lights->r, lights->g, lights->b };
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient);
}

void display() {

	// projection transformation
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective((GLdouble)view->angle, (GLfloat)winw / (GLfloat)winh, (GLdouble)view->dnear, (GLdouble)view->dfar);

	glClear(GL_ACCUM_BUFFER_BIT);

	if (current_key != 0) {
		int n = 8;
		float distance = 0;
		float aperture = 0.2;
		float right[3], temp[3], new_up[3];

		//vector = eye-object
		temp[0] = obj[0] - view->eye[0];
		temp[1] = obj[1] - view->eye[1];
		temp[2] = obj[2] - view->eye[2];

		//right = vector cross up
		right[0] = (temp[1] * view->vup[2]) - (temp[2] * view->vup[1]);
		right[1] = (temp[2] * view->vup[0]) - (temp[0] * view->vup[2]);
		right[2] = (temp[0] * view->vup[1]) - (temp[1] * view->vup[0]);

		//Normalize
		distance = sqrt(pow(right[0], 2) + pow(right[1], 2) + pow(right[2], 2));
		right[0] /= distance;
		right[1] /= distance;
		right[2] /= distance;

		for (int i = 0; i < n; i++) {
			// viewing and modeling transformation
			GLfloat bokeh[3];
			bokeh[0] = (right[0] * cos(2 * PI * i / n)) + (view->vup[0] * sin(2 * PI *i / n));
			bokeh[1] = right[1] * cos(2 * PI * i / n) + view->vup[1] * sin(2 * PI *i / n);
			bokeh[2] = right[2] * cos(2 * PI * i / n) + view->vup[2] * sin(2 * PI *i / n);

			glMatrixMode(GL_MODELVIEW);

			glLoadIdentity();
			gluLookAt((view->eye[0] + (aperture*bokeh[0])) * zoomfactor, (view->eye[1] + (aperture*bokeh[1])) * zoomfactor, (view->eye[2] + (aperture*bokeh[2])) * zoomfactor,
				obj[0], obj[1], obj[2],
				view->vup[0], view->vup[1], view->vup[2]);

			glRotated(rotatefactor, 0.f, 1.f, 0.f);
			light();

			draw_image();
			glAccum(i ? GL_ACCUM : GL_LOAD, 1.0 / n);
		}
		glAccum(GL_RETURN, 1);
	}
	else {
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		gluLookAt(view->eye[0] * zoomfactor, view->eye[1] * zoomfactor, view->eye[2] * zoomfactor,
			view->vat[0], view->vat[1], view->vat[2],
			view->vup[0], view->vup[1], view->vup[2]);
		
		glRotated(rotatefactor, 0.f, 1.f, 0.f);
		
		light();
		draw_image();
	}

	glutSwapBuffers();
}

void reshape(GLsizei w, GLsizei h) {
	winw = w;
	winh = h;

	glViewport(view->viewport[0], view->viewport[1], winw, winh);
}

void draw_image(void) {
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClearDepth(1.0f);
	glEnable(GL_STENCIL_TEST);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	int i, j;

	//first mirror area set stencil to 1
	glStencilFunc(GL_ALWAYS, 1, 0xFF);
	glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	glDepthMask(GL_FALSE);
	for (i = 0; i < scenes->count; i++) {
		int lastMaterial = -1;

		for (j = 0; j < object_count; j++) {

			if (!strcmp(scenes->obj_lists[i].object_name, file_name[j])) {
				//only do mirror part
				if (strcmp("Mirror.obj", file_name[j]))
					break;

				for (size_t k = 0; k < object[j]->fTotal; ++k)
				{
					// set material property if this face used different material
					glPushMatrix();

					glTranslatef(scenes->obj_lists[i].trans_x, scenes->obj_lists[i].trans_y, scenes->obj_lists[i].trans_z);
					glRotatef(scenes->obj_lists[i].angle, scenes->obj_lists[i].rotate_x, scenes->obj_lists[i].rotate_y, scenes->obj_lists[i].rotate_z);
					glScalef(scenes->obj_lists[i].scale_x, scenes->obj_lists[i].scale_y, scenes->obj_lists[i].scale_z);

					glBegin(GL_TRIANGLES);
					for (size_t l = 0; l < 3; ++l)
					{
						//textex corrd. object->tList[object->faceList[i][j].t].ptr
						if (scenes->obj_lists[i].texture_amount == 1) {
							glMultiTexCoord2fv(GL_TEXTURE0, object[j]->tList[object[j]->faceList[k][l].t].ptr);
						}
						else if (scenes->obj_lists[i].texture_amount == 2) {
							glMultiTexCoord2fv(GL_TEXTURE0, object[j]->tList[object[j]->faceList[k][l].t].ptr);
							glMultiTexCoord2fv(GL_TEXTURE1, object[j]->tList[object[j]->faceList[k][l].t].ptr);
						}

						glNormal3fv(object[j]->nList[object[j]->faceList[k][l].n].ptr);
						glVertex3fv(object[j]->vList[object[j]->faceList[k][l].v].ptr);
					}
					glEnd();

					glPopMatrix();

				}

				break;
			}

		}

		if (j == object_count) {
			cout << scenes->obj_lists[i].object_name << " Object not found" << endl;
			int temp;
			cin >> temp;
			exit(2);
		}
	}

	//set(stencil buffer) object covered by first mirror back to 0
	glStencilFunc(GL_EQUAL, 1, 0xFF);
	glStencilOp(GL_KEEP, GL_KEEP, GL_DECR);
	for (i = 0; i < scenes->count; i++) {
		int lastMaterial = -1;

		for (j = 0; j < object_count; j++) {
			if (!strcmp(scenes->obj_lists[i].object_name, file_name[j])) {
				if (!strcmp("Mirror.obj", file_name[j]))
					break;

				for (size_t k = 0; k < object[j]->fTotal; ++k)
				{
					glPushMatrix();

					glTranslatef(scenes->obj_lists[i].trans_x, scenes->obj_lists[i].trans_y, scenes->obj_lists[i].trans_z);
					glRotatef(scenes->obj_lists[i].angle, scenes->obj_lists[i].rotate_x, scenes->obj_lists[i].rotate_y, scenes->obj_lists[i].rotate_z);
					glScalef(scenes->obj_lists[i].scale_x, scenes->obj_lists[i].scale_y, scenes->obj_lists[i].scale_z);

					glBegin(GL_TRIANGLES);
					for (size_t l = 0; l < 3; ++l)
					{
						//textex corrd. object->tList[object->faceList[i][j].t].ptr
						if (scenes->obj_lists[i].texture_amount == 1) {
							glMultiTexCoord2fv(GL_TEXTURE0, object[j]->tList[object[j]->faceList[k][l].t].ptr);
						}
						else if (scenes->obj_lists[i].texture_amount == 2) {
							glMultiTexCoord2fv(GL_TEXTURE0, object[j]->tList[object[j]->faceList[k][l].t].ptr);
							glMultiTexCoord2fv(GL_TEXTURE1, object[j]->tList[object[j]->faceList[k][l].t].ptr);
						}

						glNormal3fv(object[j]->nList[object[j]->faceList[k][l].n].ptr);
						glVertex3fv(object[j]->vList[object[j]->faceList[k][l].v].ptr);
					}
					glEnd();

					glPopMatrix();

				}
				break;
			}

		}

		if (j == object_count) {
			cout << scenes->obj_lists[i].object_name << " Object not found" << endl;
			int temp;
			cin >> temp;
			exit(2);
		}
	}

	//draw objects stencil eual to 0
	glStencilFunc(GL_EQUAL, 0, 0xFF);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glDepthMask(GL_TRUE);
	for (i = 0; i < scenes->count; i++) {
		int lastMaterial = -1;

		for (j = 0; j < object_count; j++) {

			if (!strcmp(scenes->obj_lists[i].object_name, file_name[j])) {

				if (!strcmp("Mirror.obj", file_name[j]))
					break;

				if (scenes->obj_lists[i].texture_amount == 1) {
					cout << scenes->obj_lists[i].object_name << endl;
					glActiveTexture(GL_TEXTURE0);
					glEnable(GL_TEXTURE_2D);
					glEnable(GL_ALPHA_TEST);
					glAlphaFunc(GL_GREATER, 0.5f);

					bind_texture_map(i, 0);

					glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
				}
				else if (scenes->obj_lists[i].texture_amount == 2) {

					//bind texture 0
					glActiveTexture(GL_TEXTURE0);
					glEnable(GL_TEXTURE_2D);

					bind_texture_map(i, 0);

					glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
					glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);

					//bind texture 1
					glActiveTexture(GL_TEXTURE1);
					glEnable(GL_TEXTURE_2D);

					bind_texture_map(i, 1);

					glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
					glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);

				}
				else if (scenes->obj_lists[i].texture_amount == 6) {
					//glActiveTexture(GL_TEXTURE0);
					glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP);
					glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP);
					glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP);

					glEnable(GL_TEXTURE_GEN_S);
					glEnable(GL_TEXTURE_GEN_T);
					glEnable(GL_TEXTURE_GEN_R);

					glEnable(GL_TEXTURE_CUBE_MAP);
					glBindTexture(GL_TEXTURE_CUBE_MAP, texture_map.number[0]);

					glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
				}

				for (size_t k = 0; k < object[j]->fTotal; ++k)
				{
					// set material property if this face used different material
					if (lastMaterial != object[j]->faceList[k].m)
					{
						lastMaterial = (int)object[j]->faceList[k].m;
						glMaterialfv(GL_FRONT, GL_AMBIENT, object[j]->mList[lastMaterial].Ka);
						glMaterialfv(GL_FRONT, GL_DIFFUSE, object[j]->mList[lastMaterial].Kd);
						glMaterialfv(GL_FRONT, GL_SPECULAR, object[j]->mList[lastMaterial].Ks);
						glMaterialfv(GL_FRONT, GL_SHININESS, &object[j]->mList[lastMaterial].Ns);

						//you can obtain the texture name by object->mList[lastMaterial].map_Kd
						//load them once in the main function before mainloop
						//bind them in display function here	

					}
					glPushMatrix();

					//printf("angle = %f, %f %f %f\n", scenes->obj_lists[i].angle, scenes->obj_lists[i].rotate_x, scenes->obj_lists[i].rotate_y, scenes->obj_lists[i].rotate_z);
					glTranslatef(scenes->obj_lists[i].trans_x, scenes->obj_lists[i].trans_y, scenes->obj_lists[i].trans_z);
					glRotatef(scenes->obj_lists[i].angle, scenes->obj_lists[i].rotate_x, scenes->obj_lists[i].rotate_y, scenes->obj_lists[i].rotate_z);
					glScalef(scenes->obj_lists[i].scale_x, scenes->obj_lists[i].scale_y, scenes->obj_lists[i].scale_z);

					glBegin(GL_TRIANGLES);
					for (size_t l = 0; l < 3; ++l)
					{
						//textex corrd. object->tList[object->faceList[i][j].t].ptr
						if (scenes->obj_lists[i].texture_amount == 1) {
							glMultiTexCoord2fv(GL_TEXTURE0, object[j]->tList[object[j]->faceList[k][l].t].ptr);
						}
						else if (scenes->obj_lists[i].texture_amount == 2) {
							glMultiTexCoord2fv(GL_TEXTURE0, object[j]->tList[object[j]->faceList[k][l].t].ptr);
							glMultiTexCoord2fv(GL_TEXTURE1, object[j]->tList[object[j]->faceList[k][l].t].ptr);
						}

						glNormal3fv(object[j]->nList[object[j]->faceList[k][l].n].ptr);
						glVertex3fv(object[j]->vList[object[j]->faceList[k][l].v].ptr);
					}
					glEnd();

					glPopMatrix();

				}

				if (scenes->obj_lists[i].texture_amount == 1) {
					glActiveTexture(GL_TEXTURE0);
					glDisable(GL_TEXTURE_2D);
					glDisable(GL_ALPHA_TEST);
					glBindTexture(GL_TEXTURE_2D, 0);
				}
				else if (scenes->obj_lists[i].texture_amount == 2) {
					//unbind texture 1
					glActiveTexture(GL_TEXTURE1);
					glDisable(GL_TEXTURE_2D);
					glBindTexture(GL_TEXTURE_2D, 0);

					//unbind texture 0
					glActiveTexture(GL_TEXTURE0);
					glDisable(GL_TEXTURE_2D);
					glBindTexture(GL_TEXTURE_2D, 0);
				}
				else if (scenes->obj_lists[i].texture_amount == 6) {
					glDisable(GL_TEXTURE_GEN_S);
					glDisable(GL_TEXTURE_GEN_T);
					glDisable(GL_TEXTURE_GEN_R);
					glDisable(GL_TEXTURE_CUBE_MAP);

					//glActiveTexture(GL_TEXTURE0);
					glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
				}

				break;
			}

		}

		if (j == object_count) {
			cout << scenes->obj_lists[i].object_name << " Object not found" << endl;
			int temp;
			cin >> temp;
			exit(2);
		}
	}

	//draw reflected image which is stencil equal to 1    !!!!!!!
	glStencilFunc(GL_EQUAL, 1, 0xFF);
	for (i = 0; i < scenes->count; i++) {
		int lastMaterial = -1;
		for (j = 0; j < object_count; j++) {
			if (!strcmp(scenes->obj_lists[i].object_name, file_name[j])) {
				if (!strcmp("Mirror.obj", file_name[j]))
					break;

				for (size_t k = 0; k < object[j]->fTotal; ++k)
				{
					// set material property if this face used different material
					if (lastMaterial != object[j]->faceList[k].m)
					{
						lastMaterial = (int)object[j]->faceList[k].m;
						glMaterialfv(GL_FRONT, GL_AMBIENT, object[j]->mList[lastMaterial].Ka);
						glMaterialfv(GL_FRONT, GL_DIFFUSE, object[j]->mList[lastMaterial].Kd);
						glMaterialfv(GL_FRONT, GL_SPECULAR, object[j]->mList[lastMaterial].Ks);
						glMaterialfv(GL_FRONT, GL_SHININESS, &object[j]->mList[lastMaterial].Ns);

						//you can obtain the texture name by object->mList[lastMaterial].map_Kd
						//load them once in the main function before mainloop
						//bind them in display function here	

					}

					glPushMatrix();

					//printf("angle = %f, %f %f %f\n", scenes->obj_lists[i].angle, scenes->obj_lists[i].rotate_x, scenes->obj_lists[i].rotate_y, scenes->obj_lists[i].rotate_z);

					float temp;

					if (((rotatefactor <= 90) && (rotatefactor >= 0)) || ((rotatefactor >= 270) && (rotatefactor <= 359)))
						temp = 80;
					else
						temp = -80;

					glScalef(-1, 1, 1);

					glTranslatef(scenes->obj_lists[i].trans_x + temp, scenes->obj_lists[i].trans_y, scenes->obj_lists[i].trans_z);
					glRotatef(scenes->obj_lists[i].angle, scenes->obj_lists[i].rotate_x, scenes->obj_lists[i].rotate_y, scenes->obj_lists[i].rotate_z);
					glScalef(scenes->obj_lists[i].scale_x, scenes->obj_lists[i].scale_y, scenes->obj_lists[i].scale_z);

					glBegin(GL_TRIANGLES);
					for (size_t l = 0; l < 3; ++l)
					{
						//textex corrd. object->tList[object->faceList[i][j].t].ptr
						if (scenes->obj_lists[i].texture_amount == 1) {
							glMultiTexCoord2fv(GL_TEXTURE0, object[j]->tList[object[j]->faceList[k][l].t].ptr);
						}
						else if (scenes->obj_lists[i].texture_amount == 2) {
							glMultiTexCoord2fv(GL_TEXTURE0, object[j]->tList[object[j]->faceList[k][l].t].ptr);
							glMultiTexCoord2fv(GL_TEXTURE1, object[j]->tList[object[j]->faceList[k][l].t].ptr);
						}

						glNormal3fv(object[j]->nList[object[j]->faceList[k][l].n].ptr);
						glVertex3fv(object[j]->vList[object[j]->faceList[k][l].v].ptr);
					}
					glEnd();


					glPopMatrix();

				}

				break;
			}

		}

		if (j == object_count) {
			cout << scenes->obj_lists[i].object_name << " Object not found" << endl;
			int temp;
			cin >> temp;
			exit(2);
		}
	}

	//second mirror area set stencil to 2
	glStencilFunc(GL_EQUAL, 1, 0xFF);
	glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	glDepthMask(GL_FALSE);
	for (i = 0; i < scenes->count; i++) {
		int lastMaterial = -1;

		for (j = 0; j < object_count; j++) {

			if (!strcmp(scenes->obj_lists[i].object_name, file_name[j])) {
				//only do mirror part
				if (strcmp("Mirror.obj", file_name[j]))
					break;

				for (size_t k = 0; k < object[j]->fTotal; ++k)
				{
					// set material property if this face used different material
					glPushMatrix();

					glTranslatef(scenes->obj_lists[i].trans_x, scenes->obj_lists[i].trans_y, scenes->obj_lists[i].trans_z);
					glRotatef(scenes->obj_lists[i].angle, scenes->obj_lists[i].rotate_x, scenes->obj_lists[i].rotate_y, scenes->obj_lists[i].rotate_z);
					glScalef(scenes->obj_lists[i].scale_x, scenes->obj_lists[i].scale_y, scenes->obj_lists[i].scale_z);

					glBegin(GL_TRIANGLES);
					for (size_t l = 0; l < 3; ++l)
					{
						//textex corrd. object->tList[object->faceList[i][j].t].ptr
						if (scenes->obj_lists[i].texture_amount == 1) {
							glMultiTexCoord2fv(GL_TEXTURE0, object[j]->tList[object[j]->faceList[k][l].t].ptr);
						}
						else if (scenes->obj_lists[i].texture_amount == 2) {
							glMultiTexCoord2fv(GL_TEXTURE0, object[j]->tList[object[j]->faceList[k][l].t].ptr);
							glMultiTexCoord2fv(GL_TEXTURE1, object[j]->tList[object[j]->faceList[k][l].t].ptr);
						}

						glNormal3fv(object[j]->nList[object[j]->faceList[k][l].n].ptr);
						glVertex3fv(object[j]->vList[object[j]->faceList[k][l].v].ptr);
					}
					glEnd();

					glPopMatrix();

				}

				break;
			}

		}

		if (j == object_count) {
			cout << scenes->obj_lists[i].object_name << " Object not found" << endl;
			int temp;
			cin >> temp;
			exit(2);
		}
	}

	//set(stencil buffer) object covered by second mirror back to 1
	glStencilFunc(GL_EQUAL, 2, 0xFF);
	glStencilOp(GL_KEEP, GL_KEEP, GL_DECR);
	for (i = 0; i < scenes->count; i++) {
		int lastMaterial = -1;

		for (j = 0; j < object_count; j++) {
			if (!strcmp(scenes->obj_lists[i].object_name, file_name[j])) {
				if (!strcmp("Mirror.obj", file_name[j]))
					break;

				for (size_t k = 0; k < object[j]->fTotal; ++k)
				{
					glPushMatrix();

					glTranslatef(scenes->obj_lists[i].trans_x, scenes->obj_lists[i].trans_y, scenes->obj_lists[i].trans_z);
					glRotatef(scenes->obj_lists[i].angle, scenes->obj_lists[i].rotate_x, scenes->obj_lists[i].rotate_y, scenes->obj_lists[i].rotate_z);
					glScalef(scenes->obj_lists[i].scale_x, scenes->obj_lists[i].scale_y, scenes->obj_lists[i].scale_z);

					glBegin(GL_TRIANGLES);
					for (size_t l = 0; l < 3; ++l)
					{
						//textex corrd. object->tList[object->faceList[i][j].t].ptr
						if (scenes->obj_lists[i].texture_amount == 1) {
							glMultiTexCoord2fv(GL_TEXTURE0, object[j]->tList[object[j]->faceList[k][l].t].ptr);
						}
						else if (scenes->obj_lists[i].texture_amount == 2) {
							glMultiTexCoord2fv(GL_TEXTURE0, object[j]->tList[object[j]->faceList[k][l].t].ptr);
							glMultiTexCoord2fv(GL_TEXTURE1, object[j]->tList[object[j]->faceList[k][l].t].ptr);
						}

						glNormal3fv(object[j]->nList[object[j]->faceList[k][l].n].ptr);
						glVertex3fv(object[j]->vList[object[j]->faceList[k][l].v].ptr);
					}
					glEnd();

					glPopMatrix();

				}
				break;
			}

		}

		if (j == object_count) {
			cout << scenes->obj_lists[i].object_name << " Object not found" << endl;
			int temp;
			cin >> temp;
			exit(2);
		}
	}

	//draw reflected image which is stencil equal to 2
	glStencilFunc(GL_EQUAL, 2, 0xFF);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glDepthMask(GL_TRUE);
	for (i = 0; i < scenes->count; i++) {
		int lastMaterial = -1;
		for (j = 0; j < object_count; j++) {
			if (!strcmp(scenes->obj_lists[i].object_name, file_name[j])) {
				if (!strcmp("Mirror.obj", file_name[j]))
					break;

				for (size_t k = 0; k < object[j]->fTotal; ++k)
				{
					// set material property if this face used different material
					if (lastMaterial != object[j]->faceList[k].m)
					{
						lastMaterial = (int)object[j]->faceList[k].m;
						glMaterialfv(GL_FRONT, GL_AMBIENT, object[j]->mList[lastMaterial].Ka);
						glMaterialfv(GL_FRONT, GL_DIFFUSE, object[j]->mList[lastMaterial].Kd);
						glMaterialfv(GL_FRONT, GL_SPECULAR, object[j]->mList[lastMaterial].Ks);
						glMaterialfv(GL_FRONT, GL_SHININESS, &object[j]->mList[lastMaterial].Ns);

						//you can obtain the texture name by object->mList[lastMaterial].map_Kd
						//load them once in the main function before mainloop
						//bind them in display function here	

					}

					glPushMatrix();

					//printf("angle = %f, %f %f %f\n", scenes->obj_lists[i].angle, scenes->obj_lists[i].rotate_x, scenes->obj_lists[i].rotate_y, scenes->obj_lists[i].rotate_z);

					float temp;

					if (((rotatefactor <= 90) && (rotatefactor >= 0)) || ((rotatefactor >= 270) && (rotatefactor <= 359)))
						temp = -160;
					else
						temp = 160;

					glTranslatef(scenes->obj_lists[i].trans_x + temp, scenes->obj_lists[i].trans_y, scenes->obj_lists[i].trans_z);
					glRotatef(scenes->obj_lists[i].angle, scenes->obj_lists[i].rotate_x, scenes->obj_lists[i].rotate_y, scenes->obj_lists[i].rotate_z);
					glScalef(scenes->obj_lists[i].scale_x, scenes->obj_lists[i].scale_y, scenes->obj_lists[i].scale_z);

					glBegin(GL_TRIANGLES);
					for (size_t l = 0; l < 3; ++l)
					{
						//textex corrd. object->tList[object->faceList[i][j].t].ptr
						if (scenes->obj_lists[i].texture_amount == 1) {
							glMultiTexCoord2fv(GL_TEXTURE0, object[j]->tList[object[j]->faceList[k][l].t].ptr);
						}
						else if (scenes->obj_lists[i].texture_amount == 2) {
							glMultiTexCoord2fv(GL_TEXTURE0, object[j]->tList[object[j]->faceList[k][l].t].ptr);
							glMultiTexCoord2fv(GL_TEXTURE1, object[j]->tList[object[j]->faceList[k][l].t].ptr);
						}

						glNormal3fv(object[j]->nList[object[j]->faceList[k][l].n].ptr);
						glVertex3fv(object[j]->vList[object[j]->faceList[k][l].v].ptr);
					}
					glEnd();


					glPopMatrix();

				}

				break;
			}

		}

		if (j == object_count) {
			cout << scenes->obj_lists[i].object_name << " Object not found" << endl;
			int temp;
			cin >> temp;
			exit(2);
		}
	}


	//third mirror area set stencil to 3
	glStencilFunc(GL_EQUAL, 2, 0xFF);
	glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	glDepthMask(GL_FALSE);
	for (i = 0; i < scenes->count; i++) {
		int lastMaterial = -1;
		for (j = 0; j < object_count; j++) {

			if (!strcmp(scenes->obj_lists[i].object_name, file_name[j])) {
				//only do mirror part
				if (strcmp("Mirror.obj", file_name[j]))
					break;

				for (size_t k = 0; k < object[j]->fTotal; ++k)
				{
					// set material property if this face used different material
					glPushMatrix();

					glTranslatef(scenes->obj_lists[i].trans_x, scenes->obj_lists[i].trans_y, scenes->obj_lists[i].trans_z);
					glRotatef(scenes->obj_lists[i].angle, scenes->obj_lists[i].rotate_x, scenes->obj_lists[i].rotate_y, scenes->obj_lists[i].rotate_z);
					glScalef(scenes->obj_lists[i].scale_x, scenes->obj_lists[i].scale_y, scenes->obj_lists[i].scale_z);

					glBegin(GL_TRIANGLES);
					for (size_t l = 0; l < 3; ++l)
					{
						//textex corrd. object->tList[object->faceList[i][j].t].ptr
						if (scenes->obj_lists[i].texture_amount == 1) {
							glMultiTexCoord2fv(GL_TEXTURE0, object[j]->tList[object[j]->faceList[k][l].t].ptr);
						}
						else if (scenes->obj_lists[i].texture_amount == 2) {
							glMultiTexCoord2fv(GL_TEXTURE0, object[j]->tList[object[j]->faceList[k][l].t].ptr);
							glMultiTexCoord2fv(GL_TEXTURE1, object[j]->tList[object[j]->faceList[k][l].t].ptr);
						}

						glNormal3fv(object[j]->nList[object[j]->faceList[k][l].n].ptr);
						glVertex3fv(object[j]->vList[object[j]->faceList[k][l].v].ptr);
					}
					glEnd();

					glPopMatrix();

				}

				break;
			}

		}

		if (j == object_count) {
			cout << scenes->obj_lists[i].object_name << " Object not found" << endl;
			int temp;
			cin >> temp;
			exit(2);
		}
	}

	//set(stencil buffer) object covered by third mirror back to 2
	glStencilFunc(GL_EQUAL, 3, 0xFF);
	glStencilOp(GL_KEEP, GL_KEEP, GL_DECR);
	for (i = 0; i < scenes->count; i++) {
		int lastMaterial = -1;

		for (j = 0; j < object_count; j++) {
			if (!strcmp(scenes->obj_lists[i].object_name, file_name[j])) {
				if (!strcmp("Mirror.obj", file_name[j]))
					break;

				for (size_t k = 0; k < object[j]->fTotal; ++k)
				{
					glPushMatrix();

					glTranslatef(scenes->obj_lists[i].trans_x, scenes->obj_lists[i].trans_y, scenes->obj_lists[i].trans_z);
					glRotatef(scenes->obj_lists[i].angle, scenes->obj_lists[i].rotate_x, scenes->obj_lists[i].rotate_y, scenes->obj_lists[i].rotate_z);
					glScalef(scenes->obj_lists[i].scale_x, scenes->obj_lists[i].scale_y, scenes->obj_lists[i].scale_z);

					glBegin(GL_TRIANGLES);
					for (size_t l = 0; l < 3; ++l)
					{
						//textex corrd. object->tList[object->faceList[i][j].t].ptr
						if (scenes->obj_lists[i].texture_amount == 1) {
							glMultiTexCoord2fv(GL_TEXTURE0, object[j]->tList[object[j]->faceList[k][l].t].ptr);
						}
						else if (scenes->obj_lists[i].texture_amount == 2) {
							glMultiTexCoord2fv(GL_TEXTURE0, object[j]->tList[object[j]->faceList[k][l].t].ptr);
							glMultiTexCoord2fv(GL_TEXTURE1, object[j]->tList[object[j]->faceList[k][l].t].ptr);
						}

						glNormal3fv(object[j]->nList[object[j]->faceList[k][l].n].ptr);
						glVertex3fv(object[j]->vList[object[j]->faceList[k][l].v].ptr);
					}
					glEnd();

					glPopMatrix();

				}
				break;
			}

		}

		if (j == object_count) {
			cout << scenes->obj_lists[i].object_name << " Object not found" << endl;
			int temp;
			cin >> temp;
			exit(2);
		}
	}

	//draw reflected image which is stencil equal to 3
	glStencilFunc(GL_EQUAL, 3, 0xFF);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glDepthMask(GL_TRUE);
	for (i = 0; i < scenes->count; i++) {
		int lastMaterial = -1;
		for (j = 0; j < object_count; j++) {
			if (!strcmp(scenes->obj_lists[i].object_name, file_name[j])) {
				if (!strcmp("Mirror.obj", file_name[j]))
					break;

				for (size_t k = 0; k < object[j]->fTotal; ++k)
				{
					// set material property if this face used different material
					if (lastMaterial != object[j]->faceList[k].m)
					{
						lastMaterial = (int)object[j]->faceList[k].m;
						glMaterialfv(GL_FRONT, GL_AMBIENT, object[j]->mList[lastMaterial].Ka);
						glMaterialfv(GL_FRONT, GL_DIFFUSE, object[j]->mList[lastMaterial].Kd);
						glMaterialfv(GL_FRONT, GL_SPECULAR, object[j]->mList[lastMaterial].Ks);
						glMaterialfv(GL_FRONT, GL_SHININESS, &object[j]->mList[lastMaterial].Ns);

						//you can obtain the texture name by object->mList[lastMaterial].map_Kd
						//load them once in the main function before mainloop
						//bind them in display function here	

					}

					glPushMatrix();

					//printf("angle = %f, %f %f %f\n", scenes->obj_lists[i].angle, scenes->obj_lists[i].rotate_x, scenes->obj_lists[i].rotate_y, scenes->obj_lists[i].rotate_z);

					float temp;

					if (((rotatefactor <= 90) && (rotatefactor >= 0)) || ((rotatefactor >= 270) && (rotatefactor <= 359)))
						temp = 240;
					else
						temp = -240;

					glScalef(-1, 1, 1);

					glTranslatef(scenes->obj_lists[i].trans_x + temp, scenes->obj_lists[i].trans_y, scenes->obj_lists[i].trans_z);
					glRotatef(scenes->obj_lists[i].angle, scenes->obj_lists[i].rotate_x, scenes->obj_lists[i].rotate_y, scenes->obj_lists[i].rotate_z);
					glScalef(scenes->obj_lists[i].scale_x, scenes->obj_lists[i].scale_y, scenes->obj_lists[i].scale_z);

					glBegin(GL_TRIANGLES);
					for (size_t l = 0; l < 3; ++l)
					{
						//textex corrd. object->tList[object->faceList[i][j].t].ptr
						if (scenes->obj_lists[i].texture_amount == 1) {
							glMultiTexCoord2fv(GL_TEXTURE0, object[j]->tList[object[j]->faceList[k][l].t].ptr);
						}
						else if (scenes->obj_lists[i].texture_amount == 2) {
							glMultiTexCoord2fv(GL_TEXTURE0, object[j]->tList[object[j]->faceList[k][l].t].ptr);
							glMultiTexCoord2fv(GL_TEXTURE1, object[j]->tList[object[j]->faceList[k][l].t].ptr);
						}

						glNormal3fv(object[j]->nList[object[j]->faceList[k][l].n].ptr);
						glVertex3fv(object[j]->vList[object[j]->faceList[k][l].v].ptr);
					}
					glEnd();


					glPopMatrix();

				}

				break;
			}

		}

		if (j == object_count) {
			cout << scenes->obj_lists[i].object_name << " Object not found" << endl;
			int temp;
			cin >> temp;
			exit(2);
		}
	}

	//fourth mirror area set stencil to 4
	glStencilFunc(GL_EQUAL, 3, 0xFF);
	glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	glDepthMask(GL_FALSE);
	for (i = 0; i < scenes->count; i++) {
		int lastMaterial = -1;

		for (j = 0; j < object_count; j++) {

			if (!strcmp(scenes->obj_lists[i].object_name, file_name[j])) {
				//only do mirror part
				if (strcmp("Mirror.obj", file_name[j]))
					break;

				for (size_t k = 0; k < object[j]->fTotal; ++k)
				{
					// set material property if this face used different material
					glPushMatrix();

					glTranslatef(scenes->obj_lists[i].trans_x, scenes->obj_lists[i].trans_y, scenes->obj_lists[i].trans_z);
					glRotatef(scenes->obj_lists[i].angle, scenes->obj_lists[i].rotate_x, scenes->obj_lists[i].rotate_y, scenes->obj_lists[i].rotate_z);
					glScalef(scenes->obj_lists[i].scale_x, scenes->obj_lists[i].scale_y, scenes->obj_lists[i].scale_z);

					glBegin(GL_TRIANGLES);
					for (size_t l = 0; l < 3; ++l)
					{
						//textex corrd. object->tList[object->faceList[i][j].t].ptr
						if (scenes->obj_lists[i].texture_amount == 1) {
							glMultiTexCoord2fv(GL_TEXTURE0, object[j]->tList[object[j]->faceList[k][l].t].ptr);
						}
						else if (scenes->obj_lists[i].texture_amount == 2) {
							glMultiTexCoord2fv(GL_TEXTURE0, object[j]->tList[object[j]->faceList[k][l].t].ptr);
							glMultiTexCoord2fv(GL_TEXTURE1, object[j]->tList[object[j]->faceList[k][l].t].ptr);
						}

						glNormal3fv(object[j]->nList[object[j]->faceList[k][l].n].ptr);
						glVertex3fv(object[j]->vList[object[j]->faceList[k][l].v].ptr);
					}
					glEnd();

					glPopMatrix();

				}

				break;
			}

		}

		if (j == object_count) {
			cout << scenes->obj_lists[i].object_name << " Object not found" << endl;
			int temp;
			cin >> temp;
			exit(2);
		}
	}

	//set(stencil buffer) object covered by second mirror back to 3
	glStencilFunc(GL_EQUAL, 4, 0xFF);
	glStencilOp(GL_KEEP, GL_KEEP, GL_DECR);
	for (i = 0; i < scenes->count; i++) {
		int lastMaterial = -1;

		for (j = 0; j < object_count; j++) {
			if (!strcmp(scenes->obj_lists[i].object_name, file_name[j])) {
				if (!strcmp("Mirror.obj", file_name[j]))
					break;

				for (size_t k = 0; k < object[j]->fTotal; ++k)
				{
					glPushMatrix();

					glTranslatef(scenes->obj_lists[i].trans_x, scenes->obj_lists[i].trans_y, scenes->obj_lists[i].trans_z);
					glRotatef(scenes->obj_lists[i].angle, scenes->obj_lists[i].rotate_x, scenes->obj_lists[i].rotate_y, scenes->obj_lists[i].rotate_z);
					glScalef(scenes->obj_lists[i].scale_x, scenes->obj_lists[i].scale_y, scenes->obj_lists[i].scale_z);

					glBegin(GL_TRIANGLES);
					for (size_t l = 0; l < 3; ++l)
					{
						//textex corrd. object->tList[object->faceList[i][j].t].ptr
						if (scenes->obj_lists[i].texture_amount == 1) {
							glMultiTexCoord2fv(GL_TEXTURE0, object[j]->tList[object[j]->faceList[k][l].t].ptr);
						}
						else if (scenes->obj_lists[i].texture_amount == 2) {
							glMultiTexCoord2fv(GL_TEXTURE0, object[j]->tList[object[j]->faceList[k][l].t].ptr);
							glMultiTexCoord2fv(GL_TEXTURE1, object[j]->tList[object[j]->faceList[k][l].t].ptr);
						}

						glNormal3fv(object[j]->nList[object[j]->faceList[k][l].n].ptr);
						glVertex3fv(object[j]->vList[object[j]->faceList[k][l].v].ptr);
					}
					glEnd();

					glPopMatrix();

				}
				break;
			}

		}

		if (j == object_count) {
			cout << scenes->obj_lists[i].object_name << " Object not found" << endl;
			int temp;
			cin >> temp;
			exit(2);
		}
	}

	//draw reflected image which is stencil equal to 4
	glStencilFunc(GL_EQUAL, 4, 0xFF);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glDepthMask(GL_TRUE);
	for (i = 0; i < scenes->count; i++) {
		int lastMaterial = -1;
		for (j = 0; j < object_count; j++) {
			if (!strcmp(scenes->obj_lists[i].object_name, file_name[j])) {
				if (!strcmp("Mirror.obj", file_name[j]))
					break;

				for (size_t k = 0; k < object[j]->fTotal; ++k)
				{
					// set material property if this face used different material
					if (lastMaterial != object[j]->faceList[k].m)
					{
						lastMaterial = (int)object[j]->faceList[k].m;
						glMaterialfv(GL_FRONT, GL_AMBIENT, object[j]->mList[lastMaterial].Ka);
						glMaterialfv(GL_FRONT, GL_DIFFUSE, object[j]->mList[lastMaterial].Kd);
						glMaterialfv(GL_FRONT, GL_SPECULAR, object[j]->mList[lastMaterial].Ks);
						glMaterialfv(GL_FRONT, GL_SHININESS, &object[j]->mList[lastMaterial].Ns);

						//you can obtain the texture name by object->mList[lastMaterial].map_Kd
						//load them once in the main function before mainloop
						//bind them in display function here	

					}

					glPushMatrix();

					//printf("angle = %f, %f %f %f\n", scenes->obj_lists[i].angle, scenes->obj_lists[i].rotate_x, scenes->obj_lists[i].rotate_y, scenes->obj_lists[i].rotate_z);

					float temp;

					if (((rotatefactor <= 90) && (rotatefactor >= 0)) || ((rotatefactor >= 270) && (rotatefactor <= 359)))
						temp = -320;
					else
						temp = 320;

					glTranslatef(scenes->obj_lists[i].trans_x + temp, scenes->obj_lists[i].trans_y, scenes->obj_lists[i].trans_z);
					glRotatef(scenes->obj_lists[i].angle, scenes->obj_lists[i].rotate_x, scenes->obj_lists[i].rotate_y, scenes->obj_lists[i].rotate_z);
					glScalef(scenes->obj_lists[i].scale_x, scenes->obj_lists[i].scale_y, scenes->obj_lists[i].scale_z);

					glBegin(GL_TRIANGLES);
					for (size_t l = 0; l < 3; ++l)
					{
						//textex corrd. object->tList[object->faceList[i][j].t].ptr
						if (scenes->obj_lists[i].texture_amount == 1) {
							glMultiTexCoord2fv(GL_TEXTURE0, object[j]->tList[object[j]->faceList[k][l].t].ptr);
						}
						else if (scenes->obj_lists[i].texture_amount == 2) {
							glMultiTexCoord2fv(GL_TEXTURE0, object[j]->tList[object[j]->faceList[k][l].t].ptr);
							glMultiTexCoord2fv(GL_TEXTURE1, object[j]->tList[object[j]->faceList[k][l].t].ptr);
						}

						glNormal3fv(object[j]->nList[object[j]->faceList[k][l].n].ptr);
						glVertex3fv(object[j]->vList[object[j]->faceList[k][l].v].ptr);
					}
					glEnd();


					glPopMatrix();

				}

				break;
			}

		}

		if (j == object_count) {
			cout << scenes->obj_lists[i].object_name << " Object not found" << endl;
			int temp;
			cin >> temp;
			exit(2);
		}
	}

	glClear(GL_STENCIL_BUFFER_BIT);
	glDisable(GL_STENCIL_TEST);
}