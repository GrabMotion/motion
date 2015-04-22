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
#include "tinyxml/tinyxml.h"
#include "tinyxml/tinystr.h"
#include <string>
#include <vector>
#include <functional>


using namespace std;
using namespace cv;

// split string
string split(const string& s, char c, int position) {  
    
   string::size_type i = 0;
   string::size_type j = s.find(c);
   vector<string> v;
   
   while (j != string::npos) {
      v.push_back(s.substr(i, j-i));
      i = ++j;
      j = s.find(c, j);

      if (j == string::npos)
         v.push_back(s.substr(i, s.length()));
   }   
   return v[position];
}


// get current time
string getCurrentTime(){

    time_t currentTime;
    struct tm *localTime;

    time( &currentTime );                   // Get the current time
    localTime = localtime( &currentTime );  // Convert the current time to the local time

    int Day    = localTime->tm_mday;
    int Month  = localTime->tm_mon + 1;
    int Year   = localTime->tm_year + 1900;
    int Hour   = localTime->tm_hour;
    int Min    = localTime->tm_min;
    int Sec    = localTime->tm_sec;
    
    ostringstream _y;
    _y << Year;    
    ostringstream _m;
    _m << Month;    
    ostringstream _h;
    _h << Hour;
    ostringstream _mi;
    _mi << Min;
    ostringstream _s;
    _s << Sec;  
    
    string current = 
    _y.str()    + "_" + 
    _m.str()    + "_" + 
    _h.str()    + ":" + 
    _mi.str()   + ":" + 
    _s.str();
    
    std::cout << current << "  ";
    
    return current;
  
}

// Create initial XML file
void build_xml(const char * xmlPath)
{
    
    time_t     now = time(0);
    struct tm  tstruct;
    char       secs[80];
    tstruct = *localtime(&now);   
    strftime(secs, sizeof(secs), "%Y-%m-%d.%X", &tstruct);  
   
    TiXmlDocument doc;
    TiXmlDeclaration * decl = new TiXmlDeclaration( "1.0", "utf-8", "");
    TiXmlElement * motions = new TiXmlElement( "motions" );
    TiXmlText * text = new TiXmlText( secs );
    motions->LinkEndChild( text );
    doc.LinkEndChild( decl );
    doc.LinkEndChild( motions );
    doc.SaveFile( xmlPath );       
}

// Write XML file
// Check if the xml file exists, if not create it
// Write  the log for the motion detected.
inline void writeXMLFile (
        const string DIRECTORY, 
        const string EXTENSION, 
        const char * DIR_FORMAT, 
        const char * CROPPED_FILE_FORMAT, 
        const char * XML_FILE, 
        string time_count, 
        string frames_per_second, 
        string image_file,
        string cropped_image_file
        )
{       
   
     time_t seconds;
    struct tm * timeinfo;
    char TIME[80];
    time (&seconds);   
    timeinfo = localtime (&seconds);
     
    // Create name for the date directory
    strftime (TIME,80,DIR_FORMAT,timeinfo);
    
    string XMLFILE = DIRECTORY + TIME + "/cropped/xml/" + XML_FILE + "" + EXTENSION;
     
    std::ifstream infile( XMLFILE.c_str() );
    if(!infile.good()){
        build_xml(XMLFILE.c_str());       
    }  
    
    TiXmlDocument doc( XMLFILE.c_str() );    
    if ( doc.LoadFile() ){
        
        TiXmlElement * motions = doc.FirstChildElement();     
        TiXmlElement * start = new TiXmlElement( "start" );  
        string current = getCurrentTime();
        TiXmlText * text_start = new TiXmlText( current.c_str() );
        start->LinkEndChild( text_start );        
        start->SetAttribute("elapsed_time", time_count.c_str() ); 
        start->SetAttribute("frames_per_second", frames_per_second.c_str() );  
        
        //images links ::: original
        string li = "../../src/motion_web/";       
        string short_image_path = image_file.substr(li.length(), image_file.length());          
        string file_short =  split(short_image_path, '/', 2);        
        string url_image_file = "http://localhost/motion_web/" + short_image_path;
        
        TiXmlElement * image = new TiXmlElement( "image" );  
        TiXmlText * text_image = new TiXmlText( file_short.c_str() );
        image->SetAttribute("link", url_image_file.c_str() );
        image->LinkEndChild(text_image);
        
        //images links ::: cropped
        string lc = "../../src/motion_web/";       
        string short_cropped_path = cropped_image_file.substr (lc.length(), cropped_image_file.length());                   
        string cropped_short =  split(short_image_path, '/', 2);
        cropped_image_file = "http://localhost/motion_web/" + short_cropped_path;
        
        TiXmlElement * cropped_image = new TiXmlElement( "cropped_image" );  
        TiXmlText * text_cropped_image = new TiXmlText( cropped_short.c_str() );
        image->SetAttribute("link_cropped", cropped_image_file.c_str() );
        cropped_image->LinkEndChild(text_cropped_image);
                
        start->LinkEndChild( image );       
        start->LinkEndChild( cropped_image );          
        
        motions->LinkEndChild( start );          
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
inline string saveImg(Mat image, const string DIRECTORY, const string EXTENSION, const char * DIR_FORMAT, const char * FILE_FORMAT)
{
    stringstream ss;
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
    ss << "/cropped";
    directoryExistsOrCreate(ss.str().c_str());    
    ss << "/xml";
    directoryExistsOrCreate(ss.str().c_str());      
    
    // Create name for the image
    strftime (TIME,80,FILE_FORMAT,timeinfo);
    ss.str("");
    if(incr < 100) incr++; // quick fix for when delay < 1s && > 10ms, (when delay <= 10ms, images are overwritten)
    else incr = 0;
    ss << DIRECTORY << TIME << static_cast<int>(incr) << EXTENSION;
    string image_file = ss.str().c_str();
    imwrite(image_file, image);
    
    return image_file;
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

int main (int argc, char * const argv[])
{
    const string DIR        = "../../src/motion_web/pics/";    // directory where the images will be stored
    const string EXT        = ".jpg";           // extension of the images
    const string EXT_DATA   = ".xml";           // extension of the data
    const int DELAY         = 500;              // in mseconds, take a picture every 1/2 second
    const string LOG    = "../../src/motion_web/log";      //"/home/pi/motion_src/log";
    const string LOGCLEAR = LOG + "/log_remove";
    
       
    // fps calculated using number of frames / seconds
    double fps; 
    // start and end times
    time_t start, end; 
    // frame counter
    int counter = 0; 
    // floating point seconds elapsed since start
    double sec;
    
    // Create pics directory if not exist. 
    directoryExistsOrCreate(DIR.c_str());
    
    // Create log directory if not exist. 
    directoryExistsOrCreate(LOG.c_str());
    std::ifstream infile( LOGCLEAR.c_str() );
    if(!infile.good()){
        ofstream logfile;
        logfile.open ( LOGCLEAR.c_str() );
        logfile << "Removed files log.\n";
        logfile.close();
    } 
    
    // Format of directory
    string DIR_FORMAT           = "%d%h%Y"; // 1Jan1970
    string FILE_FORMAT          = DIR_FORMAT + "/" + "%d%h%Y_%H%M%S"; // 1Jan1970/1Jan1970_12153
    string CROPPED_FILE_FORMAT  = DIR_FORMAT + "/cropped/" + "%d%h%Y_%H%M%S"; // 1Jan1970/cropped/1Jan1970_121539
    string XML_FILE             =  "motion";
    
    // Set up camera
    CvCapture * camera = cvCaptureFromCAM(CV_CAP_ANY);
    cvSetCaptureProperty(camera, CV_CAP_PROP_FRAME_WIDTH, 1280); // width of viewport of camera
    cvSetCaptureProperty(camera, CV_CAP_PROP_FRAME_HEIGHT, 720); // height of ...
    
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
    Mat d1, d2, motion;
    int number_of_changes, number_of_sequence = 0;
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
    
    // time opencv
    double t = (double)getTickCount(); 
    
    // All settings have been set, now go in endless loop and
    // take as many pictures you want..
    while (true){   
        
        // Take a new image
        prev_frame = current_frame;
        current_frame = next_frame;
        next_frame = cvQueryFrame(camera);
        result = next_frame;
        cvtColor(next_frame, next_frame, CV_RGB2GRAY);
        
        
        // count frames per second
        // grab a frame 
        IplImage *frame = cvRetrieveFrame(camera);
        // see how much time has elapsed
        time(&end);
        // calculate current FPS
         ++counter;        
         sec = difftime (end, start);  
        

        // Calc differences between the images and do AND-operation
        // threshold image, low differences are ignored (ex. contrast change due to sunlight)
        absdiff(prev_frame, next_frame, d1);
        absdiff(next_frame, current_frame, d2);
        bitwise_and(d1, d2, motion);
        threshold(motion, motion, 35, 255, CV_THRESH_BINARY);
        erode(motion, motion, kernel_ero);
        
        number_of_changes = detectMotion(motion, result, result_cropped,  x_start, x_stop, y_start, y_stop, max_deviation, color);
        
        // If a lot of changes happened, we assume something changed.
        if(number_of_changes>=there_is_motion)
        {
            if(number_of_sequence>0){ 
                
                string image_file = saveImg (
                        result, 
                        DIR, 
                        EXT, 
                        DIR_FORMAT.c_str(), 
                        FILE_FORMAT.c_str()
                );
                
                string cropped_image_file = saveImg (
                        result_cropped,
                        DIR,
                        EXT,
                        DIR_FORMAT.c_str(),
                        CROPPED_FILE_FORMAT.c_str()
                );        
                    
                //elapsed time
                t = ((double)getTickCount() - t)/getTickFrequency(); 
                std::cout << "Times passed in seconds: " << t << std::endl;
                ostringstream strs;
                strs << t;               
                
                //frames per second
                fps = counter / sec;
                std::cout << "Frame per second: " << fps << "  ";
                ostringstream fpst;
                fpst << fps;
                //https://sublimated.wordpress.com/2011/02/17/benchmarking-frames-per-second-when-using-opencvs-cvcapturefromcam/
                
                writeXMLFile(
                        DIR,
                        EXT_DATA,
                        DIR_FORMAT.c_str(),
                        CROPPED_FILE_FORMAT.c_str(),
                        XML_FILE.c_str(), 
                        strs.str(),
                        fpst.str(), 
                        image_file,
                        cropped_image_file
                       );                   
            }
            number_of_sequence++;
        }
        else
        {
            number_of_sequence = 0;
            // Delay, wait a 1/2 second.
            cvWaitKey (DELAY);
        }
    }
    return 0;    
}