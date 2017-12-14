#pragma once
#include<iostream>

#define OBJECT_COUNT 50
#define NAMELEN 20

using namespace std;

class viewing {
public:
	int eye[3];
	int vat[3];
	int vup[3];

	int angle;

	int dnear;
	int dfar;
	
	int viewport[4];

	viewing() {
		for (int i = 0; i < 3; i++)
			eye[i] = vat[i] = vup[i] = 0;

		angle = dnear = dfar = 0;

		for (int i = 0; i < 4; i++)
			viewport[i] = 0;
	}

	viewing(char *view_file) {

		FILE *ifile;

		ifile = fopen(view_file, "r");

		if (!ifile) {
			cout << "Cannot open the file" << endl;
			exit(-1);
		}

		cout << endl;
		cout << "[Scanning] View file..." << endl;

		while (!feof(ifile)) {
			char buffer[100];

			fscanf(ifile, "%s", buffer);

			if (!strcmp(buffer, "eye")) {
				fscanf(ifile, "%d%d%d", &eye[0], &eye[1], &eye[2]);
			}
			else if (!strcmp(buffer, "vat")) {
				fscanf(ifile, "%d%d%d", &vat[0], &vat[1], &vat[2]);
			}
			else if (!strcmp(buffer, "vup")) {
				fscanf(ifile, "%d%d%d", &vup[0], &vup[1], &vup[2]);
			}
			else if (!strcmp(buffer, "fovy")) {
				fscanf(ifile, "%d", &angle);
			}
			else if (!strcmp(buffer, "dnear")) {
				fscanf(ifile, "%d", &dnear);
			}
			else if (!strcmp(buffer, "dfar")) {
				fscanf(ifile, "%d", &dfar);
			}
			else if (!strcmp(buffer, "viewport")) {
				fscanf(ifile, "%d%d%d%d", &viewport[0], &viewport[1], &viewport[2], &viewport[3]);
			}
			else if (!strcmp(buffer, "\n")) {
				continue;
			}
			else {
				cout << "No such data" << endl;
				exit(-1);
			}
		}

	}

};

class lighting {
public:
	class light {
	public:
		float x, y, z;
		GLfloat ar, ag, ab;
		GLfloat dr, dg, db;
		GLfloat sr, sg, sb;

		light() {
			x = y = z = 0;
			ar = ag = ab = 0.0;
			dr = dg = db = 0.0;
			sr = sg = sb = 0.0;
		}
	};

	int count = 0;
	light lightsource[8];
	float r, g, b;

	lighting() {
		count = 0;
	}

	lighting(char *light_file) {
		FILE *ifile;

		ifile = fopen(light_file, "r");

		if (!ifile) {
			cout << "Light file cannot be opened" << endl;
			exit(-2);
		}

		cout << endl;
		cout << "[Scanning] light file..." << endl;

		while (!feof(ifile)) {
			char buffer[100];

			fscanf(ifile, "%s", buffer);

			if (!strcmp(buffer, "light")) {
				fscanf(ifile, "%f%f%f", &(lightsource[count].x), &(lightsource[count].y), &(lightsource[count].z));
				fscanf(ifile, "%f%f%f", &(lightsource[count].ar), &(lightsource[count].ag), &(lightsource[count].ab));
				fscanf(ifile, "%f%f%f", &(lightsource[count].dr), &(lightsource[count].dg), &(lightsource[count].db));
				fscanf(ifile, "%f%f%f", &(lightsource[count].sr), &(lightsource[count].sg), &(lightsource[count].sb));
				count = count + 1;
				memset(buffer, 0, sizeof(buffer));
			}
			else if (!strcmp(buffer, "ambient")) {
				fscanf(ifile, "%f%f%f", &r, &g, &b);
			}
			else if (!strcmp(buffer, "\n")) {
				continue;
			}
			else {
				cout << buffer << " is not one of the values." << endl;
				exit(-2);
			}
		}
	}
};

class scene {
public:

	class object {
	public:
		char object_name[NAMELEN];
		char texture_name[6][NAMELEN];
		GLfloat scale_x, scale_y, scale_z;
		GLfloat angle;
		GLfloat rotate_x, rotate_y, rotate_z;
		GLfloat trans_x, trans_y, trans_z;
		int texture_amount = 0;

		object() {
			memset(object_name, 0, sizeof(object_name));
			scale_x = scale_y = scale_z = 0.0;
			angle = 0;
			rotate_x = rotate_y = rotate_z = 0.0;
			trans_x = trans_y = trans_z = 0.0;
			texture_amount = 0;
			for (int i = 0; i < 6; i++)
				memset(texture_name[i], 0, sizeof(texture_name[i]));
		}

	};

	int count = 0;
	object obj_lists[OBJECT_COUNT];

	scene() {
		count = 0;
	}
	scene(char *scene_file) {
		FILE * ifile;

		ifile = fopen(scene_file, "r");

		if (!ifile) {
			cout << "Scene file cannot be opened" << endl;
			exit(-3);
		}

		char temp[6][50];
		int map_count = 0;
		cout << endl;
		cout << "[Scanning] Scene file..." << endl;
		while (!feof(ifile)) {
			char words[50];
			fscanf(ifile, "%s", words);

			if (!strcmp(words, "no-texture")) {
				memset(temp[0], 0, sizeof(temp[0]));
				map_count = 0;
				cout << "[No texture] loaded." << endl;
			}
			else if (!strcmp(words, "single-texture")) {
				memset(temp[0], 0, sizeof(temp[0]));
				fscanf(ifile, "%s", temp[0]);
				map_count = 1;
				cout << "[Single texture] " << temp[0] << " loaded." << endl;
			}
			else if (!strcmp(words, "multi-texture")) {
				memset(temp[0], 0, sizeof(temp[0]));
				memset(temp[1], 0, sizeof(temp[1]));
				fscanf(ifile, "%s", temp[0]);
				fscanf(ifile, "%s", temp[1]);
				map_count = 2;
				cout << "[Multi texture] first = " << temp[0] << " loaded." << endl;
				cout << "[Multi texture] second = " << temp[1] << " loaded." << endl;
			}
			else if (!strcmp(words, "cube-map")) {
				for (int i = 0; i < 6; i++) {
					memset(temp[i], 0, sizeof(temp[i]));
					fscanf(ifile, "%s", temp[i]);
					cout << "[cube texture] " << i << " " << temp[i] << " loaded." << endl;
				}		
				map_count = 6;
			}
			else if (!strcmp(words, "model")) {
				fscanf(ifile, "%s", obj_lists[count].object_name);
				fscanf(ifile, "%f%f%f", &(obj_lists[count].scale_x), &(obj_lists[count].scale_y), &(obj_lists[count].scale_z));
				fscanf(ifile, "%f", &(obj_lists[count].angle));
				fscanf(ifile, "%f%f%f", &(obj_lists[count].rotate_x), &(obj_lists[count].rotate_y), &(obj_lists[count].rotate_z));
				fscanf(ifile, "%f%f%f", &(obj_lists[count].trans_x), &(obj_lists[count].trans_y), &(obj_lists[count].trans_z));
				for (int i = 0; i < map_count; i++) 
					strcpy(obj_lists[count].texture_name[i], temp[i]);

				obj_lists[count].texture_amount = map_count;

				//total amount of obj_lists
				count = count + 1;
				
				memset(words, 0, sizeof(words));
			}
			else if (!strcmp(words, "\n")) 
				continue;
			else {
				cout << words <<" object doesn't exist in the file" << endl;
				exit(-3);
			}
		}
	}
};