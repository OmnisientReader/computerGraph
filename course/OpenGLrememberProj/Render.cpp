#include "Render.h"

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
#include <math.h> 
#define PI 3.14159

bool textureMode = true;
bool lightMode = true;
bool textureReplace = true;		// (Н) Эта переменная отвечает за смену текстур

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

	if (key == 'E')											// (Н) Смена текстуры происходит по этой кнопке
	{
		textureReplace = !textureReplace;
	}
}

void keyUpEvent(OpenGL *ogl, int key)
{
	
}



GLuint texId;
GLuint texId2;									// (Н) Я хз как по другому делать, поэтому я просто скопировал часть кода, ко всем переменным в конце добавил "2", и всё работает

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
	

	//массив трехбайтных элементов  (R G B)																						(Н)	Код брался и копировался отсюда
	RGBTRIPLE *texarray;

	//массив символов, (высота*ширина*4      4, потомучто   выше, мы указали использовать по 4 байта на пиксель текстуры - R G B A)
	char *texCharArray;
	int texW, texH;
	OpenGL::LoadBMP("texture.bmp", &texW, &texH, &texarray);					// (Н) Чтоб не мучаться с картинками, я коприовал старую и рисовал на ней
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
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);																// (Н) И досюда


	//массив трехбайтных элементов  (R G B)
	RGBTRIPLE* texarray2;

	//массив символов, (высота*ширина*4      4, потомучто   выше, мы указали использовать по 4 байта на пиксель текстуры - R G B A)
	char* texCharArray2;
	int texW2, texH2;
	OpenGL::LoadBMP("texture1.bmp", &texW2, &texH2, &texarray2);
	OpenGL::RGBtoChar(texarray2, texW2, texH2, &texCharArray2);


	//генерируем ИД для текстуры
	glGenTextures(1, &texId2);
	//биндим айдишник, все что будет происходить с текстурой, будте происходить по этому ИД
	glBindTexture(GL_TEXTURE_2D, texId2);

	//загружаем текстуру в видеопямять, в оперативке нам больше  она не нужна
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texW2, texH2, 0, GL_RGBA, GL_UNSIGNED_BYTE, texCharArray2);


	//отчистка памяти
	free(texCharArray2);
	free(texarray2);

	//наводим шмон
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);


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

	if (textureReplace)														// (Н) Здесь переключаются текстуры: textureReplace - создаётся на 20 строчке
		glBindTexture(GL_TEXTURE_2D, texId);								// (Н) Назначение кнопки на 225
	else 
		glBindTexture(GL_TEXTURE_2D, texId2);

	
	//альфаналожение
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_DST_ALPHA);
	glDisable(GL_BLEND);


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
	glMaterialfv(GL_FRONT, GL_SPECULAR, spec);
	//размер блика
	glMaterialf(GL_FRONT, GL_SHININESS, sh);

	//чтоб было красиво, без квадратиков (сглаживание освещения)
	glShadeModel(GL_SMOOTH);
	//===================================
	//Прогать тут						(Н) это обман, пришлось прогать не только тут

	double aa[8][3]
	{
	{ 0, 0, 1},
	{ 1, -4, 1},
	{ 8, -2, 1 },
	{ 4, 2, 1},
	{ 6, 6, 1 },
	{ -1, 9, 1 },
	{ 0, 2, 1 },
	{ -5, -1, 1 }
	};


	double circle[101][3];
	double circle1[101][3];

	//double B[] = { };
	//double C[] = { };
	//double D[] = ;//фиол
	//double F[] =
	//double I[] = ;
	//double E[] = ;
	//double G[] = ;
	//double A[] = ;
	double r = 5.0;
	double a = atan(3.0 / 7.0) / PI * 180;
	double angle = (90.0 - a);
	double diagonal = sqrt(53);
	double diagonal1 = sqrt(58);
	double sina = 2.0 / diagonal;
	double cosa = sqrt(1 - sina * sina);
	double sina1 = sin(angle / 180.0 * PI);
	double cosa1 = sqrt(1 - sina1 * sina1);
	double xpart = diagonal / 100.0;
	double xpart1 = diagonal1 / 100.0;
	double waste = sqrt(21 / 2);
	double expart = r - waste;
	double new_x = 0;
	double new_y = 0;


	glBegin(GL_POLYGON);
	glColor3d(1, 1, 0);
	for (int i = 0; i < 101; i++) {

		circle[i][0] = (xpart * (i - 50));
		circle[i][1] = -sqrt(sqrt(pow(((diagonal / 2.0) * (diagonal / 2.0) - (circle[i][0]) * (circle[i][0])), 2)));
		circle[i][2] = 1;

		circle[i][0] = circle[i][0] * cosa - circle[i][1] * sina;
		circle[i][1] = circle[i][0] * sina + circle[i][1] * cosa;

		circle[i][0] += 4.5;
		circle[i][1] -= 2.97;

		if (i != 0) {
			glTexCoord2d(i/100.0, 0.4);
			glVertex3dv(circle[i - 1]);
			glTexCoord2d((i+1)/100.0, 0.4);
			glVertex3dv(circle[i]);
			++circle[i - 1][2];
			++circle[i][2];
			glTexCoord2d(i / 100.0, 0.4+1.0/7.0);
			glVertex3dv(circle[i]);
			glTexCoord2d((i + 1) / 100.0, 0.4+1.0/7.0);
			glVertex3dv(circle[i - 1]);
			--circle[i - 1][2];
			--circle[i][2];
		}

	}
	glEnd();

	for (int k = 0; k < 2; k++) {
		glBegin(GL_POLYGON);
		for (int i = 0; i < 101; i++) {
			circle[i][2] = 1.0 + 1.0 * k;
			glVertex3dv(circle[i]);
			circle[i][2] = 1;
		}
		glEnd();
	}

	glBegin(GL_POLYGON);
	glColor3d(1, 1, 0);
	for (int i = 0; i < 101; i++) {

		circle[i][0] = -(r - expart / 100 * (i));
		circle[i][1] = -sqrt(sqrt(pow(((r) * (r)-(circle[i][0]) * (circle[i][0])), 2)));

		circle1[i][0] = -(r - expart / 100 * (i));
		circle1[i][1] = sqrt(sqrt(pow(((r) * (r)-(circle1[i][0]) * (circle1[i][0])), 2)));

		circle1[i][2] = 1;
		circle[i][2] = 1;

		new_x = circle[i][0] * cosa1 - circle[i][1] * sina1;
		new_y = circle[i][0] * sina1 + circle[i][1] * cosa1;

		circle[i][0] = new_x;
		circle[i][1] = new_y;

		circle[i][0] += 2.5 + waste * cosa1;
		circle[i][1] += 7.5 + waste * sina1;

		new_x = circle1[i][0] * cosa1 - circle1[i][1] * sina1;
		new_y = circle1[i][0] * sina1 + circle1[i][1] * cosa1;

		circle1[i][0] = new_x;
		circle1[i][1] = new_y;

		circle1[i][0] += 2.5 + waste * cosa1;
		circle1[i][1] += 7.5 + waste * sina1;



	}
	/*for (int i = 0; i < 202; i++) {
		if (i != 0) {
			if (i < 101) {
				glVertex3dv(circle1[100 - i]);
				glVertex3dv(circle1[99 - i]);

				++circle1[100 - i][2];
				++circle1[99 - i][2];

				glVertex3dv(circle1[99 - i]);
				glVertex3dv(circle1[100 - i]);

				--circle1[100 - i][2];
				--circle1[99 - i][2];
			}
			else {
				glVertex3dv(circle[i - 101]);
				glVertex3dv(circle[i - 100]);

				++circle[i - 100][2];
				++circle[i - 101][2];

				glVertex3dv(circle[i - 101]);
				glVertex3dv(circle[i - 100]);

				--circle[i - 100][2];
				--circle[i - 101][2];
			}
		}
	}*/
	glEnd();




	for (int j = 0; j < 7; j++)
	{

		{
			if (j == 4) {
				glBegin(GL_QUADS);
				
				/*glColor3d(1, 1, 0);*/
				for (int i = 0; i < 202; i++) {
					if (i != 0) {
						if (i < 101) {
							glTexCoord2d((202.0-i)/202.0, 0.4);
							glVertex3dv(circle1[100 - i]);
							glTexCoord2d((202.0-i-1) / 202.0, 0.4);
							glVertex3dv(circle1[99 - i]);

							++circle1[100 - i][2];
							++circle1[99 - i][2];

							glTexCoord2d((202.0 - i-1) / 202.0, 0.4+1.0/7.0);
							glVertex3dv(circle1[99 - i]);
							glTexCoord2d((202.0 - i) / 202.0, 0.4+1.0/7.0);
							glVertex3dv(circle1[100 - i]);

							--circle1[100 - i][2];
							--circle1[99 - i][2];
						}
						else {
							glTexCoord2d((202.0-i) / 202.0, 0.4);
							glVertex3dv(circle[i - 100]);
							glTexCoord2d((202.0-i-1) / 202.0, 0.4);
							glVertex3dv(circle[i - 101]);

							++circle[i - 100][2];
							++circle[i - 101][2];

							glTexCoord2d((202.0-i) / 202.0,0.4+ 1.0/7.0);
							glVertex3dv(circle[i - 101]);
							glTexCoord2d((202.0-i-1) / 202.0, 0.4+1.0/7.0);
							glVertex3dv(circle[i - 100]);

							--circle[i - 100][2];
							--circle[i - 101][2];
						}
					}
				}
				glEnd();
			}
			else {
				glColor3d(0, 1, 0);
				glBegin(GL_QUADS);
				double x = aa[j + 1][0] - aa[j][0];
				double y = aa[j + 1][1] - aa[j][1];
				double z = aa[j + 1][2] - aa[j][2];
				double x1 = x;
				x = -1 * y;
				y = x1;
				glNormal3d(x,y,0);
				glTexCoord2d(0.0, 0.0);
				glVertex3dv(aa[j]);
				glTexCoord2d(1.0, 0.0);
				glVertex3dv(aa[j + 1]);
				++aa[j][2];
				++aa[j + 1][2];
				glTexCoord2d(1.0, 1.0);
				glVertex3dv(aa[j + 1]);
				glTexCoord2d(0.0, 1.0);
				glVertex3dv(aa[j]);
				--aa[j][2];
				--aa[j + 1][2];

				glEnd();
			}
		}


	}

	glColor3d(0, 1, 0);
	glBegin(GL_POLYGON);

	glVertex3dv(aa[7]);
	glVertex3dv(aa[0]);

	++aa[7][2];
	++aa[0][2];
	glVertex3dv(aa[0]);
	glVertex3dv(aa[7]);


	--aa[7][2];
	--aa[0][2];
	glEnd();


	for (int k = 0; k < 2; k++)
	{


		glBegin(GL_POLYGON);
		glNormal3d(0, 0, 1);
		glNormal3d(0, 0, -1);

		if (k == 0) {
			glColor3d(0, 0, 1);
		}
		else
			glColor3d(1, 0, 0);

		for (int j = 0; j < 8; j++)
		{
			aa[j][2] = 1.0 + 1.0 * k;
		}

		glTexCoord2d(9.0/13.0, 5.0/13.0);
		glVertex3dv(aa[0]);
		glTexCoord2d(1.0, 6.0/13.0);
		glVertex3dv(aa[1]);
		glTexCoord2d(11.0/13.0, 1.0);
		glVertex3dv(aa[2]);
		glTexCoord2d(7.0/13.0, 9.0/13.0);
		glVertex3dv(aa[3]);
		glTexCoord2d(3.0/13.0, 11.0/13.0);
		glVertex3dv(aa[4]);
		for (int i = 0; i < 200; i++) {
			if (i < 100) {
				circle[100 - i][2] = 1.0 + 1.0 * k;
				glTexCoord2d((9.0 - circle[100 - i][1]) / 13.0, (5.0 + circle[100 - i][0]) / 13.0);
				glVertex3dv(circle[100 - i]);
			}
			else {
				circle1[i - 100][2] = 1.0 + 1.0 * k;
				glTexCoord2d((9.0 - circle1[i - 100][1]) / 13.0, (5.0 + circle1[i - 100][0]) / 13.0);
				glVertex3dv(circle1[i - 100]);
			}
		}
		glTexCoord2d(0.0, 4.0/13.0);
		glVertex3dv(aa[5]);
		glTexCoord2d(7.0/13.0, 5.0/13.0);
		glVertex3dv(aa[6]);
		glTexCoord2d(10.0/13.0, 0.0);
		glVertex3dv(aa[7]);

		glEnd();
	}
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
	rec.setSize(300, 200);
	rec.setPosition(10, ogl->getHeight() - 200 - 10);


	std::stringstream ss;
	ss << "T - вкл/выкл текстур" << std::endl;
	ss << "E - Переключение текстур" << std::endl;
	ss << "L - вкл/выкл освещение" << std::endl;
	ss << "F - Свет из камеры" << std::endl;
	ss << "G - двигать свет по горизонтали" << std::endl;
	ss << "G+ЛКМ двигать свет по вертекали" << std::endl;
	ss << "Коорд. света: (" << light.pos.X() << ", " << light.pos.Y() << ", " << light.pos.Z() << ")" << std::endl;
	ss << "Коорд. камеры: (" << camera.pos.X() << ", " << camera.pos.Y() << ", " << camera.pos.Z() << ")" << std::endl;
	ss << "Параметры камеры: R="  << camera.camDist << ", fi1=" << camera.fi1 << ", fi2=" << camera.fi2 << std::endl;
	ss << "UV-развёртка" << std::endl;
	
	rec.setText(ss.str().c_str());
	rec.Draw();

	glMatrixMode(GL_PROJECTION);	  //восстанавливаем матрицы проекции и модел-вью обратьно из стека.
	glPopMatrix();


	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

}