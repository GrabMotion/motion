    /*
 * File:   main.cpp
 * Author: jose
 *
 * Created on April 19, 2015, 11:23 PM
 */
  
//
//  Created by Cedric Verstraeten on 18/02/14.
//  Copyright (c) 2014 Cedric Verstraeten. All rights reserved.
//
  
#include "../recognition/detection.h"

  
#include <iostream>
#include <fstream>
#include <time.h>
#include <dirent.h>
#include <string>
#include <sstream>
#include <sys/types.h>
#include <sys/stat.h>
  
#include "opencv2/opencv.hpp"
#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"

#include "opencv/cv.h"
#include "opencv/highgui.h"
#include "opencv/cxcore.h"
  
#include "../database/database.h"
  
#include <string>
#include <vector>
#include <functional>
#include <ctime>
#include <unistd.h>
#include <cstring>
#include <algorithm>
#include <new>
#include <fstream>

#include <signal.h>
  
#include <sys/time.h>
  
using namespace std;
using namespace cv;

/*motion::Message::MotionCamera * pcamera;
motion::Message::MotionMonth * pmonth;
motion::Message::MotionDay * pday;
motion::Message::Instance * pinstance;*/
cv::VideoWriter * videout;
//CvVideoWriter * cvvideout;

int db_recognition_setup_id;

int matrows = 0;
int matcols = 0;

pthread_t thread_store_instance, thread_image;
extern pthread_t thread_recognition;

volatile int running_image_threads = 0;
pthread_mutex_t running_image_mutex = PTHREAD_MUTEX_INITIALIZER;

int there_is_motion;
pthread_t thread_store;
void * storeproto(void * args);
/*struct proto_args
{
    motion::Message::Instance * pinstance;
    char * datasend;
    std::string DIR;
    std::string XML_FILE;
    std::string EXT_DATA;
    time_t end, begin_time, end_time, init_time;
    std::string instance;
    std::string instancecode;
};
struct proto_args ProtoArgs;*/

void * storeimage(void * args);
struct image_args
{
    Mat mat;
    std::string path;
};
struct image_args ImageArgs;
  
std::vector<cv::Point2f> stringToVectorPoint2f(std::string storedcoord)
{
    vector<Point2f> coordinates;
    std::stringstream ss(storedcoord);
    string d;
    int c=0, x=0, y=0, t=0;
    while (ss >> d)
    {
        d = d.erase( d.size() - 1 );
        bool fi = d.find("[");
        if (!fi)
        {
            d = d.erase(0 , 1);
        }
         
        float cor =  atof (d.c_str());
        if (c==0)
        {
            x = cor;
            c++;
        } else
        {
            y = cor;
            cv::Point2f p(x, y);
            coordinates.push_back(p);
            c=0;x=0;y=0;
        }
        t++;
    }
    return coordinates;
}
  
vector<Point2f> processRegionString(std::string coordstring)
{
    vector<Point2f> cvectro = stringToVectorPoint2f(coordstring);
    vector<Point2f> insideContour;
    for(int j = 0; j < matrows; j++)
    {
        for(int i = 0; i < matcols; i++)
        {
            Point2f p(i,j);
            if(pointPolygonTest(cvectro,p,false) >= 0) // yes inside
                insideContour.push_back(p);
        }
    }
    cout << "# points inside contour: " << insideContour.size() << endl;
    return insideContour;
}
  
char * CharArrayPlusChar( const char *array, char c )
{
    size_t sz = std::strlen( array );
    char *s = new char[sz + 2];
     
    std::strcpy( s, array );
    s[sz] = c;
    s[sz + 1] = '\0';
     
    return ( s );
}
   
// Check if the directory exists, if not create it
// This function will create a new directory if the image is the first
// image taken for a specific day
inline void directoryExistsOrCreate(const char* pzPath)
{
    DIR *pDir;
    // directory doesn't exists -> create it
    if ( pzPath == NULL || (pDir = opendir (pzPath)) == NULL)
        mkdir(pzPath, 0777);
    // if directory exists we opened it and we
    // have to close the directory again.
    else if(pDir != NULL)
        (void) closedir (pDir);
}

void * storeimage(void * args)
{
    struct image_args *arg = (struct image_args *) args;
    pthread_mutex_lock(&running_image_mutex);
    imwrite(arg->path, arg->mat);
    running_image_threads--;
    pthread_mutex_unlock(&running_image_mutex);
}
  
// When motion is detected we write the image to disk
//    - Check if the directory exists where the image will be stored.
//    - Build the directory and image names.
int incr = 0;
inline string saveImg(
                      Mat image,
                      const string DIRECTORY,
                      const string EXTENSION,
                      const char * DIR_FORMAT,
                      const char * FILE_FORMAT,
                      vector<int> n_o_changes,
                      motion::Message::Image * pimage,
                      motion::Message::Crop * pcrop)
{
    
    string image_file;
    
    try
    {

        stringstream ss;
        time_t seconds;
        struct tm * timeinfo;
        char TIME[80];
        time (&seconds);
        // Get the current time
        timeinfo = localtime (&seconds);

        std::string amount_str = getGlobalIntToString(n_o_changes.at(0));

        std::string n_str_file = "_" + amount_str;

        char * n_file = new char[n_str_file.size() + 1];
        std::copy(n_str_file.begin(), n_str_file.end(), n_file);
        n_file[n_str_file.size()] = '\0';

        // Create name for the image
        strftime (TIME,80,FILE_FORMAT,timeinfo);
        
        stringstream imagename;
        imagename << TIME << static_cast<int>(incr) << n_file;
        ss << DIRECTORY << imagename.str() << EXTENSION;
        
        image_file = ss.str().c_str();

        ImageArgs.mat = image;
        ImageArgs.path = image_file;
        
        pthread_mutex_lock(&running_image_mutex);
        running_image_threads++;
        pthread_mutex_unlock(&running_image_mutex);
        
        pthread_create(&thread_image, NULL, storeimage, &ImageArgs);

        struct timeval tr;
        struct tm* ptmr;
        char time_info[40];
        gettimeofday (&tr, NULL);
        ptmr = localtime (&tr.tv_sec);
        strftime (time_info, sizeof (time_info), "%Y-%m-%d %H:%M:%S %z", ptmr);
        //cout <<  ":::time_info:::: " << time_info << endl;

        stringstream rct;
        rct << n_o_changes.at(1) << " " << n_o_changes.at(2) << " " << n_o_changes.at(3) << " " << n_o_changes.at(4); 
        //cout << "rct: " << rct.str() << endl; 

        pthread_mutex_lock(&protoMutex);
        pimage->set_path(image_file.c_str());
        pimage->set_name(imagename.str());
        pimage->set_imagechanges(n_o_changes.at(0));
        pimage->set_time(time_info);
        pcrop->set_rect(rct.str());
        pthread_mutex_unlock(&protoMutex);
    
    }
    catch (std::bad_alloc& ba)
    {
      std::cerr << "bad_alloc caught: " << ba.what() << '\n';
    } 

    return image_file;
}
  
// When motion is detected we write the image to disk
//    - Check if the directory exists where the image will be stored.
//    - Build the directory and image names.
inline vector<string> createDirectoryTree(
                                const string DIRECTORY,
                                const string EXTENSION,
                                const char * DIR_FORMAT,
                                std::string instance
                                )
{
    vector<string> result;
    std::stringstream ss, zz;
    time_t seconds;
    struct tm * timeinfo;
    char TIME[80];
    time (&seconds);
    // Get the current time
    timeinfo = localtime (&seconds);
     
    // Create name for the date directory
    strftime (TIME,80,DIR_FORMAT,timeinfo);
    ss.str("");
    ss << DIRECTORY << TIME;
    //std::string dirpat = ss.str();
    result.push_back(TIME);
    directoryExistsOrCreate(ss.str().c_str());
    ss << "/xml";
    directoryExistsOrCreate(ss.str().c_str());
    zz.str("");
    zz << DIRECTORY << TIME << "/" + instance;
    //cout << "CREATE DIRECTORY:: " << zz.str() << endl;
    directoryExistsOrCreate(zz.str().c_str());
    std::string inspat = zz.str();
    result.push_back(inspat);
    //Create Crop
    //zz << "/cropped";
    //cout << "CREATE CROPPED DIRECTORY:: " << zz.str() << endl;
    //directoryExistsOrCreate(zz.str().c_str());
     
    return result;
}
  
// Check if there is motion in the result matrix
// count the number of changes and return.
inline vector<int> detectMotion(const Mat & motion, 
                        Mat & result,
                        int x_start, int x_stop, int y_start, int y_stop,
                        int max_deviation,
                        Scalar & color,
                        vector<int> changes)
{    
    // calculate the standard deviation
    Scalar mean, stddev;
    meanStdDev(motion, mean, stddev);
    // if not to much changes then the motion is real (neglect agressive snow, temporary sunlight)
    if(stddev[0] < max_deviation)
    {
        try
        {
            int number_of_changes = 0;
            int min_x = motion.cols, max_x = 0;
            int min_y = motion.rows, max_y = 0;
            // loop over image and detect changes
            for(int j = y_start; j < y_stop; j+=2){ // height
                for(int i = x_start; i < x_stop; i+=2){ // width
                    // check if at pixel (j,i) intensity is equal to 255
                    // this means that the pixel is different in the sequence
                    // of images (prev_frame, current_frame, next_frame)
                    if(static_cast<int>(motion.at<uchar>(j,i)) == 255)
                    {
                        number_of_changes++;
                        if(min_x>i) min_x = i;
                        if(max_x<i) max_x = i;
                        if(min_y>j) min_y = j;
                        if(max_y<j) max_y = j;
                    }
                }
            }
            if( number_of_changes >= there_is_motion)
            {
                changes.push_back(number_of_changes);
                //check if not out of bounds
                if(min_x-10 > 0) min_x -= 10;
                if(min_y-10 > 0) min_y -= 10;
                if(max_x+10 < result.cols-1) max_x += 10;
                if(max_y+10 < result.rows-1) max_y += 10;
                // draw rectangle round the changed pixel
                Point x(min_x,min_y);
                Point y(max_x,max_y);
                Rect rect(x,y);
                changes.push_back(min_x);
                changes.push_back(min_y);
                changes.push_back(max_x);
                changes.push_back(max_y);
                //Mat cropped = result(rect);
                //cropped.copyTo(result_cropped);
                //rectangle(result,rect,color,1);
            }
            else {
                changes.push_back(0);   
            }
        }
        catch (std::bad_alloc& ba)
        {
          std::cerr << "bad_alloc caught: " << ba.what() << '\n';
        } 
        return changes;
    }
    return changes;
}
  
// Check if there is motion in the result matrix
// count the number of changes and return.
inline vector<int> detectMotionRegion(const cv::Mat & motionmat,
                              cv::Mat & result,
                              std::vector<cv::Point2f> & region,
                              int max_deviation,
                              cv::Scalar & color,
                              vector<int> changes)
{
    // calculate the standard deviation
    Scalar mean, stddev;
    meanStdDev(motionmat, mean, stddev);
     
    // if not to much changes then the motion is real (neglect agressive snow, temporary sunlight)
    if(stddev[0] < max_deviation)
    {
        
        try
        {
            int number_of_changes = 0;
            int min_x = motionmat.cols, max_x = 0;
            int min_y = motionmat.rows, max_y = 0;
            // loop over image and detect changes
            int x, y, size = region.size();
            for(int i = 0; i < size; i++){ // loop over region
                x = region[i].x;
                y = region[i].y;
                if(static_cast<int>(motionmat.at<uchar>(y,x)) == 255)
                {
                    number_of_changes++;
                    if(min_x>x) min_x = x;
                    if(max_x<x) max_x = x;
                    if(min_y>y) min_y = y;
                    if(max_y<y) max_y = y;
                }
            }
            changes.push_back(number_of_changes);
            if(number_of_changes){
                //check if not out of bounds
                if(min_x-10 > 0) min_x -= 10;
                if(min_y-10 > 0) min_y -= 10;
                if(max_x+10 < result.cols-1) max_x += 10;
                if(max_y+10 < result.rows-1) max_y += 10;
                // draw rectangle round the changed pixel
                Point x(min_x,min_y);
                Point y(max_x,max_y);
                Rect rect(x,y);
                changes.push_back(min_x);
                changes.push_back(min_y);
                changes.push_back(max_x);
                changes.push_back(max_y);
                //Mat cropped = result(rect);
                //cropped.copyTo(result_cropped);
                
                //Draw_rectangle
                //rectangle(result,rect,color,1);
            }
            
        }    
        catch (std::bad_alloc& ba)
        {
          std::cerr << "bad_alloc caught: " << ba.what() << '\n';
        } 
        
        return changes;
    }
    return changes;
}
  
struct arg_struct
{
    motion::Message::Instance * instance;
};
  
double getFramesPerSecond(CvCapture *capture)
{
 
    // start and end times
  time_t start, end;
 
  // fps calculated using number of frames / seconds
  double fps;
 
  // frame counter
  int counter = 0;
 
  // floating point seconds elapsed since start
  double sec;
 
  // start the clock
  time(&start);
   
  while(cvGrabFrame(capture))
  {
      // grab a frame
      IplImage *frame = cvRetrieveFrame(capture);
 
      // see how much time has elapsed
      time(&end);
 
      // calculate current FPS
      ++counter;        
      sec = difftime (end, start);      
       
      fps = counter / sec;
 
      // will print out Inf until sec is greater than 0
      printf("FPS = %.2f\n", fps);
  }
  
  return fps;
    
}


void * startRecognition(void * arg)
{   
 
    pthread_detach(pthread_self());
    
    motion::Message::MotionCamera * pcamera;
    motion::Message::MotionMonth * pmonth;
    motion::Message::MotionDay * pday;
    motion::Message::Instance * pinstance;
    
    int activecamnum = R_PROTO.activecamnum();
    cout << "START RECOGNITION CAMERA" << activecamnum << endl;
    
    bool cameraexist = false;
    int sizec = R_PROTO.motioncamera_size();
    for (int i = 0; i < sizec; i++)
    {
        if (R_PROTO.mutable_motioncamera(i)->has_db_idcamera())
        { 
            if (activecamnum==R_PROTO.mutable_motioncamera(i)->cameranumber())
            {
                pcamera = R_PROTO.mutable_motioncamera(i);
                cameraexist=true;
            }
        }
    }
    
    int self = pthread_self();
    cout << "THREAD PID: " << self << endl;
    threads_recognizing_pids[activecamnum] = self;
    
    if (!cameraexist)
    {
        cout << "NO PROTO CAMERA!!" << endl;
        exit(0);
    }
    
    if (pcamera->has_fromdatabase())
        cout << "pcamera->has_fromdatabase(): "     << pcamera->has_fromdatabase() << endl;
    
    if (pcamera->has_cameraid())
        cout << "pcamera->cameraid(): "             << pcamera->cameraid()      << endl;
     
    if (pcamera->has_cameranumber())
        cout << "pcamera->cameranumber(): "         << pcamera->cameranumber()  << endl;
     
    if (pcamera->has_cameraname())
        cout << "pcamera->cameraname(): "           << pcamera->cameraname()    << endl;
     
    motion::Message::MotionRec * mrec = pcamera->mutable_motionrec(0);
    
    if (mrec->has_hasregion())
        cout << "pcamera->hasregion(): "            << mrec->has_hasregion()    << endl;
    
    bool fromcamera = false;
    if (pcamera->has_fromdatabase())
    {
        fromcamera = true;
    }
    
    //Region
    bool has_region;
    std::vector<cv::Point2f> region;
    std::string rcoords;
    if (mrec->hasregion())
    {
        rcoords = base64_decode(mrec->coordinates());        
        std::string rc = base64_decode(rcoords);
        cout << "rc: " << rc << endl;
         
        if (mrec->has_matrows())
               matrows = mrec->matrows();
        
        if (mrec->has_matcols())
            matcols = mrec->matcols();
        
        region = processRegionString(rc);
        
        if (region.size()>0)
            has_region = true;
        else
            has_region = false;
         
    }
    else
    {
        has_region = false;
    }
     
    int db_idcoordnates = mrec->db_idcoordinates();
     
    std::string instancecode;
    if (mrec->has_codename())
    {
        cout << "Has codename." << endl;
        instancecode = mrec->codename();
    }
    if (instancecode.empty())
        instancecode = "Prueba";
     
    google::protobuf::uint32 delay = 0;
    if (mrec->has_delay())
    {
        delay = mrec->delay();
    }
    
    time_t delaymark;
    
    //Month.
    string str_month;
    if (R_PROTO.has_currmonth())
    {
        str_month = R_PROTO.currmonth();
    }
    cout << "str_month: " << str_month << endl;
     
    //Check if exist month on proto or else add it.
    bool monthexist = false;
    for (int i = 0; i < pcamera->motionmonth_size(); i++)
    {
        std::string mlabel = pcamera->motionmonth(i).monthlabel();
        if (str_month.find(mlabel))
        {
            pmonth = pcamera->mutable_motionmonth(i);
            pmonth->Clear();
            monthexist=true;
        }
    }
    if(!monthexist)
    {
        pthread_mutex_lock(&protoMutex);
        pmonth = pcamera->add_motionmonth();
        pthread_mutex_unlock(&protoMutex);
    }
     
    pthread_mutex_lock(&protoMutex);
    pmonth->set_monthlabel(str_month);
    pmonth->set_db_monthid(mrec->db_idmonth());
    pthread_mutex_unlock(&protoMutex);
    
    int db_monthid = pmonth->db_monthid();
   
    //Day.
    string str_day;
    string str_day_title;
    if (R_PROTO.has_currday())
    {
        str_day = R_PROTO.currday();
        str_day_title = R_PROTO.currdaytitle();
    }
    cout << "str_day: " << str_day << endl;
    cout << "str_day_title: " << str_day_title << endl;
     
    //Check if day exist or else add it.
    bool dayexist=false;
    cout << "pmonth->motionday_size(): " << pmonth->motionday_size() << endl;
    for (int j = 0; j < pmonth->motionday_size(); j++)
    {
        std::string dlabel = pmonth->motionday(j).daylabel();
        cout << "dlabel: " << str_day << endl;
        if (str_day.find(dlabel))
        {
            pday = pmonth->mutable_motionday(j);
            dayexist = true;
        }
    }
    if (!dayexist)
    {
        cout << "!dayexist" << endl;
        //Add proto day.
        pthread_mutex_lock(&protoMutex);
        pday = pmonth->add_motionday();
        pday->set_daylabel(str_day);
        pday->set_title(str_day_title);
        pday->set_db_dayid(mrec->db_idday());
        pthread_mutex_unlock(&protoMutex);
        cout << "sigo" << endl;
    }
     
    int db_dayid = pday->db_dayid();
    
    cout << "db_dayid: " << db_dayid << endl;
    
    std::string XML_FILE;
    if (mrec->has_xmlfilepath())
    { 
        XML_FILE = mrec->xmlfilepath();  
    }
     
    bool writeImages = mrec->storeimage();
    bool writeVideo  = mrec->storevideo();
    bool send_number_detected = true;
     
    std::cout << "writeImages: " << writeImages << endl;
     
    google::protobuf::uint32 activecam = R_PROTO.activecam();
    
    //time
    struct timeval tr;
    struct tm* ptmr;
    char time_rasp[40];
    gettimeofday (&tr, NULL);
    ptmr = localtime (&tr.tv_sec);
    strftime (time_rasp, sizeof (time_rasp), "%Y-%m-%d %H:%M:%S %z", ptmr);
    
    int db_interval_id = mrec->db_intervalid();
    db_recognition_setup_id = mrec->db_recognitionsetupid();
   
    pthread_mutex_lock(&protoMutex);
    pday->set_db_dayid(db_dayid);
    pthread_mutex_unlock(&protoMutex);
     
    //Camera dir.
    std::stringstream camdir;
    camdir << sourcepath << "motion_web/pics/" << "camera" << activecamnum; 
    
    // Create camera directory
    directoryExistsOrCreate(camdir.str().c_str());
    
    std::string name = mrec->recname();
    std::transform(name.begin(), name.end(), name.begin(), ::toupper);
    
    // Rec dir.
    std::stringstream reddir;
    reddir << camdir.str() << "/" << name << "/"; 
    
    std::stringstream dumpfilename; 
    dumpfilename << activecamnum << name;
    
    //Create rec name directory
    directoryExistsOrCreate(reddir.str().c_str());
     
    const string DIR        = reddir.str();                                 // directory where the images will be stored
    //const string REGION     = sourcepath + "motion_web/pics/region/";     // directory where the regios are stored
    const string EXT        = ".jpg";                                       // extension of the images
    const string EXT_DATA   = ".xml";                                       // extension of the data
    const int DELAY         = 500;                                    // in mseconds, take a picture every 1/2 second
    const string LOG        = sourcepath + "/motion_web/log";               // log for the export
    const string LOGCLEAR = LOG + "/log_remove";
  
    // start and end times
    time_t start, end;
      
    cout << "DIR:: " << DIR << endl;
     
    // Create log directory if not exist.
    directoryExistsOrCreate(LOG.c_str());
    if(!std::ifstream(LOGCLEAR.c_str()))
    {
        ofstream logfile;
        logfile.open ( LOGCLEAR.c_str() );
        logfile << "Removed files log.\n";
        logfile.close();
    }
     
    // Format of directory
    //string DIR_FORMAT           = "%d%h%Y"; // 1Jan1970
    string FILE_FORMAT;//          = DIR_FORMAT + "/" + "%d%h%Y_%H%M%S"; // 1Jan1970/1Jan1970_12153
    string CROPPED_FILE_FORMAT;//   = DIR_FORMAT + "/cropped/" + "%d%h%Y_%H%M%S"; // 1Jan1970/cropped/1Jan1970_121539
    //string XML_FILE             =  "<import>session";
    std::string image_file_recognized;
     
    // Set up camera
    CvCapture * camera = cvCaptureFromCAM(activecamnum); //CV_CAP_ANY);
    //double fps = getFramesPerSecond(camera);
    
    int matwidth = 640; //1280
    int matheight= 480; //720
    
    cvSetCaptureProperty(camera, CV_CAP_PROP_FRAME_WIDTH, matwidth); 
    cvSetCaptureProperty(camera, CV_CAP_PROP_FRAME_HEIGHT, matheight);  
    double fps = 20; //120;
    cvSetCaptureProperty(camera, CV_CAP_PROP_FPS, fps);
    
    // Take images and convert them to gray
    Mat result; /*, result_cropped;*/
    Mat prev_frame = result = cvQueryFrame(camera);
    Mat current_frame = cvQueryFrame(camera);
    Mat next_frame = cvQueryFrame(camera);
     
    cvtColor(current_frame, current_frame, CV_RGB2GRAY);
    cvtColor(prev_frame, prev_frame, CV_RGB2GRAY);
    cvtColor(next_frame, next_frame, CV_RGB2GRAY);
            
    int width     = current_frame.cols;
    int height    = current_frame.rows;
     
    // d1 and d2 for calculating the differences
    // result, the result of and operation, calculated on d1 and d2
    // number_of_changes, the amount of changes in the result matrix.
    // color, the color for drawing the rectangle when something has changed.
    Mat d1, d2, motionmat;
    vector<int> number_of_changes; 
    int number_of_sequence = 0, count_sequence_cero = 0, count_save_image = 0;
    Scalar mean_, color(0,255,255); // yellow
     
    // Detect motion in window
    int x_start = 10, x_stop = current_frame.cols-11;
    int y_start = 350, y_stop = 530;
     
    // If more than 'there_is_motion' pixels are changed, we say there is motion
    // and store an image on disk
    there_is_motion = 3;
     
    // Maximum deviation of the image, the higher the value, the more motion is allowed
    int max_deviation = 20;
     
    // Erode kernel
    Mat kernel_ero = getStructuringElement(MORPH_RECT, Size(2,2));
     
    //count instance time
    string start_instance_time, total_elapsed_time;
    bool motion_detected = false, init_motion = false, has_instance_directory = false;
    double init_time, begin_time, end_time;
    //Directory Tree
    stringstream directoryTree;
    
    // Instance counter
    int instance_counter = 0;
    string instance;
    if (mrec->has_lastinstance())
    {
        std::string instcounter = mrec->lastinstance();
        instance_counter = atoi(instcounter.c_str());
        instance = instcounter;
    }
     
    init_time = clock();
    
    pthread_mutex_lock(&protoMutex);
    mrec->set_startrectime(time_rasp);
    pthread_mutex_unlock(&protoMutex);
    
    startrecognitiontime = time_rasp;
     
    cout << "CAM " << activecamnum << " TIME STARTED: " << time_rasp << endl;
    
    writeVideo = true;
      
    // All settings have been set, now go in endless loop and
    // take as many pictures you want..
    while (true)
    {
        try
        {
            
            if (!pcamera->recognizing())
            {
                
                while (running_image_threads > 0)
                {
                   cout << "running_image_threads: " << running_image_threads << endl; 
                   sleep(1);
                }
                running_image_threads = 0;
                
                if (camera)
                    cvReleaseCapture(&camera);
                
                std::cout << ":::::::::::: CAM" << activecamnum << "DUMP INSTANCE FINISH "<< instance << "  :::::::::::" << std::endl;
                    
                has_instance_directory = false;
                init_motion = false;

                end_time = clock();

                dumpInstance(activecamnum, pinstance, DIR, XML_FILE, EXT_DATA, init_time, begin_time, end_time, instance, instancecode, dumpfilename.str(), mrec->name(), pcamera->cameraname());
                
                pinstance->Clear();
                
                threads_recognizing_pids[activecamnum] = 0;
                
                pthread_exit(NULL);
           
            }

            // Take a new image
            prev_frame = current_frame;
            current_frame = next_frame;
            next_frame = cvQueryFrame(camera);
            result = next_frame;
            cvtColor(next_frame, next_frame, CV_RGB2GRAY);

            // Calc differences between the images and do AND-operation
            // threshold image, low differences are ignored (ex. contrast change due to sunlight)
            absdiff(prev_frame, next_frame, d1);
            absdiff(next_frame, current_frame, d2);
            bitwise_and(d1, d2, motionmat);
            threshold(motionmat, motionmat, 35, 255, CV_THRESH_BINARY);
            erode(motionmat, motionmat, kernel_ero);

            number_of_changes.clear();

            if (!has_region)
            {
               number_of_changes = detectMotion(motionmat, result, x_start, x_stop, y_start, y_stop, max_deviation, color, number_of_changes);
            }
            else
            {
                number_of_changes = detectMotionRegion(motionmat, result, region, max_deviation, color, number_of_changes);
            }
            if (number_of_changes.size()>0)
                resutl_watch_detected = number_of_changes.at(0);
          
        }
        catch (std::bad_alloc& ba)
        {
            std::cerr << "bad_alloc caught: " << ba.what() << '\n';
        } 
        
        // If a lot of changes happened, we assume something changed.
        if( resutl_watch_detected >= there_is_motion)
        {
            
            try
            {
                cout << "cam" << activecamnum << " resutl detected: " << resutl_watch_detected << endl;
                //cout << "number_of_sequence:: " << number_of_sequence << endl;

                if(number_of_sequence>0 & number_of_changes.size()>1)
                {

                    //cout << "!motion_detected:: " << motion_detected << endl;

                    init_motion = true;

                    if (!motion_detected)
                    {

                        count_sequence_cero = 0;
                        motion_detected     = true;
                        begin_time = clock();

                        //cout << "!has_instance_directory:: " << has_instance_directory << endl;

                        if (!has_instance_directory)
                        {

                            //New Instance
                            instance_counter++;
                            stringstream id;
                            id << instance_counter;
                            instance = id.str();

                            //Add proto instance.
                            cout << ":::::::::::  CAM" << activecamnum << " ADD  INSTANCE " << instance << "  ::::::::::::"  << endl;
                            pthread_mutex_lock(&protoMutex);
                            pinstance = pday->add_instance();
                            pinstance->set_idinstance(std::atoi(instance.c_str()));
                            pinstance->set_db_dayid(db_dayid);
                            pinstance->set_db_recognition_setup_id(db_recognition_setup_id);
                            pthread_mutex_unlock(&protoMutex);

                            FILE_FORMAT = DIR_FORMAT + "/" + instance + "/" + "%H%M%S";
                            //CROPPED_FILE_FORMAT = DIR_FORMAT + "/" + instance + "/cropped/" + "%d%h%Y_%H%M%S";

                            vector<string> dirs = createDirectoryTree (DIR,EXT,DIR_FORMAT.c_str(),instance );

                            if (writeVideo)
                            {    
                                std::string ext = ".mpg"; 
                                std::string  videoname = dirs.at(0) + "_" + instance + ext;
                                std::string videopath = dirs.at(1) + "/" + videoname;

                                pthread_mutex_lock(&protoMutex);
                                motion::Message::Video * pvideo = pinstance->mutable_video();
                                pvideo->set_path(videopath.c_str());
                                pvideo->set_name(videoname.c_str());
                                pvideo->set_instancefolder(dirs.at(1));
                                pthread_mutex_unlock(&protoMutex);
                            }
                            
                            has_instance_directory = true;
                        }
                    }

                    if (writeImages)
                    {
                        motion::Message::Image * pimage = pinstance->add_image();
                        motion::Message::Crop * pcrop = pinstance->add_crop();
                        image_file_recognized = saveImg (result, DIR, EXT, DIR_FORMAT.c_str(), FILE_FORMAT.c_str(), number_of_changes, pimage, pcrop);
                    }
                    
                }
                delaymark = time(0);
                number_of_sequence++;
            }
            catch (std::bad_alloc& ba)
            {
              std::cerr << "bad_alloc caught: " << ba.what() << '\n';
            } 
        }
        else
        {
             
            try
            {
                number_of_sequence = 0;

                double seconds_since_start = difftime( time(0), delaymark);

                //cout << "seconds_since_start: " << seconds_since_start << " delay: " << delay << " init_motion: " << init_motion << endl;

                if ( seconds_since_start > delay & init_motion )
                {

                    std::cout << ":::::::::::: CAM" << activecamnum << " DUMP INSTANCE "<< instance << "  :::::::::::" << std::endl;
                    
                    has_instance_directory = false;
                    init_motion = false;
                    
                    end_time = clock();
                    
                    dumpInstance(activecamnum, 
                            pinstance, 
                            DIR, XML_FILE, 
                            EXT_DATA, 
                            init_time, 
                            begin_time, 
                            end_time, 
                            instance, 
                            instancecode, 
                            dumpfilename.str(), 
                            pday->title(), 
                            pcamera->cameraname());
                    
                    pinstance->Clear();
                 
                }

                // XML Instance
                motion_detected = false;
                count_sequence_cero++;

                if (count_sequence_cero==1)
                {
                    end_time = clock();
                }
             
            }
            catch (std::bad_alloc& ba)
            {
              std::cerr << "bad_alloc caught: " << ba.what() << '\n';
            }
        }
        
        //cv::imshow(pcamera->cameraname().c_str(), current_frame);
        
        // Delay, wait a 1/2 second.
        cvWaitKey (DELAY);
    }
     
    return 0;
}

void dumpInstance(int activecamnum,
        motion::Message::Instance * pinstance, 
        std::string DIR, 
        std::string XML_FILE, 
        std::string EXT_DATA, 
        time_t init_time, 
        time_t begin_time, 
        time_t end_time, 
        std::string instance, 
        std::string instancecode,
        std::string dumpfilename,
        std::string title,
        std::string camera)
{
    
    pinstance->set_dir(DIR);
    pinstance->set_xmlfile(XML_FILE);
    pinstance->set_extdata(EXT_DATA);
    
    long int init = static_cast<long int>(init_time);
    pinstance->set_inittime(init);
    
    long int begin = static_cast<long int>(begin_time);
    pinstance->set_begintime(begin);
    
    long int end = static_cast<long int>(end_time);
    pinstance->set_endtime(end);
    
    pinstance->set_instance(instance);
    pinstance->set_instancecode(instancecode);
    
    pinstance->set_recname(title);

    pinstance->set_camera(camera);
    pinstance->set_cameranumber(activecamnum);
    
    struct timeval tr;
    struct tm* ptmr;
    char time_info[40];
    gettimeofday (&tr, NULL);
    ptmr = localtime (&tr.tv_sec);
    strftime (time_info, sizeof (time_info), "%Y-%m-%d %H:%M:%S %z", ptmr);

    std::ostringstream strtimeinfo;
    strtimeinfo << time_info;
    pinstance->set_timeinfo(strtimeinfo.str());

    //Initialize objects to serialize.
    int size = pinstance->ByteSize();
    char datasend[size];    
    pinstance->SerializePartialToArray(&datasend, size);

    std::string encoded_proto = base64_encode(reinterpret_cast<const unsigned char*>(datasend),sizeof(datasend));

    std::stringstream dumpfile;
    dumpfile << basepath << "data/instances/camera" << activecamnum << "/" << dumpfilename << "-" << instance << ".dat";
    std::ofstream out;
    out.open (dumpfile.str().c_str());
    out << encoded_proto << "\n";
    out.close();
}



bool isRecognizing()
{
    bool recognizing = false;
    int sizec = R_PROTO.motioncamera_size();
    for (int t = 0; t < sizec; t++)
    {
        pthread_mutex_lock(&protoMutex);
        motion::Message::MotionCamera * mcamera = R_PROTO.mutable_motioncamera(t);
        bool recis = mcamera->recognizing();
        pthread_mutex_unlock(&protoMutex);    
        if (recis)
        {
            recognizing = true;
        }
        
    }
    return recognizing;
}

bool isRecognizingAtCamera(int camera)
{
    bool recognizing = false;
    pthread_mutex_lock(&protoMutex);
    motion::Message::MotionCamera * mcamera = R_PROTO.mutable_motioncamera(camera);
    bool recis = mcamera->recognizing();
    pthread_mutex_unlock(&protoMutex);    
    if (recis)
    {
        recognizing = true;
    }
    return recognizing;
}

 