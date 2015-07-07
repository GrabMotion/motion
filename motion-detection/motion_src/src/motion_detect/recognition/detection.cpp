
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
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "opencv2/opencv.hpp"
#include "opencv2/highgui/highgui.hpp"

#include "../tinyxml/tinyxml.h"
#include "../tinyxml/tinystr.h"
#include <string>
#include <vector>
#include <functional>
#include <ctime>
#include <unistd.h>
#include <cstring>

using namespace std;
using namespace cv;

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
        int n_o_changes
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
    
    std::cout << "image_file: " << image_file << std::endl;
    
    return image_file;
}

// When motion is detected we write the image to disk
//    - Check if the directory exists where the image will be stored.
//    - Build the directory and image names.
inline bool createDirectoryTree(
        const string DIRECTORY, 
        const string EXTENSION, 
        const char * DIR_FORMAT, 
        const char * FILE_FORMAT,
        string instance
)
{
    stringstream ss, zz;
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
    directoryExistsOrCreate(ss.str().c_str());    
    ss << "/xml";
     directoryExistsOrCreate(ss.str().c_str());   
    zz.str("");
    zz << DIRECTORY << TIME << "/" + instance;         
    directoryExistsOrCreate(zz.str().c_str());
    zz << "/cropped";
    directoryExistsOrCreate(zz.str().c_str());    
    
    return true;
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
            //Mat cropped = result(rect);
            //cropped.copyTo(result_cropped);
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
            //Mat cropped = result(rect);
            //cropped.copyTo(result_cropped);
            rectangle(result,rect,color,1);
        }
        
        return number_of_changes;
    }
    return 0;
}

struct arg_struct
{
    motion::Message message;
};


void * startRecognition(void * arg)
{
    
    cout << "START RECOGNITION." << endl;
    
    pthread_mutex_t detectMutex;
    pthread_mutex_init(&detectMutex, 0);
    
    //struct arg_struct *args = (struct arg_struct *) arg;
    //cout << "Passing obj: " << args->message.type() << endl;
    
    is_recognizing = false;
    
    R_PROTO.Clear();
    R_PROTO = PROTO;
    
    //Region
    bool has_region;
    std::vector<cv::Point2f> region;
    if (R_PROTO.region())
    {
        cout << "Has region." << endl;
        if (R_PROTO.has_regioncoords())
        {
            cout << "Has regioncoords." << endl;
            std::string rc = R_PROTO.regioncoords();
            cout << "rc." << rc << endl;
            std::string rcoords = base64_decode(rc);
            cout << "rcoords." << rcoords << endl;
            region = processRegionString(rcoords);
            cout << "REGION SIZE :: " << region.size() << endl;
            if (region.size()>0)
                has_region = true;
            else
                has_region = false;
        }
    }
    else
    {
        has_region = false;
    }
    
    std::string instancecode;
    if (R_PROTO.has_codename())
    {
        cout << "Has code." << endl;
        std::string instancecode = R_PROTO.codename();
    }
    if (instancecode.empty())
        instancecode = "Prueba";
    
    bool writeImages = R_PROTO.storeimage();
    bool writeCroop  = R_PROTO.storecrop();
    bool send_number_detected = true;
    
    std::cout << "writeImages: " << writeImages << endl;
    std::cout << "writeCroop: " << writeCroop << endl;
    
    const string DIR        = "../../src/motion_web/pics/";          // directory where the images will be stored
    //const string REGION     = "../../src/motion_web/pics/region/";   // directory where the regios are stored
    const string EXT        = ".jpg";                           // extension of the images
    const string EXT_DATA   = ".xml";                           // extension of the data
    const int DELAY         = 500;                              // in mseconds, take a picture every 1/2 second
    const string LOG    = "../../src/motion_web/log";           // log for the export
    const string LOGCLEAR = LOG + "/log_remove";    
                                        // region vector storing xml region
    
    // fps calculated using number of frames / seconds
    double fps; 
    // start and end times
    time_t start, end; 
    // frame counter
    int counter = 0; 
    // floating point seconds elapsed since start
    double sec;
    
    // Create region directory if not exist.
    //directoryExistsOrCreate(REGION.c_str());
    //{
        // Detect motion in a region in steadof window
    
       //string file_region = REGION + "region" + EXT_DATA;
       //parseRegionXML(file_region, region);
    
    //}
    
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
    string DIR_FORMAT           = "%d%h%Y"; // 1Jan1970
    string FILE_FORMAT;//          = DIR_FORMAT + "/" + "%d%h%Y_%H%M%S"; // 1Jan1970/1Jan1970_12153
    string CROPPED_FILE_FORMAT;//   = DIR_FORMAT + "/cropped/" + "%d%h%Y_%H%M%S"; // 1Jan1970/cropped/1Jan1970_121539
    string XML_FILE             =  "<import>session";
    
    // Set up camera
    CvCapture * camera = cvCaptureFromCAM(CV_CAP_ANY);
    cvSetCaptureProperty(camera, CV_CAP_PROP_FRAME_WIDTH, 1280); //640); //1280); // width of viewport of camera
    cvSetCaptureProperty(camera, CV_CAP_PROP_FRAME_HEIGHT, 720); //480); //720); // height of ...
    
    // Take images and convert them to gray
    Mat result, result_cropped;
    Mat prev_frame = result = cvQueryFrame(camera);
    Mat current_frame = cvQueryFrame(camera);
    Mat next_frame = cvQueryFrame(camera);
    
    cvtColor(current_frame, current_frame, CV_RGB2GRAY);
    cvtColor(prev_frame, prev_frame, CV_RGB2GRAY);
    cvtColor(next_frame, next_frame, CV_RGB2GRAY);
    
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
    int countWhile;
    
    init_time = clock();
    
    // All settings have been set, now go in endless loop and
    // take as many pictures you want..
    while (true)
    {
        
        
        R_PROTO.set_recognizing(true);
        countWhile++;
        
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
        
        pthread_mutex_lock(&detectMutex);
        
                if (!has_region)
                {
                    //number_of_changes = detectMotion(motionmat, result, result_cropped,  x_start, x_stop, y_start, y_stop, max_deviation, color);
                    number_of_changes = detectMotion(motionmat, result, result_cropped,  x_start, x_stop, y_start, y_stop, max_deviation, color);
                    
                }
                else
                {
                    number_of_changes = detectMotionRegion(motionmat, result, result_cropped, region, max_deviation, color);
                }
                resutl_watch_detected = number_of_changes;
        
                std::cout << "number_of_changes = " << number_of_changes << std::endl;
        
        pthread_mutex_unlock(&detectMutex);
        
        // If a lot of changes happened, we assume something changed.
        if(number_of_changes>=there_is_motion)
        {
            
            if(number_of_sequence>0){            
                
                init_motion = true;
                
                if (!motion_detected) {                    
                    
                    count_sequence_cero = 0;
                    motion_detected     = true;
                    begin_time = clock();                             
                    
                    if (!has_instance_directory){
                        
                        instance_counter++;
                        stringstream id;
                        id << instance_counter;
                        instance = id.str(); 
                        
                        FILE_FORMAT = DIR_FORMAT + "/" + instance + "/" + "%d%h%Y_%H%M%S";
                        CROPPED_FILE_FORMAT = DIR_FORMAT + "/" + instance + "/" + "/cropped/" + "%d%h%Y_%H%M%S";
                        
                        pthread_mutex_lock(&detectMutex);
                        
                        createDirectoryTree (
                            DIR, 
                            EXT, 
                            DIR_FORMAT.c_str(), 
                            FILE_FORMAT.c_str(), 
                            instance );
                        
                        pthread_mutex_unlock(&detectMutex);
                        
                        has_instance_directory = true;
                    }    
                }    
                
                if (writeImages) {
                    
                
                    pthread_mutex_lock(&detectMutex);
                    
                    image_file_recognized = saveImg (
                        result, 
                        DIR, 
                        EXT, 
                        DIR_FORMAT.c_str(), 
                        FILE_FORMAT.c_str(),
                        number_of_changes
                    );
                
                    pthread_mutex_unlock(&detectMutex);
                
                }
                if (writeCroop)
                {
                    pthread_mutex_lock(&detectMutex);
                
                    string cropped_image_file = saveImg (
                            result_cropped,
                            DIR,
                            EXT,
                            DIR_FORMAT.c_str(),
                            CROPPED_FILE_FORMAT.c_str(),
                            number_of_changes
                    );
                    
                    pthread_mutex_unlock(&detectMutex);
                }
                                
            }
            
            number_of_sequence++;
        }
        else
        {            
            
            number_of_sequence = 0;              
            
            if ( count_sequence_cero == 20 & init_motion ) {                                  
                
                has_instance_directory = false;                  
                
                std::cout << "::::::::::::::::::::::::::::::::::::::::" << std::endl;
                std::cout << ":::::::::::: DUMP INSTANCE :::::::::::::" << std::endl;
                std::cout << "::::::::::::::::::::::::::::::::::::::::" << std::endl;
                
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
           
                pthread_mutex_lock(&detectMutex);
                
                writeXMLInstance(XMLFILE.c_str(), begin.str(), end.str(), instance);
                
                motion::Message::Instance * inst = R_PROTO.add_instance();
                inst->set_idinstance(0);
                inst->set_instanceamount(number_of_changes);
                inst->set_filepath("hola");
                
                
                pthread_mutex_unlock(&detectMutex);
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

    R_PROTO.set_recognizing(false);
    
}

