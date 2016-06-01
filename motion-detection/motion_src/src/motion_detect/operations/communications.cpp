/*
 * File:   communications.cpp
 * Author: jose
 *
 * Created on Dec 12, 2015, 7:48 PM
 */


#include "../operations/communications.h"
#include "../database/database.h"
#include "../utils/utils.h"
#include "../b64/base64.h"
#include "../http/post.h"
#include "../socket/netcvc.h"
#include "../operations/camera.h"

#include <iostream>
#include <string>
#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <unistd.h>
#include <arpa/inet.h>

#include <stdlib.h>
#include <stdio.h>
#include <netdb.h>

#include <parse.h>

using namespace std;
using namespace cv;

//HandleTCPClient
pthread_t thread_send_echo;
vector<std::string> msg_split_vector_comm;
int count_sent__split=0;
int count_vector_size=0;
std::string msg;
int inttype, protofile;

struct arg_struct
{
    motion::Message message;
};

motion::Message serializeMediaToProto(motion::Message m, cv::Mat mat)
{
    
    m.set_serverip(PROTO.serverip());
    
    int active = m.activecam();
    motion::Message::MotionCamera * mcam = m.add_motioncamera();
    m.set_activecam(active);
    
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
    m.set_datafile(oriencoded.c_str());

    return m;
}

int UDPSend(motion::Message m)
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
    if (inet_aton(server, &remaddr.sin_addr)==0) 
    {
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
    int activecam = m.activecam();
    m.set_activecam(activecam);
     
    setActiveCam(activecam);
     
    vector<std::string> user_info = getUserInfo();
    if (user_info.size()>0)
    {
        motion::Message::MotionUser * muser = m.add_motionuser();
        muser->set_clientnumber(atoi(user_info.at(0).c_str()));
        muser->set_wpuser(user_info.at(1));
        muser->set_wppassword(user_info.at(2));
        muser->set_wpserverurl(user_info.at(3));
        muser->set_wpuserid(atoi(user_info.at(4).c_str()));
        muser->set_wpclientid(atoi(user_info.at(5).c_str()));
        muser->set_wpclientmediaid(atoi(user_info.at(6).c_str()));
        muser->set_pfobjectid(user_info.at(7));
        muser->set_username(user_info.at(8));
        muser->set_email(user_info.at(9));
        muser->set_firstname(user_info.at(10)); 
        muser->set_lastname(user_info.at(11)); 
        muser->set_location(user_info.at(12));
        muser->set_uiidinstallation(user_info.at(13));
    }

    vector<std::string> device_info = getTerminalInfo();
    if (device_info.size()>0)
    {
        motion::Message::MotionDevice * mdevice = m.add_motiondevice();
        mdevice->set_ipnumber(device_info.at(0));                       //ipnumber              
        mdevice->set_ippublic(device_info.at(1));                       //ippublic              
        mdevice->set_macaddress(device_info.at(2));                     //macaddress            
        mdevice->set_hostname(device_info.at(3));                       //hostname              
        mdevice->set_city(device_info.at(4));                           //city                  
        mdevice->set_country(device_info.at(5));                        //country               
        mdevice->set_location(device_info.at(6));                       //location              
        mdevice->set_network_provider(device_info.at(7));               //network_provider      
        mdevice->set_uptime(device_info.at(8));                         //uptime                
        mdevice->set_starttime(device_info.at(9));                      //starttime             
        mdevice->set_db_local(atoi(device_info.at(10).c_str()));                 //db_local               
        mdevice->set_model(device_info.at(11));                         //model                 
        mdevice->set_hardware(device_info.at(12));                      //hardware              
        mdevice->set_serial(device_info.at(13));                         //serial                
        mdevice->set_revision(device_info.at(14));                       //revision              
        mdevice->set_disktotal(atoi(device_info.at(15).c_str()));                //disktotal              
        mdevice->set_diskused(atoi(device_info.at(16).c_str()));                 //diskused               
        mdevice->set_diskavailable(atoi(device_info.at(17).c_str()));            //diskavailable          
        mdevice->set_disk_percentage_used(atoi(device_info.at(18).c_str()));     //disk_percentage_used   
        mdevice->set_temperature(atoi(device_info.at(19).c_str()));              //temperature            
    }

    vector<int> cams;
    stringstream sql_cameras;
    sql_cameras <<
    "SELECT C._id, C.number, C.name, C.active, C.thumbnail_path FROM cameras C;";
    pthread_mutex_lock(&databaseMutex);
    vector<vector<string> > cameras_array = db_select(sql_cameras.str().c_str(), 5);
    pthread_mutex_unlock(&databaseMutex);
     
    int camsize = cameras_array.size();
    cout << "camsize: " <<  camsize << endl;
  
    std::stringstream thubnails;
    thubnails << fixedLength(camsize, 4);

    for (int q=0; q<camsize; q++)
    {
         
        cout << "q: " <<  q << endl;
         
        vector<string> rowc = cameras_array.at(q);
        int mcamsize = m.motioncamera_size();
        motion::Message::MotionCamera * mcam = m.add_motioncamera();
         
        google::protobuf::int32 camid = atoi(rowc.at(0).c_str());
        mcam->set_cameraid(camid);
        google::protobuf::int32 camnum = atoi(rowc.at(1).c_str());
        mcam->set_cameranumber(camnum);
        std::string cameraname = rowc.at(2);
        mcam->set_cameraname(cameraname);
        
        int takepicture = motion::Message::TAKE_PICTURE;
        if ((m.type() == takepicture) && (activecam==q))
        {
            mcam = takePictureToProto(activecam, mcam);
            m.set_datafile(mcam->tempdata());
            mcam->clear_tempdata();
        }

        int sengage = motion::Message::ENGAGE;
        if (m.includethubmnails())
        { 
            cout << "ENTRA THUMBNAIL" << endl;

            //blobs
            /*const char * zKey = blob_key.c_str();
            unsigned char **pzBlob;
            int *pnBlob = 0;
            readBlob(zKey, pzBlob, pnBlob);
            if( SQLITE_OK!=readBlob(blob_key.c_str(), &pzBlob, &pnBlob) )
            {
                databaseError();
            }*/
             
            std::string thubmnail_path = rowc.at(4);

            cout << "thubmnail_path: " << thubmnail_path << endl;
           
            std::string thumbencoded = get_file_contents(thubmnail_path);

            cout << "thumbencoded: " << thumbencoded.substr(0, 200) << endl; 
        
            thubnails << "THUMBNAILSTART" << q << thumbencoded << "THUMBNAILEND" << q << endl;
            
            cout << "thumb decoded: " << thumbencoded.substr(0, 200) << endl;
            
            if (q==(camsize-1))
            {
                cout << "CIERRA THUMBNAIL: " << thubnails.str().size() << endl;
                m.set_datafile(thubnails.str()); 
            }
            
        }
         
        bool active = to_bool(rowc.at(3));
        if (active)
        {
            m.set_activecam(camnum);
        }
         
        stringstream sql_rec_setup;
        sql_rec_setup   <<
        "SELECT "       <<       
        "RCS._id, "     <<
        "RCS.name "     <<
        "FROM recognition_setup AS RCS "                <<
        "JOIN cameras AS C ON RCS._id_camera = C._id "  <<
        "WHERE C.number = " << camnum << ";";
        cout << "sql_rec_setup: " << sql_rec_setup.str() << endl;
        pthread_mutex_lock(&databaseMutex);
        vector<vector<string> > rec_setup_array = db_select(sql_rec_setup.str().c_str(), 2);
        pthread_mutex_unlock(&databaseMutex);
         
        std::string recname;
        int recognitionsize = rec_setup_array.size();
        if (recognitionsize>0)
        {
            //There is rec stored
            for (int t=0; t<rec_setup_array.size(); t++ )
            {
                vector<string> rows = rec_setup_array.at(t);
                
                google::protobuf::int32 recid = atoi(rows.at(0).c_str());
                recname = rows.at(1);
                 
                stringstream sql_last_instance;
                sql_last_instance   <<
                "SELECT coalesce( MAX(I.number), 0) FROM instance AS I "                                   <<
                "JOIN rel_day_instance_recognition_setup AS RDI ON I._id = RDI._id_instance "           <<
                "JOIN recognition_setup AS RS ON RDI._id_recognition_setup = RS._id "                   <<
                "JOIN rel_month_day AS RMD ON RDI._id_day = RMD._id_day "           <<
                "JOIN day as D ON RMD._id_day = D._id "                             <<
                "JOIN month AS M ON RMD._id_month = M._id "                         <<
                "JOIN rel_camera_month AS RCM ON RMD._id_month = RMD._id_month "    <<
                "JOIN cameras AS C ON RCM._id_camera = C._id "                      <<
                "WHERE C.number = " << camnum << " AND D.label = '" << m.currday()  << "' " <<
                "AND RS.name = '" << recname << "';";
                std::string sqllaststd = sql_last_instance.str();
                cout << "sqllaststd: " << sqllaststd << endl;
                pthread_mutex_lock(&databaseMutex);
                vector<vector<string> > lastinstance_array = db_select(sqllaststd.c_str(), 1);
                pthread_mutex_unlock(&databaseMutex);
                 
                motion::Message::MotionRec * mrec = mcam->add_motionrec();
                mrec->set_name(recname);
                 
                //if (m.has_recname())
                //{
                //   recname = m.recname();
                //} else
                //{
                //    m.set_recname(recname);
                //}
                 
                std::string last = lastinstance_array.at(0).at(0).c_str();
                int ln = atoi(last.c_str());
                if (ln>0)
                {
                    mrec->set_lastinstance(last);
                } else
                {
                    mrec->set_lastinstance("0");
                }
  
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
                "IFNULL(RS.since,0), "          << // 17
                "RS.speed, "                    << // 18
                "RS.has_cron, "                 << // 19
                "RS.activerec, "                << // 20
                "MA.basefile "                  << // 21       
                "FROM rel_day_instance_recognition_setup AS RDIR "                                                          <<
                "JOIN rel_camera_recognition_setup AS RCRS ON RDIR._id_recognition_setup = RCRS._id_recognition_setup "     <<
                "JOIN recognition_setup AS RS ON RDIR._id_recognition_setup = RS._id "                                      <<
                "JOIN coordinates AS CO ON RS._id_coordinates = CO._id "                                                    <<
                "JOIN cameras AS CAM ON RS._id_camera = CAM._id "                                                           <<
                "JOIN mat AS MA ON RS._id_mat = MA._id "                                                                    <<
                "WHERE RS._id_camera = " <<  camera                                                                         << 
                " AND RDIR._id_recognition_setup IN (SELECT _id from recognition_setup WHERE name = '" << recname << "') "  <<
                "GROUP BY RDIR._id_recognition_setup;";
  
                std::string sqlcamstr =  sql_cam_req_setup.str();
                cout << "sqlcamstr: " << sqlcamstr << endl;
  
                pthread_mutex_lock(&databaseMutex);
                vector<vector<string> > crs_array = db_select(sqlcamstr.c_str(), 22);
                pthread_mutex_unlock(&databaseMutex);
  
                int size = crs_array.size();
  
                if (size>0)
                {
                    vector<string> rows = crs_array.at(0);    
                    bool hasrecjob = true;
                    mcam->set_hasrecjob(hasrecjob);
                    mcam->set_db_idcamera(camid);
                    mrec->set_storeimage(to_bool(rows.at(0)));
                    mrec->set_storevideo(to_bool(rows.at(1)));
                    mrec->set_codename(rows.at(2));
                    mrec->set_hasregion(to_bool(rows.at(3)));
                    mrec->set_coordinates(rows.at(4));
                    google::protobuf::int32 delay = atoi(rows.at(5).c_str());
                    mrec->set_delay(delay); 
                    google::protobuf::int32 camamat = atoi(rows.at(6).c_str());
                    mrec->set_activemat(camamat);
                    mrec->set_startrectime(rows.at(7));
                    google::protobuf::int32 _id_recognition_setup = atoi(rows.at(8).c_str());
                    mrec->set_db_recognitionsetupid(_id_recognition_setup);
                    mrec->set_runatstartup(to_bool(rows.at(9)));
                    google::protobuf::int32 matcols = atoi(rows.at(10).c_str());
                    mrec->set_matcols(matcols);
                    google::protobuf::int32 matrows = atoi(rows.at(11).c_str());
                    mrec->set_matrows(matrows);
                    google::protobuf::int32 matwidth = atoi(rows.at(12).c_str());
                    mrec->set_matwidth(matwidth);
                    google::protobuf::int32 matheight = atoi(rows.at(13).c_str());
                    mrec->set_matheight(matheight);
                    mrec->set_recname(rows.at(14));
                    std::string xmlpath = rows.at(15).c_str();
                    mrec->set_xmlfilepath(xmlpath);
                    google::protobuf::int32 rec = atoi(rows.at(16).c_str());
                    mcam->set_recognizing(rec);
                    mrec->set_camerasince(rows.at(17));
                    google::protobuf::int32 speed = atoi(rows.at(18).c_str());
                    mrec->set_speed(speed);
                    bool hascron = to_bool(rows.at(19));
                    if (hascron)
                    {  
                        vector<string> intervalse  = getIntervalsByCamberaAndRec(camera, recname);
                        if (intervalse.size()>0)
                        {
                            std::string timestart   = intervalse.at(0);
                            mrec->set_timestart(timestart);
                            std::string timeend     = intervalse.at(1);
                            mrec->set_timeend(timeend);
                            mrec->set_hascron(true);
                        } else
                        {
                            mrec->set_hascron(false);
                        }
  
                    } else
                    {
                        mrec->set_hascron(false);
                    }
                    cout << "Month: "   << m.currmonth() << endl;
                    cout << "Day: "     << m.currday()  << endl;
                    google::protobuf::int32 activerec = atoi(rows.at(20).c_str());
                    mrec->set_activerec(activerec);
                    mrec->set_matbasefile(rows.at(21));
                    mcam = getMonthByCameraIdMonthAndDate(mcam, rows.at(0), m.currmonth(), m.currday(), recname);
                }
                else
                {
                     
                    std::string monthlabel = getCurrentMonthLabel();
                     
                    stringstream sql_current_month;
                    sql_current_month       <<
                    "SELECT M._id FROM month AS M WHERE M.label = '" << monthlabel << "';";
                     
                    std::string sqlcurrmonth =  sql_current_month.str();
                    cout << "sqlcurrmonth: " << sqlcurrmonth << endl;
  
                    pthread_mutex_lock(&databaseMutex);
                    vector<vector<string> > curr_month_array = db_select(sqlcurrmonth.c_str(), 1);
                    pthread_mutex_unlock(&databaseMutex);
                     
                    int sizecm = curr_month_array.size();
  
                    if (sizecm>0)
                    {
                     
                        int db_currmonth = atoi(curr_month_array.at(0).at(0).c_str());
  
                        stringstream sql_month;
                        sql_month       <<
                        "SELECT M._id, M.label FROM month AS M "                    <<  
                        "JOIN rel_camera_month AS RCM ON RCM._id_month = M._id "    <<
                        "WHERE RCM._id_month = " << db_currmonth << " AND RCM._id_camera = " << camera << ";";
  
                        std::string sqlmonth =  sql_month.str();
                        cout << "sqlmonth: " << sqlmonth << endl;
  
                        pthread_mutex_lock(&databaseMutex);
                        vector<vector<string> > month_array = db_select(sqlmonth.c_str(), 2);
                        pthread_mutex_unlock(&databaseMutex);
  
                        int sizem = month_array.size();
  
                        if (sizem>0)
                        {
  
                            for (int r=0; r<month_array.size(); r++ )
                            {
  
                                vector<string> rowm = month_array.at(r);
                                motion::Message::MotionMonth * mmonth = mcam->add_motionmonth();
                                std::string month = rowm.at(1);
                                mmonth->set_monthlabel(month);
  
                                stringstream sql_day;
                                sql_day             <<
                                "SELECT " <<
                                "D._id, " <<
                                "D.label " << 
                                "FROM day AS D " <<
                                "JOIN rel_month_day AS RDM ON RDM._id_day = D._id " <<
                                "JOIN month AS M ON RDM._id_month = M._id " <<
                                "JOIN rel_camera_month AS RCM ON M._id = RCM._id_month " << 
                                "WHERE RCM._id_camera = " << camera << " AND M._id = " << db_currmonth << ";";        
                                        
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
                                        "RS.storeimage, "               <<  // 5
                                        "RS.storevideo, "               <<  // 6
                                        "RS.codename, "                 <<  // 7
                                        "RS.has_region, "               <<  // 8
                                        "RS.delay, "                    <<  // 9      
                                        "RCRS.start_rec_time, "         <<  // 10
                                        "RCRS._id_recognition_setup, "  <<  // 11
                                        "RS.runatstartup, "             <<  // 12
                                        "RS.xmlfilepath, "              <<  // 13
                                        "RS.speed, "                    <<  // 14        
                                        "RS.has_cron, "                 <<  // 15
                                        "RS.activerec, "                <<  // 16  
                                        "MA.basefile "                  <<  // 17
                                        "FROM recognition_setup AS RS "                             <<
                                        "JOIN day AS D on RS._id_day = D._id "                      <<
                                        "JOIN cameras AS C ON RS._id_camera = C._id "               <<
                                        "JOIN mat AS MA ON RS._id_mat = MA._id "                    <<
                                        "JOIN coordinates AS CO ON RS._id_coordinates = CO._id "    <<
                                        "JOIN rel_camera_recognition_setup AS RCRS ON RS._id = RCRS._id_recognition_setup "     <<
                                        "WHERE RS._id_camera = " << camera      <<  
                                        " AND RS.name = '" << recname << "'"    <<        
                                        " AND D.label = '" << day << "';";
  
                                        std::string sqlred =  sql_rec.str();
                                        cout << "sqlred: " << sqlred << endl;
  
                                        pthread_mutex_lock(&databaseMutex);
                                        vector<vector<string> > rec_array = db_select(sqlred.c_str(), 18);
                                        pthread_mutex_unlock(&databaseMutex);
  
                                        int sizer = rec_array.size();
  
                                        if (sizer>0)
                                        {
                                            vector<string> rowr = rec_array.at(0);
                                            mcam->set_db_idcamera(camid);
                                            std::string recname = rowr.at(1);
  
                                            mrec->set_recname(recname);
                                            google::protobuf::int32 camamat = atoi(rowr.at(2).c_str());
                                            mrec->set_activemat(camamat);
                                            mrec->set_coordinates(rowr.at(3));
                                            google::protobuf::int32 rec = atoi(rowr.at(4).c_str());
                                            mcam->set_recognizing(rec);
                                            bool hasrecjob = true;
                                            mcam->set_hasrecjob(hasrecjob);
                                            mrec->set_storeimage(to_bool(rowr.at(5)));
                                            mrec->set_storevideo(to_bool(rowr.at(6)));
                                            mrec->set_codename(rowr.at(7));
                                            mrec->set_hasregion(to_bool(rowr.at(8)));
                                            google::protobuf::int32 delay = atoi(rowr.at(9).c_str());
                                            mrec->set_delay(delay); 
                                            mrec->set_startrectime(rowr.at(10));
                                            google::protobuf::int32 _id_recognition_setup = atoi(rowr.at(11).c_str());
                                            mrec->set_db_recognitionsetupid(_id_recognition_setup);
                                            mrec->set_runatstartup(to_bool(rowr.at(12)));
                                            std::string xmlpath = rowr.at(13).c_str();
                                            mrec->set_xmlfilepath(xmlpath);
                                            google::protobuf::int32 speed = atoi(rowr.at(14).c_str());
                                            mrec->set_speed(speed);
                                            bool hascron = to_bool(rowr.at(15));
                                            if (hascron)
                                            {
                                                vector<string> intervalse  = getIntervalsByCamberaAndRec(camera, recname);
                                                if (intervalse.size()>0)
                                                {
                                                    std::string timestart   = intervalse.at(0);
                                                    mrec->set_timestart(timestart);
                                                    std::string timeend     = intervalse.at(1);
                                                    mrec->set_timeend(timeend);
                                                } else
                                                {
                                                    mrec->set_hascron(false);
                                                }
  
                                            } else
                                            {
                                                mrec->set_hascron(false);
                                            }   
                                            google::protobuf::int32 activerec = atoi(rowr.at(16).c_str());
                                            mrec->set_activerec(activerec);
                                            mrec->set_matbasefile(rowr.at(17));
                                        }
                                    }
                                }
                            }
                        }
                    }
                }    
            }
        }
        cout << "SIGO: " << q << endl;
    }
    return m;
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
        msg_split_vector_comm.clear();
    }
    //cout << " count_sent__split : " << count_sent__split << endl;
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

            //cout << "sql_connection: " << sql_connection.str() << endl;

            pthread_mutex_lock(&databaseMutex);
            db_execute(sql_connection.str().c_str());
            pthread_mutex_unlock(&databaseMutex);

            cout << "****************************************" << endl;
            if (m.has_type())
                cout << "ENGAGE type            :" << m.type() << endl;
            if (m.has_time())
                cout << "ENGAGE time            :" << m.time() << endl;
            if (m.has_serverip())
                cout << "ENGAGE serverip        :" << m.serverip() << endl;
            if (m.has_dataamount())
            cout << "ENGAGE serverip            :" << m.dataamount() << endl;
            if (m.has_packagesize())
            cout << "ENGAGE packagesize         :" << m.packagesize() << endl;
            if (m.has_includethubmnails())
            cout << "ENGAGE includethubmnails   :" << m.includethubmnails() << endl;
            if (m.has_imagefilepath())
            cout << "ENGAGE imagefilepath       :" << m.imagefilepath() << endl;
            if (m.has_datafile())
            cout << "ENGAGE datafile            :" << m.datafile() << endl;
            if (m.has_devicestarttime())
            cout << "ENGAGE devicestarttime     :" << m.devicestarttime() << endl;
            if (m.has_dataamount())
            cout << "ENGAGE dataamount          :" << m.dataamount() << endl;
            if (m.has_datatotal())
            cout << "ENGAGE datatotal           :" << m.datatotal() << endl;
            cout << "****************************************" << endl;
        
            m.set_type(motion::Message::ENGAGE);

            m = getRefreshProto(m);
          
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
            int activecam = m.activecam();
            m.set_activecam(activecam);
            m = saveRecognition(m);
            
            m = postRecognition(m);
            
            m.set_type(motion::Message::SAVE_OK);
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
            motion::Message::MotionRec * prec = pcamera->mutable_motionrec(0);   
            
            std::string recname = prec->recname();  
            
            std::string xml_path = prec->xmlfilepath();     
            
            bool xmlexist = file_exists(xml_path);
            
            if (xmlexist)
            {
                string xml_loaded = get_file_contents(xml_path);
                cout << "xml_loaded: " << xml_loaded << endl;
                std:string encoded_xml = base64_encode(reinterpret_cast<const unsigned char*>(xml_loaded.c_str()),xml_loaded.length());
                m.set_datafile(encoded_xml.c_str());
                
                std::string strdecoded = base64_decode(encoded_xml);
                
                std::string path =  basepath + "/xml.txt";
                std::ofstream outxml;
                outxml.open (path.c_str());
                outxml << strdecoded << "\n";
                outxml.close();
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

            //if (i != std::string::npos)
            //    videofilepath.erase(0, s.size());
            
            //stringstream strmpath;
            //strmpath << sourcepath << videofilepath;

            //std::string path = strmpath.str();
            
            string filename = m.datafile();
        
            std::string command;
            command += "cat ";
            command += videofilepath; //path;
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
            m.set_datafile(oriencoded.c_str());
            
            m.set_type(motion::Message::GET_VIDEO);
            
        }
        break;
        
        case motion::Message::GET_IMAGE:
        {
            cout << "motion::Message::GET_IMAGE" << endl;
            std::string imagefilepath = m.imagefilepath();
            
            cv::Mat mat = getImageWithTextByPath(imagefilepath);
            
            m = serializeMediaToProto(m, mat);
            m.set_type(motion::Message::GET_IMAGE);
            m.set_activecam(m.activecam());
        }
        break;
        
        case motion::Message::GET_MAT:
        {
            cout << "motion::Message::GET_MAT" << endl;
            std::string imagefilepath = m.imagefilepath();
            cv::Mat mat = cv::imread(imagefilepath);
            m = serializeMediaToProto(m, mat);
            m.set_type(motion::Message::GET_MAT);
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
            motion::Message::MotionRec * mrec = mcamera->mutable_motionrec(0);
            
            std::string name = mrec->recname();
            stringstream camera;
            camera << mcamera->cameranumber();
            
            int activecam = m.activecam();
            
            bool load = loadStartQuery(camera.str(), name);
            
            if (load)
            {
                startMainRecognition(atoi(camera.str().c_str()));
            } else 
            {
                cout << "No matching values for the current arguments." << endl; 
            }
            
            updateRecStatusByRecName(1, mrec->recname());
            
            m.Clear();
            
            motion::Message ms;
            
            std::string _month = getCurrentMonthLabel();
            std::string _day = getCurrentDayLabel();
            
            cout << "_month: " << _month << " _day: " << _day << endl; 
            
            ms.set_type(motion::Message::REC_START);
            ms.set_activecam(activecam);
            m.set_currmonth(_month);
            m.set_currday(_day);
            
            m = getRefreshProto(ms);
            
            int camsize = m.motioncamera_size();
            cout << "camsize: " << camsize << endl;
        
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
            
            motion::Message::MotionRec * mrec = mcamera->mutable_motionrec(0);
                
            updateRecStatusByRecName(0, mrec->recname());
             
            mcamera->Clear();
            mrec->Clear();
            
            m = getRefreshProto(m);
            
            m.set_type(motion::Message::REC_STOP);
            
        }
        break;
        
        case motion::Message::TAKE_PICTURE:
        {
            cout << "motion::Message::TAKE_PICTURE" << endl;
            
            m.set_type(motion::Message::TAKE_PICTURE);
            m.set_serverip(PROTO.serverip());
            m.set_time(getTimeRasp());
            
            m = getRefreshProto(m);
            
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
            std::string result_message = m.time();
            
            cout << "coming time: "  << result_message  << endl;
            
            struct tm tmremote;
            char *bufr;
            bufr = new char[result_message.length() + 1];
            strcpy(bufr, result_message.c_str());
            cout << "bufr       : " << bufr << endl;
            memset(&tmremote, 0, sizeof(struct tm));
            strptime(bufr, "%Y-%m-%d %H:%M:%S %z", &tmremote);
            
            setTimeToRaspBerry(tmremote, 0);
                   
            m.set_type(motion::Message::TIME_SET);
            m.set_serverip(PROTO.serverip());
        }
        break;
        case motion::Message::SERVER_INFO:
        {
            cout << "motion::Message::SERVER_INFO" << endl;

            struct timeval tr;
            struct tm* ptmr;
            char time_rasp[40];
            gettimeofday (&tr, NULL);
            ptmr = localtime (&tr.tv_sec);
            strftime (time_rasp, sizeof (time_rasp), "%Y-%m-%d %H:%M:%S %z", ptmr);          
            
            motion::Message::MotionUser * muser = m.mutable_motionuser(0);
            
            /// WP BASE URL AND USER ID ///
            if (muser->has_wpserverurl())
                SERVER_BASE_URL = muser->wpserverurl();  
                
            if (muser->has_wpclientid()) 
                WP_CLIENT_ID = muser->wpclientid();  
    
            std::stringstream clientid;
            clientid << muser->wpclientid() << endl;

            std::stringstream postparent;
            postparent << muser->wpparent() << endl;

            std::stringstream timerasp;
            timerasp << time_rasp << endl;   

            if (PROTO.has_serverip())
            {
                m.set_serverip(PROTO.serverip());
            }
            m.set_type(motion::Message::SERVER_INFO_OK);   
            
            m = getRefreshProto(m);
            
        }
        break;
    }

    return m;
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
      
      //cout << "RECEIVE" << endl;
      
      GOOGLE_PROTOBUF_VERIFY_VERSION;
      motion::Message ms;
      ms.ParseFromArray(strdecoded.c_str(), strdecoded.size());
      
      //cout << "PARSE" << endl;
      
      int camamounts = ms.motioncamera_size();
        
      PROTO.Clear();
      PROTO = ms;
      
      google::protobuf::uint32 chunck_size = PROTO.packagesize();
      
      cout << "count_sent__split:" << count_sent__split << endl;
      //cout << "count vector size:" << count_vector_size << endl;
        
      value = ms.type();
      
      
      //IF USER INFO STORE PROTO TO DISK
      if (value == motion::Message::SERVER_INFO)
      {
          
        motion::Message::MotionUser * muser = ms.mutable_motionuser(0);
        int db_user_id = insertUserIntoDatabase(muser);               
          
        std::string trackdatafile = basepath + "data/client"; 
        directoryExistsOrCreate(trackdatafile.c_str());                  

        std::string path =  basepath + "data/client/client.dat";

        std::ofstream outuser;
        outuser.open (path.c_str());
        outuser << strproto << "\n";
        outuser.close();            
      }

      cout << "Proto size   : " << ms.ByteSize() << endl;

      if (ms.has_serverip())
        T_PROTO.set_serverip(ms.serverip());
        
      //cout << "value:: " << value << endl;
    
      if ( ms.type()==motion::Message::RESPONSE_OK || ms.type()==motion::Message::RESPONSE_END )
      {
          //cout << "response : " <<  ms.type() << endl;
          count_sent__split = 0;
          
          google::protobuf::ShutdownProtobufLibrary();
          
          return ms.type();
      }
      else if (ms.type()==motion::Message::RESPONSE_NEXT)
      {
          
          std:string payspl = msg_split_vector_comm.at(count_sent__split);
          
          string header =
          "PROSTA" +
            fixedLength(payspl.size() +40,  4)  + "::" +
            fixedLength(count_vector_size,  4)  + "::" +
            fixedLength(count_sent__split,  4)  + "::" +
            IntToString(inttype)                + "::" +
            IntToString(protofile)              +
          "PROSTO";
          
          msg = header + payspl;

          //cout << "header 2 : " << header << endl;
          //cout << "size: " << msg.size() << endl;
          //cout << "..........................................." << endl;
          //cout << msg << endl;
          //cout << "..........................................." << endl;
          
          totalsSocket();
          
          sock->send(msg.c_str(), msg.size());
          
          google::protobuf::ShutdownProtobufLibrary();
        
          return ms.type();
      }
      
      //Elaborate response.
      motion::Message m;
      
      //Run Command.
      m = runCommand(ms);
        
      //cout << "Serializing proto response." << endl;
      
      int m_amounts = m.motioncamera_size();
      
      //Split file outside proto.
      std::string datafile;
      if (m.has_datafile())
      {
          
        std::stringstream thubm;
        thubm << m.datafile() << endl;

        /*std::string thend = "THUMBNAILEND";

        if(strstr(thubm.str().c_str(), thend.c_str()))
        {
            cout << "THUMBNAILEND CONTEINED!!!" << endl;

            //cout << endl;
            //cout << thubm.str() << endl;

        } else 
        {
            cout << "THUMBNAILEND NOT CONTEINED!!!" << endl;
        }*/

        datafile = "PROFILE" + m.datafile();
        //cout << "::::::::::::::::::::::::::::::::::::::::::::::: " << endl;
        //cout << "m size 1: " << m.ByteSize() << endl;
        m.clear_datafile();
        //cout << "m size 2: " << m.ByteSize() << endl;
        //cout << "::::::::::::::::::::::::::::::::::::::::::::::: " << endl;
        protofile = motion::Message::PROTO_HAS_FILE;
  
      } else
      {
          protofile = motion::Message::PROTO_NO_FILE;
      }
        
      //Initialize objects to serialize.
      int size = m.ByteSize();
        
      char dataresponse[size];
      
      //cout << "active cam: " << m.activecam() << endl;
      //cout << "type: " << m.type() << endl;

      m.SerializeToArray(&dataresponse, size);
     
      //cout << "Encoding." << endl;
        
      std::string encoded_proto = base64_encode(reinterpret_cast<const unsigned char*>(dataresponse),sizeof(dataresponse));
        
      std::stringstream ssenc;
        
      if (protofile == motion::Message::PROTO_HAS_FILE)
      {
          //cout << "::::::::::::::::::::::::::::::::::::::::::::::: " << endl;
          ssenc << encoded_proto;
          //cout << "encoded_proto size ::1::  " << ssenc.str().size() << endl;
          ssenc << datafile;
          //cout << "encoded_proto size ::2::  " << ssenc.str().size() << endl;
          //cout << "::::::::::::::::::::::::::::::::::::::::::::::: " << endl;

          cout << "ssenc size: " << ssenc.str().size() << endl;
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
          
          //cout << "all_encoded.length(): " << all_encoded.length() << endl;
          //cout << "chunck_size: " << chunck_size << endl;
          
          for (unsigned i = 0; i < all_encoded.length(); i += chunck_size)
          {
              cout << "i: " << i << endl;
              msg_split_vector_comm.push_back(all_encoded.substr(i, chunck_size));
          }
          
          count_vector_size = msg_split_vector_comm.size();
          std::string payspl = msg_split_vector_comm.at(count_sent__split);
          
          header =
          "PROSTA" +
            fixedLength(payspl.size()+40,   4)  + "::" +
            fixedLength(count_vector_size,  4)  + "::" +
            fixedLength(count_sent__split,  4)  + "::" +
            IntToString(inttype)                + "::" +
            IntToString(protofile)              +
          "PROSTO";
          msg = header + payspl;

          //cout << "header 1 : " << header << endl;
          //cout << "size: " << msg.size() << endl;
          //cout << "..........................................." << endl;
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
          
          //cout << "header 0 : " << header << endl;
          //cout << "size: " << msg.size() << endl;
          //cout << "..........................................." << endl;
          
          //cout << msg << endl;
          //cout << "..........................................." << endl;
         
        }
     
        ms.Clear();
        //google::protobuf::ShutdownProtobufLibrary();
    
        totalsSocket();
        
        //cout << "Socket Sent Size: " << msg.size() << endl;
        
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