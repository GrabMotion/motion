
/*
 * File:   database.cpp
 * Author: jose
 *
 * Created on Julio 22, 2015, 11:23 AM
 */

#include "../database/database.h"

using namespace google::protobuf::io;

sqlite3 *db;
char *zErrMsg = 0;
int  cd, rc;

static int callback(void *NotUsed, int argc, char **argv, char **azColName)
{
    int i;
    for(i=0; i<argc; i++)
    {
        printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    }
    printf("\n");
    return 0;
}

/*void db_create()
{
    cd = sqlite3_open_v2("test.db", &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL );
    if (cd != SQLITE_OK) 
    {
        cerr << "SELECT failed: " << sqlite3_errmsg(db) << endl;
        //return NULL; // or throw
    }
}*/

void db_open()
{
    /* Open database */
    std::string dbpath = basepath + "database/motion.db";
    rc = sqlite3_open(dbpath.c_str(), &db);
    //rc = sqlite3_open("database/motion.db", &db);
    if ( rc )
    {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
    }
    //else{
    //    fprintf(stdout, "Opened database successfully\n");
    //}

}


vector<vector<string> > db_select(const char *sql, int columns)
{
    db_open();
    
    vector<vector<string> > resutl;
    
    sqlite3_stmt *stmt;
    rc = sqlite3_prepare_v2(db, sql,
                            -1, &stmt, NULL);
    if (rc != SQLITE_OK) 
    {
        cerr << "SELECT failed: " << sqlite3_errmsg(db) << endl;
        //return NULL; // or throw
    }
    
    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        int id = sqlite3_column_int(stmt, 0);
        vector<string> v;
        for (int j=0; j<columns; j++)
        {
            v.push_back(reinterpret_cast<const char*>(sqlite3_column_text(stmt, j)));
        }
        resutl.push_back(v);
    }
    sqlite3_finalize(stmt);
    
    db_close();
    
    return resutl;
    
}

void db_execute(const char * sql)
{
    db_open();
    
    rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);
    if( rc != SQLITE_OK )
    {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }
    //else
    //{
    //    fprintf(stdout, "Records created successfully\n");
    //}
    db_close();
}

void db_close()
{
    sqlite3_close(db);
}

void updateRecStatus(int status, int camera, std::string recname)
{
    
    struct timeval tr;
    struct tm* ptmr;
    char time_rasp[40];
    gettimeofday (&tr, NULL);
    ptmr = localtime (&tr.tv_sec);
    strftime (time_rasp, sizeof (time_rasp), "%Y-%m-%d %H:%M:%S %z", ptmr);
    
    stringstream sql_updatecameras;
    sql_updatecameras <<
    "UPDATE recognition_setup set recognizing = " << status << ", since = '" << time_rasp << "' "  <<
    "WHERE name = '" << recname << "';";
    cout << "sql_updatecameras: " << sql_updatecameras.str() << endl;
    pthread_mutex_lock(&databaseMutex);
    db_execute(sql_updatecameras.str().c_str());
    pthread_mutex_unlock(&databaseMutex);
    
}

int db_cpuinfo()
{

        std::ifstream cpuinfo("/proc/cpuinfo");
        std::string line;
        
        std::string model;
        std::string hardware;
        std::string revision;
        std::string serial;
        
        while (!cpuinfo.eof())
        {
            std::getline(cpuinfo,line);
            
            if (!line.size())
                continue;
            
            if(line.find("model name") != std::string::npos)
            {
                model = line;
            }
            else if (line.find("Hardware") != std::string::npos)
            {
                hardware = line;
            }
            else if (line.find("Revision") != std::string::npos)
            {
                revision = line;
            }
            else if (line.find("Serial") != std::string::npos)
            {
                serial = line;
            }
            
        }
        
        std::string testquery = "SELECT name FROM sqlite_master WHERE type='table';";
        vector<vector<string> > table_array = db_select(testquery.c_str(), 1);  
      
        model    = splitString(model, ":").at(1);
        std:string initmodel = model.substr(0, 1);
        
        if (initmodel == " ")
            model = model.substr(1, model.size());
        
        hardware    = splitString(hardware, ": ").at(1);
        revision    = splitString(revision, ": ").at(1);
        serial      = splitString(serial, ": ").at(1);
        
        stringstream sql;
        sql <<
        "INSERT INTO hardware (model, hardware, serial, revision) " <<
        "SELECT '"  << model        << "'"
        ", '"       << hardware     << "'"
        ", '"       << serial       << "'"
        ", '"       << revision     << "'" <<
        " WHERE NOT EXISTS (SELECT * FROM hardware WHERE model  = '" << model    << "' " <<
        " AND hardware  = '"    << hardware     << "' " <<
        " AND serial    = '"    << serial       << "' " <<
        " AND revision  = '"    << revision     << "' " << ");";
        std::string stdquery = sql.str().c_str();
        pthread_mutex_lock(&databaseMutex);
        db_execute(stdquery.c_str());
        pthread_mutex_unlock(&databaseMutex);
    
        std::string last_hardware_id_query = "SELECT MAX(_id) FROM hardware;";
        pthread_mutex_lock(&databaseMutex);
        vector<vector<string> > hardware_array = db_select(last_hardware_id_query.c_str(), 1);
        pthread_mutex_unlock(&databaseMutex);
        int db_hardware_id = atoi(hardware_array.at(0).at(0).c_str());
        cout << "db_hardware_id: " << db_hardware_id << endl;
         
        FILE *in;
	char buff[512];

	if(!(in = popen("df", "r"))){
		return 1;
	}

        std::vector<std::string>   result;
	while(fgets(buff, sizeof(buff), in)!=NULL)
        {
            stringstream bus;
            bus << buff;
            std::string dfline = bus.str();
            if (dfline.find("/dev/root") != std::string::npos)
            {
                std::stringstream  data(dfline);
                std::string dfline;
                while(std::getline(data, dfline,' '))
                {
                    if (!dfline.empty())
                        result.push_back(dfline); // Note: You may get a couple of blank lines
                                            // When multiple underscores are beside each other.
                }
            }
	}
	pclose(in);
        
        cout << "0: " << result.at(0) << endl;
        cout << "1: " << result.at(1) << endl;
        cout << "2: " << result.at(2) << endl;
        cout << "3: " << result.at(3) << endl;
        cout << "4: " << result.at(4) << endl;
        
        stringstream sql_update_hard;
        sql_update_hard <<
        "UPDATE hardware SET " <<
        "disktotal = "      << result.at(1)     << ", "
        "diskused = "       << result.at(2)     << ", "
        "diskailable = "    << result.at(3)     << ", "
        "disktotal = '"      << result.at(4)     << "' "
        "WHERE _id  = " << db_hardware_id    << ";";
        std::string stdhardquery = sql_update_hard.str().c_str();
        pthread_mutex_lock(&databaseMutex);
        db_execute(stdhardquery.c_str());
        pthread_mutex_unlock(&databaseMutex);
       
    return db_hardware_id;
    
    
}


std::vector<int> db_cams(std::vector<int> cams)
{
    
    std::vector<int> camsarray;
    
    stringstream sql_active_cam;
    sql_active_cam   <<
    "SELECT C._id FROM cameras AS C WHERE C.active = 1;";
    cout << "sql_active_cam: " << sql_active_cam.str() << endl;
    pthread_mutex_lock(&databaseMutex);
    vector<vector<string> > rec_active_cam = db_select(sql_active_cam.str().c_str(), 1);
    pthread_mutex_unlock(&databaseMutex);
    
    int id_active;
    bool hasactive;
    if (rec_active_cam.size()>0)
    {
        id_active = atoi(rec_active_cam.at(0).at(0).c_str());
        hasactive = true;
    }

    for(int i=0; i<cams.size(); i++)
    {
        
        struct timeval tr;
        struct tm* ptmr;
        char time_rasp[40];
        gettimeofday (&tr, NULL);
        ptmr = localtime (&tr.tv_sec);
        strftime (time_rasp, sizeof (time_rasp), "%Y-%m-%d %H:%M:%S %z", ptmr);
        
        std::stringstream command;
        command << "/sys/class/video4linux/video" << cams.at(i) << "/name";
        std::string file = command.str();
        
        stringstream nfile;
        nfile << basepath << "data/camera_" << cams.at(i) << ".txt";
        //nfile << "data/camera_" << cams.at(i) << ".txt";
        string newfile = nfile.str();
        
        std::ifstream  src(file.c_str(), std::ios::binary);
        std::ofstream  dst(newfile.c_str(),  std::ios::binary);
        dst << src.rdbuf();
        dst.close();
        
        std::string camera = get_file_contents(newfile);
        camera.erase(std::remove(camera.begin(), camera.end(), '\n'), camera.end());
        
        bool active = false;
        if (hasactive)
        {
            if (i==id_active)
            {
               active = true; 
            }
            
        } else 
        {
            if (i==0)
            {
                active = true;
            }
        }
      
        stringstream insert_camera_query;
        insert_camera_query <<
        "INSERT INTO cameras (number, name, active, created) " <<
        "SELECT " << cams.at(i) << ",'" << camera << "'," << active << ",'" << time_rasp << "' " << 
        " WHERE NOT EXISTS (SELECT * FROM cameras WHERE name = '" << camera << "');";
        cout << "insert_camera_query: " << insert_camera_query.str() << endl;
        pthread_mutex_lock(&databaseMutex);
        db_execute(insert_camera_query.str().c_str());
        pthread_mutex_unlock(&databaseMutex);
       
        std::string last_cameras_id_query = "SELECT MAX(_id) FROM cameras";
        pthread_mutex_lock(&databaseMutex);
        vector<vector<string> > cameras_array = db_select(last_cameras_id_query.c_str(), 1);
        pthread_mutex_unlock(&databaseMutex);
        int db_camera_id = atoi(cameras_array.at(0).at(0).c_str());
        cout << "db_camera_id: " << db_camera_id << endl;
        
        //updateCameraDB(0, cams.at(i));
        
        camsarray.push_back(db_camera_id);
        
    }
    
    return camsarray;
}

void status()  
{
     
    stringstream sql_status;
    sql_status << "SELECT " <<
    "RS.codename, "         << // 0   
    "RS.recognizing, "      << // 1
    "C.number, "            << // 2
    "C.name, "              << // 3
    "INT.timestart, "       << // 4   
    "INT.timeend, "         << // 5
    "RS.since "             << // 6
    "FROM interval AS INT " <<
    "JOIN rel_interval_crontab AS RIC ON RIC._id_interval = INT._id "                                           <<          
    "JOIN rel_camera_recognition_setup_interval_crontab RCRSIC ON RIC._id = RCRSIC._id_rel_interval_crontab "   <<
    "JOIN rel_camera_recognition_setup AS RCRS ON RCRS._id = RCRSIC._id_rel_camera_recognition_setup "          <<
    "JOIN recognition_setup AS RS ON RCRS._id_recognition_setup = RS._id "                                      <<
    "JOIN cameras AS C ON RCRS._id_camera = C._id "                                                             <<
    "GROUP BY INT._id AND RS._id;";
    pthread_mutex_lock(&databaseMutex);
    vector<vector<string> > status_array = db_select(sql_status.str().c_str(), 7);
    pthread_mutex_unlock(&databaseMutex);
    cout <<     ":::::::::::Motion Recognition Jobs::::::::::::::"   << endl;
    for (int i=0; i<status_array.size(); i++)
    {
        vector<string> rowj = status_array.at(i);
        
        std::string working;
        if (rowj.at(1)=="1")
            working = "YES";
        else
            working = "NO";
        
        cout << "Name           " << rowj.at(0)       << "   <" << endl;
        cout << "Recognizing    " << working          << endl;
        if (working == "YES")
            cout << "Since          " << rowj.at(6)   << endl;
        cout << "Startint at    " << rowj.at(4)       << endl;
        cout << "Stopping at    " << rowj.at(5)       << endl;
        if ( (i+1) < status_array.size())
            cout << "-----------------------------------" << endl; 
    }
    
    std::string sql_cameras = "SELECT C.number, C.name FROM cameras AS C;";
    pthread_mutex_lock(&databaseMutex);
    vector<vector<string> > cameras_array = db_select(sql_cameras.c_str(), 2);
    pthread_mutex_unlock(&databaseMutex);
    cout <<     ":::::::::::Motion Cameras::::::::::::::"   << endl;
    for (int i=0; i<cameras_array.size(); i++)
    {
        vector<string> rowc = cameras_array.at(i);   
        cout << "Camera         " << rowc.at(0)       << " " << rowc.at(1) << endl;
    }
    cout << "-----------------------------------" << endl; 
    
}

void getCamerasQuery()
{
    std::string sql_cameras = "SELECT C.number, C.name FROM cameras AS C;";
    pthread_mutex_lock(&databaseMutex);
    vector<vector<string> > cameras_array = db_select(sql_cameras.c_str(), 2);
    pthread_mutex_unlock(&databaseMutex);
    cout << "Cameras:"   << endl;
    for (int i=0; i<cameras_array.size(); i++)
    {
        vector<string> rowc = cameras_array.at(i);   
        cout << rowc.at(0) << " " << rowc.at(1) << endl;   
    }
}

bool loadStartQuery(std::string camera, std::string recname)
{
    
    stringstream sql_db_cam;
    sql_db_cam      << 
    "SELECT RS._id_camera, C.number FROM recognition_setup AS RS JOIN cameras AS C ON RS._id_camera = C._id WHERE RS.name = '" << recname << "';";
    pthread_mutex_lock(&databaseMutex);
    vector<vector<string> > db_cam_array = db_select(sql_db_cam.str().c_str(), 2);
    pthread_mutex_unlock(&databaseMutex);
    int db_cam_id = atoi(db_cam_array.at(0).at(0).c_str());
    int camnum = atoi(db_cam_array.at(0).at(1).c_str());
    cout << "db_cam_id: " << db_cam_id << endl;
    
    std::string _month = getCurrentMonthLabel();
    int db_month_id = insertMonthIntoDatabase(_month, db_cam_id);
    
    std::string _day = getCurrentDayLabel();
    
    std::string XML_FILE = "<import>session"; 
    
    std::string xml_path = getXMLFilePathAndName(camnum, recname, _day, XML_FILE);
  
    int db_dayid = insertDayIntoDatabase(_day, db_month_id);
    
    stringstream sql_rec_day_update;
    sql_rec_day_update <<
    "UPDATE recognition_setup set _id_day = " << db_dayid <<
    " WHERE name = '" << recname << "';";
    pthread_mutex_lock(&databaseMutex);
    db_execute(sql_rec_day_update.str().c_str());
    pthread_mutex_unlock(&databaseMutex);
    
    stringstream sql_load_recognition;
    sql_load_recognition << "SELECT " <<
    "RC.codename, "         << // 0
    "RC.has_region, "       << // 1
    "RC.runatstartup, "     << // 2
    "RC.delay, "            << // 3
    "RC.storeimage, "       << // 4
    "RC.storevideo, "       << // 5
    "RC._id_mat AS matid, " << // 6
    "CO.coordinates, "      << // 7
    "C._id, "               << // 8
    "C.name, "              << // 9
    "C.number, "            << // 10
    "RC.name, "             << // 11
    "M.matcols, "           << // 12 
    "M.matrows, "           << // 13 
    "M.matwidth, "          << // 14
    "M.matheight, "         << // 15
    "D._id, "               << // 16
    "MO._id, "              << // 17
    "RC.speed, "            << // 18           	
    "RC._id, "              << // 19             	
    "RC.xmlfilepath "       << // 20  
    "FROM recognition_setup RC "                                << 
    "JOIN coordinates AS CO ON RC._id_coordinates = CO._id "    <<
    "JOIN cameras AS C ON RC._id_camera = C._id "               <<
    "JOIN mat AS M ON RC._id_mat = M._id "                      <<
    "JOIN day AS D ON RC._id_day = D._id "                      <<
    "JOIN rel_month_day AS RMD ON D._id = RMD._id_day "         <<
    "JOIN month AS MO ON RMD._id_month = MO._id "                 <<        
    "WHERE C.number = " << camera << " AND RC.name = '" << recname << "';";

    cout << "sql_load_recognition: " << sql_load_recognition.str() << endl;
    pthread_mutex_lock(&databaseMutex);
    vector<vector<string> > start_array = db_select(sql_load_recognition.str().c_str(), 21);
    pthread_mutex_unlock(&databaseMutex);
    int size = start_array.size();
    if (size>0)
    {
        vector<string> rows = start_array.at(0);

        motion::Message m;
        
        google::protobuf::int32 activecam = atoi(rows.at(8).c_str());
        m.set_activecam(activecam);
        
        google::protobuf::int32 cam32 = atoi(camera.c_str());
        m.set_activecam(cam32);

        m.set_type(motion::Message_ActionType_REC_START);
        motion::Message::MotionCamera * mcamera = m.add_motioncamera();
        
        mcamera->set_recognizing(true);

        motion::Message::MotionRec * mrec = mcamera->add_motionrec();
        
        bool hasregion = to_bool(rows.at(1));
        mrec->set_hasregion(hasregion);
        if (hasregion)
        {
            std::string res = rows.at(7);
            std::string resencoded = base64_encode(reinterpret_cast<const unsigned char*>(res.c_str()), res.length());
            mrec->set_coordinates(resencoded.c_str());
        }
        
        std::string codename = rows.at(0);
        mrec->set_codename(codename);
        
        mcamera->set_fromdatabase(true);
        
        mrec->set_runatstartup(to_bool(rows.at(2)));
        
        google::protobuf::int32 delay = atoi(rows.at(3).c_str());
        mrec->set_delay(delay);
        
        mrec->set_storeimage(to_bool(rows.at(4)));
        
        mrec->set_storevideo(to_bool(rows.at(5)));
        
        google::protobuf::int32 dbmat = atoi(rows.at(11).c_str());
        mrec->set_db_idmat(dbmat);
        
        google::protobuf::int32 dbidcamera = atoi(rows.at(8).c_str());
        mcamera->set_db_idcamera(dbidcamera);

        mcamera->set_cameraname(rows.at(9));
        
        google::protobuf::int32 camnum = atoi(rows.at(10).c_str());
        mcamera->set_cameranumber(camnum);
        
        mrec->set_recname(rows.at(11));
        
        google::protobuf::int32 matcols = atoi(rows.at(12).c_str());
        mrec->set_matcols(matcols);
        
        google::protobuf::int32 matrows = atoi(rows.at(13).c_str());
        mrec->set_matrows(matrows);
     
        std::string _day = getCurrentDayLabel();
        m.set_currday(_day);
        
        std::string _month = getCurrentMonthLabel();
        m.set_currmonth(_month);
        
        google::protobuf::int32 dbidday = atoi(rows.at(16).c_str());
        mrec->set_db_idday(dbidday);
        
        google::protobuf::int32 dbidmonth = atoi(rows.at(17).c_str());
        mrec->set_db_idmonth(dbidmonth);
        
        google::protobuf::int32 speed = atoi(rows.at(18).c_str());
        mrec->set_speed(speed);
        
        google::protobuf::int32 recid = atoi(rows.at(19).c_str());
        mrec->set_db_recognitionsetupid(recid);
        
        mrec->set_xmlfilepath(rows.at(20).c_str());
        
        stringstream sql_last_instance;
        sql_last_instance   <<
        "SELECT coalesce( MAX(I.number), 0) FROM instance AS I "                                <<
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
        
        std::string last = lastinstance_array.at(0).at(0).c_str();
        int ln = atoi(last.c_str());
        if (ln>0)
        {
            mrec->set_lastinstance(last);
        } else 
        {
            mrec->set_lastinstance("0");
        }
        
        PROTO.Clear();
        PROTO = m;
        
        return true;
    
    } 
    else 
    {
        return false;
    }
}

vector<string> getMaxImageByPath(google::protobuf::int32 instanceid)
{
    vector<string> maximage;
    
    stringstream check_rel_day_instance_recognition_setup;
    check_rel_day_instance_recognition_setup <<
    "SELECT _id FROM rel_day_instance_recognition_setup WHERE _id_instance = " << instanceid << ";";
    
    pthread_mutex_lock(&databaseMutex);
    vector<vector<string> > rel_day_instance_recognition_setup_array = db_select(check_rel_day_instance_recognition_setup.str().c_str(), 1);
    pthread_mutex_unlock(&databaseMutex);
    
    if (rel_day_instance_recognition_setup_array.size()>0)
    {
    
        stringstream check_istances;
        check_istances <<
        "SELECT "                   <<
        "MAX(I.imagechanges), "     <<
        "I.path, "                  <<
        "I.name, "                  <<
        "D.label, "                 <<
        "RS.name, "                 << 
        "I.time "                   <<
        "FROM instance AS INS "     <<
        "JOIN rel_instance_image AS RII ON INS._id = RII._id_instance " <<
        "JOIN image AS I ON RII._id_image = I._id "                     <<
        "JOIN rel_day_instance_recognition_setup AS RDIRS ON RII._id_instance = RDIRS._id_instance "    <<
        "JOIN day AS D ON RDIRS._id_day = D._id "                                                       <<
        "JOIN recognition_setup AS RS ON RDIRS._id_recognition_setup = RS._id "                         <<  
        "WHERE INS._id = " << instanceid << ";";        

        cout << "check_istances: " << check_istances.str() << endl;

        pthread_mutex_lock(&databaseMutex);
        vector<vector<string> > image_array = db_select(check_istances.str().c_str(), 6);
        pthread_mutex_unlock(&databaseMutex); 

        if (image_array.size()>0)
        {
            std::string path = image_array.at(0).at(1);
            maximage.push_back(path);
            std::string name = image_array.at(0).at(2);
            maximage.push_back(name);
            std::string day = image_array.at(0).at(3);
            maximage.push_back(day);
            std::string rec = image_array.at(0).at(4);
            maximage.push_back(rec);
            std::string time = image_array.at(0).at(5);
            maximage.push_back(time);   
        }
    }
    return maximage;  
}

int insertTracking(int db_instance_id, std::string maximagepath, int db_srv_idmedia, int db_srv_idpost)
{
    stringstream sql_insert_tracking;
    sql_insert_tracking <<
    "INSERT INTO tracking (_id_db_instance, imagepath, _id_dbsrv_media, _id_dbsrv_post) "   <<
    "SELECT " << db_instance_id << ",'"  << maximagepath << "', " << db_srv_idmedia << ", " << db_srv_idpost <<
    " WHERE NOT EXISTS (SELECT * FROM tracking WHERE _id_db_instance = " << db_instance_id  << ");";
    pthread_mutex_lock(&databaseMutex);
    db_execute(sql_insert_tracking.str().c_str());
    pthread_mutex_unlock(&databaseMutex); 
    
    stringstream sql_update_instance;
    sql_update_instance <<
    "UPDATE instance SET tracked = 1 " <<
    "WHERE _id = " << db_instance_id << ";";
    pthread_mutex_lock(&databaseMutex);
    db_execute(sql_update_instance.str().c_str());
    pthread_mutex_unlock(&databaseMutex); 
    
    std::stringstream last_tracking_query;
    last_tracking_query << "SELECT _id FROM tracking WHERE _id_db_instance = " << db_instance_id << ";";
    pthread_mutex_lock(&databaseMutex);
    vector<vector<string> > tracking_array = db_select(last_tracking_query.str().c_str(), 1);
    pthread_mutex_unlock(&databaseMutex);
    int db_tracking_id;
    if (tracking_array.size()>0)
    {
        db_tracking_id= atoi(tracking_array.at(0).at(0).c_str());
        cout << "db_tracking_id: " << db_tracking_id << endl;
    } else 
    {
        db_tracking_id = 0;
    }
    return db_tracking_id;
    
}

vector<vector<string> > getNotTrackedInstance()
{
    std::string not_tracker_instance = "SELECT _id FROM instance WHERE tracked =0;";
    pthread_mutex_lock(&databaseMutex);
    vector<vector<string> > not_tracker_instance_array = db_select(not_tracker_instance.c_str(), 1);
    pthread_mutex_unlock(&databaseMutex);
    return not_tracker_instance_array;
}

vector<string> getImageByPath(std::string path)
{
    vector<string> rows;
     
    stringstream check_image;
    check_image         <<
    "SELECT "           <<
    "I.name, "          <<
    "I.imagechanges, "  <<
    "I.time, "          <<
    "C.rect, "          <<
    "M.matrows, "       <<
    "M.matcols, "       <<
    "CA.name "          <<        
    "FROM image I "     <<
    "JOIN rel_instance_image AS RII ON I._id = RII._id_image " <<
    "JOIN rel_day_instance_recognition_setup AS RDIRS ON RII._id_instance = RII._id_instance " <<
    "JOIN recognition_setup AS RS ON RDIRS._id_recognition_setup = RS._id " <<
    "JOIN crop AS C ON I._id = C._id_image_father " <<
    "JOIN mat as M ON RS._id_mat = M._id "          <<    
    "JOIN cameras AS CA ON RS._id_camera = CA._id " <<       
    "WHERE I.path = '" << path << "' GROUP BY I.imagechanges;";        
            
    cout << "check_image: " << check_image.str() << endl;
    vector<vector<string> > run_array = db_select(check_image.str().c_str(), 7);
    if (run_array.size()>0)
    {
        rows = run_array.at(0);
    }
    return rows;
}

vector<string> startIfNotRunningQuery(std::string camera, char * time)
{
    vector<string> rows;
     
    //If time in interval and not recording
    stringstream check_run_strart;
    check_run_strart    <<
    "SELECT "   << 
    "RC._id, "  <<
    "RC.name "  <<         
    "FROM recognition_setup RC "    <<
    "JOIN cameras AS C ON RC._id_camera = C._id "   <<
    "WHERE RC.runatstartup  = 1 "                   <<
    "AND RC.recognizing     = 0 "                   <<
    "AND C.number = "    << camera << " "           <<
    "AND (SELECT COUNT(*) FROM interval AS I "              <<
    "WHERE strftime('%H-%M-%S', '" << time << "') "    <<
    "BETWEEN I.timestart AND I.timeend) = 1;"; 
    
    cout << "check_running_strart: " << check_run_strart.str() << endl;
    vector<vector<string> > run_array = db_select(check_run_strart.str().c_str(), 2);
    if (run_array.size()>0)
    {
        rows = run_array.at(0);
    }
    
    return rows;
    
}

int insertMonthIntoDatabase(std::string str_month, int db_camera_id)
{
    int db_month_id;
    
    stringstream sql_exist_month;
    sql_exist_month <<
    "SELECT CASE WHEN EXISTS "      <<
    "(SELECT * FROM month AS M JOIN rel_camera_month RCM ON M._id = RCM._id_month WHERE label = '" << str_month << "' AND RCM._id_camera = " << db_camera_id << " ) " <<
    "THEN CAST(1 AS BIT) ELSE CAST(0 AS BIT) END;";  
    cout << "sql_exist_month: " << sql_exist_month.str() << endl;
    pthread_mutex_lock(&databaseMutex);
    vector<vector<string> > month_array = db_select(sql_exist_month.str().c_str(), 1);
    pthread_mutex_unlock(&databaseMutex);
    int has_month = atoi(month_array.at(0).at(0).c_str());
    
    if (has_month==0)
    {
        //month database.
        stringstream sql_month;
        sql_month <<
        "INSERT INTO month (label) "       <<
        "SELECT '" << str_month << "' "  <<
        "WHERE NOT EXISTS (SELECT * FROM month WHERE label = '" + str_month + "');";
        db_execute(sql_month.str().c_str());
        std::string last_month_id_query = "SELECT MAX(_id) FROM month";
        pthread_mutex_lock(&databaseMutex);
        vector<vector<string> > month_array = db_select(last_month_id_query.c_str(), 1);
        pthread_mutex_unlock(&databaseMutex);
        db_month_id = atoi(month_array.at(0).at(0).c_str());
        cout << "db_month_id: " << db_month_id << endl;

        //rel_camera_month.
        stringstream sql_rel_camera_month;
        sql_rel_camera_month <<
        "INSERT INTO rel_camera_month (_id_camera, _id_month) "       <<
        "SELECT " << db_camera_id << ", "  << db_month_id <<
        " WHERE NOT EXISTS (SELECT * FROM rel_camera_month WHERE _id_camera = " << db_camera_id <<
        " AND _id_month = " << db_month_id << ");";
        pthread_mutex_lock(&databaseMutex);
        db_execute(sql_rel_camera_month.str().c_str());
        pthread_mutex_unlock(&databaseMutex); 
        // get rel id.
        std::string last_rel_camera_month_query = "SELECT MAX(_id) FROM rel_camera_month";
        pthread_mutex_lock(&databaseMutex);
        vector<vector<string> > rel_camera_month_array = db_select(last_rel_camera_month_query.c_str(), 1);
        pthread_mutex_unlock(&databaseMutex);
        int db_rel_camera_month_id = atoi(rel_camera_month_array.at(0).at(0).c_str());
        cout << "db_rel_camera_month_id: " << db_rel_camera_month_id << endl;
        
    } else if (has_month==1)
    {
        
        stringstream sql_month;
        sql_month <<
        "SELECT _id FROM month where label = '" << str_month << "';";
        pthread_mutex_lock(&databaseMutex);
        vector<vector<string> > month = db_select(sql_month.str().c_str(), 1);
        pthread_mutex_unlock(&databaseMutex);
        db_month_id = atoi(month.at(0).at(0).c_str());
       
    }
    
    return db_month_id;
}
 
int insertRegionIntoDatabase(std::string rcoords)
{
    std::replace( rcoords.begin(), rcoords.end(), '\n', ' ');
    stringstream sql_coord;
    sql_coord <<
    "INSERT INTO coordinates (coordinates) " <<
    "VALUES ('" << rcoords    << "');";
    pthread_mutex_lock(&databaseMutex);
    db_execute(sql_coord.str().c_str());
    std::string last_coordinates_query = "SELECT MAX(_id) FROM coordinates";
    vector<vector<string> > coords_array = db_select(last_coordinates_query.c_str(), 1);
    pthread_mutex_unlock(&databaseMutex);
    cout << "coords_array: " << endl;
    int db_coordnates_id = atoi(coords_array.at(0).at(0).c_str());
    cout << "db_coordnates_id: " << db_coordnates_id << endl;
    return db_coordnates_id;
}

void updateRegionIntoDatabase(std::string rcoords, int db_recognitionid)
{
    std::replace( rcoords.begin(), rcoords.end(), '\n', ' ');
    stringstream sql_coord_update;
    sql_coord_update <<
    "UPDATE coordinates set coordinates = '" << rcoords << "' " <<
    "WHERE _id = " << db_recognitionid << ";";
    pthread_mutex_lock(&databaseMutex);
    db_execute(sql_coord_update.str().c_str());
    pthread_mutex_unlock(&databaseMutex);
}

int insertDayIntoDatabase(std::string str_day, int db_month_id)
{
    
    int db_day_id;
    
    stringstream sql_exist_day;
    sql_exist_day <<
    "SELECT CASE WHEN EXISTS (SELECT * FROM [day] WHERE label = '" << str_day << "') " <<
    "THEN CAST(1 AS BIT) ELSE CAST(0 AS BIT) END";
    pthread_mutex_lock(&databaseMutex);
    vector<vector<string> > day_array = db_select(sql_exist_day.str().c_str(), 1);
    pthread_mutex_unlock(&databaseMutex);
    int has_day = atoi(day_array.at(0).at(0).c_str());
    
    if (has_day==0)
    {    
        vector<vector<string> > day_array;
        //Day database.
        stringstream sql_day;
        sql_day <<
        "INSERT INTO day (label) " <<
        "SELECT '" << str_day << "' " <<
        "WHERE NOT EXISTS (SELECT * FROM day WHERE label = '" << str_day    << "');";
        pthread_mutex_lock(&databaseMutex);
        db_execute(sql_day.str().c_str());
        pthread_mutex_unlock(&databaseMutex);
        std::string last_day_id_query = "SELECT MAX(_id) FROM day";
        pthread_mutex_lock(&databaseMutex);
        day_array = db_select(last_day_id_query.c_str(), 1);
        pthread_mutex_unlock(&databaseMutex);
        db_day_id = atoi(day_array.at(0).at(0).c_str());
        //cout << "db_day_id: " << db_day_id << endl;

        //Rel day month.
        stringstream sql_rel_day_month;
        sql_rel_day_month <<
        "INSERT INTO rel_month_day (_id_month, _id_day) " <<
        "SELECT " << db_month_id << ", " << db_day_id << 
        " WHERE NOT EXISTS (SELECT * FROM rel_month_day WHERE _id_month = " << db_month_id <<
        " AND _id_day = " << db_day_id << ");";
        pthread_mutex_lock(&databaseMutex);
        db_execute(sql_rel_day_month.str().c_str());
        pthread_mutex_unlock(&databaseMutex);
    
    } else if (has_day==1)
    {
        stringstream sql_day;
        sql_day <<
        "SELECT _id FROM day where label = '" << str_day << "';";
        pthread_mutex_lock(&databaseMutex);
        vector<vector<string> > day = db_select(sql_day.str().c_str(), 1);
        pthread_mutex_unlock(&databaseMutex);
        db_day_id = atoi(day.at(0).at(0).c_str());  
    }
    
    return db_day_id;
}

int insertIntervalCrontabIntoDatabase(motion::Message::MotionCamera * pcamera, motion::Message::MotionRec * prec, int camera_recognition_setupl_array)
{
    //Alarm Interval Start End
    stringstream sql_interval;
    sql_interval <<
    "INSERT INTO interval (timestart, timeend) " <<
    "SELECT '" << prec->timestart() << "', '" << prec->timeend() << "' " << 
    "WHERE NOT EXISTS (SELECT * FROM interval WHERE timestart = '" << prec->timestart() << "' " <<
    "AND timeend = '" << prec->timeend() << "');";
    
    pthread_mutex_lock(&databaseMutex);
    db_execute(sql_interval.str().c_str());
    pthread_mutex_unlock(&databaseMutex);
    
    vector<vector<string> > interval_array;
    std::string last_interval_id_query = "SELECT MAX(_id) FROM interval";
    pthread_mutex_lock(&databaseMutex);
    interval_array = db_select(last_interval_id_query.c_str(), 1);
    pthread_mutex_unlock(&databaseMutex);
    int db_interval_id = atoi(interval_array.at(0).at(0).c_str());
    
    int cronsize = prec->motioncron_size();
    
    for (int i = 0; i < cronsize; i++)
    {
        motion::Message::MotionCron * pcronstart = prec->mutable_motioncron(i);
        
        stringstream sql_interval;
        sql_interval <<
        "INSERT INTO crontab (command, program) " <<
        "SELECT '" << pcronstart->command() << "', '" << pcronstart->program() << "' " << 
        "WHERE NOT EXISTS (SELECT * FROM crontab WHERE command = '" << pcronstart->command() << "' " <<
        "AND program = '" << pcronstart->program() << "');";
        cout << "sql_interval: " << sql_interval.str() << endl;
        pthread_mutex_lock(&databaseMutex);
        db_execute(sql_interval.str().c_str());
        pthread_mutex_unlock(&databaseMutex);
       
        stringstream command;
        command <<
        "crontab -l ; echo '" << pcronstart->command() << " " << pcronstart->program()<<  "' | crontab -";
        cout << "command: " << command.str() << endl;
        char *cstr = new char[command.str().length() + 1];
        strcpy(cstr, command.str().c_str());
        exec_command(cstr);
        
        vector<vector<string> > crontab_array;
        std::string last_crontab_id_query = "SELECT MAX(_id) FROM crontab";
        pthread_mutex_lock(&databaseMutex);
        crontab_array = db_select(last_crontab_id_query.c_str(), 1);
        pthread_mutex_unlock(&databaseMutex);
        int db_crontab_id = atoi(crontab_array.at(0).at(0).c_str());
        
        stringstream sql_interval_crontab;
        sql_interval_crontab <<
        "INSERT INTO rel_interval_crontab (_id_interval, _id_crontab) " <<
        "SELECT " << db_interval_id << ", " << db_crontab_id << " " << 
        "WHERE NOT EXISTS (SELECT * FROM rel_interval_crontab WHERE _id_interval = " << db_interval_id << " " <<
        "AND _id_crontab = " << db_crontab_id << ");";
        pthread_mutex_lock(&databaseMutex);
        db_execute(sql_interval_crontab.str().c_str());
        pthread_mutex_unlock(&databaseMutex);
        
        vector<vector<string> > interval_crontab_array;
        std::string last_interval_crontab_id_query = "SELECT MAX(_id) FROM rel_interval_crontab";
        pthread_mutex_lock(&databaseMutex);
        interval_crontab_array = db_select(last_interval_crontab_id_query.c_str(), 1);
        pthread_mutex_unlock(&databaseMutex);
        int db_rel_interval_crontab = atoi(interval_crontab_array.at(0).at(0).c_str());
        
        stringstream sql_camera_recognition_setup_interval_crontab;
        sql_camera_recognition_setup_interval_crontab <<
        "INSERT INTO rel_camera_recognition_setup_interval_crontab (_id_rel_interval_crontab, _id_rel_camera_recognition_setup) " <<
        "SELECT " << db_rel_interval_crontab << ", " << camera_recognition_setupl_array << " " << 
        "WHERE NOT EXISTS (SELECT * FROM rel_camera_recognition_setup_interval_crontab WHERE _id_rel_interval_crontab = " << db_rel_interval_crontab << " " <<
        "AND _id_rel_camera_recognition_setup = " << camera_recognition_setupl_array << ");";
        pthread_mutex_lock(&databaseMutex);
        db_execute(sql_camera_recognition_setup_interval_crontab.str().c_str());
        pthread_mutex_unlock(&databaseMutex);
      
    }
}

/*void updateIntervalCrontabIntoDatabase(motion::Message::MotionCamera * pcamera)
{
    stringstream sql_interval_update;
    sql_interval_update <<
    "UPDATE interval set timestart = '" << pcamera->timestart() << "', "
    " timeend = '" << pcamera->timeend()        << "' " <<
    "WHERE _id = " << pcamera->db_intervalid()  << ";";        
    pthread_mutex_lock(&databaseMutex);
    db_execute(sql_interval_update.str().c_str());
    pthread_mutex_unlock(&databaseMutex);
}*/

int insertIntoRecognitionSetup(
        motion::Message::MotionRec * prec, 
        int db_day_id,
        int db_camera_id,
        int db_coordnates_id,
        std::string xmlfilepath)
{
    
    stringstream sql_recognition_setup_active_update;
    sql_recognition_setup_active_update <<
    "UPDATE recognition_setup set activerec = 0 WHERE _id_camera = " << db_camera_id << ";";
    pthread_mutex_lock(&databaseMutex);
    db_execute(sql_recognition_setup_active_update.str().c_str());
    pthread_mutex_unlock(&databaseMutex);   
    
    cout << "id_mat: " << prec->db_idmat() << endl;
    
    //recognition_setup database.
    stringstream sql_recognition_setup;
    sql_recognition_setup <<
    "INSERT INTO recognition_setup " <<
    "(name, "                   <<
            "activerec, "       <<
            "_id_day, "         <<
            "_id_camera, "      <<
            "_id_mat, "         <<
            "storeimage, "      <<
            "storevideo, "      <<
            "codename, "        <<
            "has_region, "      <<
            "has_cron, "        <<
            "_id_coordinates, " <<
            "delay, "           <<
            "speed, "           <<
            "xmlfilepath, "     <<
            "runatstartup) "    <<
    "SELECT "            << "'"      << 
    prec->recname()      << "', "    <<
    prec->activerec()    << ", "     <<        
    db_day_id            << ", "     <<
    db_camera_id         << ", "     <<
    prec->db_idmat()     << ", "     <<
    prec->storeimage()   << ", "     <<
    prec->storevideo()   << ", '"    <<
    prec->codename()     << "' ,"    <<
    prec->hasregion()    << ", "     <<
    prec->hascron()      << ", "     <<        
    db_coordnates_id     << ", "     <<
    prec->delay()        << ", "     <<
    prec->speed()        << ", '"    <<
    xmlfilepath          << "', "    <<        
    prec->runatstartup() <<    
    " WHERE NOT EXISTS (SELECT * FROM recognition_setup WHERE"       <<
    " name              = '"    << prec->recname()       << "' AND"  <<
    " activerec         = "     << prec->activerec()     << " AND"   <<
    " _id_day           = "     << db_day_id             << " AND"   <<
    " _id_camera        = "     << db_camera_id          << " AND"   <<
    " _id_mat           = "     << prec->db_idmat()      << " AND"   <<
    " storeimage        = "     << prec->storeimage()    << " AND"   <<
    " storevideo        = "     << prec->storevideo()    << " AND"   <<
    " codename          = '"    << prec->codename()      << "' AND"  <<
    " has_region        = "     << prec->hasregion()     << " AND"   <<
    " has_cron          = "     << prec->hascron()       << " AND"   <<
    " speed             = "     << prec->speed()         << " AND"   <<
    " xmlfilepath       = '"    << xmlfilepath           << "' AND"  <<
    " runatstartup      = "     << prec->runatstartup()  << ");";
    std::string sqlrecsetup = sql_recognition_setup.str();
    cout << "rec setup sql: " << sqlrecsetup << endl;
    pthread_mutex_lock(&databaseMutex);
    db_execute(sqlrecsetup.c_str());
    pthread_mutex_unlock(&databaseMutex);
    std::string last_recognition_setup_id_query = "SELECT MAX(_id) FROM recognition_setup";
    pthread_mutex_lock(&databaseMutex);
    vector<vector<string> > recognition_setup_array = db_select(last_recognition_setup_id_query.c_str(), 1);
    pthread_mutex_unlock(&databaseMutex);
    int db_recognition_setup_id = atoi(recognition_setup_array.at(0).at(0).c_str());
    
    return db_recognition_setup_id;
}

void updateRecognitionSetup(int db_idcamera, motion::Message::MotionRec * prec, motion::Message::MotionDay * pday)
{
    stringstream sql_recognition_setup;
    sql_recognition_setup <<
    "UPDATE recognition_setup set "                  <<
    "name = '"          << prec->recname()           << "', "    <<
    "_id_day = "        << pday->db_dayid()          << ", "     <<
    "_id_camera = "     << db_idcamera               << ", "     <<        
    "_id_mat = "        << prec->db_idmat()          << ", "     <<
    "storeimage = "     << prec->storeimage()        << ", "     <<
    "storevideo = "     << prec->storevideo()        << ", "     <<
    "codename = '"      << prec->codename()          << "', "    <<          
    "has_region = "     << prec->hasregion()         << ", "     <<
    "_id_coordinates = "<< prec->db_idcoordinates()  << ", "     <<
    "delay = "          << prec->delay()             << ", "     <<        
    "runatstartup = "   << prec->runatstartup()      << ", "     <<        
    "WHERE _id = "      << prec->db_recognitionsetupid()         << ";";                
    pthread_mutex_lock(&databaseMutex);
    db_execute(sql_recognition_setup.str().c_str());
    pthread_mutex_unlock(&databaseMutex);    
}

int insertIntoRelCameraRecognitionSetup(char * time_rasp, int db_recognitionsetup_id, int db_camera_id)
{
    //rel_camera_month.
    stringstream sql_rel_camera_recognition_setup;
    sql_rel_camera_recognition_setup <<
    "INSERT INTO rel_camera_recognition_setup (_id_camera, _id_recognition_setup, start_rec_time) " <<
    "SELECT " << db_camera_id << ", "  << db_recognitionsetup_id << ", '" << time_rasp << "' " << 
    " WHERE NOT EXISTS (SELECT * FROM rel_camera_recognition_setup WHERE _id_camera = " << db_camera_id <<
    " AND _id_recognition_setup = " << db_recognitionsetup_id << ");";
    cout << "sql_rel_camera_recognition_setup: " << sql_rel_camera_recognition_setup.str() << endl;
    pthread_mutex_lock(&databaseMutex); 
    db_execute(sql_rel_camera_recognition_setup.str().c_str());
    pthread_mutex_unlock(&databaseMutex);
    
    std::string last_rel_camera_recognition_setup = "SELECT MAX(_id) FROM rel_camera_recognition_setup";
    pthread_mutex_lock(&databaseMutex);
    vector<vector<string> > camera_recognition_setupl_array = db_select(last_rel_camera_recognition_setup.c_str(), 1);
    pthread_mutex_unlock(&databaseMutex);
    int db_camera_recognition_setupl_array = atoi(camera_recognition_setupl_array.at(0).at(0).c_str());
    cout << "db_camera_recognition_setupl_array: " << db_camera_recognition_setupl_array << endl;
    
    return db_camera_recognition_setupl_array;
}

void updateCameraMonth(char * time_rasp, int db_recognitionsetupid)
{
    stringstream sql_cameramonth_update;
    sql_cameramonth_update <<
    "UPDATE rel_camera_recognition_setup set "      <<
    "start_rec_time = '"  << time_rasp << "', "     <<
    "WHERE _id = "        << db_recognitionsetupid  << ";";                
    pthread_mutex_lock(&databaseMutex);
    db_execute(sql_cameramonth_update.str().c_str());
    pthread_mutex_unlock(&databaseMutex);    
}

vector<string> getIntervalsByCamberaAndRec(std::string camera, std::string recname)
{
    vector<string> intervals;
    stringstream sql_intervals;
    sql_intervals       <<   
    "SELECT "               <<
    "INT.timestart, "       <<
    "INT.timeend "          <<
    "FROM interval AS INT " <<
    "JOIN rel_interval_crontab AS RIC ON RIC._id_interval = INT._id "                                           <<
    "JOIN rel_camera_recognition_setup_interval_crontab RCRSIC ON RIC._id = RCRSIC._id_rel_interval_crontab "   <<
    "JOIN rel_camera_recognition_setup AS RCRS ON RCRS._id = RCRSIC._id_rel_camera_recognition_setup "          <<
    "JOIN recognition_setup AS RS ON RCRS._id_recognition_setup = RS._id "                                      <<
    "JOIN cameras AS C ON RCRS._id_camera = C._id "                                                             <<
    "WHERE C._id = " << camera << " AND RS.name = '" << recname << "' "                                         << 
    "GROUP BY INT._id;";
    pthread_mutex_lock(&databaseMutex);
    vector<vector<string> > interval_array = db_select(sql_intervals.str().c_str(), 2);
    pthread_mutex_unlock(&databaseMutex);
    
    intervals.push_back(interval_array.at(0).at(0));
    intervals.push_back(interval_array.at(0).at(1));
    
    return intervals;
}


void insertIntoHost(std::string publicip, std::string hostname, std::string city, std::string region, std::string country, std::string loc, std::string org)
{
    struct timeval tr;
    struct tm* ptmr;
    char time_rasp[40];
    gettimeofday (&tr, NULL);
    ptmr = localtime (&tr.tv_sec);
    strftime (time_rasp, sizeof (time_rasp), "%Y-%m-%d %H:%M:%S %z", ptmr);
    
    //rel_camera_month.
    stringstream sql_host;
    sql_host <<
    "INSERT INTO host (publicip, hostname, city, region, country, location, organization, time) " <<
    "SELECT '" << publicip << "', '"  << hostname << "', '" << city << "' " << region << "' " << country << "' " << loc << "' " << org << "'" << time_rasp << "'" << 
    " WHERE NOT EXISTS (SELECT * FROM host WHERE publicip = '" << publicip << "');";
    cout << "sql_host: " << sql_host.str() << endl;
    pthread_mutex_lock(&databaseMutex); 
    db_execute(sql_host.str().c_str());
    pthread_mutex_unlock(&databaseMutex);
}


// INSTANCE 

int insertIntoInstance(std::string number, motion::Message::Instance * pinstance, const char * time_info, int db_video_id, vector<int> images)
{
                
        std::string instancestart   = pinstance->instancestart();
        std::string instanceend     = pinstance->instanceend();

        //Instance
        stringstream sql_instance;
        sql_instance <<        
        "INSERT INTO instance (number, instancestart, instanceend, _id_video, time, tracked) " <<
        "SELECT "   << number                   << 
        ",' "       << instancestart            << "'" << 
        ", '"       << instanceend              << "'" <<
        ", "        << db_video_id              <<
        ", '"       << time_info                << "'" << 
        ", 0 "      <<
        " WHERE NOT EXISTS (SELECT * FROM instance WHERE number = " << number       <<
        " AND instancestart = '"  << pinstance->instancestart()     <<    "'"       <<
        " AND instanceend   = '"  << pinstance->instanceend()       <<    "'"       <<
        " AND _id_video     = "   << db_video_id                    << 
        " AND time          = '"  << time_info                      <<    "'"       
        " AND tracked       = 0"        << ");";
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

        int dayid = pinstance->db_dayid();
        int db_recognition_setup_id = pinstance->db_recognition_setup_id();
        
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
        cout << "sql_rel_day_instance: " << sql_rel_day_instance << endl;
        pthread_mutex_lock(&databaseMutex);
        db_execute(sql_rel_day_instance.str().c_str());
        pthread_mutex_unlock(&databaseMutex);
         
        std::string last_rel_day_instance_recognition_setup_query = "SELECT MAX(_id) FROM rel_day_instance_recognition_setup";
        pthread_mutex_lock(&databaseMutex);
        vector<vector<string> > last_rel_day_instance_recognition_setup_array = db_select(last_rel_day_instance_recognition_setup_query.c_str(), 1);
        pthread_mutex_unlock(&databaseMutex);
        int db_day_instance_recognition_setup_id = atoi(last_rel_day_instance_recognition_setup_array.at(0).at(0).c_str());
        
        if (last_rel_day_instance_recognition_setup_array.size()==0)
        {
            cout << "CANNOT SAVE last_rel_day_instance_recognition_setup_array" << endl;
        }
        
        return db_instance_id;
         
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
void writeXMLInstance (
                              std::string XMLFILE,
                              std::string time_start,
                              std::string time_end,
                              std::string instance,
                              std::string instancecode
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

void setActiveCam(int activecam)
{
    stringstream sql_active_cams_update;
    sql_active_cams_update <<
    "UPDATE cameras set active = 0;";
    pthread_mutex_lock(&databaseMutex);
    db_execute(sql_active_cams_update.str().c_str());
    pthread_mutex_unlock(&databaseMutex);
    
    stringstream sql_active_cam_update;
    sql_active_cam_update <<
    "UPDATE cameras set active = 1 WHERE number = " << activecam << ";";
    pthread_mutex_lock(&databaseMutex);
    db_execute(sql_active_cam_update.str().c_str());
    pthread_mutex_unlock(&databaseMutex);
         
}