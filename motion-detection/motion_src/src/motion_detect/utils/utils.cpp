/*
 * File:   utils.h
 * Author: jose
 *
 * Created on Dec 12, 2015, 7:48 PM
 */

#include "../utils/utils.h"
#include "../database/database.h"
#include "../operations/camera.h"
#include "../http/post.h"

#include <iostream>     // std::cout
#include <fstream>      // std::ifstream

#include <sys/time.h>

#include <dirent.h>
#include <sys/stat.h>

#include <string>

#include <errno.h>

std::string getXMLFilePathAndName(std::string sourcepath, int cam, std::string recname, std::string currday, std::string name)
{
    stringstream DIR;
    DIR << sourcepath << "motion_web/pics/" << "camera" << cam << "/" << recname << "/" << currday << "/";
    std::string XML_FILE  =  "xml/" + name;
    std::string xml_path = DIR.str() + XML_FILE + ".xml";
    return xml_path;
}

std::string fixedLength(int value, int digits)
{
    unsigned int uvalue = value;
    if (value < 0) {
        uvalue = -uvalue;
    }
    std::string result;
    while (digits-- > 0) {
        result += ('0' + uvalue % 10);
        uvalue /= 10;
    }
    if (value < 0) {
        result += '-';
    }
    std::reverse(result.begin(), result.end());
    return result;
}

void set_file_permission(std::string file, std::string permission)
{
    std::stringstream perm;
    perm << "chmod " <<  permission << " " << file;
    std::string pstring = perm.str();
    system(pstring.c_str());
}

char * getTimeRasp()
{
    struct timeval tr;
    struct tm* ptmr;
    char time_rasp[40];
    gettimeofday (&tr, NULL);
    ptmr = localtime (&tr.tv_sec);
    strftime (time_rasp, sizeof (time_rasp), "%Y-%m-%d %H:%M:%S %z", ptmr);
    return time_rasp;
}

char *getShortTimeRasp()
{
    struct timeval tr;
    struct tm* ptmr;
    char time_rasp[40];
    gettimeofday (&tr, NULL); 
    ptmr = localtime (&tr.tv_sec);
    strftime (time_rasp, sizeof (time_rasp), "%H:%M:%S", ptmr);
    return time_rasp;
}

// Check if the directory exists, if not create it
// This function will create a new directory if the image is the first
// image taken for a specific day
void directoryExistsOrCreate(const char* pzPath)
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

bool checkFile(const std::string &file)
{
    FILE *fin = fopen((file).c_str(), "r");
    if (fin)
    {
        fclose(fin);
        return false;
    }
    return true;
}

char * setMessageValueBody(int value, std::string body)
{
    
    std::string command = getGlobalIntToString(value);
    
    char * message = new char[command.size() + 1];
    std::copy(command.begin(), command.end(), message);
    message[command.size()] = '\0';
    
    char * action = new char[body.size() + 1];
    std::copy(body.begin(), body.end(), action);
    action[body.size()] = '\0'; // don't forget the terminating 0
    
    char buffer[256];
    strncpy(buffer, message, sizeof(buffer));
    strncat(buffer, action, sizeof(buffer));
    
    return buffer;
}

std::string get_file_contents(std::string filename)
{
    std::ifstream in(filename.c_str(), std::ios::in | std::ios::binary);
    if (in)
    {
        std::string contents;
        in.seekg(0, std::ios::end);
        contents.resize(in.tellg());
        in.seekg(0, std::ios::beg);
        in.read(&contents[0], contents.size());
        in.close();
        return(contents);
    }
    throw(errno);
}

bool to_bool(std::string const& s)
{
    return s != "0";
}

int getGlobalStringToInt(std::string id)
{
    return atoi( id.c_str() );
}

std::string getCurrentMonthLabel()
{
    struct timeval tm;
    struct tm* ptm;
    char month_rasp[5];
    gettimeofday (&tm, NULL);
    ptm = localtime (&tm.tv_sec);
    strftime (month_rasp, sizeof (month_rasp), "%h", ptm);
    std::string _month(month_rasp, 3);
    return _month;
}

std::string getCurrentDayLabel()
{
    struct timeval td;
    struct tm* ptd;
    char day_rasp[11];
    gettimeofday (&td, NULL);
    ptd = localtime (&td.tv_sec);
    const char * dir = "%d%h%Y";
    strftime (day_rasp, sizeof (day_rasp), dir, ptd);
    std::string _day(day_rasp, 9);
    return _day;
}

std::string getCurrentDayTitle()
{
    struct timeval td;
    struct tm* ptd;
    char day_rasp[10];
    gettimeofday (&td, NULL);
    ptd = localtime (&td.tv_sec);
    const char * dir = "%h%a%d";
    strftime (day_rasp, sizeof (day_rasp), dir, ptd);
    std::string _day(day_rasp, 8);
    return _day;
}

std::string getGlobalIntToString(int id)
{
    std::stringstream strm;
    strm << id;
    return strm.str();
}

vector<string> splitString(string input, string delimiter)
{
    vector<string> output;
    char *pch;
    char *str = strdup(input.c_str());
    pch = strtok (str, delimiter.c_str());
    while (pch != NULL)
    {
        output.push_back(pch);
        pch = strtok (NULL,  delimiter.c_str());
    }
    free(str);
    return output;
}

std::string IntToString ( int number )
{
    std::ostringstream oss;
    oss<< number;
    return oss.str();
}

int div_ceil(int numerator, int denominator)
{
    std::div_t res = std::div(numerator, denominator);
    return res.rem ? (res.quot + 1) : res.quot;
}

bool file_exists (const std::string& name) {
    ifstream f(name.c_str());
    if (f.good()) {
        f.close();
        return true;
    } else {
        f.close();
        return false;
    }   
}

//Location
#define pi 3.14159265358979323846
#define earthRadiusKm 6371.0
//  This function converts radians to decimal degrees
double rad2deg(double rad) {
  return (rad * 180 / pi);
}

char * setTimeToRaspBerry(struct tm tmremote, int timezone_adjust)
{
    
    char * text_time;
    std::cout << std::endl;
    //std::cout << " Seconds  :"  << tmremote.tm_sec  << std::endl;
    //std::cout << " Minutes  :"  << tmremote.tm_min  << std::endl;
    //std::cout << " Hours    :"  << tmremote.tm_hour << std::endl;
    //std::cout << " Day      :"  << tmremote.tm_mday << std::endl;
    //std::cout << " Month    :"  << tmremote.tm_mon  << std::endl;
    //std::cout << " Year     :"  << tmremote.tm_year << std::endl;
    //std::cout << std::endl;
    struct tm mytime;
    struct timeval tv;
    time_t epoch_time;
    struct timezone timez;

    int hour = tmremote.tm_hour + timezone_adjust;
    mytime.tm_sec 	= tmremote.tm_sec  ;
    mytime.tm_min 	= tmremote.tm_min  ;
    mytime.tm_hour  = hour;
    mytime.tm_mday  = tmremote.tm_mday ;
    mytime.tm_mon   = tmremote.tm_mon  ;
    mytime.tm_year  = tmremote.tm_year ;
    epoch_time = mktime(&mytime);
    
    cout << "epoch_time : " << epoch_time << endl;
    /* Now set the clock to this time */
    tv.tv_sec = epoch_time;
    tv.tv_usec = 0;
    // Set new system time.
    if (settimeofday(&tv, NULL) != 0)
    {
        cout << "Cannot set system time" << endl;
    }
    // Get current date & time since Epoch.
    if (gettimeofday(&tv, NULL) != 0)
    {
        printf("Cannot get current date & time since Epoch.");
    }
    text_time = ctime(&tv.tv_sec);
    
    return text_time;
}

std::string exec_command(char* cmd)
{
    FILE* pipe = popen(cmd, "r");
    if (!pipe) return "ERROR";
    char buffer[128];
    std::string result = "";
    while(!feof(pipe)) {
    	if(fgets(buffer, 128, pipe) != NULL)
    		result += buffer;
    }
    pclose(pipe);
    return result;
}

void split(const string& s, char c, vector<string>& v)
{
   string::size_type i = 0;
   string::size_type j = s.find(c);
   while (j != string::npos)
   {
      v.push_back(s.substr(i, j-i));
      i = ++j;
      j = s.find(c, j);

      if (j == string::npos)
         v.push_back(s.substr(i, s.length()));
   }
}

std::string escape(const std::string& input) {
    std::ostringstream ss;
    //for (auto iter = input.cbegin(); iter != input.cend(); iter++) {
    //C++98/03:
    for (std::string::const_iterator iter = input.begin(); iter != input.end(); iter++) {
        switch (*iter) {
            case '\\': ss << "\\\\"; break;
            case '"': ss << "\\\""; break;
            case '/': ss << "\\/";  break;
            case '\b': ss << "\\b"; break;
            case '\f': ss << "\\f"; break;
            case '\n': ss << "\\n"; break;
            case '\r': ss << "\\r"; break;
            case '\t': ss << "\\t"; break;
            default: ss << *iter; break;
        }
    }
    return ss.str();
}


cv::Mat getImageWithTextByPath(std::string imagefilepath)
{
    cv::Mat mat = cv::imread(imagefilepath);
    
    vector<string> array_image = getImageByPath(imagefilepath);
            
    std::string name = array_image.at(0);
    std::string imagechanges = array_image.at(1);
    std::string time = array_image.at(2);
    std::string rect = array_image.at(3);

    int matrows = atoi(array_image.at(4).c_str());
    int matcols = atoi(array_image.at(5).c_str());

    std::string camera = array_image.at(6);

    putText(mat, name,          cv::Point(30, 50),  cv::FONT_HERSHEY_PLAIN, 1.0, CV_RGB(0,255,0), 2.0);
    putText(mat, time,          cv::Point(30, 150), cv::FONT_HERSHEY_PLAIN, 1.0, CV_RGB(0,255,0), 2.0);
    putText(mat, imagechanges,  cv::Point(30, 200), cv::FONT_HERSHEY_PLAIN, 1.0, CV_RGB(0,255,0), 2.0);
    putText(mat, camera,        cv::Point(30, 300), cv::FONT_HERSHEY_PLAIN, 1.0, CV_RGB(0,255,0), 2.0);
    
    return mat;
}

cv::Mat drawRectFromCoordinate(std::string coords, cv::Mat mat, cv::Scalar color)
{
    cout << "coords: " << coords << endl;
    std::string coordstring;
    
    if(
       (coords.find("[") != std::string::npos) &&
       (coords.find("]") != std::string::npos) &&
       (coords.find(";") != std::string::npos) )
    {
        coordstring = coords;
    } else
    {
        coordstring = base64_decode(coords);
    }  
         
    vector<cv::Point> coordinates;
    std::stringstream ss(coordstring);
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
            cv::Point p(x, y);
            cout << "x: " << x << " y: " << y << endl; 
            coordinates.push_back(p);
            c=0;x=0;y=0;
        }
        t++;
    }
    
    for (int i=0; i<coordinates.size()-1; i++ )
    {
        cout << i << " : " << coordinates.at(i) << " " << i + 1 <<  " : " << coordinates.at(i+1) << endl;
        cv::line(mat, coordinates.at(i), coordinates.at(i+1), color, 1, CV_AA); 
    }     
    return mat;
}

cv::Mat extractMat(string loadedmat)
{
    std::string oridecoded = base64_decode(loadedmat);

    stringstream decoded;
    decoded << oridecoded;

    // The data we need to deserialize.
    int width_d = 0;
    int height_d = 0;
    int type_d = 0;
    int size_d = 0;

    // Read the width, height, type and size of the buffer
    decoded.read((char*)(&width_d), sizeof(int));
    decoded.read((char*)(&height_d), sizeof(int));
    decoded.read((char*)(&type_d), sizeof(int));
    decoded.read((char*)(&size_d), sizeof(int));

    // Allocate a buffer for the pixels
    char* data_d = new char[size_d];
    // Read the pixels from the stringstream
    decoded.read(data_d, size_d);

    cv::Mat extracted = cv::Mat(height_d, width_d, type_d, data_d).clone();

    // Delete our buffer
    delete[]data_d;

    return extracted;
}

void loadJobFromFile()
{
    
    //CREATE JOBS

     vector<vector<string> > cameras = getCamerasFromDB();

     int size = cameras.size();
     
     for (int i=0; i<size; i++)
     {
         int cameranumber = atoi(cameras.at(i).at(0).c_str());
         
         int db_camera = atoi(cameras.at(i).at(3).c_str());     
         
         if (db_camera==1) //REMOVE THIS FOR MULTI CAMERA
         {                         

             vector<std::string> camerainfo = getTrackPostByTypeAndIdLocal("camera", db_camera);
             if (camerainfo.size()>0)
             {
                 struct timeval tr;
                 struct tm* ptmr;
                 char time_rasp[40];
                 gettimeofday (&tr, NULL);
                 ptmr = localtime (&tr.tv_sec);
                 strftime (time_rasp, sizeof (time_rasp), "%Y-%m-%d %H:%M:%S %z", ptmr);          

                 std::string post_parent = camerainfo.at(9).c_str();           

                 std::stringstream dumpfile;
                 dumpfile << basepath << "data/client/client.dat";

                 std::string strproto = get_file_contents(dumpfile.str());
                 std::string strdecoded = base64_decode(strproto);

                 //cout << "RECEIVE" << endl;

                 GOOGLE_PROTOBUF_VERIFY_VERSION;
                 motion::Message m;
                 m.ParseFromArray(strdecoded.c_str(), strdecoded.size());

                 motion::Message::MotionUser * muser = m.mutable_motionuser(0);
                 
                 google::protobuf::uint32 db_user_id = getUserIdByWpUserName(muser->username());

                 std::stringstream clientid;
                 clientid << muser->wpclientid() << endl;

                 std::stringstream timerasp;
                 timerasp << time_rasp << endl;                    

                 cout << "****************************************" << endl;
                 cout << "SERVER_INFO clientid                   :" << clientid.str()           << endl;
                 cout << "SERVER_INFO timerasp                   :" << timerasp.str()           << endl;
                 cout << "SERVER_INFO wpmodified                 :" << muser->wpmodified()      << endl;
                 cout << "SERVER_INFO wpslug                     :" << muser->wpslug()          << endl;
                 cout << "SERVER_INFO wptype                     :" << muser->wptype()          << endl;
                 cout << "SERVER_INFO wplink                     :" << muser->wplink()          << endl;
                 cout << "SERVER_INFO wpapilink                  :" << muser->wpapilink()       << endl;
                 cout << "SERVER_INFO wpfeaturedimage            :" << muser->wpfeaturedimage() << endl;            
                 cout << "SERVER_INFO postparent                 :" << post_parent              << endl;            
                 cout << "****************************************" << endl;

                int db_track = insertIntoPosts(clientid.str(), timerasp.str(), muser->wpmodified(), muser->wpslug(), muser->wptype(), muser->wplink(), muser->wpapilink(), muser->wpfeaturedimage(), post_parent, db_user_id);                           

                 //CREATE JOB STATIC
                 createJobManually(db_camera, "test", 2, "BASIC");
             } 
         }
     }    
    
}




int createJobManually(
    int db_camera, 
    std::string codename,
    google::protobuf::uint32 delay,        
    std::string recname)
{
    std::string coords;
    createJobManually(db_camera, codename, delay, recname, coords);
}


int createJobManually(
    int db_camera, 
    std::string codename,
    google::protobuf::uint32 delay,
    std::string recname,
    std::string coords)
{
    
    motion::Message m;    
    
    std::vector<string> camera_vector = getCameraByCameraDbId(db_camera);   
    
    int _id = atoi(camera_vector.at(0).c_str());
    std::string name = camera_vector.at(1); 
    int matrows     = atoi(camera_vector.at(2).c_str());
    int matcols     = atoi(camera_vector.at(3).c_str());
    int matheight   = atoi(camera_vector.at(4).c_str());
    int matwidth    = atoi(camera_vector.at(5).c_str());
    int _idmat      = atoi(camera_vector.at(6).c_str());
    int cameranumber = atoi(camera_vector.at(7).c_str());
    
    m.set_activecam(cameranumber);

    motion::Message::MotionCamera * mcamera = m.add_motioncamera();  
    
    mcamera->set_cameraname(name);
    
    mcamera->set_db_idcamera(_id);
        
    mcamera->set_cameraname(name);

    mcamera->set_cameranumber(cameranumber);
    mcamera->set_fromdatabase(false); 
    
    mcamera = takePictureToProto(cameranumber, mcamera);
    
    motion::Message::MotionRec * mrec = mcamera->add_motionrec();
    
    mrec->set_activerec(true);
    
    std::string _month = getCurrentMonthLabel();
    m.set_currmonth(_month);

    motion::Message::MotionMonth * pmonth = mcamera->add_motionmonth();
    pmonth->set_monthlabel(_month);

    std::string _day = getCurrentDayLabel();
    m.set_currday(_day);  
    
    mrec->set_codename(codename);

    mrec->set_delay(delay);

    mrec->set_storevideo(true);

    mrec->set_storeimage(true);   

    mrec->set_recname(recname);
    
    if (!coords.empty())
    {
        mrec->set_hasregion(true);    

        std::stringstream coords;
        coords      << 
        "[0, 0;"    <<
        "480, 0;"   <<  
        "640, 640;" <<
        "0, 640;"   <<
        "0, 0]";

        mrec->set_coordinates(coords.str());
    } else 
    {
        mrec->set_hasregion(false);
    }
    
    mrec->set_db_idmat(_idmat);
    mrec->set_matrows(matrows);   
    mrec->set_matcols(matcols);   
    mrec->set_matheight(matwidth);
    mrec->set_matwidth(matheight);   
    
    mrec->set_runatstartup(true);
    
    mrec->set_hascron(true);    
    mrec->set_timestart("00:00:00");
    mrec->set_timeend("24:00:00");       
    
    m = saveRecognition(m);
   
    postRecognition(m);
    
    return _id;
    
}



// String Generator
/*char genRandom()
{
    static const char alphanum[] =
    "0123456789"
    "!@#$%^&*"
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz";

    int stringLength = sizeof(alphanum) - 1;

    return alphanum[rand() % stringLength];
}

std::string random_string(size_t length)
{
    static const std::string alphanums =
        "0123456789"
        "abcdefghijklmnopqrstuvwxyz"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

    static std::mt19937 rg{std::random_device{}()};
    static std::uniform_int_distribution<> pick(0, alphanums.size() - 1);

    std::string s;

    s.reserve(length);

    while(length--)
        s += alphanums[pick(rg)];

    return s;
}*/