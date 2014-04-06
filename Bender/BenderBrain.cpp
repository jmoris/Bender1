/* (c) 2013, 2014 - Jesus Moris
 *
 *  This file is part of Bender1.
 *
 *  Leela is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Leela is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Leela. If not, see <http://www.gnu.org/licenses/>.
 */

#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

//Funcion para obtener la imagen con el Threshold
IplImage* GetThresholdedImage(IplImage* img)
{
	// Convertimos la img en HSV
	IplImage* imgHSV = cvCreateImage(cvGetSize(img), 8, 3);
	cvCvtColor(img, imgHSV, CV_BGR2HSV);

	IplImage* imgThreshed = cvCreateImage(cvGetSize(img), 8, 1);

	// Asignamos valores HSV para resaltar la linea
	cvInRangeS(imgHSV, cvScalar(0, 0, 0), cvScalar(185, 255, 95), imgThreshed);

	cvReleaseImage(&imgHSV);
	
	// Se devuelve la posicion de la linea
	return imgThreshed;
}

int main()
{
	// Iniciamos la captura en tiempo real de la camara
	CvCapture* capture = 0;
	capture = cvCaptureFromCAM(0);	

	// Si no hay camara, envia mensaje y se cierra
	if(!capture){
        	cout << "[Bender]No se pudo iniciar la camara." << endl;
        return -1;
    	}

	// Creamos 2 ventanas Original y Threshold
    	cvNamedWindow("Original");
	cvNamedWindow("Threshold");

	// Ciclo sin fin
	for(;;){
		// Creamos una imagen nueva
		IplImage* frame = 0;
		frame = cvQueryFrame(capture);

		// Si la imagen esta vacia se cierra
        if(!frame)
            break;

		// Obtenemos la imagen con Threshold
		IplImage* Linea = GetThresholdedImage(frame);

		// Calcula el momento para obtener una aproximacion de la posicion
		CvMoments *moments = (CvMoments*)malloc(sizeof(CvMoments));
		cvMoments(Linea, moments, 1);

		// Valores actuales del momento
		double moment10 = cvGetSpatialMoment(moments, 1, 0); // X 
		double moment01 = cvGetSpatialMoment(moments, 0, 1); // Y
		double area = cvGetCentralMoment(moments, 0, 0); // Centro

		// Posiciones en el momento
		static int posX = 0;
		static int posY = 0;
		// Mitad de la captura
		int mitad = 240;
		int x1, x2;
		posX = moment10/(area+20);
		posY = moment01/(area+20);
		// Posicion de la linea respecto el centro
		if(posX > 320){
			x2 = 640 - posX;
			x2 = 320 - x2;
			x1 = 0;
		}
		if(posX < 320){
			x1 = 320 - posX;
			x2 = 0;
		}
		// Hacemos un debug de los valores
		cout << "Posicion al centro [" << x1 << "] - [" << x2 << "]" << endl;
		// Dibujamos un rectangulo sobre la linea para ver la posicion
		cvRectangle(frame, cvPoint(posX-60, 200), cvPoint(posX+20, 240), cvScalar(0,255,255));

		//Mostramos los 2 cuadros en las ventanas anteriormente creadas
		cvShowImage("Threshold", Linea);
		cvShowImage("Original", frame);

		// Liberamos la imagen threshold
		cvReleaseImage(&Linea);
		if(waitKey(50) >= 0) break; // SALIR AL APRETAR EL ESCAPE
		delete moments;
    }

	// Cuando se cierre el ciclo liberamos la camara
	cvReleaseCapture(&capture);
    return 0;
}
