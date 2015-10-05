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

#include "../tinyxml/tinyxml.h"
#include "../tinyxml/tinystr.h"
  
#include "../database/database.h"
  
#include <string>
#include <vector>
#include <functional>
#include <ctime>
#include <unistd.h>
#include <cstring>
#include <algorithm>
#include <new>

#include <signal.h>
  
#include <sys/time.h>
  
using namespace std;
using namespace cv;

//database
int insertIntoIMage(const motion::Message::Image & img);
void insertIntoCrop(const motion::Message::Crop & crop, int db_image_id);
int insertIntoVideo(motion::Message::Video dvideo);
void insertIntoInstance(std::string number, motion::Message::Instance * pinstance, char * time_info, int db_video_id, vector<int> images);
  
motion::Message::MotionCamera * pcamera;
motion::Message::MotionMonth * pmonth;
motion::Message::MotionDay * pday;
motion::Message::Instance * pinstance;
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
struct proto_args
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
struct proto_args ProtoArgs;

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
  
// Create initial XML file
void build_xml(const char * xmlPath)
{
    time_t     now = time(0);
    struct tm  tstruct;
    char       secs[80];
    tstruct = *localtime(&now);
    //strftime(secs, sizeof(secs), "%Y:%m:%d %X", &tstruct);
    strftime(secs, sizeof(secs), "%Y-%m-%d %H:%M:%S %z", &tstruct);
     
    char result[100];   // array to hold the result.
     
    strcpy(result, secs); // copy string one into the result.
     
    TiXmlDocument doc;
    TiXmlDeclaration * decl = new TiXmlDeclaration( "1.0", "utf-8", "");
    TiXmlElement * file = new TiXmlElement( "file" );
    TiXmlElement * session_info = new TiXmlElement( "SESSION_INFO" );
    TiXmlElement * start_time = new TiXmlElement( "start_time" );
    TiXmlText * text_start_time = new TiXmlText( result );
    TiXmlElement * all_instances = new TiXmlElement( "ALL_INSTANCES" );
     
    start_time->LinkEndChild( text_start_time );
    session_info->LinkEndChild(start_time);
    file->LinkEndChild(session_info);
    file->LinkEndChild(all_instances);
    doc.LinkEndChild( decl );
    doc.LinkEndChild( file );
    doc.SaveFile( xmlPath );
}
  
  
// Write XML file
// Check if the xml file exists, if not create it
// Write  the log for the motion detected.
inline void writeXMLInstance (
                              string XMLFILE,
                              string time_start,
                              string time_end,
                              string instance,
                              string instancecode
                              )
{
     
    TiXmlDocument doc( XMLFILE.c_str() );
    if ( doc.LoadFile() )
    {
         
        TiXmlElement* file = doc.FirstChildElement();
         
        TiXmlElement* session_info = file->FirstChildElement();
         
        TiXmlElement* all_instances = session_info->NextSiblingElement();
         
        TiXmlElement * ID = new TiXmlElement( "ID" );
        TiXmlText * text_ID = new TiXmlText( instance.c_str() );
        ID->LinkEndChild(text_ID);
         
        TiXmlElement * start = new TiXmlElement( "start" );
        TiXmlText * text_start = new TiXmlText( time_start.c_str()  );
        start->LinkEndChild(text_start);
         
        TiXmlElement * end = new TiXmlElement( "end" );
        TiXmlText * text_end = new TiXmlText( time_end.c_str() );
        end->LinkEndChild(text_end);
         
        TiXmlElement * code = new TiXmlElement( "code" );
        TiXmlText * text_code = new TiXmlText( instancecode.c_str() );
        code->LinkEndChild(text_code);
         
        TiXmlElement * instance = new TiXmlElement( "instance" );
        instance->LinkEndChild(ID);
        instance->LinkEndChild(start);
        instance->LinkEndChild(end);
        instance->LinkEndChild(code);
         
        all_instances->LinkEndChild(instance);
         
        doc.SaveFile( XMLFILE.c_str() );
         
    }
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
                      vector<int> n_o_changes)
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
        //if(incr < 100) incr++; // quick fix for when delay < 1s && > 10ms, (when delay <= 10ms, images are overwritten)
        //else incr = 0;
        ss << DIRECTORY << TIME << static_cast<int>(incr) << n_file << EXTENSION;
        image_file = ss.str().c_str();

        ImageArgs.mat = image;
        ImageArgs.path = image_file;
        
        pthread_mutex_lock(&running_image_mutex);
        running_image_threads++;
        pthread_mutex_unlock(&running_image_mutex);
        
        pthread_create(&thread_image, NULL, storeimage, &ImageArgs);

        //imwrite(image_file, image);

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
        motion::Message::Image * pimage = pinstance->add_image();
        pimage->set_path(image_file.c_str());
        pimage->set_name(n_str_file);
        pimage->set_imagechanges(n_o_changes.at(0));
        pimage->set_time(time_info);
        motion::Message::Crop * pcrop = pinstance->add_crop();
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
                rectangle(result,rect,color,1);
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
                rectangle(result,rect,color,1);
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
     
    cout << "START RECOGNITION." << endl;
      
    pthread_detach(pthread_self());
      
    R_PROTO.Clear();
    R_PROTO = PROTO;
     
    //Camera.
    int cam = R_PROTO.activecam();
     
    //Get Month abr.
    struct timeval tm;
    struct tm* ptm;
    char month_rasp[3];
    gettimeofday (&tm, NULL);
    ptm = localtime (&tm.tv_sec);
    strftime (month_rasp, sizeof (month_rasp), "%h", ptm);
     
    vector<vector<string> > camera_array;
    //database camera id.
    stringstream camera_id_query;
    camera_id_query <<
    "SELECT _id, name, number FROM cameras where number = " << cam << ";";
    cout << " camera_id_query " << camera_id_query.str() << endl;
    pthread_mutex_lock(&databaseMutex);
    camera_array = db_select(camera_id_query.str().c_str(), 3);
    pthread_mutex_unlock(&databaseMutex);
    
    int db_camera_id        = atoi(camera_array.at(0).at(0).c_str());
    string str_camera   = camera_array.at(0).at(1);
    int number          = atoi(camera_array.at(0).at(2).c_str());
    cout << "str_camera: "      << str_camera << endl;
    cout << "db_camera_id: "    << db_camera_id << endl;
    cout << "number: "          << number << endl;
  
    //Check if exist month on proto or else add it.
    int sizec = R_PROTO.motioncamera_size();
    cout << "sizec: " << sizec << endl;
    bool cameraexist = false;
    for (int i = 0; i < sizec; i++)
    {
        //cout << "entra" << endl;
        motion::Message::MotionCamera * mcamera = R_PROTO.mutable_motioncamera(i);
        if (mcamera->has_db_idcamera())
        {
            //std::string camera = mcamera->cameraname();
            cout << "camera: " << camera << endl;
            cout << "mcamera->cameranumber(): " << mcamera->cameranumber() << endl;
             
            //std::replace( camera.begin(),       camera.end(),       '\n', ' ');
            //std::replace( str_camera.begin(),   str_camera.end(),   '\n', ' ');
             
            if (number==mcamera->cameranumber())
            {
                cout << "has camera" << endl;
                pcamera = R_PROTO.mutable_motioncamera(i);
                cameraexist=true;
            }
        }
    }
    if(!cameraexist)
    {
        cout << "!cameraexist" << endl;
        //Add proto camera.
        pthread_mutex_lock(&protoMutex);
        pcamera = R_PROTO.add_motioncamera();
        pcamera->set_cameraname(str_camera);
        pthread_mutex_unlock(&protoMutex);
        cout << "sigo" << endl;
    }
    
    if (pcamera->has_fromdatabase())
        cout << "pcamera->has_fromdatabase(): "     << pcamera->has_fromdatabase() << endl;
    
    if (pcamera->has_cameraid())
        cout << "pcamera->cameraid(): "     << pcamera->cameraid()      << endl;
     
    if (pcamera->has_cameranumber())
        cout << "pcamera->cameranumber(): " << pcamera->cameranumber()  << endl;
     
    if (pcamera->has_cameraname())
        cout << "pcamera->cameraname(): "   << pcamera->cameraname()    << endl;
     
    if (pcamera->has_hasregion())
        cout << "pcamera->hasregion(): "   << pcamera->has_hasregion()    << endl;
    
    bool fromcamera = false;
    if (pcamera->has_fromdatabase())
    {
        fromcamera = true;
    }
    
    //Region
    bool has_region;
    std::vector<cv::Point2f> region;
    std::string rcoords;
    if (pcamera->hasregion())
    {
        std::string rc = pcamera->coordinates(); 
        rcoords = base64_decode(rc);
        cout << "rcoords." << rcoords << endl;
         
        if (pcamera->has_matrows())
               matrows = pcamera->matrows();
        
        if (pcamera->has_matcols())
            matcols = pcamera->matcols();
        
        region = processRegionString(rcoords);
        
        if (region.size()>0)
            has_region = true;
        else
            has_region = false;
         
    }
    else
    {
        has_region = false;
    }
     
    int db_idcoordnates = pcamera->db_idcoordinates();
     
    std::string instancecode;
    if (pcamera->has_codename())
    {
        cout << "Has codename." << endl;
        instancecode = pcamera->codename();
    }
    if (instancecode.empty())
        instancecode = "Prueba";
     
    google::protobuf::uint32 delay = 0;
    if (pcamera->has_delay())
    {
        delay = pcamera->delay();
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
    //int sizem = R_PROTO.motionmonth_size();
    //cout << "sizem: " << sizem << endl;
    bool monthexist = false;
    cout << "pcamera->motionmonth_size(): " << pcamera->motionmonth_size() << endl;
    for (int i = 0; i < pcamera->motionmonth_size(); i++)
    {
        std::string mlabel = pcamera->motionmonth(i).monthlabel();
        cout << "mlabel: " << str_month << endl;
        if (str_month.find(mlabel))
        {
            cout << "has month" << endl;
            pmonth = pcamera->mutable_motionmonth(i);
            monthexist=true;
        }
    }
    if(!monthexist)
    {
        cout << "!monthexist" << endl;
        //Add proto month.
        pthread_mutex_lock(&protoMutex);
        pmonth = pcamera->add_motionmonth();
        pmonth->set_monthlabel(str_month);
        pmonth->set_db_monthid(pcamera->db_idmonth());
        pthread_mutex_unlock(&protoMutex);
        cout << "sigo" << endl;
    }
     
    int db_monthid = pmonth->db_monthid();
   
    //Day.
    string str_day;
    if (R_PROTO.has_currday())
    {
        str_day = R_PROTO.currday();
    }
    cout << "str_day: " << str_day << endl;
     
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
        pday->set_db_dayid(pcamera->db_idday());
        pthread_mutex_unlock(&protoMutex);
        cout << "sigo" << endl;
    }
     
    int db_dayid = pday->db_dayid();
    
    std::string XML_FILE;
    if (pcamera->has_xmlfilepath())
    { 
        XML_FILE = pcamera->xmlfilepath();  
    }
     
    bool writeImages = pcamera->storeimage();
    bool writeVideo  = pcamera->storevideo();
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
    
    int db_interval_id = pcamera->db_intervalid();
    db_recognition_setup_id = pcamera->db_recognitionsetupid();
   
    pthread_mutex_lock(&protoMutex);
    pday->set_db_dayid(db_dayid);
    pthread_mutex_unlock(&protoMutex);
     
    //Camera dir.
    std::stringstream camdir;
    camdir << sourcepath << "motion_web/pics/" << "camera" << cam; 
    
    // Create camera directory
    directoryExistsOrCreate(camdir.str().c_str());
    
    std::string name = pcamera->recname();
    std::transform(name.begin(), name.end(), name.begin(), ::toupper);
    
    // Rec dir.
    std::stringstream reddir;
    reddir << camdir.str() << "/" << name << "/"; 
    
    //Create rec name directory
    directoryExistsOrCreate(reddir.str().c_str());
     
    const string DIR        = reddir.str();                                 // directory where the images will be stored
    //const string REGION     = sourcepath + "motion_web/pics/region/";     // directory where the regios are stored
    const string EXT        = ".jpg";                                       // extension of the images
    const string EXT_DATA   = ".xml";                                       // extension of the data
    const int DELAY         = 50; //500;                                    // in mseconds, take a picture every 1/2 second
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
    camera = cvCaptureFromCAM(cam); //CV_CAP_ANY);
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
    if (pcamera->has_lastinstance())
    {
        std::string instcounter = pcamera->lastinstance();
        instance_counter = atoi(instcounter.c_str());
        instance = instcounter;
    }
     
    init_time = clock();
    
    pthread_mutex_lock(&protoMutex);
    pcamera->set_startrectime(time_rasp);
    pthread_mutex_unlock(&protoMutex);
    
    startrecognitiontime = time_rasp;
     
    cout << "TIME STARTED: " << time_rasp << endl;
    
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
                
                std::cout << ":::::::::::: DUMP INSTANCE FINISH "<< instance << "  :::::::::::" << std::endl;
                    
                has_instance_directory = false;
                init_motion = false;

                end = clock();

                ProtoArgs.pinstance     = pinstance; //pday->mutable_instance(0);
                ProtoArgs.DIR           = DIR;
                ProtoArgs.XML_FILE      = XML_FILE;
                ProtoArgs.EXT_DATA      = EXT_DATA;
                ProtoArgs.init_time     = init_time;
                ProtoArgs.begin_time    = begin_time;
                ProtoArgs.end_time      = end_time;
                ProtoArgs.end           = end; 
                ProtoArgs.instance      = instance;
                ProtoArgs.instancecode  = instancecode;
                
                cout << "pthread_create" << endl;
                int runb = pthread_create(&thread_store, NULL, storeproto, &ProtoArgs);
                pthread_join(thread_store,  (void**) &runb);
               
                pthread_cancel(thread_recognition); 
                 
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
        
        //cout << "resutl_watch_detected: " << resutl_watch_detected << endl;
         
        // If a lot of changes happened, we assume something changed.
        if( resutl_watch_detected >= there_is_motion)
        {
            
            try
            {
                cout << "resutl_watch_detected: " << resutl_watch_detected << endl;
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
                            cout << ":::::::::::  ADD  INSTANCE " << instance << "  ::::::::::::"  << endl;
                            pthread_mutex_lock(&protoMutex);
                            pinstance = pday->add_instance();
                            pinstance->set_idinstance(std::atoi(instance.c_str()));
                            pthread_mutex_unlock(&protoMutex);

                            FILE_FORMAT = DIR_FORMAT + "/" + instance + "/" + "%H%M%S";
                            //CROPPED_FILE_FORMAT = DIR_FORMAT + "/" + instance + "/cropped/" + "%d%h%Y_%H%M%S";

                            vector<string> dirs = createDirectoryTree (DIR,EXT,DIR_FORMAT.c_str(),instance );

                            if (writeVideo)
                            {    

                                //std::string ext = ".avi";
                                //std::string ext = ".mpeg";
                                //std::string ext = ".mp4";
                                //std::string ext = ".divx";
                                //std::string ext = ".flv";

                                std::string ext = ".mpg"; 

                                //std::string ext = ".mjpeg";

                                std::string  videoname = dirs.at(0) + "_" + instance + ext;
                                std::string videopath = dirs.at(1) + "/" + videoname;

                                //int codec = CV_FOURCC('I','4','2','0');
                                //int codec = CV_FOURCC('A','V','C','1');
                                //int codec = CV_FOURCC('Y','U','V','1');
                                //int codec = CV_FOURCC('P','I','M','1'); 
                                //int codec = CV_FOURCC('M','J','P','G');
                                //int codec = CV_FOURCC('M','P','4','2');
                                //int codec = CV_FOURCC('D','I','V','3');
                                //int codec = CV_FOURCC('D','I','V','X');
                                //int codec = CV_FOURCC('U','2','6','3');
                                //int codec = CV_FOURCC('I','2','6','3');
                                //int codec = CV_FOURCC('F','L','V','1');
                                //int codec = CV_FOURCC('H','2','6','4');
                                //int codec = CV_FOURCC('A','Y','U','V');
                                //int codec = CV_FOURCC('I','U','Y','V');
                                //int codec = CV_FOURCC('X','V','I','D');
                                //int codec = CV_FOURCC('P','I','M','1');
                                //int codec = CV_FOURCC('W','M','V','2');
                                //int codec = -1;


                                //cv::Size size = cv::Size(640,480);

                                /*for (int t=800000000; t<5448695113; t++)
                                {
                                    if( (cvvideout = cvCreateVideoWriter(videopath.c_str(), t, fps, size, true)) == NULL )
                                    {
                                        cout << "An error occured whilst making the video file. " << t <<  endl;
                                    }
                                     else
                                    {
                                        cout << "Video creation successful!" << endl;
                                    }
                                }*/


                                //videout = new cv::VideoWriter(videopath.c_str(), codec, fps, size, true);
                                //if (videout->open(videopath, codec, fps, size, true))
                                //{
                                //    cout << "opened" << endl;
                                //}

                                /*if( (cvvideout = cvCreateVideoWriter(videopath.c_str(), codec, fps, size, true)) == NULL )
                                {
                                    cout << "An error occured whilst making the video file." << endl;
                                }
                                else
                                {
                                    cout << "Video creation successful!" << endl;
                                }*/

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
                        image_file_recognized = saveImg (result, DIR, EXT, DIR_FORMAT.c_str(), FILE_FORMAT.c_str(), number_of_changes);
                    }

                    //if (writeVideo)
                    //{
                    //    videout->write(result);
                    //}

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

                    std::cout << ":::::::::::: DUMP INSTANCE "<< instance << "  :::::::::::" << std::endl;
                    
                    has_instance_directory = false;
                    init_motion = false;
                    
                    end = clock();
                    
                    ProtoArgs.pinstance     = pinstance; //pday->mutable_instance(0);
                    ProtoArgs.DIR           = DIR;
                    ProtoArgs.XML_FILE      = XML_FILE;
                    ProtoArgs.EXT_DATA      = EXT_DATA;
                    ProtoArgs.init_time     = init_time;
                    ProtoArgs.begin_time    = begin_time;
                    ProtoArgs.end_time      = end_time;
                    ProtoArgs.end           = end; 
                    ProtoArgs.instance      = instance;

                    cout << "pthread_create" << endl;
                    int runb = pthread_create(&thread_store, NULL, storeproto, &ProtoArgs);
                    
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
        
         // Delay, wait a 1/2 second.
        //cvWaitKey (DELAY);
    }
     
    return 0;
}


void * storeproto(void * args)
{
    
    struct proto_args *arg = (struct proto_args *) args;
    
    //create xml file if not exists
    time_t seconds;
    struct tm * timeinfo;
    char TIME[80];
    time (&seconds);
    timeinfo = localtime (&seconds);

    string XMLFILE = arg->XML_FILE; 
    if(!std::ifstream(XMLFILE.c_str()))
    {
        build_xml(XMLFILE.c_str());
    }

    std::ostringstream begin;
    begin << (arg->begin_time - arg->init_time) / CLOCKS_PER_SEC;
    std::ostringstream end;
    end << (arg->end_time - arg->init_time) / CLOCKS_PER_SEC;
    writeXMLInstance(XMLFILE.c_str(), begin.str(), end.str(), arg->instance, arg->instancecode);
    
    motion::Message::Instance * pinstance = arg->pinstance;
    
    pthread_mutex_lock(&protoMutex);
    pinstance->set_instancestart(begin.str());
    pinstance->set_instanceend(end.str());
    pthread_mutex_unlock(&protoMutex);

    struct timeval tr;
    struct tm* ptmr;
    char time_info[40];
    gettimeofday (&tr, NULL);
    ptmr = localtime (&tr.tv_sec);
    strftime (time_info, sizeof (time_info), "%Y-%m-%d %H:%M:%S %z", ptmr);

    vector<int> images;
    images.clear();

    int imagesize = pinstance->image_size();

    for (int j = 0; j < imagesize; j++)
    {
        //Image
        int db_image_id;
        const motion::Message::Image & img = pinstance->image(j);
        db_image_id = insertIntoIMage(img);
        const motion::Message::Crop & crop = pinstance->crop(j);                
        insertIntoCrop(crop, db_image_id);
        images.push_back(db_image_id);
    } 

    motion::Message::Video dvideo = pinstance->video();
    int db_video_id = insertIntoVideo(dvideo);
    insertIntoInstance(arg->instance, pinstance, time_info, db_video_id, images);
    
    pinstance->Clear();
    arg->pinstance->Clear();
    pinstance->Clear();
    
    cout << "INSTANCE CLEANED: " << endl; 
    pthread_cancel(thread_store);
                
}


void insertIntoInstance(std::string number, motion::Message::Instance * pinstance, char * time_info, int db_video_id, vector<int> images)
{
                
        std::string instancestart   = pinstance->instancestart();
        std::string instanceend     = pinstance->instanceend();

        //Instance
        stringstream sql_instance;
        sql_instance <<        
        "INSERT INTO instance (number, instancestart, instanceend, _id_video, time) " <<
        "SELECT "   << number                   << 
        ",' "       << instancestart            << "'" << 
        ", '"       << instanceend              << "'" <<
        ", "        << db_video_id              <<
        ", '"       << time_info                << "'" << 
        " WHERE NOT EXISTS (SELECT * FROM instance WHERE number = " << number       <<
        " AND instancestart = '"  << pinstance->instancestart()     <<    "'"       <<
        " AND instanceend   = '"  << pinstance->instanceend()       <<    "'"       <<
        " AND _id_video     = "   << db_video_id                    << 
        " AND time          = '"  << time_info                      <<    "'"       << ");";
        std::string sqlinst = sql_instance.str();
        //cout << "sqlinst: " << sqlinst << endl;
        pthread_mutex_lock(&databaseMutex);
        db_execute(sqlinst.c_str());
        std::string last_instance_query = "SELECT MAX(_id) FROM instance";
        vector<vector<string> > instance_array = db_select(last_instance_query.c_str(), 1);
        pthread_mutex_unlock(&databaseMutex);
        int db_instance_id = atoi(instance_array.at(0).at(0).c_str());
        //cout << "db_instance_id: " << db_instance_id << endl;
        
        for (int t=0; t<images.size(); t++)
        {
            //Rel Instance Image.
            int db_image_id = images.at(t);
            stringstream sql_rel_instance_image;
            sql_rel_instance_image <<
            "INSERT INTO rel_instance_image (_id_instance, _id_image) " <<
            "SELECT "  << db_instance_id    <<
            ", "       << db_image_id       <<
            " WHERE NOT EXISTS (SELECT * FROM rel_instance_image WHERE _id_instance = " << db_instance_id << 
            " AND _id_image = " << db_image_id << ");";
            pthread_mutex_lock(&databaseMutex);
            db_execute(sql_rel_instance_image.str().c_str());
            pthread_mutex_unlock(&databaseMutex);
        }

        int dayid = pday->db_dayid();

        //Instance.
        stringstream sql_rel_day_instance;
        sql_rel_day_instance <<
        "INSERT INTO rel_day_instance_recognition_setup (_id_day, _id_instance, _id_recognition_setup, time) " <<
        "SELECT "  << dayid                     <<
        ", "       << db_instance_id            <<
        ", "       << db_recognition_setup_id   <<
        ", '"       << time_info         << "'" <<
        " WHERE NOT EXISTS (SELECT * FROM rel_day_instance_recognition_setup WHERE _id_day = " << dayid << 
        " AND _id_instance = " << db_instance_id << " AND _id_recognition_setup = " << db_recognition_setup_id << ");";
        pthread_mutex_lock(&databaseMutex);
        db_execute(sql_rel_day_instance.str().c_str());
        pthread_mutex_unlock(&databaseMutex);
         
}


int insertIntoVideo(motion::Message::Video dvideo)
{      
    
    std::string name = dvideo.name();
    std::string path = dvideo.path();
    
    stringstream sql_video;
    sql_video <<
    "INSERT INTO video (path, name) " <<
    "SELECT '"  << path << "'" <<
    ", '"       << name << "'" <<
    " WHERE NOT EXISTS (SELECT * FROM video WHERE path = '" << path << "'" <<
    " AND name = '" << name << "');";

    pthread_mutex_lock(&databaseMutex);
    db_execute(sql_video.str().c_str());
    std::string last_video_query = "SELECT MAX(_id) FROM video";
    vector<vector<string> > video_array = db_select(last_video_query.c_str(), 1);
    pthread_mutex_unlock(&databaseMutex);
    int db_video_id = atoi(video_array.at(0).at(0).c_str());
    //cout << "db_video_id: " << db_video_id << endl;
    
    return db_video_id;
}

void insertIntoCrop(const motion::Message::Crop & crop, int db_image_id)
{
     //Crop
    int db_crop_id;
    stringstream sql_crop;
    sql_crop <<
    "INSERT INTO crop (rect, _id_image_father) " <<
    "SELECT '"  << crop.rect()    <<
    "', "       << db_image_id      <<
    " WHERE NOT EXISTS (SELECT * FROM crop WHERE rect = '"    << crop.rect() << "'" <<
    " AND _id_image_father = "      << db_image_id              << ");";
    pthread_mutex_lock(&databaseMutex);
    db_execute(sql_crop.str().c_str());
    std::string last_crop_query = "SELECT MAX(_id) FROM crop";
    vector<vector<string> > crop_array = db_select(last_crop_query.c_str(), 1);
    pthread_mutex_unlock(&databaseMutex);
    db_crop_id = atoi(crop_array.at(0).at(0).c_str());
    //cout << "db_crop_id: " << db_crop_id << endl;
    
}
    

int insertIntoIMage(const motion::Message::Image & img)
{
    
    stringstream sql_img;
    sql_img <<
    "INSERT INTO image (path, name, imagechanges, time) " <<
    "SELECT '"  << img.path()           <<
    "', '"      << img.name()           <<
    "', "       << img.imagechanges()   <<
    ",'"       <<  img.time()           << "'"
    " WHERE NOT EXISTS (SELECT * FROM image WHERE path = '" << img.path() << "'" <<
    " AND name = '" << img.name() << "'" <<
    " AND imagechanges = " << img.imagechanges() << ");";
    std::string sql_imgstd = sql_img.str();
    //cout << "sql_imgstd: " << sql_imgstd;
    pthread_mutex_lock(&databaseMutex);
    db_execute(sql_imgstd.c_str());
    std::string last_image_query = "SELECT MAX(_id) FROM image";
    vector<vector<string> > image_array = db_select(last_image_query.c_str(), 1);
    pthread_mutex_unlock(&databaseMutex);
    int db_image_id = atoi(image_array.at(0).at(0).c_str());
    //cout << "db_image_id: " << db_image_id << endl;
    return db_image_id;
}




 