#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include <gtkgl/gtkglarea.h>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <GL/gl.h>
#include <math.h>

GtkWidget       *MainWindow;
GtkWidget       *MainCamArea;
GtkWidget       *MWContainerCam;
GtkWidget       *OutputFileSelect;
GtkWidget       *ImageLoadButton;

GtkWidget       *EqualizeHistogramButton;
GtkWidget       *GammaCorrectButton;
GtkWidget       *LogTransformButton;
GtkWidget       *GaussianBlurButton;
GtkWidget       *ButterworthSharpeningButton;
GtkWidget       *UndoChangesButton;
GtkWidget       *SaveImageButton;
GtkWidget       *ThresholdSlider;

GtkWidget       *DisplayImageButton;
GtkWidget       *DisplayHistogramButton;
GtkWidget       *DisplayFourierMagnitudeButton;
GtkWidget       *DisplayFourierPhaseButton;
int             AttrList[] = {GDK_GL_RGBA,GDK_GL_DEPTH_SIZE, 24,
                    GDK_GL_DOUBLEBUFFER, GDK_GL_NONE};
int             BtnNum;
static GLuint   Texture;
char Filename[1024];

cv::Mat img,temp,out;
IplImage Glimg;

void InitTexture2(IplImage *Image)
{
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glShadeModel(GL_FLAT);
    glEnable(GL_DEPTH_TEST);

    glGenTextures(1, &Texture);
    glBindTexture(GL_TEXTURE_2D, Texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    GLint swizzleMask[] = {GL_RED, GL_RED, GL_RED, GL_RED};
glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glTexImage2D(GL_TEXTURE_2D, 0, 3, Image->width, Image->height, 0,
        GL_RED, GL_UNSIGNED_BYTE, Image->imageData);
}
void InitTexture(IplImage *Image)
{
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glShadeModel(GL_FLAT);
    glEnable(GL_DEPTH_TEST);

    glGenTextures(1, &Texture);
    glBindTexture(GL_TEXTURE_2D, Texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glTexImage2D(GL_TEXTURE_2D, 0, 3, Image->width, Image->height, 0,
        GL_BGR, GL_UNSIGNED_BYTE, Image->imageData);
}

void ApplyTexture(int DisplayWidth,
    int DisplayHeight)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_TEXTURE_2D);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
    glBindTexture(GL_TEXTURE_2D, Texture);
    glBegin(GL_QUADS);
    
    glTexCoord2f(0, 0);
    glVertex3f(0, 0, 0);
    glTexCoord2f(0, 1);
    glVertex3f(0, DisplayHeight, 0);
    glTexCoord2f(1, 1);
    glVertex3f(DisplayWidth, DisplayHeight, 0);
    glTexCoord2f(1, 0);
    glVertex3f(DisplayWidth, 0, 0);
    
    glEnd();
    glFlush();
    glDisable(GL_TEXTURE_2D);
}
void LoadImage2(IplImage *Image, int DisplayWidth, int DisplayHeight)
{
    InitTexture2(Image);
    glViewport(0, 0, DisplayWidth , DisplayHeight);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, DisplayWidth, DisplayHeight,0, 0, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    ApplyTexture(DisplayWidth, DisplayHeight);
    gtk_gl_area_swapbuffers(GTK_GL_AREA(MainCamArea));
}
void LoadImage(IplImage *Image, int DisplayWidth, int DisplayHeight)
{
    InitTexture(Image);
    glViewport(0, 0, DisplayWidth , DisplayHeight);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, DisplayWidth, DisplayHeight,0, 0, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    ApplyTexture(DisplayWidth, DisplayHeight);
    gtk_gl_area_swapbuffers(GTK_GL_AREA(MainCamArea));
}

cv::Mat GammaCorrection(cv::Mat& img, double gamma) 
{
    double inverse_gamma = 1.0 / gamma;
    cv::Mat lut_matrix(1, 256, CV_8UC1 );
    uchar * ptr = lut_matrix.ptr();
    for( int i = 0; i < 256; i++ )
    ptr[i] = (int)( pow( (double) i / 255.0, inverse_gamma ) * 255.0 );
    cv::Mat result;
    LUT( img, lut_matrix, result );
    return result;
}

void InitAll()
{
    MainCamArea = gtk_gl_area_new (AttrList);
    MWContainerCam = gtk_layout_new(NULL, NULL);
    ThresholdSlider = gtk_spin_button_new_with_range((gdouble) 0,
        (gdouble)2, (gdouble)0.1);

    EqualizeHistogramButton = gtk_button_new_with_mnemonic("Equalize Histogram");
    ImageLoadButton = gtk_button_new_with_mnemonic("Load Image");
    GammaCorrectButton = gtk_button_new_with_mnemonic("Gamma Correct");
    LogTransformButton = gtk_button_new_with_mnemonic("Log Transform");
    GaussianBlurButton = gtk_button_new_with_mnemonic("Gaussian Blur");
    ButterworthSharpeningButton = gtk_button_new_with_mnemonic("Butterworth");
    UndoChangesButton = gtk_button_new_with_mnemonic("Undo Changes");
    SaveImageButton = gtk_button_new_with_mnemonic("Save Image");
    
    DisplayImageButton = gtk_button_new_with_mnemonic("Display Image");
    DisplayHistogramButton = gtk_button_new_with_mnemonic("Show Histogram");
    DisplayFourierMagnitudeButton = gtk_button_new_with_mnemonic("Fourier Magnitude");
    DisplayFourierPhaseButton = gtk_button_new_with_mnemonic("Fourier Phase");
    

    gtk_widget_set_size_request(MainCamArea,
        640, 480);
    gtk_widget_set_usize(MainCamArea, 640, 480);
    
    gtk_widget_set_size_request(MWContainerCam,
        640 + 50, 600);
    gtk_widget_set_usize(MWContainerCam, 640 + 50, 600);
    
    gtk_layout_put(GTK_LAYOUT(MWContainerCam), ThresholdSlider ,
        640 +50, 240);

    gtk_layout_put(GTK_LAYOUT(MWContainerCam), MainCamArea, 50, 50);
    gtk_layout_put(GTK_LAYOUT(MWContainerCam), ImageLoadButton,
        50, 50 + 480 + 10);
    gtk_layout_put(GTK_LAYOUT (MWContainerCam), GammaCorrectButton,
        20 + 150, 50 + 480 + 10);
    gtk_layout_put(GTK_LAYOUT (MWContainerCam), EqualizeHistogramButton,
        20 + (150 * 2), 50 + 480 + 10);
    gtk_layout_put(GTK_LAYOUT (MWContainerCam), LogTransformButton,
        50 + (150 * 3), 50 + 480 + 10);
        
    gtk_layout_put(GTK_LAYOUT (MWContainerCam), GaussianBlurButton,
        50, 100 + 480 + 10);
    gtk_layout_put(GTK_LAYOUT (MWContainerCam), ButterworthSharpeningButton,
        50 + (150), 100 + 480 + 10);
    gtk_layout_put(GTK_LAYOUT (MWContainerCam), UndoChangesButton,
        50 + (150 * 2), 100 + 480 + 10);
    gtk_layout_put(GTK_LAYOUT (MWContainerCam), SaveImageButton,
        50 + (150 * 3), 100 + 480 + 10);
        
    gtk_layout_put(GTK_LAYOUT (MWContainerCam), DisplayImageButton,
        50 , 150 + 480 + 10);
    gtk_layout_put(GTK_LAYOUT (MWContainerCam), DisplayHistogramButton,
        50 + (150), 150 + 480 + 10);
    gtk_layout_put(GTK_LAYOUT (MWContainerCam), DisplayFourierMagnitudeButton,
        50 + (150 * 2), 150 + 480 + 10);
    gtk_layout_put(GTK_LAYOUT (MWContainerCam), DisplayFourierPhaseButton,
        50 + (150 * 3), 150 + 480 + 10);

    gtk_container_add(GTK_CONTAINER(MainWindow), MWContainerCam);
    gtk_widget_show_all(MWContainerCam);

    gtk_widget_set_events(GTK_WIDGET(MainCamArea), GDK_EXPOSURE_MASK);
    gtk_quit_add_destroy(1, GTK_OBJECT(MainCamArea));
}
void MainWindowClosing(GtkWidget *widget, gpointer callback_data)
{
    gtk_main_quit();
}
static void ValidateFileName(GtkWidget *w, GtkFileSelection *fs)
{
     if(BtnNum==1)
     {
    try
    {
        img = cv::imread((char*)gtk_file_selection_get_filename(
            GTK_FILE_SELECTION(fs)), CV_LOAD_IMAGE_COLOR);
    }
    catch(...)
    {
        return;
    }
    if(!img.data)
    {
        return;
    }
    else
    {
        //cv::flip(img, img, 0);
        cvtColor(img, temp, CV_RGB2GRAY,3);
        Glimg=temp;
        if (gtk_gl_area_begingl(GTK_GL_AREA(MainCamArea)))
            {
                LoadImage2(&Glimg, 640, 480);
                gtk_gl_area_endgl(GTK_GL_AREA(MainCamArea));
            }
    }
    }
    else if(BtnNum==2)
    {
        cv::imwrite((char*)gtk_file_selection_get_filename(
            GTK_FILE_SELECTION(fs)), temp);
    }
    gtk_widget_destroy(OutputFileSelect);
    
}

void GetImgLocation()
{
    BtnNum = 1;
    OutputFileSelect = gtk_file_selection_new("Open File");
    g_signal_connect (GTK_FILE_SELECTION (OutputFileSelect)->ok_button,
       "clicked", G_CALLBACK(ValidateFileName),
       (gpointer) OutputFileSelect);
    g_signal_connect_swapped(
    GTK_FILE_SELECTION(OutputFileSelect)->cancel_button, "clicked",
    G_CALLBACK(gtk_widget_destroy), OutputFileSelect);
        gtk_widget_show(OutputFileSelect);
}
void SetImgLocation()
{
    BtnNum = 2;
    OutputFileSelect = gtk_file_selection_new("Save File");
    g_signal_connect (GTK_FILE_SELECTION (OutputFileSelect)->ok_button,
       "clicked", G_CALLBACK(ValidateFileName),
       (gpointer) OutputFileSelect);
    g_signal_connect_swapped(
    GTK_FILE_SELECTION(OutputFileSelect)->cancel_button, "clicked",
    G_CALLBACK(gtk_widget_destroy), OutputFileSelect);
        gtk_widget_show(OutputFileSelect);
}
void LogTransform()
{
    cv::Mat fg;
    temp.convertTo(fg,CV_32F);
    fg = fg + 1;
    cv::log(fg,fg);
    cv::convertScaleAbs(fg,fg);
    cv::normalize(fg,temp,0,255,cv::NORM_MINMAX);
}

void ButterworthSharpening(cv::Mat &dft_Filter, int D, int n, int W)
{
    cv::Mat tmp = cv::Mat(dft_Filter.rows, dft_Filter.cols, CV_32F);

    cv::Point centre = cv::Point(dft_Filter.rows / 2, dft_Filter.cols / 2);
    double radius;
    for (int i = 0; i < dft_Filter.rows; i++)
    {
        for (int j = 0; j < dft_Filter.cols; j++)
        {
            radius = (double) sqrt(pow((i - centre.x),
                2.0) + pow((double) (j - centre.y), 2.0));
            tmp.at<float>(i, j) = (float)( 1 / (1 + pow((double) (radius * W) / 
                ( pow((double)radius, 2) - D * D ), (double) (2 * n))));
        }
    }

    cv::Mat toMerge[] = {tmp, tmp};
    merge(toMerge, 2, dft_Filter);
}
void DisplayImage()
{
    Glimg=temp;
            if (gtk_gl_area_begingl(GTK_GL_AREA(MainCamArea)))
            {
                LoadImage2(&Glimg, 640, 480);
                gtk_gl_area_endgl(GTK_GL_AREA(MainCamArea));
            }
}

void DisplayHistogram()
{
cv::vector<cv::Mat> bgr_planes;
  split( img, bgr_planes );
  int histSize = 256;

  float range[] = { 0, 256 } ;
  const float* histRange = { range };

  bool uniform = true; bool accumulate = false;

  cv::Mat b_hist, g_hist, r_hist;

  calcHist( &bgr_planes[0], 1, 0, cv::Mat(), b_hist, 1, &histSize, &histRange, uniform, accumulate );
  calcHist( &bgr_planes[1], 1, 0, cv::Mat(), g_hist, 1, &histSize, &histRange, uniform, accumulate );
  calcHist( &bgr_planes[2], 1, 0, cv::Mat(), r_hist, 1, &histSize, &histRange, uniform, accumulate );

  int hist_w = 512; int hist_h = 400;
  int bin_w = cvRound( (double) hist_w/histSize );

  cv::Mat histImage( hist_h, hist_w, CV_8UC3, cv::Scalar( 0,0,0) );

  normalize(b_hist, b_hist, 0, histImage.rows, cv::NORM_MINMAX, -1, cv::Mat() );
  normalize(g_hist, g_hist, 0, histImage.rows, cv::NORM_MINMAX, -1, cv::Mat() );
  normalize(r_hist, r_hist, 0, histImage.rows, cv::NORM_MINMAX, -1, cv::Mat() );

  for( int i = 1; i < histSize; i++ )
  {
      line( histImage, cv::Point( bin_w*(i-1), hist_h - cvRound(b_hist.at<float>(i-1)) ) ,
                       cv::Point( bin_w*(i), hist_h - cvRound(b_hist.at<float>(i)) ),
                       cv::Scalar( 255, 0, 0), 2, 8, 0  );
      line( histImage, cv::Point( bin_w*(i-1), hist_h - cvRound(g_hist.at<float>(i-1)) ) ,
                       cv::Point( bin_w*(i), hist_h - cvRound(g_hist.at<float>(i)) ),
                       cv::Scalar( 0, 255, 0), 2, 8, 0  );
      line( histImage, cv::Point( bin_w*(i-1), hist_h - cvRound(r_hist.at<float>(i-1)) ) ,
                       cv::Point( bin_w*(i), hist_h - cvRound(r_hist.at<float>(i)) ),
                       cv::Scalar( 0, 0, 255), 2, 8, 0  );
  }
  Glimg=histImage;
            if (gtk_gl_area_begingl(GTK_GL_AREA(MainCamArea)))
            {
                LoadImage(&Glimg, 640, 480);
                gtk_gl_area_endgl(GTK_GL_AREA(MainCamArea));
            }
}

void DisplayFourierMagnitude()
{
    cv::Mat padded;
    int m = cv::getOptimalDFTSize( temp.rows );
    int n = cv::getOptimalDFTSize( temp.cols );
    copyMakeBorder(temp, padded, 0, m - temp.rows, 0, n - temp.cols, cv::BORDER_CONSTANT, cv::Scalar::all(0));

    cv::Mat planes[] = {cv::Mat_<float>(padded), cv::Mat::zeros(padded.size(), CV_32F)};
    cv::Mat complexI;
    cv::merge(planes, 2, complexI);

    cv::dft(complexI, complexI);
    cv::split(complexI, planes);
    cv::magnitude(planes[0], planes[1], planes[0]);
    cv::Mat magI = planes[0];

    magI += cv::Scalar::all(1);
    log(magI, magI);

    magI = magI(cv::Rect(0, 0, magI.cols & -2, magI.rows & -2));

    int cx = magI.cols/2;
    int cy = magI.rows/2;

    cv::Mat q0(magI, cv::Rect(0, 0, cx, cy));
    cv::Mat q1(magI, cv::Rect(cx, 0, cx, cy));
    cv::Mat q2(magI, cv::Rect(0, cy, cx, cy));
    cv::Mat q3(magI, cv::Rect(cx, cy, cx, cy));

    cv::Mat tmp;
    q0.copyTo(tmp);
    q3.copyTo(q0);
    tmp.copyTo(q3);

    q1.copyTo(tmp);
    q2.copyTo(q1);
    tmp.copyTo(q2);

    cv::normalize(magI, magI, 0, 1, CV_MINMAX);
    Glimg=magI;
            if (gtk_gl_area_begingl(GTK_GL_AREA(MainCamArea)))
            {
                LoadImage2(&Glimg, 640, 480);
                gtk_gl_area_endgl(GTK_GL_AREA(MainCamArea));
            }
}
void DisplayFourierPhase()
{
    cv::Mat padded;
    int m = cv::getOptimalDFTSize( temp.rows );
    int n = cv::getOptimalDFTSize( temp.cols );
    copyMakeBorder(temp, padded, 0, m - temp.rows, 0, n - temp.cols, cv::BORDER_CONSTANT, cv::Scalar::all(0));

    cv::Mat planes[] = {cv::Mat_<float>(padded), cv::Mat::zeros(padded.size(), CV_32F)};
    cv::Mat complexI;
    cv::merge(planes, 2, complexI);

    cv::dft(complexI, complexI);
    cv::split(complexI, planes);
    cv::phase(planes[0], planes[1], planes[0]);
    cv::Mat magI = planes[0];

    magI += cv::Scalar::all(1);
    log(magI, magI);

    magI = magI(cv::Rect(0, 0, magI.cols & -2, magI.rows & -2));

    int cx = magI.cols/2;
    int cy = magI.rows/2;

    cv::Mat q0(magI, cv::Rect(0, 0, cx, cy));
    cv::Mat q1(magI, cv::Rect(cx, 0, cx, cy));
    cv::Mat q2(magI, cv::Rect(0, cy, cx, cy));
    cv::Mat q3(magI, cv::Rect(cx, cy, cx, cy));

    cv::Mat tmp;
    q0.copyTo(tmp);
    q3.copyTo(q0);
    tmp.copyTo(q3);

    q1.copyTo(tmp);
    q2.copyTo(q1);
    tmp.copyTo(q2);

    cv::normalize(magI, magI, 0, 1, CV_MINMAX);
    Glimg=magI;
            if (gtk_gl_area_begingl(GTK_GL_AREA(MainCamArea)))
            {
                LoadImage2(&Glimg, 640, 480);
                gtk_gl_area_endgl(GTK_GL_AREA(MainCamArea));
            }
}
static void ButtonClicked(GtkWidget *w, gpointer data)
{
    int Btn = atoi((char*)data);
    switch(Btn)
    {
        case 0:
            GetImgLocation();
        break;
        case 1:
            equalizeHist(temp,temp);
            Glimg=temp;
            if (gtk_gl_area_begingl(GTK_GL_AREA(MainCamArea)))
            {
                LoadImage2(&Glimg, 640, 480);
                gtk_gl_area_endgl(GTK_GL_AREA(MainCamArea));
            }
        break;
        case 2:
            temp = GammaCorrection(temp,(double)gtk_spin_button_get_value(
                GTK_SPIN_BUTTON(ThresholdSlider)));
            Glimg=temp;
            if (gtk_gl_area_begingl(GTK_GL_AREA(MainCamArea)))
            {
                LoadImage2(&Glimg, 640, 480);
                gtk_gl_area_endgl(GTK_GL_AREA(MainCamArea));
            }
        break;
        case 3:
            LogTransform();
            Glimg=temp;
            if (gtk_gl_area_begingl(GTK_GL_AREA(MainCamArea)))
            {
                LoadImage2(&Glimg, 640, 480);
                gtk_gl_area_endgl(GTK_GL_AREA(MainCamArea));
            }
        break;
        case 4:
            GaussianBlur(temp, temp, cv::Size( 3, 3 ), 0, 0 );
            Glimg=temp;
            if (gtk_gl_area_begingl(GTK_GL_AREA(MainCamArea)))
            {
                LoadImage2(&Glimg, 640, 480);
                gtk_gl_area_endgl(GTK_GL_AREA(MainCamArea));
            }
        break;
        case 5:
            ButterworthSharpening(temp,3,2,5);
            Glimg=temp;
            if (gtk_gl_area_begingl(GTK_GL_AREA(MainCamArea)))
            {
                LoadImage2(&Glimg, 640, 480);
                gtk_gl_area_endgl(GTK_GL_AREA(MainCamArea));
            }
        break;
        case 6:
            cvtColor(img, temp, CV_RGB2GRAY,3);
            Glimg=temp;
            if (gtk_gl_area_begingl(GTK_GL_AREA(MainCamArea)))
            {
                LoadImage2(&Glimg, 640, 480);
                gtk_gl_area_endgl(GTK_GL_AREA(MainCamArea));
            }
        break;
        case 7:
            //SaveImage();
            SetImgLocation();
        break;
        case 8:
            DisplayImage();
        break;
        case 9:
            DisplayHistogram();
        break;
        case 10:
            DisplayFourierMagnitude();
        break;
        case 11:
            DisplayFourierPhase();
        break;
    }
}

int main(int argc, char *argv[])
{

    gdk_threads_init();

    gdk_threads_enter();

    gtk_init(&argc, &argv);
    MainWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title (GTK_WINDOW (MainWindow), "Image Editor");
    gtk_window_set_position(GTK_WINDOW(MainWindow), GTK_WIN_POS_CENTER);
    gtk_window_set_default_size(GTK_WINDOW(MainWindow), 640 , 480);
    gtk_window_maximize (GTK_WINDOW (MainWindow));
    InitAll();
    g_signal_connect(G_OBJECT(MainWindow), "destroy",
        G_CALLBACK(MainWindowClosing), NULL);
    g_signal_connect (ImageLoadButton, "clicked",
        G_CALLBACK (ButtonClicked), (gpointer) "0");
        
    g_signal_connect (EqualizeHistogramButton, "clicked",
        G_CALLBACK (ButtonClicked), (gpointer) "1");
    g_signal_connect (GammaCorrectButton, "clicked",
        G_CALLBACK (ButtonClicked), (gpointer) "2");
    g_signal_connect (LogTransformButton, "clicked",
        G_CALLBACK (ButtonClicked), (gpointer) "3");
    g_signal_connect (GaussianBlurButton, "clicked",
        G_CALLBACK (ButtonClicked), (gpointer) "4");
    g_signal_connect (ButterworthSharpeningButton, "clicked",
        G_CALLBACK (ButtonClicked), (gpointer) "5");
    g_signal_connect (UndoChangesButton, "clicked",
        G_CALLBACK (ButtonClicked), (gpointer) "6");
    g_signal_connect (SaveImageButton, "clicked",
        G_CALLBACK (ButtonClicked), (gpointer) "7");
    g_signal_connect (DisplayImageButton, "clicked",
        G_CALLBACK (ButtonClicked), (gpointer) "8");
    g_signal_connect (DisplayHistogramButton, "clicked",
        G_CALLBACK (ButtonClicked), (gpointer) "9");
    g_signal_connect (DisplayFourierMagnitudeButton, "clicked",
        G_CALLBACK (ButtonClicked), (gpointer) "10");
    g_signal_connect (DisplayFourierPhaseButton, "clicked",
        G_CALLBACK (ButtonClicked), (gpointer) "11");

    gtk_widget_show_all(MainWindow);
    gtk_main();
    gdk_threads_leave();
    return 0;
}
