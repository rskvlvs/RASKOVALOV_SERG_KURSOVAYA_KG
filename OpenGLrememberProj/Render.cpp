#include "Render.h"

#include <vector> 
//Ставлю библиотеку, чтобы подгрузить объекты рыб
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


ObjFile fishModel;//Объект с рыбкой
// Класс для представления рыбы
class Fish {
public:
	Vector3 position;
	std::vector<double> color;
	double t; // Параметр для кривой Безье
	std::vector<Vector3> controlPoints; // Контрольные точки кривой Безье

	Fish(const std::vector<Vector3>& points, const std::vector<double> _color)
		: t(0), controlPoints(points), color(_color) {}

	// Функция для вычисления позиции на кривой Безье
	Vector3 bezierCurve(double t) {
		double u = 1 - t;
		double tt = t * t;
		double uu = u * u;
		double uuu = uu * u;
		double ttt = tt * t;

		Vector3 p = controlPoints[0] * uuu; // Первая точка
		p = p + controlPoints[1] * (3 * uu * t); // Вторая точка
		p = p + controlPoints[2] * (3 * u * tt); // Третья точка
		p = p + controlPoints[3] * ttt; // Четвертая точка

		return p;
	}

	// Обновление позиции рыбы
	void update() {
		t += 0.01; // Увеличиваем t для движения по кривой
		if (t > 1) t = 0; // Зацикливаем t
		position = bezierCurve(t);
	}

	// Отрисовка рыбы
	void draw() {
		glPushMatrix();
		glTranslated(position.X(), position.Y(), position.Z());
		glColor3d(1.0, 0.0, 0.0); // Цвет рыбы
		glBegin(GL_TRIANGLES);
		glVertex3d(0.0, 0.1, 0.0);
		glVertex3d(-0.1, -0.1, 0.0);
		glVertex3d(0.1, -0.1, 0.0);
		glEnd();
		glPopMatrix();
	}

	//Отрисовка объектов рыб
	void drawObj() {
		glPushMatrix();
		glColor3d(color[0], color[1], color[2]);
		glTranslated(position.X(), position.Y(), position.Z());
		glScalef(0.09f, 0.09f, 0.09f);
		fishModel.DrawObj(); 
		glPopMatrix();
	}

};
std::vector<Fish> fishes; //Рыбки

//класс для настройки камеры
class CustomCamera : public Camera
{
public:
	//дистанция камеры
	double camDist;
	//углы поворота камеры
	double fi1, fi2;

	
	//значния масеры по умолчанию
	CustomCamera()
	{
		camDist = 15;
		fi1 = 1;
		fi2 = 1;
	}

	
	//считает позицию камеры, исходя из углов поворота, вызывается движком
	void SetUpCamera()
	{
		//отвечает за поворот камеры мышкой
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
		//функция настройки камеры
		gluLookAt(pos.X(), pos.Y(), pos.Z(), lookPoint.X(), lookPoint.Y(), lookPoint.Z(), normal.X(), normal.Y(), normal.Z());
	}



}  camera;   //создаем объект камеры


//Класс для настройки света
class CustomLight : public Light
{
public:
	CustomLight()
	{
		//начальная позиция света
		pos = Vector3(1, 1, 3);
	}

	
	//рисует сферу и линии под источником света, вызывается движком
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
			//линия от источника света до окружности
			glBegin(GL_LINES);
			glVertex3d(pos.X(), pos.Y(), pos.Z());
			glVertex3d(pos.X(), pos.Y(), 0);
			glEnd();

			//рисуем окруность
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

		// параметры источника света
		glLightfv(GL_LIGHT0, GL_POSITION, position);
		// характеристики излучаемого света
		// фоновое освещение (рассеянный свет)
		glLightfv(GL_LIGHT0, GL_AMBIENT, amb);
		// диффузная составляющая света
		glLightfv(GL_LIGHT0, GL_DIFFUSE, dif);
		// зеркально отражаемая составляющая света
		glLightfv(GL_LIGHT0, GL_SPECULAR, spec);

		glEnable(GL_LIGHT0);
	}


} light;  //создаем источник света




//старые координаты мыши
int mouseX = 0, mouseY = 0;

void mouseEvent(OpenGL *ogl, int mX, int mY)
{
	int dx = mouseX - mX;
	int dy = mouseY - mY;
	mouseX = mX;
	mouseY = mY;

	//меняем углы камеры при нажатой левой кнопке мыши
	if (OpenGL::isKeyPressed(VK_RBUTTON))
	{
		camera.fi1 += 0.01*dx;
		camera.fi2 += -0.01*dy;
	}

	
	//двигаем свет по плоскости, в точку где мышь
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

//выполняется перед первым рендером
void initRender(OpenGL *ogl)
{
	//настройка текстур

	//4 байта на хранение пикселя
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	//настройка режима наложения текстур
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	//включаем текстуры
	glEnable(GL_TEXTURE_2D);
	

	//массив трехбайтных элементов  (R G B)
	RGBTRIPLE *texarray;

	//массив символов, (высота*ширина*4      4, потомучто   выше, мы указали использовать по 4 байта на пиксель текстуры - R G B A)
	char *texCharArray;
	int texW, texH;
	//OpenGL::LoadBMP("texture.bmp", &texW, &texH, &texarray);
	OpenGL::LoadBMP("texture.bmp", &texW, &texH, &texarray);
	OpenGL::RGBtoChar(texarray, texW, texH, &texCharArray);


	
	//генерируем ИД для текстуры
	glGenTextures(1, &texId);
	//биндим айдишник, все что будет происходить с текстурой, будте происходить по этому ИД
	glBindTexture(GL_TEXTURE_2D, texId);

	//загружаем текстуру в видеопямять, в оперативке нам больше  она не нужна
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texW, texH, 0, GL_RGBA, GL_UNSIGNED_BYTE, texCharArray);

	//отчистка памяти
	free(texCharArray);
	free(texarray);

	//наводим шмон
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);


	//камеру и свет привязываем к "движку"
	ogl->mainCamera = &camera;
	ogl->mainLight = &light;

	// нормализация нормалей : их длины будет равна 1
	glEnable(GL_NORMALIZE);

	// устранение ступенчатости для линий
	glEnable(GL_LINE_SMOOTH); 


	//   задать параметры освещения
	//  параметр GL_LIGHT_MODEL_TWO_SIDE - 
	//                0 -  лицевые и изнаночные рисуются одинаково(по умолчанию), 
	//                1 - лицевые и изнаночные обрабатываются разными режимами       
	//                соответственно лицевым и изнаночным свойствам материалов.    
	//  параметр GL_LIGHT_MODEL_AMBIENT - задать фоновое освещение, 
	//                не зависящее от сточников
	// по умолчанию (0.2, 0.2, 0.2, 1.0)

	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 0);

	camera.fi1 = -1.3;
	camera.fi2 = 0.8;

	//Объект с рыбкой
	loadModel("SuperFish.obj", &fishModel);


	// Добавляем рыб
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
	glBegin(GL_QUADS);
	// Верхняя грань
	glVertex3d(-halfSize, halfSize, -halfSize);
	glVertex3d(halfSize, halfSize, -halfSize);
	glVertex3d(halfSize, halfSize, halfSize);
	glVertex3d(-halfSize, halfSize, halfSize);

	// Нижняя грань
	glVertex3d(-halfSize, -halfSize, -halfSize);
	glVertex3d(halfSize, -halfSize, -halfSize);
	glVertex3d(halfSize, -halfSize, halfSize);
	glVertex3d(-halfSize, -halfSize, halfSize);

	// Передняя грань
	glVertex3d(-halfSize, -halfSize, halfSize);
	glVertex3d(halfSize, -halfSize, halfSize);
	glVertex3d(halfSize, halfSize, halfSize);
	glVertex3d(-halfSize, halfSize, halfSize);

	// Задняя грань
	glVertex3d(-halfSize, -halfSize, -halfSize);
	glVertex3d(halfSize, -halfSize, -halfSize);
	glVertex3d(halfSize, halfSize, -halfSize);
	glVertex3d(-halfSize, halfSize, -halfSize);

	// Левая грань
	glVertex3d(-halfSize, -halfSize, -halfSize);
	glVertex3d(-halfSize, -halfSize, halfSize);
	glVertex3d(-halfSize, halfSize, halfSize);
	glVertex3d(-halfSize, halfSize, -halfSize);

	// Правая грань
	glVertex3d(halfSize, -halfSize, -halfSize);
	glVertex3d(halfSize, -halfSize, halfSize);
	glVertex3d(halfSize, halfSize, halfSize);
	glVertex3d(halfSize, halfSize, -halfSize);
	glEnd();
}

void drawTable() {
	// Рисуем столешницу
	glColor3d(0.6, 0.3, 0.1); // Цвет стола
	glPushMatrix();
	glTranslated(0.0, 0.0, -0.54);
	glRotated(90.0, 1.0, 0.0, 0.0); // Поворачиваем стол, чтобы его верх стал параллельным плоскости XY
	glScaled(15, 1, 10); // Увеличиваем площадь стола
	drawCube(1.0); // Куб как основа стола
	glPopMatrix();

	// Рисуем ножки стола
	glPushMatrix();
	glColor3d(0.4, 0.2, 0.1);
	for (double x = -6.5; x <= 6.5; x += 13.0) { // Увеличиваем интервал между ножками
		for (double y = -3.0; y <= 3.0; y += 6.0) {
			glPushMatrix();
			glTranslated(x, y, -5.0); // Подставляем ножки к столу
			glScaled(1.0, 1.0, 10.0); // Увеличиваем длину ножек
			drawCube(1.0); // Куб как ножка стола
			glPopMatrix();
		}
	}
	glPopMatrix();

}

void drawRoom() {
	//Прорисовка пола для комнаты
	glPushMatrix();
	double A[] = { -20, -20, -10 };
	double B[] = { -20, 20, -10 };
	double C[] = { 20, 20, -10 };
	double D[] = { 20, -20, -10 };
	glBegin(GL_QUADS);
	glColor3d(0.0, 0.4, 0.0);
	glVertex3dv(A);
	glVertex3dv(B);
	glVertex3dv(C);
	glVertex3dv(D);
	glEnd();
	glPopMatrix();

	//Делаю стенки для комнаты
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
	// Сначала вывожу низ аквариума
	// Задняя сторона
	glBegin(GL_QUADS);
	glColor3d(0.0, 0.0, 0.0);
	glNormal3f(0.0f, 0.0f, 1.0f);
	glVertex3f(-5.0f, -4.0f, 0.0f);
	glVertex3f(5.0f, -4.0f, 0.0f);
	glVertex3f(5.0f, 4.0f, 0.0f);
	glVertex3f(-5.0f, 4.0f, 0.0f);
	glEnd();
	// Рисуем аквариум
	// Включаем альфа-наложение для прозрачности
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	// Нижняя сторона
	glBegin(GL_QUADS);
	glColor4d(0.5, 0.5, 1.0, 0.7);
	glNormal3f(0.0f, -1.0f, 0.0f);
	glVertex3f(-5.0f, -4.0f, 0.0f);
	glVertex3f(5.0f, -4.0f, 0.0f);
	glVertex3f(5.0f, -4.0f, 6.0f);
	glVertex3f(-5.0f, -4.0f, 6.0f);
	glEnd();

	// Левая сторона
	glBegin(GL_QUADS);
	glColor4d(0.5, 0.5, 1.0, 0.7);
	glNormal3f(-1.0f, 0.0f, 0.0f);
	glVertex3f(-5.0f, -4.0f, 0.0f);
	glVertex3f(-5.0f, -4.0f, 6.0f);
	glVertex3f(-5.0f, 4.0f, 6.0f);
	glVertex3f(-5.0f, 4.0f, 0.0f);
	glEnd();

	// Правая сторона
	glBegin(GL_QUADS);
	glColor4d(0.5, 0.5, 1.0, 0.7);
	glNormal3f(1.0f, 0.0f, 0.0f);
	glVertex3f(5.0f, -4.0f, 0.0f);
	glVertex3f(5.0f, -4.0f, 6.0f);
	glVertex3f(5.0f, 4.0f, 6.0f);
	glVertex3f(5.0f, 4.0f, 0.0f);
	glEnd();

	// Верхняя сторона
	glBegin(GL_QUADS);
	glColor4d(0.5, 0.5, 1.0, 0.7);
	glNormal3f(0.0f, 1.0f, 0.0f);
	glVertex3f(-5.0f, 4.0f, 0.0f);
	glVertex3f(5.0f, 4.0f, 0.0f);
	glVertex3f(5.0f, 4.0f, 6.0f);
	glVertex3f(-5.0f, 4.0f, 6.0f);
	glEnd();

	// Лицевая сторона
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

	//альфаналожение
	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	//настройка материала
	GLfloat amb[] = { 0.2, 0.2, 0.1, 1. };
	GLfloat dif[] = { 0.4, 0.65, 0.5, 1. };
	GLfloat spec[] = { 0.9, 0.8, 0.3, 1. };
	GLfloat sh = 0.1f * 256;


	//фоновая
	glMaterialfv(GL_FRONT, GL_AMBIENT, amb);
	//дифузная
	glMaterialfv(GL_FRONT, GL_DIFFUSE, dif);
	//зеркальная
	glMaterialfv(GL_FRONT, GL_SPECULAR, spec); \
		//размер блика
		glMaterialf(GL_FRONT, GL_SHININESS, sh);

	//чтоб было красиво, без квадратиков (сглаживание освещения)
	glShadeModel(GL_SMOOTH);
	//===================================
	//Прогать тут  
	// Делаю комнату
	drawRoom();
	// Ставлю стол
	drawTable();
	// Рисую грани аквариума
	grani(); 

	// Обновление и отрисовка рыб
	glPushMatrix();
	glBindTexture(GL_TEXTURE_2D, texId); //Добавить текстуру рыбы не получается
	for (Fish& fish : fishes) {
		fish.update();
		fish.drawObj();
	}
	glPopMatrix(); 


	// Рисуем аквариум
	aquar();

   //Сообщение вверху экрана

	
	glMatrixMode(GL_PROJECTION);	//Делаем активной матрицу проекций. 
	                                //(всек матричные операции, будут ее видоизменять.)
	glPushMatrix();   //сохраняем текущую матрицу проецирования (которая описывает перспективную проекцию) в стек 				    
	glLoadIdentity();	  //Загружаем единичную матрицу
	glOrtho(0, ogl->getWidth(), 0, ogl->getHeight(), 0, 1);	 //врубаем режим ортогональной проекции

	glMatrixMode(GL_MODELVIEW);		//переключаемся на модел-вью матрицу
	glPushMatrix();			  //сохраняем текущую матрицу в стек (положение камеры, фактически)
	glLoadIdentity();		  //сбрасываем ее в дефолт

	glDisable(GL_LIGHTING);



	GuiTextRectangle rec;		   //классик моего авторства для удобной работы с рендером текста.
	rec.setSize(300, 150);
	rec.setPosition(10, ogl->getHeight() - 150 - 10);


	std::stringstream ss;
	ss << "T - вкл/выкл текстур" << std::endl;
	ss << "L - вкл/выкл освещение" << std::endl;
	ss << "F - Свет из камеры" << std::endl;
	ss << "G - двигать свет по горизонтали" << std::endl;
	ss << "G+ЛКМ двигать свет по вертекали" << std::endl;
	ss << "Коорд. света: (" << light.pos.X() << ", " << light.pos.Y() << ", " << light.pos.Z() << ")" << std::endl;
	ss << "Коорд. камеры: (" << camera.pos.X() << ", " << camera.pos.Y() << ", " << camera.pos.Z() << ")" << std::endl;
	ss << "Параметры камеры: R="  << camera.camDist << ", fi1=" << camera.fi1 << ", fi2=" << camera.fi2 << std::endl;
	
	rec.setText(ss.str().c_str());
	rec.Draw();

	glMatrixMode(GL_PROJECTION);	  //восстанавливаем матрицы проекции и модел-вью обратьно из стека.
	glPopMatrix();


	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	

}