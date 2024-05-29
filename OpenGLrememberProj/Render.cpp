#include "Render.h"

#include <vector> 
//������ ����������, ����� ���������� ������� ���
#include "ObjLoader.h"

#include <sstream>
#include <iostream>

#include <windows.h>
#include <GL\GL.h>
#include <GL\GLU.h>

#include "MyOGL.h"

#include "Camera.h"
#include "Light.h"
#include "Primitives.h"

#include "GUItextRectangle.h"

bool textureMode = true;
bool lightMode = true;


ObjFile fishModel;//������ � ������
ObjFile GuyModel;//������ � ������
ObjFile Zamok; //�����
// ����� ��� ������������� ����
class Fish {
public:
	Vector3 position;
	std::vector<double> color;
	double t; // �������� ��� ������ �����
	std::vector<Vector3> controlPoints; // ����������� ����� ������ �����

	Fish(const std::vector<Vector3>& points, const std::vector<double> _color)
		: t(0), controlPoints(points), color(_color) {}

	// ������� ��� ���������� ������� �� ������ �����
	Vector3 bezierCurve(double t) {
		double u = 1 - t;
		double tt = t * t;
		double uu = u * u;
		double uuu = uu * u;
		double ttt = tt * t;

		Vector3 p = controlPoints[0] * uuu; // ������ �����
		p = p + controlPoints[1] * (3 * uu * t); // ������ �����
		p = p + controlPoints[2] * (3 * u * tt); // ������ �����
		p = p + controlPoints[3] * ttt; // ��������� �����

		return p;
	}

	// ���������� ������� ����
	void update() {
		t += 0.01; // ����������� t ��� �������� �� ������
		if (t > 1) t = 0; // ����������� t
		position = bezierCurve(t);
	}

	// ��������� ����
	void draw() {
		glPushMatrix();
		glTranslated(position.X(), position.Y(), position.Z());
		glColor3d(1.0, 0.0, 0.0); // ���� ����
		glBegin(GL_TRIANGLES);
		glVertex3d(0.0, 0.1, 0.0);
		glVertex3d(-0.1, -0.1, 0.0);
		glVertex3d(0.1, -0.1, 0.0);
		glEnd();
		glPopMatrix();
	}

	//��������� �������� ���
	void drawObj() {
		glPushMatrix();
		glColor3d(color[0], color[1], color[2]);
		glTranslated(position.X(), position.Y(), position.Z());
		glScalef(0.09f, 0.09f, 0.09f);
		fishModel.DrawObj(); 
		glPopMatrix();
	}

};
std::vector<Fish> fishes; //�����

//����� ��� ��������� ������
class CustomCamera : public Camera
{
public:
	//��������� ������
	double camDist;
	//���� �������� ������
	double fi1, fi2;

	
	//������� ������ �� ���������
	CustomCamera()
	{
		camDist = 15;
		fi1 = 1;
		fi2 = 1;
	}

	
	//������� ������� ������, ������ �� ����� ��������, ���������� �������
	void SetUpCamera()
	{
		//�������� �� ������� ������ ������
		lookPoint.setCoords(0, 0, 0);

		pos.setCoords(camDist*cos(fi2)*cos(fi1),
			camDist*cos(fi2)*sin(fi1),
			camDist*sin(fi2));

		if (cos(fi2) <= 0)
			normal.setCoords(0, 0, -1);
		else
			normal.setCoords(0, 0, 1);

		LookAt();
	}

	void CustomCamera::LookAt()
	{
		//������� ��������� ������
		gluLookAt(pos.X(), pos.Y(), pos.Z(), lookPoint.X(), lookPoint.Y(), lookPoint.Z(), normal.X(), normal.Y(), normal.Z());
	}



}  camera;   //������� ������ ������


//����� ��� ��������� �����
class CustomLight : public Light
{
public:
	CustomLight()
	{
		//��������� ������� �����
		pos = Vector3(1, 1, 3);
	}

	
	//������ ����� � ����� ��� ���������� �����, ���������� �������
	void  DrawLightGhismo()
	{
		glDisable(GL_LIGHTING);

		
		glColor3d(0.9, 0.8, 0);
		Sphere s;
		s.pos = pos;
		s.scale = s.scale*0.08;
		s.Show();
		
		if (OpenGL::isKeyPressed('G'))
		{
			glColor3d(0, 0, 0);
			//����� �� ��������� ����� �� ����������
			glBegin(GL_LINES);
			glVertex3d(pos.X(), pos.Y(), pos.Z());
			glVertex3d(pos.X(), pos.Y(), 0);
			glEnd();

			//������ ���������
			Circle c;
			c.pos.setCoords(pos.X(), pos.Y(), 0);
			c.scale = c.scale*1.5;
			c.Show();
		}

	}

	void SetUpLight()
	{
		GLfloat amb[] = { 0.2, 0.2, 0.2, 0 };
		GLfloat dif[] = { 1.0, 1.0, 1.0, 0 };
		GLfloat spec[] = { .7, .7, .7, 0 };
		GLfloat position[] = { pos.X(), pos.Y(), pos.Z(), 1. };

		// ��������� ��������� �����
		glLightfv(GL_LIGHT0, GL_POSITION, position);
		// �������������� ����������� �����
		// ������� ��������� (���������� ����)
		glLightfv(GL_LIGHT0, GL_AMBIENT, amb);
		// ��������� ������������ �����
		glLightfv(GL_LIGHT0, GL_DIFFUSE, dif);
		// ��������� ���������� ������������ �����
		glLightfv(GL_LIGHT0, GL_SPECULAR, spec);

		glEnable(GL_LIGHT0);
	}


} light;  //������� �������� �����




//������ ���������� ����
int mouseX = 0, mouseY = 0;

void mouseEvent(OpenGL *ogl, int mX, int mY)
{
	int dx = mouseX - mX;
	int dy = mouseY - mY;
	mouseX = mX;
	mouseY = mY;

	//������ ���� ������ ��� ������� ����� ������ ����
	if (OpenGL::isKeyPressed(VK_RBUTTON))
	{
		camera.fi1 += 0.01*dx;
		camera.fi2 += -0.01*dy;
	}

	
	//������� ���� �� ���������, � ����� ��� ����
	if (OpenGL::isKeyPressed('G') && !OpenGL::isKeyPressed(VK_LBUTTON))
	{
		LPPOINT POINT = new tagPOINT();
		GetCursorPos(POINT);
		ScreenToClient(ogl->getHwnd(), POINT);
		POINT->y = ogl->getHeight() - POINT->y;

		Ray r = camera.getLookRay(POINT->x, POINT->y);

		double z = light.pos.Z();

		double k = 0, x = 0, y = 0;
		if (r.direction.Z() == 0)
			k = 0;
		else
			k = (z - r.origin.Z()) / r.direction.Z();

		x = k*r.direction.X() + r.origin.X();
		y = k*r.direction.Y() + r.origin.Y();

		light.pos = Vector3(x, y, z);
	}

	if (OpenGL::isKeyPressed('G') && OpenGL::isKeyPressed(VK_LBUTTON))
	{
		light.pos = light.pos + Vector3(0, 0, 0.02*dy);
	}

	
}

void mouseWheelEvent(OpenGL *ogl, int delta)
{

	if (delta < 0 && camera.camDist <= 1)
		return;
	if (delta > 0 && camera.camDist >= 100)
		return;

	camera.camDist += 0.01*delta;

}


void keyDownEvent(OpenGL *ogl, int key)
{
	if (key == 'L')
	{
		lightMode = !lightMode;
	}

	if (key == 'T')
	{
		textureMode = !textureMode;
	}

	if (key == 'R')
	{
		camera.fi1 = 1;
		camera.fi2 = 1;
		camera.camDist = 15;

		light.pos = Vector3(1, 1, 3);
	}

	if (key == 'F')
	{
		light.pos = camera.pos;
	}
}

void keyUpEvent(OpenGL *ogl, int key)
{
	
}



GLuint texId;
GLuint tableTexture; 
GLuint zamokTexture;
GLuint vodolazTexture;

//����������� ����� ������ ��������
void initRender(OpenGL *ogl)
{
	//��������� �������

	//4 ����� �� �������� �������
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	//��������� ������ ��������� �������
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	//�������� ��������
	glEnable(GL_TEXTURE_2D);
	

	//������ ����������� ���������  (R G B)
	RGBTRIPLE *texarray;

	//������ ��������, (������*������*4      4, ���������   ����, �� ������� ������������ �� 4 ����� �� ������� �������� - R G B A)
	char *texCharArray;
	int texW, texH;
	//OpenGL::LoadBMP("texture.bmp", &texW, &texH, &texarray);
	OpenGL::LoadBMP("texture.bmp", &texW, &texH, &texarray);
	OpenGL::RGBtoChar(texarray, texW, texH, &texCharArray);


	
	//���������� �� ��� ��������
	glGenTextures(1, &texId);
	//������ ��������, ��� ��� ����� ����������� � ���������, ����� ����������� �� ����� ��
	glBindTexture(GL_TEXTURE_2D, texId);

	//��������� �������� � �����������, � ���������� ��� ������  ��� �� �����
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texW, texH, 0, GL_RGBA, GL_UNSIGNED_BYTE, texCharArray);

	//�������� ������
	free(texCharArray);
	free(texarray);
	//������� ����
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);


	//�������� �����
	RGBTRIPLE* texarray1;

	//������ ��������, (������*������*4      4, ���������   ����, �� ������� ������������ �� 4 ����� �� ������� �������� - R G B A)
	char* texCharArray1;
	int texW1, texH1;
	//OpenGL::LoadBMP("texture.bmp", &texW, &texH, &texarray);
	OpenGL::LoadBMP("tableTexture.bmp", &texW1, &texH1, &texarray1);
	OpenGL::RGBtoChar(texarray1, texW1, texH1, &texCharArray1);



	//���������� �� ��� ��������
	glGenTextures(1, &tableTexture);
	//������ ��������, ��� ��� ����� ����������� � ���������, ����� ����������� �� ����� ��
	glBindTexture(GL_TEXTURE_2D, tableTexture);

	//��������� �������� � �����������, � ���������� ��� ������  ��� �� �����
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texW1, texH1, 0, GL_RGBA, GL_UNSIGNED_BYTE, texCharArray1);

	//�������� ������
	free(texCharArray1);
	free(texarray1);
	//������� ����
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	

	//�������� �����
	RGBTRIPLE* texarray2;

	//������ ��������, (������*������*4      4, ���������   ����, �� ������� ������������ �� 4 ����� �� ������� �������� - R G B A)
	char* texCharArray2;
	int texW2, texH2;
	//OpenGL::LoadBMP("texture.bmp", &texW, &texH, &texarray);
	OpenGL::LoadBMP("zamokTexture.bmp", &texW2, &texH2, &texarray2);
	OpenGL::RGBtoChar(texarray2, texW2, texH2, &texCharArray2);



	//���������� �� ��� ��������
	glGenTextures(1, &zamokTexture);
	//������ ��������, ��� ��� ����� ����������� � ���������, ����� ����������� �� ����� ��
	glBindTexture(GL_TEXTURE_2D, zamokTexture);

	//��������� �������� � �����������, � ���������� ��� ������  ��� �� �����
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texW1, texH1, 0, GL_RGBA, GL_UNSIGNED_BYTE, texCharArray2);

	//�������� ������
	free(texCharArray2);
	free(texarray2);
	//������� ����
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	//�������� �������� 
	RGBTRIPLE* texarray3;

	//������ ��������, (������*������*4      4, ���������   ����, �� ������� ������������ �� 4 ����� �� ������� �������� - R G B A)
	char* texCharArray3;
	int texW3, texH3;
	//OpenGL::LoadBMP("texture.bmp", &texW, &texH, &texarray);
	OpenGL::LoadBMP("vodolazTexture.bmp", &texW3, &texH3, &texarray3);
	OpenGL::RGBtoChar(texarray3, texW3, texH3, &texCharArray3);



	//���������� �� ��� ��������
	glGenTextures(1, &vodolazTexture);
	//������ ��������, ��� ��� ����� ����������� � ���������, ����� ����������� �� ����� ��
	glBindTexture(GL_TEXTURE_2D, vodolazTexture);

	//��������� �������� � �����������, � ���������� ��� ������  ��� �� �����
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texW1, texH1, 0, GL_RGBA, GL_UNSIGNED_BYTE, texCharArray3);

	//�������� ������
	free(texCharArray3);
	free(texarray3);
	//������� ����
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);


	//������ � ���� ����������� � "������"
	ogl->mainCamera = &camera;
	ogl->mainLight = &light;

	// ������������ �������� : �� ����� ����� ����� 1
	glEnable(GL_NORMALIZE);

	// ���������� ������������� ��� �����
	glEnable(GL_LINE_SMOOTH); 


	//   ������ ��������� ���������
	//  �������� GL_LIGHT_MODEL_TWO_SIDE - 
	//                0 -  ������� � ���������� �������� ���������(�� ���������), 
	//                1 - ������� � ���������� �������������� ������� ��������       
	//                �������������� ������� � ���������� ��������� ����������.    
	//  �������� GL_LIGHT_MODEL_AMBIENT - ������ ������� ���������, 
	//                �� ��������� �� ���������
	// �� ��������� (0.2, 0.2, 0.2, 1.0)

	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 0);

	camera.fi1 = -1.3;
	camera.fi2 = 0.8;

	//������ � ������
	loadModel("SuperFish.obj", &fishModel);

	//������ ��������� ������
	loadModel("SuperGuy.obj", &GuyModel);

	//������ �����
	loadModel("Zamok.obj", &Zamok);

	// ��������� ���
	std::vector<Vector3> controlPoints = {
		Vector3(3, 3, 3),
		Vector3(-2, 3, 3),
		Vector3(1, 4, 3),
		Vector3(3, 0, 3)
	};
	std::vector<double> color = { 1.0, 0.5, 0.0 };
	fishes.push_back(Fish(controlPoints, color));

	controlPoints = {
		Vector3(-3, -3, 2),
		Vector3(1, 1, 2),
		Vector3(2, -3, 2),
		Vector3(3, -2, 2)
	};
	color = { 1.0, 0.5, 0.0 };
	fishes.push_back(Fish(controlPoints, color));

	controlPoints = {
		Vector3(2, -3, 2),
		Vector3(1, 4, 1),
		Vector3(2, 1, 2),
		Vector3(-3, -1, 3)
	};
	color = { 1.0, 0.5, 0.0 };
	fishes.push_back(Fish(controlPoints, color));

	controlPoints = {
		Vector3(3, 1, 2),
		Vector3(1, -2, 2),
		Vector3(-1, -2, 2),
		Vector3(3, 3, 2)
	};
	color = { 0.5f, 0.0f, 0.5f };
	fishes.push_back(Fish(controlPoints, color));

	controlPoints = {
		Vector3(1, 2, 1),
		Vector3(-1, 0, 4),
		Vector3(2, 0, 1),
		Vector3(-3, 3, 2)
	};
	color = { 0.5f, 0.0f, 0.5f };
	fishes.push_back(Fish(controlPoints, color));

	controlPoints = {
		Vector3(-1, 3, 5),
		Vector3(1, 3, 2),
		Vector3(-1, 3, 2),
		Vector3(3, -3, 2)
	};
	color = { 0.5f, 0.0f, 0.5f };
	fishes.push_back(Fish(controlPoints, color));

	controlPoints = {
		Vector3(1, 3, 2),
		Vector3(3, -3, 2),
		Vector3(-2, -3, 4),
		Vector3(1, 3, 2),
	};
	color = { 0.5f, 0.0f, 0.5f };
	fishes.push_back(Fish(controlPoints, color));
}


void drawCube(double size) {
	double halfSize = size / 2.0;
	glBindTexture(GL_TEXTURE_2D, tableTexture);
	glBegin(GL_QUADS);
	// ������� �����
	glTexCoord2d(0, 0);
	glVertex3d(-halfSize, halfSize, -halfSize);
	glTexCoord2d(0, 1);
	glVertex3d(halfSize, halfSize, -halfSize);
	glTexCoord2d(1, 1);
	glVertex3d(halfSize, halfSize, halfSize);
	glTexCoord2d(1, 0);
	glVertex3d(-halfSize, halfSize, halfSize);

	// ������ �����
	glTexCoord2d(0, 0);
	glVertex3d(-halfSize, -halfSize, -halfSize);
	glTexCoord2d(0, 1);
	glVertex3d(halfSize, -halfSize, -halfSize);
	glTexCoord2d(1, 1);
	glVertex3d(halfSize, -halfSize, halfSize);
	glTexCoord2d(1, 0);
	glVertex3d(-halfSize, -halfSize, halfSize);

	// �������� �����
	glTexCoord2d(0, 0);
	glVertex3d(-halfSize, -halfSize, halfSize);
	glTexCoord2d(0, 1);
	glVertex3d(halfSize, -halfSize, halfSize);
	glTexCoord2d(1, 1);
	glVertex3d(halfSize, halfSize, halfSize);
	glTexCoord2d(1, 0);
	glVertex3d(-halfSize, halfSize, halfSize);

	// ������ �����
	glTexCoord2d(0, 0);
	glVertex3d(-halfSize, -halfSize, -halfSize);
	glTexCoord2d(0, 1);
	glVertex3d(halfSize, -halfSize, -halfSize);
	glTexCoord2d(1, 1);
	glVertex3d(halfSize, halfSize, -halfSize);
	glTexCoord2d(1, 0);
	glVertex3d(-halfSize, halfSize, -halfSize);

	// ����� �����
	glTexCoord2d(0, 0);
	glVertex3d(-halfSize, -halfSize, -halfSize);
	glTexCoord2d(0, 1);
	glVertex3d(-halfSize, -halfSize, halfSize);
	glTexCoord2d(1, 1);
	glVertex3d(-halfSize, halfSize, halfSize);
	glTexCoord2d(1, 0);
	glVertex3d(-halfSize, halfSize, -halfSize);

	// ������ �����
	glTexCoord2d(0, 0);
	glVertex3d(halfSize, -halfSize, -halfSize);
	glTexCoord2d(0, 1);
	glVertex3d(halfSize, -halfSize, halfSize);
	glTexCoord2d(1, 1);
	glVertex3d(halfSize, halfSize, halfSize);
	glTexCoord2d(1, 0);
	glVertex3d(halfSize, halfSize, -halfSize);
	glEnd();
}

void drawTable() {
	// ������ ����������
	glColor3d(0.6, 0.3, 0.1); // ���� �����
	glPushMatrix();
	glTranslated(0.0, 0.0, -0.54);
	glNormal3d(0, 0, 1);
	glRotated(90.0, 1.0, 0.0, 0.0); // ������������ ����, ����� ��� ���� ���� ������������ ��������� XY
	glScaled(15, 1, 10); // ����������� ������� �����
	drawCube(1.0); // ��� ��� ������ �����
	glPopMatrix();

	// ������ ����� �����
	glPushMatrix();
	glColor3d(0.4, 0.2, 0.1);
	for (double x = -6.5; x <= 6.5; x += 13.0) { // ����������� �������� ����� �������
		for (double y = -3.0; y <= 3.0; y += 6.0) {
			glPushMatrix();
			glTranslated(x, y, -5.0); // ����������� ����� � �����
			glScaled(1.0, 1.0, 10.0); // ����������� ����� �����
			drawCube(1.0); // ��� ��� ����� �����
			glPopMatrix();
		}
	}
	glPopMatrix();

}

void drawRoom() {
	//���������� ���� ��� �������
	glPushMatrix();
	double A[] = { -30, -30, -10 };
	double B[] = { -30, 30, -10 };
	double C[] = { 30, 30, -10 };
	double D[] = { 30, -30, -10 };
	glBindTexture(GL_TEXTURE_2D, tableTexture);
	glBegin(GL_QUADS);
	glColor3f(0.5f, 0.5f, 0.5f);
	glTexCoord2d(0, 0);
	glVertex3dv(A);
	glTexCoord2d(0, 1);
	glVertex3dv(B);
	glTexCoord2d(1, 1);
	glVertex3dv(C);
	glTexCoord2d(1, 0);
	glVertex3dv(D);
	glEnd();
	glPopMatrix();
}

void grani() {
	glBegin(GL_LINES);
	glColor3f(0, 0, 0);
	double A[] = { -5, -4, 0 };
	double B[] = { -5, -4, 6 };
	double C[] = { -5, 4, 0 };
	double D[] = { -5, 4, 6 };
	double E[] = { 5, -4, 0 };
	double F[] = { 5, -4, 6 };
	double G[] = { 5, 4, 0 };
	double H[] = { 5, 4, 6 };

	glVertex3dv(A);
	glVertex3dv(B);

	glVertex3dv(C);
	glVertex3dv(D);

	glVertex3dv(E);
	glVertex3dv(F);

	glVertex3dv(G);
	glVertex3dv(H);
	glEnd();
}

void aquar() {
	//������ ���������� �����
	glPushMatrix(); 
	glColor3f(0.83f, 0.69f, 0.22f);
	glBindTexture(GL_TEXTURE_2D, vodolazTexture);
	glTranslated(-2, 3, 0);
	glRotated(180, 0, 0, 1); 
	glScalef(0.2f, 0.2f, 0.2f);
	GuyModel.DrawObj();
	glPopMatrix();
	//������ �����
	glPushMatrix();
	glColor3f(0.5f, 0.5f, 0.5f);
	glBindTexture(GL_TEXTURE_2D, zamokTexture);
	glTranslated(-2, -2, 0);
	glScalef(0.3f, 0.3f, 0.3f);
	Zamok.DrawObj();
	glPopMatrix();
	// ������� ������ ��� ���������
	// ������ �������
	glBegin(GL_QUADS);
	glColor3f(0.5f, 0.5f, 0.5f);
	glNormal3f(0.0f, 0.0f, 1.0f);
	glVertex3f(-5.0f, -4.0f, 0.0f);
	glVertex3f(5.0f, -4.0f, 0.0f);
	glVertex3f(5.0f, 4.0f, 0.0f);
	glVertex3f(-5.0f, 4.0f, 0.0f);
	glEnd();
	// ������ ��������
	// �������� �����-��������� ��� ������������
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	// ������ �������
	glBegin(GL_QUADS);
	glColor4d(0.5, 0.5, 1.0, 0.7);
	glNormal3f(0.0f, -1.0f, 0.0f);
	glVertex3f(-5.0f, -4.0f, 0.0f);
	glVertex3f(5.0f, -4.0f, 0.0f);
	glVertex3f(5.0f, -4.0f, 6.0f);
	glVertex3f(-5.0f, -4.0f, 6.0f);
	glEnd();

	// ����� �������
	glBegin(GL_QUADS);
	glColor4d(0.5, 0.5, 1.0, 0.7);
	glNormal3f(-1.0f, 0.0f, 0.0f);
	glVertex3f(-5.0f, -4.0f, 0.0f);
	glVertex3f(-5.0f, -4.0f, 6.0f);
	glVertex3f(-5.0f, 4.0f, 6.0f);
	glVertex3f(-5.0f, 4.0f, 0.0f);
	glEnd();

	// ������ �������
	glBegin(GL_QUADS);
	glColor4d(0.5, 0.5, 1.0, 0.7);
	glNormal3f(1.0f, 0.0f, 0.0f);
	glVertex3f(5.0f, -4.0f, 0.0f);
	glVertex3f(5.0f, -4.0f, 6.0f);
	glVertex3f(5.0f, 4.0f, 6.0f);
	glVertex3f(5.0f, 4.0f, 0.0f);
	glEnd();

	// ������� �������
	glBegin(GL_QUADS);
	glColor4d(0.5, 0.5, 1.0, 0.7);
	glNormal3f(0.0f, 1.0f, 0.0f);
	glVertex3f(-5.0f, 4.0f, 0.0f);
	glVertex3f(5.0f, 4.0f, 0.0f);
	glVertex3f(5.0f, 4.0f, 6.0f);
	glVertex3f(-5.0f, 4.0f, 6.0f);
	glEnd();

	// ������� �������
	glBegin(GL_QUADS);
	glColor4d(0.5, 0.5, 1.0, 0.7);
	glNormal3f(0.0f, 0.0f, 1.0f);
	glVertex3f(-5.0f, -4.0f, 6.0f);
	glVertex3f(5.0f, -4.0f, 6.0f);
	glVertex3f(5.0f, 4.0f, 6.0f);
	glVertex3f(-5.0f, 4.0f, 6.0f);
	glEnd();
}

void Render(OpenGL *ogl)
{
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);

	glEnable(GL_DEPTH_TEST);
	if (textureMode)
		glEnable(GL_TEXTURE_2D);

	if (lightMode)
		glEnable(GL_LIGHTING);

	//��������������
	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	//��������� ���������
	GLfloat amb[] = { 0.2, 0.2, 0.1, 1. };
	GLfloat dif[] = { 0.4, 0.65, 0.5, 1. };
	GLfloat spec[] = { 0.9, 0.8, 0.3, 1. };
	GLfloat sh = 0.1f * 256;


	//�������
	glMaterialfv(GL_FRONT, GL_AMBIENT, amb);
	//��������
	glMaterialfv(GL_FRONT, GL_DIFFUSE, dif);
	//����������
	glMaterialfv(GL_FRONT, GL_SPECULAR, spec); \
		//������ �����
		glMaterialf(GL_FRONT, GL_SHININESS, sh);

	//���� ���� �������, ��� ����������� (����������� ���������)
	glShadeModel(GL_SMOOTH);
	//===================================
	//������� ���  
	// ����� �������
	drawRoom();
	// ������ ����
	drawTable();
	// ����� ����� ���������
	grani(); 

	// ���������� � ��������� ���
	glPushMatrix();
	glBindTexture(GL_TEXTURE_2D, texId); //�������� �������� ���� �� ����������
	for (Fish& fish : fishes) {
		fish.update();
		fish.drawObj();
	}
	glPopMatrix(); 


	// ������ ��������
	aquar();

   //��������� ������ ������

	
	glMatrixMode(GL_PROJECTION);	//������ �������� ������� ��������. 
	                                //(���� ��������� ��������, ����� �� ������������.)
	glPushMatrix();   //��������� ������� ������� ������������� (������� ��������� ������������� ��������) � ���� 				    
	glLoadIdentity();	  //��������� ��������� �������
	glOrtho(0, ogl->getWidth(), 0, ogl->getHeight(), 0, 1);	 //������� ����� ������������� ��������

	glMatrixMode(GL_MODELVIEW);		//������������� �� �����-��� �������
	glPushMatrix();			  //��������� ������� ������� � ���� (��������� ������, ����������)
	glLoadIdentity();		  //���������� �� � ������

	glDisable(GL_LIGHTING);



	GuiTextRectangle rec;		   //������� ����� ��������� ��� ������� ������ � �������� ������.
	rec.setSize(300, 150);
	rec.setPosition(10, ogl->getHeight() - 150 - 10);


	std::stringstream ss;
	ss << "T - ���/���� �������" << std::endl;
	ss << "L - ���/���� ���������" << std::endl;
	ss << "F - ���� �� ������" << std::endl;
	ss << "G - ������� ���� �� �����������" << std::endl;
	ss << "G+��� ������� ���� �� ���������" << std::endl;
	ss << "�����. �����: (" << light.pos.X() << ", " << light.pos.Y() << ", " << light.pos.Z() << ")" << std::endl;
	ss << "�����. ������: (" << camera.pos.X() << ", " << camera.pos.Y() << ", " << camera.pos.Z() << ")" << std::endl;
	ss << "��������� ������: R="  << camera.camDist << ", fi1=" << camera.fi1 << ", fi2=" << camera.fi2 << std::endl;
	
	rec.setText(ss.str().c_str());
	rec.Draw();

	glMatrixMode(GL_PROJECTION);	  //��������������� ������� �������� � �����-��� �������� �� �����.
	glPopMatrix();


	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	

}