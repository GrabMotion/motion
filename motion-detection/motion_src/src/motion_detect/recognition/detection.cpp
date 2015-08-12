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
  
#include <sys/time.h>
  
using namespace std;
using namespace cv;
  
motion::Message::MotionCamera * pcamera;
motion::Message::MotionMonth * pmonth;
motion::Message::MotionDay * pday;
motion::Message::Instance * pinstance;
cv::VideoWriter * videout;
CvVideoWriter * cvvideout;
  
pthread_t thread_store_instance;
  
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
    for(int j = 0; j < picture.rows; j++)
    {
        for(int i = 0; i < picture.cols; i++)
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
                              string instance
                              )
{
     
    TiXmlDocument doc( XMLFILE.c_str() );
    if ( doc.LoadFile() ){
         
        TiXmlElement* file = doc.FirstChildElement();
         
        TiXmlElement* session_info = file->FirstChildElement();
         
        TiXmlElement* all_instances = session_info->NextSiblingElement();;
         
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
        TiXmlText * text_code = new TiXmlText( "Prueba" );
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
                      int n_o_changes,
                      string img
                      )
{
    stringstream ss;
    time_t seconds;
    struct tm * timeinfo;
    char TIME[80];
    time (&seconds);
    // Get the current time
    timeinfo = localtime (&seconds);
     
    std::string amount_str = getGlobalIntToString(n_o_changes);
     
    std::string n_str_file = "_" + amount_str;
     
    char * n_file = new char[n_str_file.size() + 1];
    std::copy(n_str_file.begin(), n_str_file.end(), n_file);
    n_file[n_str_file.size()] = '\0';
     
    //std::cout << "n_file: " << n_file << std::endl;
     
    // Create name for the image
    strftime (TIME,80,FILE_FORMAT,timeinfo);
    if(incr < 100) incr++; // quick fix for when delay < 1s && > 10ms, (when delay <= 10ms, images are overwritten)
    else incr = 0;
    ss << DIRECTORY << TIME << static_cast<int>(incr) << n_file << EXTENSION;
    string image_file = ss.str().c_str();
    imwrite(image_file, image);
    
    struct timeval tr;
    struct tm* ptmr;
    char time_info[40];
    gettimeofday (&tr, NULL);
    ptmr = localtime (&tr.tv_sec);
    strftime (time_info, sizeof (time_info), "%Y-%m-%d %H:%M:%S %z", ptmr);
    //cout <<  ":::time_info:::: " << time_info << endl;
     
    if (img.empty())
    {
        motion::Message::Image * pimage = pinstance->add_image();
        pimage->set_path(image_file.c_str());
        pimage->set_name(n_str_file);
        pimage->set_imagechanges(n_o_changes);
        pimage->set_timeimage(time_info);
        //std::cout << "image_file: " << image_file << std::endl;
    } else
    {
        motion::Message::Crop * pcrop = pinstance->add_crop();
        pcrop->set_path(image_file.c_str());
        pcrop->set_name(n_str_file);
        pcrop->set_imagefather(img);
        //std::cout << "image_crop_file: " << image_file << std::endl;
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
                                const char * FILE_FORMAT,
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
    zz << "/cropped";
    //cout << "CREATE CROPPED DIRECTORY:: " << zz.str() << endl;
    directoryExistsOrCreate(zz.str().c_str());
     
    return result;
}
  
// Check if there is motion in the result matrix
// count the number of changes and return.
inline int detectMotion(const Mat & motion, Mat & result, Mat & result_cropped,
                        int x_start, int x_stop, int y_start, int y_stop,
                        int max_deviation,
                        Scalar & color)
{
    // calculate the standard deviation
    Scalar mean, stddev;
    meanStdDev(motion, mean, stddev);
    // if not to much changes then the motion is real (neglect agressive snow, temporary sunlight)
    if(stddev[0] < max_deviation)
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
            Mat cropped = result(rect);
            cropped.copyTo(result_cropped);
            rectangle(result,rect,color,1);
        }
        return number_of_changes;
    }
    return 0;
}
  
// Check if there is motion in the result matrix
// count the number of changes and return.
inline int detectMotionRegion(const cv::Mat & motionmat,
                              cv::Mat & result,
                              cv::Mat & result_cropped,
                              std::vector<cv::Point2f> & region,
                              int max_deviation,
                              cv::Scalar & color)
{
     
    // calculate the standard deviation
    Scalar mean, stddev;
    meanStdDev(motionmat, mean, stddev);
     
    // if not to much changes then the motion is real (neglect agressive snow, temporary sunlight)
    if(stddev[0] < max_deviation)
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
            Mat cropped = result(rect);
            cropped.copyTo(result_cropped);
            rectangle(result,rect,color,1);
        }
         
        return number_of_changes;
    }
    return 0;
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
     
    is_recognizing = false;
     
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
     
    //database camera id.
    stringstream camera_id_query;
    camera_id_query <<
    "SELECT _id, name, number FROM cameras where number = " << cam << ";";
    cout << " camera_id_query " << camera_id_query.str() << endl;
    vector<vector<string> > camera_array = db_select(camera_id_query.str().c_str(), 3);
    db_camera_id        = atoi(camera_array.at(0).at(0).c_str());
    string str_camera   = camera_array.at(0).at(1);
    int number          = atoi(camera_array.at(0).at(2).c_str());
    cout << "str_camera: " << str_camera << endl;
    cout << "db_camera_id: " << db_camera_id << endl;
    cout << "number: " << number << endl;
  
    //Check if exist month on proto or else add it.
    int sizec = R_PROTO.motioncamera_size();
    cout << "sizec: " << sizec << endl;
    bool cameraexist = false;
    for (int i = 0; i < sizec; i++)
    {
        cout << "entra" << endl;
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
        pcamera = R_PROTO.add_motioncamera();
        pcamera->set_cameraname(str_camera);
        cout << "sigo" << endl;
    }
    if (pcamera->has_cameraid())
        cout << "pcamera->cameraid(): "     << pcamera->cameraid()      << endl;
     
    if (pcamera->has_cameranumber())
        cout << "pcamera->cameranumber(): " << pcamera->cameranumber()  << endl;
     
    if (pcamera->has_cameraname())
        cout << "pcamera->cameraname(): "   << pcamera->cameraname()    << endl;
     
    //Region
    bool has_region;
    std::vector<cv::Point2f> region;
    std::string rcoords;
    if (pcamera->hasregion())
    {
        cout << "Has region." << endl;
        std::string rc = pcamera->coordinates();
        cout << "rc." << rc << endl;
        rcoords = base64_decode(rc);
        cout << "rcoords." << rcoords << endl;
        region = processRegionString(rcoords);
        cout << "REGION SIZE :: " << region.size() << endl;
        if (region.size()>0)
            has_region = true;
        else
            has_region = false;
         
    }
    else
    {
        has_region = false;
    }
     
    if (has_region)
    {
        std::replace( rcoords.begin(), rcoords.end(), '\n', ' ');
        stringstream sql_coord;
        sql_coord <<
        "INSERT INTO coordinates (coordinates) " <<
        "VALUES ('" << rcoords    << "');";
        db_execute(sql_coord.str().c_str());
        cout << "pasa" << endl;
        std::string last_coordinates_query = "SELECT MAX(_id) FROM coordinates";
        vector<vector<string> > coords_array = db_select(last_coordinates_query.c_str(), 1);
        cout << "coords_array: " << endl;
        db_coordnates_id = atoi(coords_array.at(0).at(0).c_str());
        cout << "db_coordnates_id: " << db_coordnates_id << endl;
    }
     
    std::string instancecode;
    string XML_FILE;
    if (pcamera->has_codename())
    {
        cout << "Has codename." << endl;
        instancecode = pcamera->codename();
    }
    if (instancecode.empty())
        instancecode = "Prueba";
     
    double delay;
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
        pmonth = pcamera->add_motionmonth();
        pmonth->set_monthlabel(str_month);
        cout << "sigo" << endl;
    }
     
    //month database.
    stringstream sql_month;
    sql_month <<
    "INSERT INTO month (label) "       <<
    "SELECT '" << str_month << "' "  <<
    "WHERE NOT EXISTS (SELECT * FROM month WHERE label = '" + str_month + "');";
    db_execute(sql_month.str().c_str());
    std::string last_month_id_query = "SELECT MAX(_id) FROM month";
    vector<vector<string> > month_array = db_select(last_month_id_query.c_str(), 1);
    db_month_id = atoi(month_array.at(0).at(0).c_str());
    cout << "db_month_id: " << db_month_id << endl;
     
    //rel_camera_month.
    stringstream sql_rel_camera_month;
    sql_rel_camera_month <<
    "INSERT INTO rel_camera_month (_id_camera, _id_month) "       <<
    "SELECT " << db_camera_id << ", "  << db_month_id <<
    " WHERE NOT EXISTS (SELECT * FROM rel_camera_month WHERE _id_camera = " << db_camera_id <<
    " AND _id_month = " << db_month_id << ");";
    db_execute(sql_rel_camera_month.str().c_str());
     
    // get rel id.
    std::string last_rel_camera_month_query = "SELECT MAX(_id) FROM rel_camera_month";
    vector<vector<string> > rel_camera_month_array = db_select(last_rel_camera_month_query.c_str(), 1);
    db_rel_camera_month_id = atoi(rel_camera_month_array.at(0).at(0).c_str());
    cout << "db_rel_camera_month_id: " << db_rel_camera_month_id << endl;
     
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
        pday = pmonth->add_motionday();
        pday->set_daylabel(str_day);
        cout << "sigo" << endl;
    }
     
    if (pday->has_xmlfilename())
    {
        cout << "has_xmlfile." << endl;
        XML_FILE = pday->xmlfilename();
    }
    else
    {
        XML_FILE  =  "<import>session";
        pday->set_xmlfilename(XML_FILE);
    }
     
     
    bool writeImages = pcamera->storeimage();
    bool writeCroop  = pcamera->storecrop();
    bool writeVideo  = pcamera->storevideo();
    bool send_number_detected = true;
     
    std::cout << "writeImages: " << writeImages << endl;
    std::cout << "writeCroop: " << writeCroop << endl;
     
    google::protobuf::uint32 activecam = R_PROTO.activecam();
     
    std::string xml_path = getXMLFilePathAndName(activecam, R_PROTO, XML_FILE);
     
    //Day database.
    stringstream sql_day;
    sql_day <<
    "INSERT INTO day (label, xmlfile, xmlfilepath) " <<
    "SELECT '" << str_day << "', '" << XML_FILE << "', '" << xml_path   << "' " <<
    "WHERE NOT EXISTS (SELECT * FROM day WHERE label = '" << str_day    << "' " <<
    "AND xmlfile        = '" << XML_FILE << "' " <<
    "AND xmlfilepath    = '" << xml_path << "');";
    db_execute(sql_day.str().c_str());
     
    std::string last_day_id_query = "SELECT MAX(_id) FROM day";
    vector<vector<string> > day_array = db_select(last_day_id_query.c_str(), 1);
    db_day_id = atoi(day_array.at(0).at(0).c_str());
    cout << "db_day_id: " << db_day_id << endl;
     
    
    //Rel day month.
    stringstream sql_rel_day_month;
    sql_rel_day_month <<
    "INSERT INTO rel_month_day (_id_month, _id_day) " <<
    "SELECT " << db_month_id << ", " << db_day_id << 
    " WHERE NOT EXISTS (SELECT * FROM rel_month_day WHERE _id_month = " << db_month_id <<
    " AND _id_day = " << db_day_id << ");";
    db_execute(sql_rel_day_month.str().c_str());
    
     
    //Alarm Interval Start End
    stringstream sql_interval;
    sql_interval <<
    "INSERT INTO interval (timestart, timeend) " <<
    "SELECT '" << pcamera->timestart() << "', '" << pcamera->timeend() << "' " << 
    "WHERE NOT EXISTS (SELECT * FROM interval WHERE timestart = '" << pcamera->timestart() << "' " <<
    "AND timeend = '" << pcamera->timeend() << "');";
    db_execute(sql_interval.str().c_str());
     
    std::string last_interval_id_query = "SELECT MAX(_id) FROM interval";
    vector<vector<string> > interval_array = db_select(last_interval_id_query.c_str(), 1);
    db_interval_id = atoi(interval_array.at(0).at(0).c_str());
    cout << "db_interval_id: " << db_interval_id << endl;
             
    //recognition_setup database.
    stringstream sql_recognition_setup;
    sql_recognition_setup <<
    "INSERT INTO recognition_setup " <<
    "(name, _id_interval, _id_day, _id_camera, _id_mat, storeimage, storecrop, storevideo, codename, has_region, _id_coordinates, delay, runatstartup) " <<
    "SELECT "           << "'"      << 
    pcamera->name()     << "', "    <<
    db_interval_id      << ", "     <<
    db_day_id           << ", "     <<
    db_camera_id        << ", "     <<
    pcamera->db_idmat() << ", "     <<
    writeImages         << ", "     <<
    writeCroop          << ", "     <<
    writeVideo          << ", '"    <<
    instancecode        << "' ,"    <<
    has_region          << ", "     <<
    db_coordnates_id    << ", "     <<
    delay               << ", "     <<
    pcamera->runatstartup()         <<    
    " WHERE NOT EXISTS (SELECT * FROM recognition_setup WHERE"  <<
    " name              = '" << pcamera->name()     << "' AND"  <<
    " _id_interval      = " << db_interval_id       << " AND"   <<
    " _id_day           = " << db_day_id            << " AND"   <<
    " _id_camera        = " << db_camera_id         << " AND"   <<
    " _id_mat           = " << pcamera->db_idmat()  << " AND"   <<
    " storeimage        = " << writeImages          << " AND"   <<
    " storecrop         = " << writeCroop           << " AND"   <<
    " storevideo        = " << writeVideo           << " AND"   <<
    " codename          = '" << instancecode        <<"' AND"   <<
    " has_region        = " << has_region           <<"' AND"   <<
    " runatstartup      = " << pcamera->runatstartup()          << ");";
    std::string sqlrecsetup = sql_recognition_setup.str();
    //cout << "rec setup sql: " << sqlrecsetup << endl;
    db_execute(sqlrecsetup.c_str());
    std::string last_recognition_setup_id_query = "SELECT MAX(_id) FROM recognition_setup";
    vector<vector<string> > recognition_setup_array = db_select(last_recognition_setup_id_query.c_str(), 1);
    db_recognition_setup_id = atoi(day_array.at(0).at(0).c_str());
    cout << "db_recognition_setup_id: " << db_day_id << endl;
     
    //time
    struct timeval tr;
    struct tm* ptmr;
    char time_rasp[40];
    gettimeofday (&tr, NULL);
    ptmr = localtime (&tr.tv_sec);
    strftime (time_rasp, sizeof (time_rasp), "%Y-%m-%d %H:%M:%S %z", ptmr);
     
    //rel_camera_month.
    stringstream sql_rel_camera_recognition_setup;
    sql_rel_camera_recognition_setup <<
    "INSERT INTO rel_camera_recognition_setup (_id_camera, _id_recognition_setup, start_rec_time) " <<
    "SELECT " << db_camera_id << ", "  << db_recognition_setup_id << ", '" << time_rasp << "' "
    " WHERE NOT EXISTS (SELECT * FROM rel_camera_recognition_setup WHERE _id_camera = " << db_camera_id <<
    " AND _id_recognition_setup = " << db_recognition_setup_id << ");";
    cout << "sql_rel_camera_recognition_setup: " << sql_rel_camera_recognition_setup.str() << endl;
    db_execute(sql_rel_camera_recognition_setup.str().c_str());
     
    pday->set_db_dayid(db_day_id);
    pday->set_db_recognitionsetupid(db_recognition_setup_id);
     
    //Camera dir.
    std::stringstream camdir;
    camdir << sourcepath << "motion_web/pics/" << "camera" << cam << "/";
    std::string cmd = camdir.str();
    
    
    //Create camera directory
    directoryExistsOrCreate(camdir.str().c_str());
     
    const string DIR        = camdir.str();          // directory where the images will be stored
    //const string REGION     = sourcepath + "motion_web/pics/region/";   // directory where the regios are stored
    const string EXT        = ".jpg";                           // extension of the images
    const string EXT_DATA   = ".xml";                           // extension of the data
    const int DELAY         = 100; //500;                              // in mseconds, take a picture every 1/2 second
    const string LOG        = sourcepath + "/motion_web/log";           // log for the export
    const string LOGCLEAR = LOG + "/log_remove";
    // region vector storing xml region
   
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
    cvSetCaptureProperty(camera, CV_CAP_PROP_FRAME_WIDTH, 1280); //640); //1280); // width of viewport of camera
    cvSetCaptureProperty(camera, CV_CAP_PROP_FRAME_HEIGHT, 720); //480); //720); // height of ...
    double fps = 20; //120;
    cvSetCaptureProperty(camera, CV_CAP_PROP_FPS, fps);
    
    // Take images and convert them to gray
    Mat result, result_cropped;
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
    int number_of_changes, number_of_sequence = 0, count_sequence_cero = 0, count_save_image = 0;
    Scalar mean_, color(0,255,255); // yellow
     
    // Detect motion in window
    int x_start = 10, x_stop = current_frame.cols-11;
    int y_start = 350, y_stop = 530;
     
    // If more than 'there_is_motion' pixels are changed, we say there is motion
    // and store an image on disk
    int there_is_motion = 5;
     
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
     
    init_time = clock();
     
    pcamera->set_startrectime(time_rasp);
    startrecognitiontime = time_rasp;
     
    cout << "TIME STARTED: " << time_rasp << endl;
      
    // All settings have been set, now go in endless loop and
    // take as many pictures you want..
    while (true)
    {
         
        is_recognizing = true;
         
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
         
        if (!has_region)
        {
           number_of_changes = detectMotion(motionmat, result, result_cropped,  x_start, x_stop, y_start, y_stop, max_deviation, color);
        }
        else
        {
            number_of_changes = detectMotionRegion(motionmat, result, result_cropped, region, max_deviation, color);
        }
        resutl_watch_detected = number_of_changes;
         
        // If a lot of changes happened, we assume something changed.
        if(number_of_changes>=there_is_motion)
        {
             
            //cout << "number_of_changes>=there_is_motion" << endl;
             
            //cout << "number_of_sequence:: " << number_of_sequence << endl;
             
            if(number_of_sequence>0)
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
                        pinstance = pday->add_instance();
                        pinstance->set_idinstance(std::atoi(instance.c_str()));
                         
                        FILE_FORMAT = DIR_FORMAT + "/" + instance + "/" + "%d%h%Y_%H%M%S";
                        CROPPED_FILE_FORMAT = DIR_FORMAT + "/" + instance + "/cropped/" + "%d%h%Y_%H%M%S";
                         
                        vector<string> dirs = createDirectoryTree (DIR,EXT,DIR_FORMAT.c_str(),FILE_FORMAT.c_str(),instance );
                        
                        if (writeVideo)
                        {    
                            
                            //std::string ext = ".avi";
                            //std::string ext = ".mpeg";
                            //std::string ext = ".mp4";
                            //std::string ext = ".divx";
                            //std::string ext = ".flv";
                            std::string ext = ".mpg";
                           
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
                            int codec = CV_FOURCC('A','Y','U','V');
                            //int codec = CV_FOURCC('I','U','Y','V');
                            //int codec = CV_FOURCC('X','V','I','D');
                            //int codec = CV_FOURCC('P','I','M','1');
                            //int codec = CV_FOURCC('W','M','V','2');
                            //int codec = -1;
                            
                            
                            cv::Size size = cv::Size(1280,720);
                            
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

                            
                            /*videout = new cv::VideoWriter(videopath.c_str(), codec, fps, size, true);
                            if (videout->open(videopath, codec, fps, size, true))
                            {
                                cout << "opened" << endl;
                            }
                
                            if( (cvvideout = cvCreateVideoWriter(videopath.c_str(), codec, fps, size, true)) == NULL )
                            {
                                cout << "An error occured whilst making the video file." << endl;
                            }
                            else
                            {
                                cout << "Video creation successful!" << endl;
                            }*/
                           
                            motion::Message::Video * pvideo = pinstance->mutable_video();
                            pvideo->set_path(videopath.c_str());
                            pvideo->set_name(videoname.c_str());
                            pvideo->set_instancefolder(dirs.at(1));
                        }
                    
                        has_instance_directory = true;
                         
                    }
                }
                
                if (writeImages)
                {
                    std::string emptystr = string();
                    image_file_recognized = saveImg (result,DIR,EXT,DIR_FORMAT.c_str(),FILE_FORMAT.c_str(),number_of_changes,emptystr);
                }
                if (writeCroop)
                {
                    string cropped_image_file = saveImg (result_cropped,DIR,EXT,DIR_FORMAT.c_str(),CROPPED_FILE_FORMAT.c_str(),number_of_changes,image_file_recognized);
                }
                 
                //if (writeVideo)
                //{
                //    IplImage* img = cvQueryFrame(camera); 
                //    videout->write(img);
                //}
                    
            }
            delaymark = time(0);
            number_of_sequence++;
        }
        else
        {
             
            number_of_sequence = 0;
             
            double seconds_since_start = difftime( time(0), delaymark);
             
            if ( seconds_since_start > delay & init_motion )
            {
                 
                has_instance_directory = false;
                init_motion = false;
                 
                //std::cout << "::::::::::::::::::::::::::::::::::::::::" << std::endl;
                std::cout << ":::::::::::: DUMP INSTANCE "<< instance << "  :::::::::::" << std::endl;
                //std::cout << "::::::::::::::::::::::::::::::::::::::::" << std::endl;
                 
                end = clock();
                 
                //create xml file if not exists
                time_t seconds;
                struct tm * timeinfo;
                char TIME[80];
                time (&seconds);
                timeinfo = localtime (&seconds);
                 
                // Create name for the date directory
                const char * dir = DIR_FORMAT.c_str();
                strftime (TIME,80,dir,timeinfo);
                string XMLFILE = DIR + TIME + "/xml/" + XML_FILE + "" + EXT_DATA;
                if(!std::ifstream(XMLFILE.c_str()))
                {
                    build_xml(XMLFILE.c_str());
                }
                 
                std::ostringstream begin;
                begin << (begin_time - init_time) / CLOCKS_PER_SEC;
                 
                std::ostringstream end;
                end << (end_time - init_time) / CLOCKS_PER_SEC;
                 
                writeXMLInstance(XMLFILE.c_str(), begin.str(), end.str(), instance);
                 
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
                
                //cout <<  ":::time_info:::: " << time_info << endl;
                //cout << "ins.image_size():: " << pinstance->image_size() << endl;

                vector<int> images;
                images.clear();

                int imagesize = pinstance->image_size();

                for (int j = 0; j < imagesize; j++)
                {
                    //Image
                    int db_image_id;
                    const motion::Message::Image & img = pinstance->image(j);

                    //cout << "img.path():: " << img.path() << endl;

                    stringstream sql_img;
                    sql_img <<
                    "INSERT INTO image (path, name, imagechanges, time) " <<
                    "SELECT '"  << img.path()           <<
                    "', '"      << img.name()           <<
                    "', "       << img.imagechanges()   <<
                    ",'"       <<  img.timeimage()           << "'"
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
                    db_image_id = atoi(image_array.at(0).at(0).c_str());

                    //cout << "db_image_id: " << db_image_id << endl;

                    if (writeCroop)
                    {
                        //Crop
                        int db_crop_id;
                        const motion::Message::Crop & crop = pinstance->crop(j);
                        string path = crop.path();
                        stringstream sql_crop;
                        sql_crop <<
                        "INSERT INTO crop (path, name, _id_image_father) " <<
                        "SELECT '"  << crop.path() <<
                        "', '"      << crop.name() <<
                        "', "       << db_image_id <<
                        " WHERE NOT EXISTS (SELECT * FROM crop WHERE path = '" << crop.path() << "'" <<
                        " AND name = '"             << crop.name()      << "'"  <<
                        " AND _id_image_father = "  << db_image_id      << ");";
                        pthread_mutex_lock(&databaseMutex);
                        db_execute(sql_crop.str().c_str());
                        std::string last_crop_query = "SELECT MAX(_id) FROM crop";
                        vector<vector<string> > crop_array = db_select(last_crop_query.c_str(), 1);
                        pthread_mutex_unlock(&databaseMutex);
                        db_crop_id = atoi(crop_array.at(0).at(0).c_str());
                        //cout << "db_crop_id: " << db_crop_id << endl;
                    }
                   
                    images.push_back(db_image_id);

                }
                
                /*if (writeVideo)
                {
                    if (videout->isOpened())
                    {
                        videout->release();
                    }
                }*/
                
                motion::Message::Video dvideo = pinstance->video();
                std::string name = dvideo.name();
                std::string path = dvideo.path();
                
                stringstream sql_video;
                sql_video <<
                "INSERT INTO video (path, name) " <<
                "SELECT '"  << path << "'" <<
                ", '"       << name << "'" <<
                " WHERE NOT EXISTS (SELECT * FROM video WHERE path = '" << path << "'" <<
                " AND name = '" << name << "');";
                //cout << "sql_video: " << sql_video.str() << endl;
                
                pthread_mutex_lock(&databaseMutex);
                db_execute(sql_video.str().c_str());
                std::string last_video_query = "SELECT MAX(_id) FROM video";
                vector<vector<string> > video_array = db_select(last_video_query.c_str(), 1);
                pthread_mutex_unlock(&databaseMutex);
                int db_video_id = atoi(video_array.at(0).at(0).c_str());
                //cout << "db_video_id: " << db_video_id << endl;

                std::string instancestart   = pinstance->instancestart();
                std::string instanceend     = pinstance->instanceend();

                //std::stringstream imagesarray;
                //std::copy(images.begin(), images.end(), std::ostream_iterator<int>(imagesarray, " "));

                //Instance
                stringstream sql_instance;
                sql_instance <<        
                "INSERT INTO instance (instancestart, instanceend, _id_video, time) " <<
                "SELECT '"  << instancestart            << "'" << 
                ", '"       << instanceend              << "'" <<
                ", "        << db_video_id              <<
                ", '"       << time_info                << "'" << 
                " WHERE NOT EXISTS (SELECT * FROM instance WHERE instancestart = '" << pinstance->instancestart() << "'" <<
                " AND instanceend   = '"  << pinstance->instanceend()   <<    "'"    <<
                " AND _id_video     = "   << db_video_id                << 
                " AND time          = '"  << time_info                  <<    "'"    << ");";
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
                "INSERT INTO rel_day_instance (_id_day, _id_instance, time) " <<
                "SELECT "  << dayid    <<
                ", "       << db_instance_id    <<
                ", '"       << time_info         << "'" <<
                " WHERE NOT EXISTS (SELECT * FROM rel_day_instance WHERE _id_day = " << dayid << 
                " AND _id_instance = " << db_instance_id << ");";
                pthread_mutex_lock(&databaseMutex);
                db_execute(sql_rel_day_instance.str().c_str());
                pthread_mutex_unlock(&databaseMutex);

                //day.clear_instance();
                pinstance->Clear();
                
            }
             
            // XML Instance
            motion_detected = false;
            count_sequence_cero++;
             
            if (count_sequence_cero==1){
                end_time = clock();
            }
             
            // Delay, wait a 1/2 second.
            cvWaitKey (DELAY);
        }
    }
     
    return 0;
}
 