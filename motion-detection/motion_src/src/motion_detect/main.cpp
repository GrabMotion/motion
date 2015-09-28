/* 
 * File:   main.cpp
 * Author: jose
 *
 * Created on April 19, 2015, 11:23 PM
 */

#include <cstring>
#include <iostream>
#include <string>
#include <sstream>
#include <stdio.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <signal.h>

#include <cstdlib>           
#include <unistd.h>    
#include <string.h> /* for strncpy */
#include <sys/ioctl.h>
#include <net/if.h>
#include <ctime>
#include <sys/time.h>
#include <errno.h>
#include <vector>
#include <functional>
#include <fstream>
#include <dirent.h>
#include <sys/stat.h>

#include "opencv2/opencv.hpp"
#include "opencv2/highgui/highgui.hpp"

#include "practical/PracticalSocket.h" 
#include "recognition/detection.h"

#include "protobuffer/motion.pb.h"
//#include "protobuffer/server.pb.h"

#include "socket/streamlistener.h"
#include "socket/netcvc.h"
#include "database/database.h"

#include "b64/base64.h"

#include "http/httppost.c"

#include <unistd.h>
#include <google/protobuf/message.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl_lite.h>

#include <stdlib.h>
#include <stdio.h>
#include <netdb.h>

#include <usb.h>

#include <curl/curl.h>

using namespace google::protobuf::io;

using namespace std;
using namespace cv;

//Folder
std::string basepath;
std::string sourcepath;

void * ThreadMain(void *clntSock);
motion::Message::ActionType HandleTCPClient(TCPSocket *sock);
void RunUICommand(int result, string from_ip);
void * watch_amount (void * t);
std::string getXMLFilePathAndName(int cam, std::string recname, std::string currday, std::string curmonth, std::string name);
void directoryExistsOrCreate(const char* pzPath);
void startMainRecognition();

//xml
std::string XML_FILE = "<import>session";

// Threading
pthread_t thread_broadcast, thread_echo, thread_socket,
thread_message, thread_recognition, thread_wait_echo,
thread_observer, thread_send_echo;

//Threads
int runt, runb, runs, runr, runl, runm, runw, runss, runo, ruse;

/// TCP Streaming
int         clientSock;
char*     	server_ip;
int       	server_port;
int       	server_camera;

//Threads
pthread_cond_t echo_response;
pthread_mutex_t protoMutex, fileMutex, databaseMutex;
bool echo_received;
motion::Message::ActionType resutl_echo;
std::string result_message;
std::string from_ip;

//TCP
std::string local_ip;
string NETWORK_IP;
vector<std::string> msg_split_vector;
motion::Message::ActionType value_response;
int count_sent__split=0;
int count_vector_size=0;
std::string msg;
int div_ceil(int numerator, int denominator);
std::string IntToString ( int number );
std::string fixedLength(int value, int digits);
int inttype, protofile;

//Send
void * sendEcho(motion::Message m);

//Protobuffer
motion::Message PROTO;
motion::Message R_PROTO;
motion::Message T_PROTO;
motion::Message takePictureToProto(motion::Message);
std::string starttime;

//Recognition
CvCapture * camera;
VideoCapture * videocam;
bool stop_capture;
cv::Mat picture;
//bool stop_recognizing;
int number_of_changes;
int resutl_watch_detected;
std::string startrecognitiontime;
string DIR_FORMAT           = "%d%h%Y"; // 1Jan1970

//Database ids
/*int db_hardware_id;
int db_camera_id;
int db_rel_hardware_camera_id;
int db_status_id;
int db_month_id;
int db_day_id;
int db_rel_camera_month_id;
int db_coordnates_id;
int db_recognition_setup_id;
int db_mats_id;
int db_interval_id;
int db_image_id;
int db_crop_id;*/

//UDP
int udpsend(motion::Message m);

#define RCVBUFSIZE 1024

//const unsigned int RCVBUFSIZE = 100000; //4096; //32;     // Size of receive buffer
const int MAXRCVSTRING = 4096;          // Longest string to receive

//Operations
vector<string> splitString(string input, string delimiter);
std::string getGlobalIntToString(int id);
std::string getCurrentDayLabel();
std::string getCurrentMonthLabel();

int getGlobalStringToInt(std::string id);
char * setMessageValueBody(int value, std::string body);
bool checkFile(const std::string &file);
std::string exec_command(char* cmd);

inline bool file_exists (const std::string& name) {
    ifstream f(name.c_str());
    if (f.good()) {
        f.close();
        return true;
    } else {
        f.close();
        return false;
    }   
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

bool to_bool(std::string const& s)
{
    return s != "0";
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

void set_file_permission(std::string file, std::string permission)
{
    std::stringstream perm;
    perm << "chmod " <<  permission << " " << file;
    std::string pstring = perm.str();
    system(pstring.c_str());
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


inline bool file_exist (const std::string& name) {
    ifstream f(name.c_str());
    if (f.good()) {
        f.close();
        return true;
    } else {
        f.close();
        return false;
    }
}

struct arg_struct
{
    motion::Message message;
};
void * startRecognition(void * arg);
void * startObserver(void * arg);
motion::Message runCommand(motion::Message m);
int getGlobalStringToInt(std::string id)
{
    return atoi( id.c_str() );
}
std::string getGlobalIntToString(int id)
{
    std::stringstream strm;
    strm << id;
    return strm.str();
}
std::string getIpAddress();

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


#define BUFLEN 2048
#define MSGS 5	/* number of messages to send */
#define SERVICE_PORT	21234	/* hard-coded port number */

int udpsend(motion::Message m)
{
    
    struct sockaddr_in myaddr, remaddr;
    int fd, i, slen=sizeof(remaddr);
    char *server = "192.168.1.35";	/* change this to use a different server */
    //char buf[BUFLEN];
    
    printf("Sending packet %d to %s port %d size %d\n", i, server, SERVICE_PORT, m.ByteSize()); //strlen(buf));
    
    
    std::string buf;
    m.SerializeToString(&buf);
    
    /* create a socket */
    
    if ((fd=socket(AF_INET, SOCK_DGRAM, 0))==-1)
        printf("socket created\n");
    
    /* bind it to all local addresses and pick any port number */
    
    memset((char *)&myaddr, 0, sizeof(myaddr));
    myaddr.sin_family = AF_INET;
    myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    myaddr.sin_port = htons(0);
    
    if (bind(fd, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0) {
        perror("bind failed");
        return 0;
    }
    
    /* now define remaddr, the address to whom we want to send messages */
    /* For convenience, the host address is expressed as a numeric IP address */
    /* that we will convert to a binary format via inet_aton */
    
    memset((char *) &remaddr, 0, sizeof(remaddr));
    remaddr.sin_family = AF_INET;
    remaddr.sin_port = htons(SERVICE_PORT);
    if (inet_aton(server, &remaddr.sin_addr)==0) {
        fprintf(stderr, "inet_aton() failed\n");
        exit(1);
    }
    
    /* now let's send the messages */
    
    //for (i=0; i < MSGS; i++) {
    
    printf("Sending packet %d to %s port %d size %d\n", i, server, SERVICE_PORT, m.ByteSize()); //strlen(buf));
    //sprintf(buf, "This is packet %d", i);
    //if (sendto(fd, buf, strlen(buf), 0, (struct sockaddr *)&remaddr, slen)==-1)
    
    if(sendto(fd, buf.data(), strlen(buf.c_str()), 0, (struct sockaddr *)&remaddr, sizeof(remaddr)))
        perror("sendto");
    
    //}
    close(fd);
    return 0;
}

motion::Message serializeMediaToProto(motion::Message m, Mat mat)
{
    
    m.set_serverip(PROTO.serverip());
    
    int active = m.activecam();
    motion::Message::MotionCamera * mcam = m.add_motioncamera();
    mcam->set_activemat(active);
    
    struct timeval tr;
    struct tm* ptmr;
    char time_rasp[40];
    gettimeofday (&tr, NULL);
    ptmr = localtime (&tr.tv_sec);
    strftime (time_rasp, sizeof (time_rasp), "%Y-%m-%d %H:%M:%S %z", ptmr);
            
    m.set_time(time_rasp);
    
    // We will need to also serialize the width, height, type and size of the matrix
    int width_s     = mat.cols;
    int height_s    = mat.rows;
    int type_s      = mat.type();
    int size_s      = mat.total() * mat.elemSize();
    
    cout << "width_s: " << width_s << endl;
    cout << "height_s: " << height_s << endl;
    cout << "type_s: " << type_s << endl;
    cout << "size_s: " << size_s << endl;
    
    // Initialize a stringstream and write the data
    int size_init = m.ByteSize();
    
    cout << "m.ByteSize: " << m.ByteSize() << endl;
    
    // Write the whole image data
    std::stringstream ss;
    ss.write((char*)    (&width_s),     sizeof(int));
    ss.write((char*)    (&height_s),    sizeof(int));
    ss.write((char*)    (&type_s),      sizeof(int));
    ss.write((char*)    (&size_s),      sizeof(int));
    ss.write((char*)     mat.data,      size_s);
    
    std::string ssstring = ss.str();
    
    std::string oriencoded = base64_encode(reinterpret_cast<const unsigned char*>(ssstring.c_str()), ssstring.length());
    
    cout << " oriencoded size : " << oriencoded.size() << endl;
    
    //Store into proto
    m.set_data(oriencoded.c_str());

    return m;
}

motion::Message takePictureToProto(motion::Message m)
{
    
    cout << "CAPTURING !!!!!!!!!!!!!" << endl;
    
    int camera = m.activecam();
    
    CvCapture* capture = cvCreateCameraCapture(camera);
    if (capture == NULL)
    {
        std::cout << "No cam found." << std::endl;

    }
    
    int w = 640; //1280; //320;
    int h = 480; //720;  //240;
    
    cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_WIDTH, w); //max. logitech 1280
    cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_HEIGHT, h); //max. logitech 720
    
    //IplImage* img=0;
    //img = cvQueryFrame( capture );
    //cvSaveImage("IplImage.JPG",img);
    
    Mat mat; //(h, w, CV_8U); //CV_8U); // CV_8UC3);
    mat = cvQueryFrame(capture);
    //cvtColor(mat, mat, CV_RGB2GRAY);
    
    //imwrite("MAT.jpg", mat);
    
    //Shared mat
    picture = mat;
    
    cout << "+++++++++++CREATING PROTO++++++++++++++" << endl;
    
    GOOGLE_PROTOBUF_VERIFY_VERSION;
    
    m.set_type(motion::Message::TAKE_PICTURE);
    m.set_serverip(PROTO.serverip());
    m.set_time(getTimeRasp());
    
    // We will need to also serialize the width, height, type and size of the matrix
    int width_s     = mat.cols;
    int height_s    = mat.rows;
    int type_s      = mat.type();
    int size_s      = mat.total() * mat.elemSize();
    
    cout << "width_s: " << width_s << endl;
    cout << "height_s: " << height_s << endl;
    cout << "type_s: " << type_s << endl;
    cout << "size_s: " << size_s << endl;
    
    // Write the whole image data
    std::stringstream ss;
    ss.write((char*)    (&width_s),     sizeof(int));
    ss.write((char*)    (&height_s),    sizeof(int));
    ss.write((char*)    (&type_s),      sizeof(int));
    ss.write((char*)    (&size_s),      sizeof(int));
    ss.write((char*)     mat.data,      size_s);
    
    std::string ssstring = ss.str();
    
    std::string oriencoded = base64_encode(reinterpret_cast<const unsigned char*>(ssstring.c_str()), ssstring.length());
    
    //Store into proto
    m.set_data(oriencoded.c_str());
      
    std::time_t t = std::time(0);
    stringstream tst;
    tst << t;
    int activemat = atoi(tst.str().c_str());

    cout << "activemat::::: " << activemat << endl; 
    
    motion::Message::MotionCamera * mcam = m.add_motioncamera();
    mcam->set_activemat(activemat);
    mcam->set_matrows(height_s);
    mcam->set_matcols(width_s);
    mcam->set_matheight(h);
    mcam->set_matwidth(w);
    
    // Initialize a stringstream and write the data
    int size_init = m.ByteSize();
    
    cout << "mcam->db_idcamera::" << mcam->db_idcamera() << endl;
    
    //Write base64 to file for checking.
    std::string basefile = "data/mat/" + IntToString(activemat);
    
    std::ofstream out;
    out.open (basefile.c_str());
    out << m.data() << "\n";
    out.close();
    
    mcam->set_matrows(height_s);
    mcam->set_matcols(width_s);
    mcam->set_matheight(h);
    mcam->set_matwidth(w);
    
    stringstream insert_mats_query;
    insert_mats_query <<
    "INSERT INTO mat (matcols, matrows, matwidth, matheight, matfile, data) " <<
    "SELECT " << width_s << ", " << height_s << ", " << w << ", " << h << ", " << activemat << ",'" << m.data() << "'"
    " WHERE NOT EXISTS (SELECT * FROM mat WHERE matfile = " << activemat << ");";
    db_execute(insert_mats_query.str().c_str());
    std::string last_mats_query = "SELECT MAX(_id) FROM mat";
    vector<vector<string> > mats_array = db_select(last_mats_query.c_str(), 1);
    int db_mats_id = atoi(mats_array.at(0).at(0).c_str());
    cout << "db_mats_id: " << db_mats_id << endl;
    
    mcam->set_db_idmat(db_mats_id);

    stringstream insert_rel_cameras_mats_query;
    insert_rel_cameras_mats_query <<
    "INSERT INTO rel_camera_mat (_id_camera, _id_mat) " <<
    "SELECT " << mcam->db_idcamera() << "," << db_mats_id <<
    " WHERE NOT EXISTS (SELECT * FROM rel_camera_mat WHERE _id_camera = "
    << mcam->db_idcamera() << " AND _id_mat = " << db_mats_id << ");";
    db_execute(insert_rel_cameras_mats_query.str().c_str());

    cout << "ByteSize: " << size_init <<  endl;
    
    cvReleaseCapture(&capture);
    
    return m;
}

void * sendProto (void * arg)
{
    
    struct arg_struct *args = (struct arg_struct *) arg;
    motion::Message me      = args->message;
    string servAddress      = T_PROTO.serverip();
    
    google::protobuf::uint32 pport = motion::Message::TCP_ECHO_PORT;
    google::protobuf::uint32 buffersize = motion::Message::SOCKET_BUFFER_NANO_SIZE + 40;
    
    cout << "serverIp: " << servAddress << endl;
    cout << "serverPort: " << pport << endl;
    
    int echoServPort = pport;
    char echoBuffer[buffersize];

    //string data;
    me.set_time(getTimeRasp());
   
    //Initialize objects to serialize.
    int size = me.ByteSize();
    char datasend[size];
    string datastr;
    me.SerializeToArray(&datasend, size);
    google::protobuf::ShutdownProtobufLibrary();
    
    std:string encoded_proto = base64_encode(reinterpret_cast<const unsigned char*>(datasend),sizeof(datasend));
    
    cout << "encoded_proto: " << encoded_proto << endl;
    
    char * message = new char[encoded_proto.size() + 1];
    std::copy(encoded_proto.begin(), encoded_proto.end(), message);
    message[encoded_proto.size()] = '\0'; // don't forget the terminating 0
    
    try
    {
        TCPSocket sock(servAddress, echoServPort);
        sock.send(message, sizeof(message));
        
    } catch(SocketException &e)
    {
        cerr << e.what() << endl;
    }
}

void * sendEcho(motion::Message m)
{
    struct arg_struct arguments;
    arguments.message = m;
    
    ruse = pthread_create(&thread_send_echo, NULL, sendProto, (void*) &arguments);
    
    if ( ruse  != 0)
    {
        cerr << "Unable to create thread" << endl;
        cout << "startRecognition pthread_create failed." << endl;
    }
}

void * startObserver(void * arg)
{
    cout << "startObserver" << endl;
    while (true)
    {
        int sizec = R_PROTO.motioncamera_size();
        for (int i = 0; i < sizec; i++)
        {
            try
            {
                motion::Message::MotionCamera * mcamera = R_PROTO.mutable_motioncamera(i);
                if (mcamera->recognizing())
                {
                    std::cout << "number_of_changes = " << resutl_watch_detected << std::endl;
                    if (!mcamera->recognizing_flag())
                    {
                        //updateCameraDB(1, mcamera->cameranumber());
                        pthread_mutex_lock(&protoMutex);
                        mcamera->set_recognizing_flag(true);
                        pthread_mutex_unlock(&protoMutex);
                    }
                } else
                {
                    if (mcamera->recognizing_flag())
                    {
                        //updateCameraDB(0, mcamera->cameranumber());
                        pthread_mutex_lock(&protoMutex);
                        mcamera->set_recognizing_flag(false);
                        pthread_mutex_unlock(&protoMutex);
                    }
                }
            }
            catch (std::bad_alloc& ba)
            {
              std::cerr << "bad_alloc caught startObserver: " << ba.what() << '\n';
            } 
            sleep(100);
        }
        sleep(100);
    }
    cout << "stopObserver" << endl;
    
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

motion::Message getLocalPtoro()
{
    motion::Message mlocal;
    std::string protofile = "data/data/localproto.txt";
    
    if (file_exist(protofile))
    {
        mlocal.Clear();
        std::string backfile = protofile;
        pthread_mutex_lock(&fileMutex);
        string loaded = get_file_contents(backfile);
        pthread_mutex_unlock(&fileMutex);
        std::string oridecoded = base64_decode(loaded);
        mlocal.ParseFromArray(oridecoded.c_str(), oridecoded.size());
        mlocal.set_time(getTimeRasp());
    }
    return mlocal;

}

motion::Message::MotionCamera * getMonthByCameraIdMonthAndDate(
                motion::Message::MotionCamera * mcam,   
                std::string camid, 
                std::string month, 
                std::string day,
                std::string rec)
{
     
    stringstream sql_month;
    sql_month                   <<
    "SELECT "                   <<
    "D._id AS dayid, "          <<  //0
    "D.label, "                 <<  //1    
    "I.instancestart, "         <<  //2  
    "I.instanceend, "           <<  //3
    "IM._id, "                  <<  //4
    "IM.imagechanges, "         <<  //5
    "IM.name AS imagename, "    <<  //6
    "IM.path AS imagepath, "    <<  //7
    "C.rect, "                  <<  //8
    "I.number, "                <<  //9
    "I._id AS instanceid, "     <<  //10
    "V.name, "                  <<  //11
    "V.path "                   <<  //12
    "FROM rel_day_instance_recognition_setup AS RDIR " <<
    "JOIN day AS D ON RDIR._id_day = D._id " << 
    "JOIN recognition_setup AS RS ON RDIR._id_recognition_setup = RS._id " << 
    "JOIN instance AS I ON RDIR._id_instance = I._id " << 
    "JOIN rel_instance_image AS RII ON I._id = RII._id_instance " << 
    "JOIN image AS IM ON RII._id_image = IM._id " << 
    "JOIN crop AS C ON C._id_image_father = IM._id " << 
    "JOIN video AS V ON I._id_video = V._id " << 
    "WHERE RS._id_camera = " << camid << 
    " AND RDIR._id_day IN (SELECT _id from day WHERE label = '" << day << "') " << 
    "AND RDIR._id_recognition_setup IN (SELECT _id from recognition_setup WHERE name = '" << rec << "');";

    std::string sql_monthstr = sql_month.str();
    cout << "sql_monthstr: " << sql_monthstr << endl;
    pthread_mutex_lock(&databaseMutex);
    vector<vector<string> > rcm_array = db_select(sql_monthstr.c_str(), 13);
    pthread_mutex_unlock(&databaseMutex);
    
    if (rcm_array.size()>0)
    {
        motion::Message::MotionMonth * mmonth = mcam->add_motionmonth();
        mmonth->set_monthlabel(month);
        
        motion::Message::MotionDay * mday = mmonth->add_motionday();
        mday->set_daylabel(day);
        
        google::protobuf::int32 dayid = atoi(rcm_array.at(0).at(0).c_str());
        mday->set_db_dayid(dayid);
    
        int instancecounter = 0;
        motion::Message::Instance * minstance;

        for (int i=0; i< rcm_array.size(); i++)
        {
            vector<string> rowi = rcm_array.at(i);

            google::protobuf::int32 instanceid = atoi(rowi.at(10).c_str());

            if (instanceid != instancecounter)
            {    
                minstance = mday->add_instance();
                minstance->set_instancestart(rowi.at(2));
                minstance->set_instanceend(rowi.at(3));

                cout << "instancecounter:: " << instancecounter << " instanceid:: " << instanceid << endl;

                std::string last = rowi.at(10).c_str();
                google::protobuf::int32 idinstance = atoi(last.c_str());
                minstance->set_idinstance(idinstance);
                instancecounter = instanceid;

                motion::Message::Video * mvideo = minstance->mutable_video();
                std::string vname = rowi.at(11);
                mvideo->set_name(vname);        
                std::string vpath = rowi.at(12);
                mvideo->set_path(vpath);
            }

            motion::Message::Image * mimage = minstance->add_image();
            int imgid = atoi(rowi.at(4).c_str());
            mimage->set_imagechanges(atoi(rowi.at(5).c_str()));
            mimage->set_name(rowi.at(6));
            mimage->set_path(rowi.at(7));   

            motion::Message::Crop * mcrop = minstance->add_crop();
            mcrop->set_db_imagefatherid(imgid);
            mcrop->set_rect(rowi.at(8));
            
        }
    }
    return mcam;
}

std::string getXMLFilePathAndName(int cam, std::string recname, std::string currday, std::string name)
{
    stringstream DIR;
    DIR << sourcepath << "motion_web/pics/" << "camera" << cam << "/" << recname << "/" << currday << "/";
    std::string XML_FILE  =  "xml/" + name;
    std::string xml_path = DIR.str() + XML_FILE + ".xml";
    return xml_path;
}

motion::Message getRefreshProto(motion::Message m)
{
    
    struct timeval tr;
    struct tm* ptmr;
    char time_rasp[40];
    gettimeofday (&tr, NULL);
    ptmr = localtime (&tr.tv_sec);
    strftime (time_rasp, sizeof (time_rasp), "%Y-%m-%d %H:%M:%S %z", ptmr);
    cout <<  ":::start time:::: " << time_rasp << endl;

    m.set_time(time_rasp);    
    
    vector<int> cams;
    stringstream sql_cameras;
    sql_cameras <<
    "SELECT C._id, C.number, C.name, C.active FROM cameras C;";
    pthread_mutex_lock(&databaseMutex);
    vector<vector<string> > cameras_array = db_select(sql_cameras.str().c_str(), 4);
    pthread_mutex_unlock(&databaseMutex);
     
    for (int i=0; i<cameras_array.size(); i++ )
    {
        vector<string> rowc = cameras_array.at(i);
        motion::Message::MotionCamera * mcam = m.add_motioncamera();
        
        stringstream sql_rec_setup;
        sql_rec_setup   <<
        "SELECT RCS._id, RCS.name FROM recognition_setup AS RCS;";
        cout << "sql_rec_setup: " << sql_rec_setup.str() << endl;
        pthread_mutex_lock(&databaseMutex);
        vector<vector<string> > rec_setup_array = db_select(sql_rec_setup.str().c_str(), 2);
        pthread_mutex_unlock(&databaseMutex);
        
        std::string recname;
        if (rec_setup_array.size()>0)
        {
            for (int q=0; q<rec_setup_array.size(); q++ )
            {
                vector<string> rows = rec_setup_array.at(q);
                motion::Message::MotionRecognition * mrec = mcam->add_motionrec();
                google::protobuf::int32 recid = atoi(rows.at(0).c_str());
                mrec->set_db_idrec(recid);
                recname = rows.at(1);
                mrec->set_name(recname);
            }
        }
        
        if (m.has_recname())
        {
           recname = m.recname();
        }
        
        google::protobuf::int32 camid = atoi(rowc.at(0).c_str());
        mcam->set_cameraid(camid);
        google::protobuf::int32 camnum = atoi(rowc.at(1).c_str());
        mcam->set_cameranumber(camnum);
        std::string cameraname = rowc.at(2);
        mcam->set_cameraname(cameraname);
        
        bool active = to_bool(rowc.at(3));
        if (active)
        {
            m.set_activecam(camnum);
        }
       
        stringstream sql_last_instance;
        sql_last_instance   <<
        "SELECT coalesce( MAX(I._id), 0) FROM instance AS I "                                   <<
        "JOIN rel_day_instance_recognition_setup AS RDI ON I._id = RDI._id_instance "           <<
        "JOIN rel_month_day AS RMD ON RDI._id_day = RMD._id_day "           <<
        "JOIN day as D ON RMD._id_day = D._id "                             <<
        "JOIN month AS M ON RMD._id_month = M._id "                         <<
        "JOIN rel_camera_month AS RCM ON RMD._id_month = RMD._id_month "    <<
        "JOIN cameras AS C ON RCM._id_camera = C._id "                      <<
        "WHERE C.number = " << camnum << " AND D.label = '" << m.currday()  << "';";
        std::string sqllaststd = sql_last_instance.str();
        cout << "sqllaststd: " << sqllaststd << endl;
        pthread_mutex_lock(&databaseMutex);
        vector<vector<string> > lastinstance_array = db_select(sqllaststd.c_str(), 1);
        pthread_mutex_unlock(&databaseMutex);
        
        std::string last = lastinstance_array.at(0).at(0).c_str();
        int ln = atoi(last.c_str());
        if (ln>0)
            mcam->set_lastinstance(last);
        
        string camera = rowc.at(0);
        
        stringstream sql_cam_req_setup;
        sql_cam_req_setup               <<
        "SELECT "                       <<
        "RS.storeimage, "               << // 0
        "RS.storevideo, "               << // 1
        "RS.codename, "                 << // 2
        "RS.has_region, "               << // 3
        "CO.coordinates, "              << // 4
        "RS.delay, "                    << // 5
        "MA.matfile, "                  << // 6
        "RCRS.start_rec_time, "         << // 7
        "RCRS._id_recognition_setup, "  << // 8
        "RS.runatstartup, "             << // 9 
        "MA.matcols, "                  << // 10
        "MA.matrows, "                  << // 11
        "MA.matwidth, "                 << // 12
        "MA.matheight, "                << // 13
        "RS.name, "                     << // 14
        "RS.xmlfilepath, "              << // 15
        "IFNULL(RS.recognizing,0), "    << // 16
        "RS.since "                     << // 17
        "FROM rel_day_instance_recognition_setup AS RDIR "                                                          <<
        "JOIN rel_camera_recognition_setup AS RCRS ON RDIR._id_recognition_setup = RCRS._id_recognition_setup "     <<
        "JOIN recognition_setup AS RS ON RDIR._id_recognition_setup = RS._id "                                      <<
        "JOIN coordinates AS CO ON RS._id_coordinates = CO._id "                                                    <<
        "JOIN cameras AS CAM ON RS._id_camera = CAM._id "                                                           <<
        "JOIN mat AS MA ON RS._id_mat = MA._id "                                                                    <<
        "WHERE RS._id_camera = " <<  camera                                                                         << 
        " AND RDIR._id_recognition_setup IN (SELECT _id from recognition_setup WHERE name = '" << recname << "') "   <<
        "GROUP BY RDIR._id_recognition_setup;";
        
        std::string sqlcamstr =  sql_cam_req_setup.str();
        cout << "sqlcamstr: " << sqlcamstr << endl;
       
        pthread_mutex_lock(&databaseMutex);
        vector<vector<string> > crs_array = db_select(sqlcamstr.c_str(), 18);
        pthread_mutex_unlock(&databaseMutex);
        
        int size = crs_array.size();
        
        if (size>0)
        {
            
            for (int i=0; i<crs_array.size(); i++ )
            {
                vector<string> rows = crs_array.at(i);    

                bool hasrecjob = true;
                mcam->set_hasrecjob(hasrecjob);
                mcam->set_db_idcamera(camid);
                mcam->set_storeimage(to_bool(rows.at(0)));
                mcam->set_storevideo(to_bool(rows.at(1)));
                mcam->set_codename(rows.at(2));
                mcam->set_hasregion(to_bool(rows.at(3)));
                mcam->set_coordinates(rows.at(4));
                google::protobuf::int32 delay = atoi(rows.at(5).c_str());
                mcam->set_delay(delay); 
                google::protobuf::int32 camamat = atoi(rows.at(6).c_str());
                mcam->set_activemat(camamat);
                mcam->set_startrectime(rows.at(7));
                google::protobuf::int32 _id_recognition_setup = atoi(rows.at(8).c_str());
                mcam->set_db_recognitionsetupid(_id_recognition_setup);
                mcam->set_runatstartup(to_bool(rows.at(9)));
                google::protobuf::int32 matcols = atoi(rows.at(10).c_str());
                mcam->set_matcols(matcols);
                google::protobuf::int32 matrows = atoi(rows.at(11).c_str());
                mcam->set_matrows(matrows);
                google::protobuf::int32 matwidth = atoi(rows.at(12).c_str());
                mcam->set_matwidth(matwidth);
                google::protobuf::int32 matheight = atoi(rows.at(13).c_str());
                mcam->set_matheight(matheight);
                mcam->set_recname(rows.at(14));
                std::string xmlpath = rows.at(15).c_str();
                mcam->set_xmlfilepath(xmlpath);
                google::protobuf::int32 rec = atoi(rows.at(16).c_str());
                mcam->set_recognizing(rec);
                mcam->set_camerasince(rows.at(17));
                cout << "Month: "   << m.currmonth() << endl;
                cout << "Day: "     << m.currday()  << endl;
                mcam = getMonthByCameraIdMonthAndDate(mcam, rows.at(0), m.currmonth(), m.currday(), recname);
                
            }
        }
        else 
        {
            stringstream sql_month;
            sql_month       <<
            "SELECT "               <<
            "M._id, "               <<
            "M.label "              <<
            "FROM month AS M "      <<
            "JOIN rel_camera_month AS RCM ON M._id = RCM._id " <<
            "WHERE RCM._id_camera = " << camera;
            
            std::string sqlmonth =  sql_month.str();
            cout << "sqlmonth: " << sqlmonth << endl;

            pthread_mutex_lock(&databaseMutex);
            vector<vector<string> > month_array = db_select(sqlmonth.c_str(), 2);
            pthread_mutex_unlock(&databaseMutex);

            int sizem = month_array.size();

            if (sizem>0)
            {
                
                for (int i=0; i<month_array.size(); i++ )
                {
                
                    vector<string> rowm = month_array.at(i);
                    motion::Message::MotionMonth * mmonth = mcam->add_motionmonth();
                    std::string month = rowm.at(1);
                    mmonth->set_monthlabel(month);

                    stringstream sql_day;
                    sql_day             <<
                    "SELECT "           <<
                    "D._id, "           << //0 
                    "D.label "          << //1
                    "FROM day AS D "    <<
                    "JOIN rel_month_day AS RDM ON RDM._id_day = D._id "     <<
                    "JOIN month AS M ON RDM._id_month = M._id "             <<
                    "JOIN rel_camera_month AS RCM ON M._id = RCM._id "      <<
                    "WHERE RCM._id_camera = " << camera << 
                    " AND M.label = '" << month <<  "'";        

                    std::string sqlday =  sql_day.str();
                    cout << "sqlday: " << sqlday << endl;

                    pthread_mutex_lock(&databaseMutex);
                    vector<vector<string> > day_array = db_select(sqlday.c_str(), 2);
                    pthread_mutex_unlock(&databaseMutex);

                    int sized = day_array.size();

                    if (sized>0)
                    {
                        
                        for (int j=0; j<day_array.size(); j++ )
                        {
                            
                            vector<string> rowd = day_array.at(j);
                            motion::Message::MotionDay * mday = mmonth->add_motionday();
                            std::string day = rowd.at(1);
                            mday->set_daylabel(day);

                            stringstream sql_rec;
                            sql_rec                         <<
                            "SELECT "                       <<
                            "RS._id, "                      <<  // 0
                            "RS.name, "                     <<  // 1
                            "MA.matfile, "                  <<  // 2
                            "CO.coordinates, "              <<  // 3
                            "IFNULL(RS.recognizing,0), "    <<  // 4
                            "RS.storeimage, "                <<  // 5
                            "RS.storevideo, "                <<  // 6
                            "RS.codename, "                  <<  // 7
                            "RS.has_region, "                <<  // 8
                            "RS.delay, "                     <<  // 9      
                            "RCRS.start_rec_time, "         <<  // 10
                            "RCRS._id_recognition_setup, "  <<  // 11
                            "RS.runatstartup, "              <<  // 12
                            "RS.xmlfilepath "               <<   // 13
                            "FROM recognition_setup AS RS "                             <<
                            "JOIN day AS D on RS._id_day = D._id "                      <<
                            "JOIN cameras AS C ON RS._id_camera = C._id "               <<
                            "JOIN mat AS MA ON RS._id_mat = MA._id "                    <<
                            "JOIN coordinates AS CO ON RS._id_coordinates = CO._id "    <<
                            "JOIN rel_camera_recognition_setup AS RCRS ON RS._id = RCRS._id_recognition_setup "     <<
                            "WHERE RS._id_camera = " << camera <<  
                            " AND D.label = '" << day << "';";

                            std::string sqlred =  sql_rec.str();
                            cout << "sqlred: " << sqlred << endl;

                            pthread_mutex_lock(&databaseMutex);
                            vector<vector<string> > rec_array = db_select(sqlred.c_str(), 14);
                            pthread_mutex_unlock(&databaseMutex);

                            int sizer = rec_array.size();

                            if (sizer>0)
                            {
                                for (int t=0; t<rec_array.size(); t++ )
                                {    
                                    vector<string> rowr = rec_array.at(t);
                                    mcam->set_db_idcamera(camid);
                                    std::string recname = rowr.at(1);
                                    mcam->set_recname(recname);
                                    google::protobuf::int32 camamat = atoi(rowr.at(2).c_str());
                                    mcam->set_activemat(camamat);
                                    mcam->set_coordinates(rowr.at(3));
                                    google::protobuf::int32 rec = atoi(rowr.at(4).c_str());
                                    mcam->set_recognizing(rec);
                                    bool hasrecjob = true;
                                    mcam->set_hasrecjob(hasrecjob);
                                    mcam->set_storeimage(to_bool(rowr.at(5)));
                                    mcam->set_storevideo(to_bool(rowr.at(6)));
                                    mcam->set_codename(rowr.at(7));
                                    mcam->set_hasregion(to_bool(rowr.at(8)));
                                    google::protobuf::int32 delay = atoi(rowr.at(9).c_str());
                                    mcam->set_delay(delay); 
                                    mcam->set_startrectime(rowr.at(10));
                                    google::protobuf::int32 _id_recognition_setup = atoi(rowr.at(11).c_str());
                                    mcam->set_db_recognitionsetupid(_id_recognition_setup);
                                    mcam->set_runatstartup(to_bool(rowr.at(12)));
                                    std::string xmlpath = rowr.at(13).c_str();
                                    mcam->set_xmlfilepath(xmlpath);
                                    
                                }
                            }
                        }
                    }
                }
            }
        }      
    }
    return m;
    
}

void updateRecognition(motion::Message m)
{
    
    motion::Message::MotionCamera * pcamera = m.mutable_motioncamera(0);   
    motion::Message::MotionMonth * pmonth = pcamera->mutable_motionmonth(0);
    motion::Message::MotionDay * pday = pmonth->mutable_motionday(0);
    
    string str_month;
    if (m.has_currmonth())
    {
        str_month = m.currmonth();
    }

    string str_day;
    if (m.has_currday())
    {
        str_day = m.currday();
    }
    
    std::string rcoords;
    if (pcamera->hasregion())
    {
        std::string rc = pcamera->coordinates(); 
        rcoords = base64_decode(rc);
        updateRegionIntoDatabase(rcoords, pcamera->db_recognitionsetupid());
    }
     
    google::protobuf::uint32 activecam = R_PROTO.activecam();
    
    std::string xml_path = getXMLFilePathAndName(activecam, pcamera->recname(), str_day, XML_FILE);
    
     //time
    struct timeval tr;
    struct tm* ptmr;
    char time_rasp[40];
    gettimeofday (&tr, NULL);
    ptmr = localtime (&tr.tv_sec);
    strftime (time_rasp, sizeof (time_rasp), "%Y-%m-%d %H:%M:%S %z", ptmr);
    
    updateIntervalsIntoDatabase(pcamera);
    updateRecognitionSetup(pcamera, pday);
    updateCameraMonth(time_rasp, pcamera->db_recognitionsetupid());
    
}

motion::Message saveRecognition(motion::Message m)
{
    motion::Message::MotionCamera * pcamera = m.mutable_motioncamera(0);
    
    int sizec = m.motioncamera_size();
    
    cout << "sizec: " << sizec << endl;
    bool cameraexist = false;
    
    std::string rcoords;
    int db_coordnatesid; 
    if (pcamera->hasregion())
    {
        std::string rc = pcamera->coordinates(); 
        rcoords = base64_decode(rc);
        db_coordnatesid = insertRegionIntoDatabase(rcoords);     
    }
    
     //Month.
    string str_month;
    if (m.has_currmonth())
    {
        str_month = m.currmonth();
    }
    cout << "str_month: " << str_month << endl;
   
    motion::Message::MotionMonth * pmonth = pcamera->mutable_motionmonth(0);
    
    std::string cameraname = pcamera->cameraname();
    
    stringstream sql_camera;
    sql_camera   <<
    "SELECT C._id FROM cameras AS C WHERE name = '" << cameraname << "';";
    pthread_mutex_lock(&databaseMutex);
    vector<vector<string> > camera_array = db_select(sql_camera.str().c_str(), 1);
    pthread_mutex_unlock(&databaseMutex);
    
    int db_camera_id = atoi(camera_array.at(0).at(0).c_str());
    
    int db_month_id = insertMonthIntoDatabase(str_month, db_camera_id);
    
    string str_day;
    if (m.has_currday())
    {
        str_day = m.currday();
    }
    
      
    google::protobuf::uint32 activecam = R_PROTO.activecam();
    
    std::string xml_path = getXMLFilePathAndName(activecam, pcamera->recname(), str_day, XML_FILE);
    
     //time
    struct timeval tr;
    struct tm* ptmr;
    char time_rasp[40];
    gettimeofday (&tr, NULL);
    ptmr = localtime (&tr.tv_sec);
    strftime (time_rasp, sizeof (time_rasp), "%Y-%m-%d %H:%M:%S %z", ptmr);
    
    int db_dayid;
    int db_recognition_setupid;
   
    db_dayid = insertDayIntoDatabase(str_day, db_month_id);
    int db_intervalid = insertIntervalsIntoDatabase(pcamera);
    int db_recognitionsetup_id = insertIntoRecognitionSetup(pcamera, db_intervalid, db_dayid, db_camera_id, db_coordnatesid, xml_path);
    
    
    insertIntoCameraMonth(time_rasp, db_recognitionsetup_id, db_camera_id);
    
    return m;
   
}


void startMainRecognition()
{
    runr = pthread_create(&thread_recognition, NULL, startRecognition, NULL);
    if ( runr  != 0) {
        cerr << "Unable to create thread" << endl;
    }
}

motion::Message runCommand(motion::Message m)
{
    GOOGLE_PROTOBUF_VERIFY_VERSION;
    
    cout << "runCommand:: " << m.type() << endl;
    
    switch (m.type())
    {
        case motion::Message::ENGAGE:
        {
            cout << "motion::Message::ENGAGE" << endl;
            
            struct timeval tr;
            struct tm* ptmr;
            char time_rasp[40];
            gettimeofday (&tr, NULL);
            ptmr = localtime (&tr.tv_sec);
            strftime (time_rasp, sizeof (time_rasp), "%Y-%m-%d %H:%M:%S %z", ptmr);
            
            //Activity
            stringstream sql_connection;
            sql_connection <<
            "INSERT into connections (serverip, time) VALUES ('"
            << m.serverip() << "', '" << time_rasp << "');";
            cout << "sql_connection: " << sql_connection.str() << endl;
            pthread_mutex_lock(&databaseMutex);
            db_execute(sql_connection.str().c_str());
            pthread_mutex_unlock(&databaseMutex);
            
            m.set_type(motion::Message::ENGAGE);

            m = getRefreshProto(m);
          
            stringstream sql_starttime;
            sql_starttime << "SELECT starttime FROM status;";
            pthread_mutex_lock(&databaseMutex);
            vector<vector<string> > starttime_array = db_select(sql_starttime.str().c_str(), 1);
            pthread_mutex_unlock(&databaseMutex);
            std::string starttime = starttime_array.at(0).at(0);
            m.set_devicestarttime(starttime);
            cout << "starttime: " << starttime << endl;
        }
        break;
        
        case motion::Message::REFRESH:
        {
            cout << "motion::Message::REFRESH" << endl;
            int cam = m.activecam();
            m.set_time(getTimeRasp());
            m.set_type(motion::Message::REFRESH);
            m = getRefreshProto(m);
        }
        break;
        
        case motion::Message::SAVE:
        {
            cout << "motion::Message::SAVE" << endl;
            m = saveRecognition(m);
            m.set_type(motion::Message::SAVE_OK);
            m = getRefreshProto(m);
        }
        break;
        
        case motion::Message::UPDATE:
        {
            cout << "motion::Message::UPDATE" << endl;
            updateRecognition(m);
            m.set_type(motion::Message::UPDATE_OK);
        }
        break;
        
        case motion::Message::OPEN:
        {
            cout << "motion::Message::OPEN" << endl;
        }
        break;
        
        case motion::Message::GET_XML:
        {
            cout << "motion::Message::GET_XML" << endl;

            int cam = m.activecam();
            
            std::string str_month = m.currmonth();
            std::string str_day = m.currday();
            
            motion::Message::MotionCamera * pcamera = m.mutable_motioncamera(0);   
            
            std::string recname = pcamera->recname();  
            
            std::string xml_path = pcamera->xmlfilepath(); 
            
            bool xmlexist = file_exists(xml_path);
            
            if (xmlexist)
            {
                string xml_loaded = get_file_contents(xml_path);
                std:string encoded_xml = base64_encode(reinterpret_cast<const unsigned char*>(xml_loaded.c_str()),xml_loaded.length());
                m.set_data(encoded_xml.c_str());
            }
            m.set_type(motion::Message::GET_XML);
           
        }
        break;
        
        case motion::Message::GET_VIDEO:
        {
            
            cout << "motion::Message::GET_VIDEO" << endl;
            int cam = m.activecam();
           
            std::string videofilepath = m.videofilepath();
            cout << "imagefilepath : " << videofilepath << endl;

            std::string s = "src/";

            std::string::size_type i = videofilepath.find(s);

            if (i != std::string::npos)
                videofilepath.erase(0, s.size());
            
            stringstream strmpath;
            strmpath << sourcepath << videofilepath;

            std::string path = strmpath.str();
            
            string filename = m.data();
        
            std::string command;
            command += "cat ";
            command += path;
            command += "*.jpg | ffmpeg -framerate 20  -f image2pipe -c:v mjpeg -i - ";
            command += filename;
            cout << "command: " << command << endl;
            char *cstr = new char[command.length() + 1];
            strcpy(cstr, command.c_str());
            std::string resutl_commnd = exec_command(cstr);
            delete [] cstr;
            
            std::ifstream in(filename.c_str());
            std::stringstream buffer;
            buffer << in.rdbuf();
            std::string contents(buffer.str());
            
            std::string oriencoded = base64_encode(reinterpret_cast<const unsigned char*>(contents.c_str()), contents.length());
    
            int size = oriencoded.size();
            
            //Store into proto
            m.set_data(oriencoded.c_str());
            
            m.set_type(motion::Message::GET_VIDEO);
            
        }
        break;
        
        case motion::Message::GET_IMAGE:
        {
            cout << "motion::Message::GET_IMAGE" << endl;
            std::string imagefilepath = m.imagefilepath();
            Mat mat = imread(imagefilepath);
            m = serializeMediaToProto(m, mat);
            m.set_type(motion::Message::GET_IMAGE);
            m.set_activecam(m.activecam());
        }
        break;
        
        case motion::Message::DISSCONNECT:
        {
            cout << "motion::Message::DISSCONNECT" << endl;
        }
        break;
        
        case motion::Message::REC_START:
        {
            cout << "motion::Message::REC_START" << endl;
             
            motion::Message::MotionCamera * mcamera = m.mutable_motioncamera(0);
            
            std::string name = mcamera->recname();
            stringstream camera;
             camera << mcamera->cameranumber();
            
            const motion::Message::MotionRecognition & mrec = mcamera->motionrec(0);
            
            std::string rname = mrec.name();
            int db_idrec = mrec.db_idrec();
            
            if (loadStartQuery(camera.str(), name))
            {
                startMainRecognition();
            } else 
            {
                cout << "No matching values for the current arguments." << endl; 
            }
           
            updateRecStatus(1, mcamera->cameranumber(), mcamera->recname());
            
            m = getRefreshProto(m);
            
            m.set_type(motion::Message::REC_START);
            
        }
        break;
        
        case motion::Message::REC_STOP:
        {
            cout << "motion::Message::REC_STOP" << endl;
              
            int cam = m.activecam();
            pthread_mutex_lock(&protoMutex);
            motion::Message::MotionCamera * mcamera = R_PROTO.mutable_motioncamera(cam);
            mcamera->set_recognizing(false);
            pthread_mutex_unlock(&protoMutex);
                
            updateRecStatus(0, mcamera->cameranumber(), mcamera->recname());
            
            m.set_type(motion::Message::REC_STOP);
            
        }
        break;
        
        case motion::Message::TAKE_PICTURE:
        {
            cout << "motion::Message::TAKE_PICTURE" << endl;
            m = takePictureToProto(m);
            
            struct timeval tr;
            struct tm* ptmr;
            char time_rasp[40];
            gettimeofday (&tr, NULL);
            ptmr = localtime (&tr.tv_sec);
            strftime (time_rasp, sizeof (time_rasp), "%Y-%m-%d %H:%M:%S %z", ptmr);
            
            m.set_time(time_rasp);
            m.set_type(motion::Message::TAKE_PICTURE);
        }
        break;
        
        case motion::Message::STRM_START:
        {
            cout << "motion::Message::STRM_START" << endl;
            netcvc();
        }
        break;
        
        case motion::Message::STRM_STOP:
        {
            cout << "motion::Message::STRM_STOP" << endl;
            stop_capture = true;
        }
        break;
        
        case motion::Message::GET_TIME:
        {
            cout << "motion::Message::GET_TIME" << endl;
            struct timeval tr;
            struct tm* ptmr;
            char time_rasp[40];
            gettimeofday (&tr, NULL);
            ptmr = localtime (&tr.tv_sec);
            strftime (time_rasp, sizeof (time_rasp), "%Y-%m-%d %H:%M:%S %z", ptmr);
            
            cout << "time_rasp: " << time_rasp << endl;
  
            m.set_time(time_rasp);
            m.set_type(motion::Message::GET_TIME);
        }
        break;
        
        case motion::Message::SET_TIME:
        {
            cout << "motion::Message::SET_TIME" << endl;
            result_message = m.time();
            cout << "coming time: "  << result_message  << endl;
            struct tm tmremote;
            char *bufr;
            bufr = new char[result_message.length() + 1];
            strcpy(bufr, result_message.c_str());
            cout << "bufr       : " << bufr << endl;
            memset(&tmremote, 0, sizeof(struct tm));
            strptime(bufr, "%Y-%m-%d %H:%M:%S %z", &tmremote);
            std::cout << std::endl;
            std::cout << " Seconds  :"  << tmremote.tm_sec  << std::endl;
            std::cout << " Minutes  :"  << tmremote.tm_min  << std::endl;
            std::cout << " Hours    :"  << tmremote.tm_hour << std::endl;
            std::cout << " Day      :"  << tmremote.tm_mday << std::endl;
            std::cout << " Month    :"  << tmremote.tm_mon  << std::endl;
            std::cout << " Year     :"  << tmremote.tm_year << std::endl;
            std::cout << std::endl;
            struct tm mytime;
            struct timeval tv;
            time_t epoch_time;
            struct timezone timez;
            char * text_time;
            int hour = tmremote.tm_hour; // + 3;
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
            printf("The system time is set to %s\n", text_time);
            m.set_type(motion::Message::TIME_SET);
            m.set_serverip(PROTO.serverip());
        }
        break;
    }

    return m;
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

void totalsSocket()
{
    //cout << "count_vector_size : " << count_vector_size << " count_sent__split : " << count_sent__split << endl;
    if ( count_sent__split < (count_vector_size -1))
    {
        count_sent__split++;
    } else
    {
        count_sent__split = 0;
        msg_split_vector.clear();
    }
    //cout << " count_sent__split : " << count_sent__split << endl;
}

// TCP client handling function
motion::Message::ActionType HandleTCPClient(TCPSocket *sock)
{
    
    motion::Message::ActionType value;
    int bytesReceived = 0;              // Bytes read on each recv()
    int totalBytesReceived = 0;         // Total bytes read
  
try
{
    
  // Send received string and receive again until the end of transmission
  char echoBuffer[motion::Message::SOCKET_BUFFER_NANO_SIZE + 40];
  int recvMsgSize;
  std::string message;
    

    while ((recvMsgSize = sock->recv(echoBuffer, motion::Message::SOCKET_BUFFER_NANO_SIZE)) > 0)
    {

      totalBytesReceived += recvMsgSize;     // Keep tally of total bytes
      echoBuffer[recvMsgSize] = '\0';        // Terminate the string!
        
      stringstream sss;
      sss << echoBuffer;
      string strproto = sss.str();
     
      std::string strdecoded;
      strdecoded.clear();
      strdecoded = base64_decode(strproto);
      
      cout << "RECEIVE" << endl;
      
      GOOGLE_PROTOBUF_VERIFY_VERSION;
      motion::Message ms;
      ms.ParseFromArray(strdecoded.c_str(), strdecoded.size());
      
      cout << "PARSE" << endl;
      
      int camamounts = ms.motioncamera_size();
        
      PROTO.Clear();
      PROTO = ms;
      
      google::protobuf::uint32 chunck_size = PROTO.packagesize();
        
      value = ms.type();
      if (ms.has_serverip())
        T_PROTO.set_serverip(ms.serverip());
        
      cout << "value:: " << value << endl;
    
      if ( ms.type()==motion::Message::RESPONSE_OK || ms.type()==motion::Message::RESPONSE_END )
      {
          cout << "response : " <<  ms.type() << endl;
          count_sent__split = 0;
          
          google::protobuf::ShutdownProtobufLibrary();
          
          return ms.type();
      }
      else if (ms.type()==motion::Message::RESPONSE_NEXT)
      {
          
          std:string payspl = msg_split_vector.at(count_sent__split);
          
          string header =
          "PROSTA" +
            fixedLength(payspl.size() +40,  4)  + "::" +
            fixedLength(count_vector_size,  4)  + "::" +
            fixedLength(count_sent__split,  4)  + "::" +
            IntToString(inttype)                + "::" +
            IntToString(protofile)              +
          "PROSTO";
          
          msg = header + payspl;

          cout << "header 2 : " << header << endl;
          cout << "size: " << msg.size() << endl;
          cout << "..........................................." << endl;
          //cout << msg << endl;
          //cout << "..........................................." << endl;
          
          totalsSocket();
          
          //if (PROTO.type()==motion::Message::TAKE_PICTURE)
          //{
              //std::string basefile = "data/data/MAT_";
              //stringstream rr;
              //rr << basefile << count_sent__split << ".txt";
              //std::ofstream out;
              //out.open (rr.str().c_str());
              //out << msg << "\n";
              //out.close();
          //}
          
          sock->send(msg.c_str(), msg.size());
          
          google::protobuf::ShutdownProtobufLibrary();
        
          return ms.type();
      }
      
      //Elaborate response.
      motion::Message m;
      
      //Run Command.
      m = runCommand(ms);
        
      cout << "Serializing proto response." << endl;
      
      int m_amounts = m.motioncamera_size();
      
      //Split file outside proto.
      std::string datafile;
      if (m.has_data())
      {
          
          datafile = "PROFILE" + m.data();
          cout << "::::::::::::::::::::::::::::::::::::::::::::::: " << endl;
          cout << "m size 1: " << m.ByteSize() << endl;
          m.clear_data();
          cout << "m size 2: " << m.ByteSize() << endl;
          cout << "::::::::::::::::::::::::::::::::::::::::::::::: " << endl;
          protofile = motion::Message::PROTO_HAS_FILE;
          
      } else
      {
          protofile = motion::Message::PROTO_NO_FILE;
      }
        
      //Initialize objects to serialize.
      int size = m.ByteSize();
        
      char dataresponse[size];
    
      cout << "Proto size   : " << size << endl;
      
      cout << "active cam: " << m.activecam() << endl;
      cout << "type: " << m.type() << endl;

      m.SerializeToArray(&dataresponse, size);
     
      cout << "Encoding." << endl;
        
      std::string encoded_proto = base64_encode(reinterpret_cast<const unsigned char*>(dataresponse),sizeof(dataresponse));
        
      std::stringstream ssenc;
        
      if (protofile == motion::Message::PROTO_HAS_FILE)
      {
          cout << "::::::::::::::::::::::::::::::::::::::::::::::: " << endl;
          ssenc << encoded_proto;
          cout << "encoded_proto size ::1::  " << ssenc.str().size() << endl;
          ssenc << datafile;
          cout << "encoded_proto size ::2::  " << ssenc.str().size() << endl;
          cout << "::::::::::::::::::::::::::::::::::::::::::::::: " << endl;
      }
      else
      {
          ssenc << encoded_proto;
      }
        
      string header;
      inttype = ms.type();
        
      std::string all_encoded = ssenc.str();
      int final_size = all_encoded.size();
             
      if ( final_size > chunck_size )
      {
          for (unsigned i = 0; i < all_encoded.length(); i += chunck_size)
          {
              msg_split_vector.push_back(all_encoded.substr(i, chunck_size));
          }
          
          count_vector_size = msg_split_vector.size();
          std::string payspl = msg_split_vector.at(count_sent__split);
          
          header =
          "PROSTA" +
            fixedLength(payspl.size()+40,   4)  + "::" +
            fixedLength(count_vector_size,  4)  + "::" +
            fixedLength(count_sent__split,  4)  + "::" +
            IntToString(inttype)                + "::" +
            IntToString(protofile)              +
          "PROSTO";
          msg = header + payspl;

          cout << "header 1 : " << header << endl;
          cout << "size: " << msg.size() << endl;
          cout << "..........................................." << endl;
          //cout << msg << endl;
          //cout << "..........................................." << endl;

      }
      else
      {
          
          header =
          "PROSTA";
          header += fixedLength(all_encoded.size()+40,4);
          header +=
          "::"
          "0001"
          "::"
          "0000"
          "::";
          header += IntToString(inttype);
          header += "::";
          header += IntToString(protofile);
          header += "PROSTO";
          msg = header + all_encoded;
          
          count_vector_size = 0;
          
          cout << "header 0 : " << header << endl;
          cout << "size: " << msg.size() << endl;
          cout << "..........................................." << endl;
          
          //cout << msg << endl;
          //cout << "..........................................." << endl;
          
      }
      
        //if (PROTO.type()==motion::Message::TAKE_PICTURE)
        //{
            //std::string basefile = "data/PROTO_";
            //stringstream rr;
            //rr << basefile << count_sent__split << ".txt";
            //std::ofstream out;
            //out.open (rr.str().c_str());
            //out << msg << "\n";
            //out.close();
        //}

        
        ms.Clear();
        //google::protobuf::ShutdownProtobufLibrary();
    
        totalsSocket();
        
        cout << "Socket Sent Size: " << msg.size() << endl;
        
        sock->send(msg.c_str(), msg.size());
        
        return ms.type();
        
    }
}
catch(SocketException &e)
{
        cout << "::error:: " << e.what() << endl;
        cerr << e.what() << endl;
}
  return value;
}

void * ThreadMain(void *clntSock)
{
    // Guarantees that thread resources are deallocated upon return
    pthread_detach(pthread_self());
    resutl_echo = HandleTCPClient((TCPSocket *) clntSock);
    delete (TCPSocket *) clntSock;
    pthread_exit(NULL);
    return (void *) resutl_echo;
}

void * socketThread (void * args)
{
    // TCP
    unsigned short tcp_port = motion::Message::TCP_ECHO_PORT;
    void *status;
    pthread_cond_init(&echo_response, NULL);
    try
    {
        TCPServerSocket servSock(tcp_port);
        for (;;)
        {
            TCPSocket *clntSock = servSock.accept();
            runt =  pthread_create(&thread_echo, NULL, ThreadMain, (void *) clntSock);
            if ( runt  != 0)
            {
                cerr << "Unable to create ThreadMain thread" << endl;
                exit(1);
            }
            pthread_join(    thread_echo,               (void**) &runt);
        }
    } catch (SocketException &e) {
        cerr << e.what() << endl;
        exit(1);
    }
}

std::string getIpAddress (std::string iface)
{
    int fd;
    struct ifreq ifr;
    fd = socket(AF_INET, SOCK_DGRAM, 0);
    /* I want to get an IPv4 IP address */
    ifr.ifr_addr.sa_family = AF_INET;
    /* I want IP address attached to "eth0" */
    strncpy(ifr.ifr_name, iface.c_str(), IFNAMSIZ-1);
    //strncpy(ifr.ifr_name, "eth0", IFNAMSIZ-1);
    ioctl(fd, SIOCGIFADDR, &ifr);
    close(fd);
    char * ipaddrs = inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr);
    std::string ip_txt(ipaddrs);
    std::cout << " IP TERMINAL :: " << ip_txt << '\n';
    return ip_txt;
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

void * broadcastsender ( void * args )
{
    
   std::string destAddress = NETWORK_IP + ".255";
   unsigned short destPort = motion::Message::UDP_PORT;
   char *sendString = new char[local_ip.length() + 1];
   std::strcpy(sendString, local_ip.c_str());
   char recvString[MAXRCVSTRING + 1];
    try 
    {
    UDPSocket sock;
    int countud;
    for (;;) {
        sock.sendTo(sendString, strlen(sendString), destAddress, destPort);
        //cout << "UPD Send: " << countud << " " << getShortTimeRasp() << endl;
        countud++;
        sleep(3);
    }
    delete [] sendString;
  
  } catch (SocketException &e) {
    cerr << e.what() << endl;
    cout  <<  "Error: " << cerr << endl;
    exit(1);
  }
}

std::vector<int> getCameras()
{
    std::vector<int> camsv;
    CvCapture* temp_camera;
    int maxTested = 2;
    for (int i = 0; i < maxTested; i++){
        temp_camera = cvCreateCameraCapture(i);
        if (temp_camera!=NULL)
        {
            camsv.push_back(i);
            cvReleaseCapture(&temp_camera);
        }
    }
    return camsv;
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



int main (int argc, char * const av[])
{
    
    //motion 
        //status
        //start name
        //stop name
       
    const char **argv = (const char **) av;
    
    //argc = 3;
    //argv[1] = "-start"; //"start";
    //argv[2] = "0";
    //argv[3] = "VEREDA";
   
    cout << "argv[0]: " << argv[0] << endl;
    
    if (argc==2)
            cout << "argv[1]: " << argv[1] << endl;
    
    if (argc==3)
        cout << "argv[2]: " << argv[2] << endl;
    
    
    std::string runparam = argv[0];
    if (runparam=="./motion_detect_raspberry")
    {
        basepath = "";
        sourcepath = "../../src/";
        
    }
    else if (runparam.find("/home/pi/motion/motion-detection/motion_src/src/motion_detect/motion_detect_raspberry") != std::string::npos )
    {
        basepath    = "src/motion_detect/";
        sourcepath  = "src/";
    }
    
    //Create database.
    //db_create();
    cout << "Getting hard info." << endl;
    int db_hardware_id = db_cpuinfo();
    cout << "db_hardware_id: " << db_hardware_id << endl;
    
    //Rasp Variables.
    std::vector<int> cams = getCameras();
    stringstream ss;
    copy( cams.begin(), cams.end(), ostream_iterator<int>(ss, " "));
    std::string cameras = ss.str();
    cameras = cameras.substr(0, cameras.length()-1);
   
    struct timeval tr;
    struct tm* ptmr;
    char time_rasp[40];
    gettimeofday (&tr, NULL);
    ptmr = localtime (&tr.tv_sec);
    strftime (time_rasp, sizeof (time_rasp), "%Y-%m-%d %H:%M:%S %z", ptmr);
    cout <<  ":::start time:::: " << time_rasp << endl;
    
    std::string basedatafile = basepath + "data"; //"data";
    directoryExistsOrCreate(basedatafile.c_str());
    
     //Store into database.
    vector<int> camsarray = db_cams(cams);
     
    //Activity
    stringstream allparams;
    allparams << argv;
    stringstream sql_activity;
    sql_activity <<
    "INSERT into activity (params, run_time) VALUES ('"<< allparams.str() << "', '" << time_rasp << "');";
    db_execute(sql_activity.str().c_str());
    
    std::string checketh = "cat /sys/class/net/eth0/operstate";
    char *cestr = new char[checketh.length() + 1];
    strcpy(cestr, checketh.c_str());
    std::string resutl_eth0 = exec_command(cestr);
    resutl_eth0.erase(std::remove(resutl_eth0.begin(), resutl_eth0.end(), '\n'), resutl_eth0.end());
     
    std::string checkwlan = "cat /sys/class/net/wlan0/operstate";
    char *cwstr = new char[checketh.length() + 1];
    strcpy(cwstr, checkwlan.c_str());
    std::string resutl_wlan0 = exec_command(cwstr);
    resutl_wlan0.erase(std::remove(resutl_wlan0.begin(), resutl_wlan0.end(), '\n'), resutl_wlan0.end());
    
    if (resutl_eth0 == "up")
    {
        local_ip = getIpAddress("eth0");
        
    } else if ((resutl_eth0 == "down") && (resutl_wlan0 == "up"))
    {
        local_ip = getIpAddress("wlan0");
        
    } else if ((resutl_eth0 == "down") && (resutl_wlan0 == "down"))
    {
        cout << "NO NETWORK INTERFACE UP." << endl;
        return 0;
    }
    
    
    cout  <<  "IP: " << local_ip << endl;
    local_ip = local_ip;
    vector<string> ip_vector;
    split(local_ip, '.', ip_vector);
    for (int i=0; i<ip_vector.size(); i++)
    {
        if ( i==0 | i==1)
        {
            NETWORK_IP +=  ip_vector[i] + ".";
        }
        else if ( i==2 )
        {
            NETWORK_IP +=  ip_vector[i];
        }
    }
    
    FILE *in;
    char buff[512];

    //if(!(in = popen("curl ifconfig.me", "r"))){
    //        return 1;
    //}

    std::string publicip= "200.200.200.222";
    //while(fgets(buff, sizeof(buff), in)!=NULL)
    //{
    //    stringstream bus;
    //    bus << buff;
    //    publicip = bus.str();
   // }
    
    cout << "ipnumber: " << NETWORK_IP << endl;
    cout << "publicip: " << publicip << endl;
    
     //Status
    stringstream sql_network;
    sql_network <<
    "INSERT INTO network (ipnumber, ippublic) " <<
    "SELECT '"  << local_ip        << "'"
    ", '"       << publicip        << "' "
    "WHERE NOT EXISTS (SELECT * FROM network WHERE ipnumber = '"<< local_ip << "' " <<
    "AND ippublic = '" << publicip << "');";
    db_execute(sql_network.str().c_str());
    
    if(!(in = popen("uptime", "r"))){
            return 1;
    }

    std::string uptime;
    while(fgets(buff, sizeof(buff), in)!=NULL)
    {
        stringstream bus;
        bus << buff;
        uptime = bus.str();
    }
    
    //Status
    stringstream sql_status;
    sql_status <<
    "INSERT INTO status (uptime, starttime) " <<
    "SELECT '"  << uptime         << "'"
    ", '"       << time_rasp        << "' "
    "WHERE NOT EXISTS (SELECT * FROM status WHERE uptime = '"<< uptime << "' " <<
    "AND starttime = '" << time_rasp << "');";
    db_execute(sql_status.str().c_str());
    
    std::string last_status_id_query = "SELECT MAX(_id) FROM status";
    vector<vector<string> > status_array = db_select(last_status_id_query.c_str(), 1);
    int db_status_id = atoi(status_array.at(0).at(0).c_str());
    cout << "db_status_id: " << db_status_id << endl;
    
    stringstream sql_status_update;
    sql_status_update <<
    "UPDATE status SET "
    "uptime = '"    << uptime << "',"
    "starttime = '" << time_rasp << "' "
    "WHERE _id = " << db_status_id << ";";
    db_execute(sql_status_update.str().c_str());
    
    cout << "Getting hard info." << endl;
    vector<int> camhard;
    for (int i=0; i< camsarray.size(); i++)
    {
        stringstream insert_camera_query;
        insert_camera_query <<
        "INSERT INTO rel_hardware_camera (_id_hardware, _id_camera) " <<
        "SELECT " << db_hardware_id << "," << camsarray.at(i) <<
        " WHERE NOT EXISTS (SELECT * FROM rel_hardware_camera WHERE _id_hardware = "
        << camera << " AND _id_camera = " << camsarray.at(i) << ");";
        db_execute(insert_camera_query.str().c_str());
            
        std::string last_har_cam_id_query = "SELECT MAX(_id) FROM rel_hardware_camera";
        vector<vector<string> > camhard_array = db_select(last_har_cam_id_query.c_str(), 1);
        int db_cam_hard_id = atoi(camhard_array.at(0).at(0).c_str());
        cout << "db_cam_hard_id: " << db_cam_hard_id << endl;
            
        camhard.push_back(db_cam_hard_id);
        
    }
    
    //rel_hardware_camera_status
    for (int i=0; i< camhard.size(); i++)
    {
        stringstream insert_rel_hardcamsta_query;
        insert_rel_hardcamsta_query <<
        "INSERT INTO rel_hardware_camera_status (_id_hardware_camera, _id_status) " <<
        "SELECT " << camhard.at(i) << "," << db_status_id <<
        " WHERE NOT EXISTS (SELECT * FROM rel_hardware_camera_status WHERE _id_hardware_camera = "
        << camhard.at(i) << " AND _id_status = " << db_status_id << ");";
        db_execute(insert_rel_hardcamsta_query.str().c_str());
    }
    
    starttime = time_rasp;

    std::string secdatafile = basepath + "data/data";
    directoryExistsOrCreate(secdatafile.c_str());
    std::string matdatafile = basepath + "data/mat";
    directoryExistsOrCreate(matdatafile.c_str());
    
    //Params
    if ( argc >= 2 ) 
    {
        std::string doparam = argv[1];
        std::string param = argv[2];
        if (doparam=="-status")
        {
            status();
            return 0;
        }
        else if (doparam=="-start")
        {
            std::string param_camera = argv[2];
            std::string param_name   = argv[3];
            
            if (loadStartQuery(param_camera, param_name))
            {
                startMainRecognition();
            } else 
            {
                cout << "No matching values for the current arguments." << endl; 
            }
        } 
        else if (doparam=="-stop")
        {
            
        }
    }
    
    for (int t=0; t< cams.size(); t++)
    {
       
        //struct timeval tr;
        //struct tm* ptmr;
        char time_interval[8];
        //gettimeofday (&tr, NULL);
        //ptmr = localtime (&tr.tv_sec);
        //strftime (time_interval, sizeof (time_interval), "%H:%M:%S", ptmr);
        
     
        std::string timecompare = "11:00:00";
          
        char *cstr = new char[timecompare.length() + 1];
        strcpy(cstr, timecompare.c_str());
        
    
       stringstream camstr;
       camstr << cams.at(t);
       vector<string> runvector = startIfNotRunningQuery(camstr.str(), cstr);
       if (runvector.size()>0)
       {
           std::string camer    = camstr.str();
           std::string recname  = runvector.at(1); 
           if (loadStartQuery(camer, recname))
            {
                startMainRecognition();
            } else 
            {
                cout << "No matching values for the current arguments." << endl; 
            }
       }
    }
    
    cout << "Start Time:: " << starttime << endl;

    pthread_mutex_init(&protoMutex, 0);
    pthread_mutex_init(&fileMutex, 0);
    pthread_mutex_init(&databaseMutex, 0);
    
    // UDP
   runb = pthread_create(&thread_broadcast, NULL, broadcastsender, NULL);
   if ( runb  != 0) {
       cerr << "Unable to create thread" << endl;
   }
   
    //Socket
    runs = pthread_create(&thread_socket, NULL, socketThread, NULL);
    if ( runs  != 0) {
        cerr << "Unable to create thread" << endl;
    }
    
    runo = pthread_create(&thread_observer, NULL, startObserver, NULL);
    if ( runo  != 0) {
        cerr << "Unable to create thread" << endl;
    }
    
    //Stream Socket Server.
    //StreamListener * stream_listener = new StreamListener();
    //stream_listener->startListening();

    pthread_join(    thread_broadcast,  (void**) &runb);
    pthread_join(    thread_socket,     (void**) &runs);
    pthread_join(    thread_observer,   (void**) &runr);
    
    cout << "THREAD TERMINATED!!!!!!!!!!!!!!!!!!!!! = " << runs << endl;
    return 0;
    
}

            
        


