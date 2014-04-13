/* (c) 2013, 2014 - Jesus Moris
 *
 *  This file is part of Bender1.
 *
 *  Leela is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Bender is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Leela. If not, see <http://www.gnu.org/licenses/>.
 */

#include <opencv2/opencv.hpp>
#include <stdio.h>

using namespace std;
using namespace cv;

int x1, x2;
// Posiciones en el momento
static int posX = 0;
static int posY = 0;

IplImage* Cortar( IplImage* src,  CvRect roi){
  // Must have dimensions of output image
  IplImage* cropped = cvCreateImage( cvSize(roi.width,roi.height),src->depth,src->nChannels);

  // Say what the source region is
  cvSetImageROI( src, roi );

  // Do the copy
  cvCopy( src, cropped );
  cvResetImageROI( src );

  return cropped;
}

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

void LimpiarX(double posx){
	if(posX > 320){
		x2 = 640 - posX;
		x2 = 320 - x2;
		x1 = 0;
	}
	if(posX < 320){
		x1 = 320 - posX;
		x2 = 0;
	}
	//Calculamos una constante respecto a la mitad de la pantalla, 1.028
	if((x1*1.028) > 320){
		x1=320;
	}else{
		x1=x1*1.028;
	}
	if((x2*1.028) > 320){
		x2=320;
	}else{
		x2=x2*1.028;
	}
	if(x1 < 0){
		x1 = 0;
	}
	if(x2 < 0){
		x2 = 0;
	}
}

int main()
{
	// Iniciamos la captura en tiempo real de la camara
	/*CvCapture* capture = 0;
	capture = cvCaptureFromCAM(0);	
	// Si no hay camara, envia mensaje y se cierra
	if(!capture){
        	cout << "[Bender]No se pudo iniciar la camara." << endl;
        return -1;
    	}*/
	VideoCapture cap(0);
	// Creamos 2 ventanas Original y Threshold
    	cvNamedWindow("Original");
	cvNamedWindow("Threshold");

	// Ciclo sin fin
	for(;;){
		Mat srx;
		cap >> srx;
		IplImage copy = srx;
		// Creamos una imagen nueva
		IplImage* frame = 0;
		frame = &copy;

		// Si la imagen esta vacia se cierra
        if(!frame)
            break;
		IplImage* cropped = Cortar(frame, cvRect( 0, 160, 640,200 ));

		// Obtenemos la imagen con Threshold
		IplImage* Linea = GetThresholdedImage(cropped);

		// Calcula el momento para obtener una aproximacion de la posicion
		CvMoments *moments = (CvMoments*)malloc(sizeof(CvMoments));
		cvMoments(Linea, moments, 1);

		// Valores actualesmi del momento
		double moment10 = cvGetSpatialMoment(moments, 1, 0); // X 
		double area = cvGetCentralMoment(moments, 0, 0); // Centro

		posX = moment10/area;
		// Posicion de la linea respecto el centro
		LimpiarX(posX);
		// Hacemos un debug de los valores
		cout << "Posicion al centro [" << x1 << "] - [" << x2 << "]" << endl;
		// Dibujamos un rectangulo sobre la linea para ver la posicion
		cvRectangle(frame, cvPoint(0, 160), cvPoint(640, 320), cvScalar(0, 0, 255), 2);
		cvRectangle(frame, cvPoint(posX-40, 220), cvPoint(posX+40, 260), cvScalar(0,255,255));
		cvLine(frame, cvPoint(320, 0), cvPoint(320, 480), cvScalar(0,255,255), 2);
		char punto1[16];
		char punto2[16];
		sprintf(punto1,"%d", x1);
		sprintf(punto2, "%d", x2);
		CvFont font;
		cvInitFont(&font, CV_FONT_HERSHEY_COMPLEX, 0.8, 0.8);
		cvPutText(frame, punto1, cvPoint(50, 220), &font, cvScalar(0,255,255));
		cvPutText(frame, punto2, cvPoint(550, 220), &font, cvScalar(0,255,255));
		//Mostramos los 2 cuadros en las ventanas anteriormente creadas
		cvShowImage("Threshold", Linea);
		cvShowImage("Original", frame);

		// Liberamos la imagen threshold
		cvReleaseImage(&Linea);
		if(waitKey(50) >= 0){ break; } // SALIR AL APRETAR EL ESCAPE
		delete moments;
    }

	// Cuando se cierre el ciclo liberamos la camara
	//cvReleaseCapture(&capture);
    return 0;
}
