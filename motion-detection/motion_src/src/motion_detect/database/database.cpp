
/*
 * File:   database.cpp
 * Author: jose
 *
 * Created on Julio 22, 2015, 11:23 AM
 */

#include "../database/database.h"
#include "../utils/utils.h"

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
        
        stringstream sql_terminal;
        sql_terminal <<
        "INSERT INTO terminal (model, hardware, serial, revision) " <<
        "SELECT '"  << model        << "'"
        ", '"       << hardware     << "'"
        ", '"       << serial       << "'"
        ", '"       << revision     << "'" <<
        " WHERE NOT EXISTS (SELECT * FROM terminal WHERE model  = '" << model    << "' " <<
        " AND hardware  = '"    << hardware     << "' " <<
        " AND serial    = '"    << serial       << "' " <<
        " AND revision  = '"    << revision     << "' " << ");";
        std::string stdquery = sql_terminal.str().c_str();
        pthread_mutex_lock(&databaseMutex);
        db_execute(stdquery.c_str());
        pthread_mutex_unlock(&databaseMutex);
    
        std::string last_terminal_id_query = "SELECT MAX(_id) FROM terminal;";
        pthread_mutex_lock(&databaseMutex);
        vector<vector<string> > terminal_array = db_select(last_terminal_id_query.c_str(), 1);
        pthread_mutex_unlock(&databaseMutex);
        int db_terminal_id = atoi(terminal_array.at(0).at(0).c_str());
        cout << "db_terminal_id: " << db_terminal_id << endl;
         
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
        
        FILE *temp;
        char bufftemp[512];
        if(!(temp = popen("vcgencmd measure_temp", "r")))
        {
            return 1;
        }

        std::string temperature;
        while(fgets(bufftemp, sizeof(bufftemp), temp)!=NULL)
        {
            stringstream bus;
            bus << bufftemp;
            temperature = bus.str();
        }
        pclose(temp);
        
        cout << "0: " << result.at(0) << endl;
        cout << "1: " << result.at(1) << endl;
        cout << "2: " << result.at(2) << endl;
        cout << "3: " << result.at(3) << endl;
        cout << "4: " << result.at(4) << endl;
        
        std::string tempnotparsed = splitString(temperature, "'").at(0);
        std::string tmp = splitString(tempnotparsed, "=").at(1); 
     
        stringstream sql_update_terminal;
        sql_update_terminal <<
        "UPDATE terminal SET "                  <<
        "disktotal = "      << result.at(1)     << ", "
        "diskused = "       << result.at(2)     << ", "
        "diskavailable = "  << result.at(3)     << ", "
        "diskper = '"       << result.at(4)     << "', "
        "temperature = '"   << tmp              << "' "
        "WHERE _id  = "     << db_terminal_id   << ";";
        std::string stdterminalquery = sql_update_terminal.str().c_str();
        cout << "stdterminalquery: " << stdterminalquery << endl;
        pthread_mutex_lock(&databaseMutex);
        db_execute(stdterminalquery.c_str());
        pthread_mutex_unlock(&databaseMutex);
       
    return db_terminal_id;
   
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
    
    setActiveCam(atoi(camera.c_str()));
    
    stringstream sql_db_cam;
    sql_db_cam      << 
    "SELECT RS._id_camera, C.number, C.name FROM recognition_setup AS RS JOIN cameras AS C ON RS._id_camera = C._id WHERE RS.name = '" << recname << "';";
    pthread_mutex_lock(&databaseMutex);
    vector<vector<string> > db_cam_array = db_select(sql_db_cam.str().c_str(), 3);
    pthread_mutex_unlock(&databaseMutex);
    int db_cam_id = atoi(db_cam_array.at(0).at(0).c_str());
    int camnum = atoi(db_cam_array.at(0).at(1).c_str());
    std::string cameraname = db_cam_array.at(0).at(2);
    cout << "db_cam_id: " << db_cam_id << endl;
    
    std::string _month = getCurrentMonthLabel();
    int db_month_id = insertMonthIntoDatabase(_month, db_cam_id);
    
    std::string _day = getCurrentDayLabel();
    
    std::string XML_FILE = "<import>session"; 
    
    
    std::string xml_path = getXMLFilePathAndName(sourcepath, camnum, recname, _day, XML_FILE);
  
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
    
    google::protobuf::int32 activecam; 
    google::protobuf::int32 activecamnum;
    
    motion::Message::MotionCamera * mcamera;
    
    int size = start_array.size();
    if (size>0)
    {
        vector<string> rows = start_array.at(0);

        activecam = atoi(rows.at(8).c_str());
        activecamnum = atoi(rows.at(10).c_str());
    
        bool cameraexist = false;
        int sizer = R_PROTO.motioncamera_size();
        for (int i = 0; i < sizer; i++)
        {
            if (mcamera->has_db_idcamera())
            {
                if (activecamnum == R_PROTO.mutable_motioncamera(i)->cameranumber())
                {
                    mcamera = R_PROTO.mutable_motioncamera(i);
                    mcamera->Clear();
                    cameraexist=true;
                }
            }
        }
        if(!cameraexist)
        {
            pthread_mutex_lock(&protoMutex);
            mcamera = R_PROTO.add_motioncamera();
            pthread_mutex_unlock(&protoMutex);
        }
        
        int sizer2 = R_PROTO.motioncamera_size();
        
        motion::Message::MotionRec * mrec;
        
        pthread_mutex_lock(&protoMutex);
        R_PROTO.set_activecam(activecam);
        R_PROTO.set_activecamnum(activecamnum);
        R_PROTO.set_type(motion::Message_ActionType_REC_START);
        mcamera->set_cameraname(cameraname);
        mcamera->set_recognizing(true);
        mrec = mcamera->add_motionrec();
        pthread_mutex_unlock(&protoMutex);
        
        bool hasregion = to_bool(rows.at(1));
        std::string resencoded;
        if (hasregion)
        {
            std::string res = rows.at(7);
            resencoded = base64_encode(reinterpret_cast<const unsigned char*>(res.c_str()), res.length());
        }
        
        std::string codename = rows.at(0);
        google::protobuf::int32 delay = atoi(rows.at(3).c_str());
        google::protobuf::int32 dbmat = atoi(rows.at(11).c_str());
        google::protobuf::int32 dbidcamera = atoi(rows.at(8).c_str());
        google::protobuf::int32 matcols = atoi(rows.at(12).c_str());
        google::protobuf::int32 matrows = atoi(rows.at(13).c_str());
        std::string _day = getCurrentDayLabel();
        std::string _month = getCurrentMonthLabel();
        google::protobuf::int32 dbidday = atoi(rows.at(16).c_str());
        google::protobuf::int32 dbidmonth = atoi(rows.at(17).c_str());
        google::protobuf::int32 speed = atoi(rows.at(18).c_str());
        google::protobuf::int32 recid = atoi(rows.at(19).c_str());
        
        pthread_mutex_lock(&protoMutex);
        mrec->set_hasregion(hasregion);
        mrec->set_coordinates(resencoded.c_str());
        mrec->set_codename(codename);
        mrec->set_runatstartup(to_bool(rows.at(2)));
        mrec->set_delay(delay);
        mrec->set_storeimage(to_bool(rows.at(4)));
        mrec->set_storevideo(to_bool(rows.at(5)));
        mrec->set_db_idmat(dbmat);
        mcamera->set_db_idcamera(dbidcamera);
        mcamera->set_cameraname(rows.at(9));
        mcamera->set_cameranumber(atoi(rows.at(10).c_str()));
        mrec->set_recname(rows.at(11));
        mrec->set_matcols(matcols);
        mrec->set_matrows(matrows);
        R_PROTO.set_currday(_day);
        R_PROTO.set_currmonth(_month);
        mrec->set_db_idday(dbidday);
        mrec->set_db_idmonth(dbidmonth);
        mrec->set_speed(speed);
        mrec->set_db_recognitionsetupid(recid);
        mrec->set_xmlfilepath(rows.at(20).c_str());
        pthread_mutex_unlock(&protoMutex);
        
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
        "WHERE C.number = " << camnum << " AND D.label = '" << R_PROTO.currday()  << "' " <<
        "AND RS.name = '" << recname << "';";
        std::string sqllaststd = sql_last_instance.str();
        cout << "sqllaststd: " << sqllaststd << endl;
        pthread_mutex_lock(&databaseMutex);
        vector<vector<string> > lastinstance_array = db_select(sqllaststd.c_str(), 1);
        pthread_mutex_unlock(&databaseMutex);
        
        std::string last;
        int ln = atoi(last.c_str());
        if (ln>0)
        {
            last = lastinstance_array.at(0).at(0).c_str();
        } else 
        {
            last = "0";
        }
          
        pthread_mutex_lock(&protoMutex);
        mrec->set_lastinstance(last);
        pthread_mutex_unlock(&protoMutex);
        
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
        "I._id, "                   <<        
        "I.path, "                  <<
        "I.name, "                  <<
        "D.label, "                 <<
        "RS.name, "                 << 
        "I.time, "                  <<
        "C.coordinates "            <<
        "FROM instance AS INS "     <<
        "JOIN rel_instance_image AS RII ON INS._id = RII._id_instance " <<
        "JOIN image AS I ON RII._id_image = I._id "                     <<
        "JOIN rel_day_instance_recognition_setup AS RDIRS ON RII._id_instance = RDIRS._id_instance "    <<
        "JOIN day AS D ON RDIRS._id_day = D._id "                                                       <<
        "JOIN recognition_setup AS RS ON RDIRS._id_recognition_setup = RS._id "                         <<  
        "JOIN coordinates AS C ON RII._id_coordinates = C._id "        
        "WHERE INS._id = " << instanceid << ";";        

        cout << "check_istances: " << check_istances.str() << endl;

        pthread_mutex_lock(&databaseMutex);
        vector<vector<string> > image_array = db_select(check_istances.str().c_str(), 8);
        pthread_mutex_unlock(&databaseMutex); 

        if (image_array.size()>0)
        {
            std::string _id = image_array.at(0).at(1);
            maximage.push_back(_id);
            std::string path = image_array.at(0).at(2);
            maximage.push_back(path);
            std::string name = image_array.at(0).at(3);
            maximage.push_back(name);
            std::string day = image_array.at(0).at(4);
            maximage.push_back(day);
            std::string rec = image_array.at(0).at(5);
            maximage.push_back(rec);
            std::string time = image_array.at(0).at(6);
            maximage.push_back(time);   
            std::string coords = image_array.at(0).at(7);
            maximage.push_back(coords);   
        }
    }
    return maximage;  
}

int insertTracking(int db_instance_id, std::string maximagepath, int db_srv_idmedia, int db_srv_idpost)
{
    stringstream sql_insert_tracking;
    sql_insert_tracking <<
    "INSERT INTO track_instances (_id_db_instance, imagepath, _id_dbsrv_media, _id_dbsrv_post) "   <<
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

int insertServerIntoDatabase(int clientnumber, std::string clientname, std::string base)
{
    std::stringstream sql_insert_server;
    sql_insert_server << "INSERT OR REPLACE INTO server (_id, client_number, client_name, base_url) values (" <<
    "1, " << clientnumber << ", '" << clientname << "', '" << base << "');";
    
    cout << "sql_insert_server: " << sql_insert_server.str() << endl;
    
    pthread_mutex_lock(&databaseMutex);
    db_execute(sql_insert_server.str().c_str());   
    
    std::string last_server_query = "SELECT MAX(_id) FROM server";
    vector<vector<string> > server_array = db_select(last_server_query.c_str(), 1);
    pthread_mutex_unlock(&databaseMutex);
    cout << "server_array: " << endl;
    int db_server_id = atoi(server_array.at(0).at(0).c_str());
    cout << "db_server_id: " << db_server_id << endl;
    return db_server_id;
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


void insertIntoLocation(std::string publicip, std::string hostname, std::string city, std::string region, std::string country, std::string loc, std::string org)
{
    struct timeval tr;
    struct tm* ptmr;
    char time_rasp[40];
    gettimeofday (&tr, NULL);
    ptmr = localtime (&tr.tv_sec);
    strftime (time_rasp, sizeof (time_rasp), "%Y-%m-%d %H:%M:%S %z", ptmr);
    
    //rel_camera_month.
    stringstream sql_location;
    sql_location <<
    "INSERT INTO location (publicip, hostname, city, region, country, location, organization, time) " <<
    "SELECT '" << publicip << "', '"  << hostname << "', '" << city << "', '" << region << "', '" << country << "', '" << loc << "', '" << org << "', '" << time_rasp << "'" << 
    " WHERE NOT EXISTS (SELECT * FROM location WHERE publicip = '" << publicip << "');";
    cout << "sql_location: " << sql_location.str() << endl;
    pthread_mutex_lock(&databaseMutex); 
    db_execute(sql_location.str().c_str());
    pthread_mutex_unlock(&databaseMutex);
}

int insertIntoPosts(std::string id, 
        std::string date, 
        std::string modified, 
        std::string slug, 
        std::string type, 
        std::string link, 
        std::string api_link, 
        std::string featured_image, 
        std::string post_parent,
        int db_local)
{
    struct timeval tr;
    struct tm* ptmr;
    char time_rasp[40];
    gettimeofday (&tr, NULL);
    ptmr = localtime (&tr.tv_sec);
    strftime (time_rasp, sizeof (time_rasp), "%Y-%m-%d %H:%M:%S %z", ptmr);
    
    //rel_camera_month.
    stringstream sql_posts;
    sql_posts <<
    "INSERT INTO track_posts (id, date, modified, slug, type, link, api_link, featured_image, post_parent, count_update, db_local, time_rasp) " <<
    "SELECT " << id << ", '"  << date << "', '" << modified << "', '" << slug << "', '" << type << "', '" << link << "', '" << api_link << "', '" << featured_image << "', '" << post_parent << "', 0, " << db_local << ",'" << time_rasp << "'" << 
    " WHERE NOT EXISTS (SELECT * FROM track_posts WHERE id = " << id << ");";
    cout << "sql_posts: " << sql_posts.str() << endl;
    pthread_mutex_lock(&databaseMutex); 
    db_execute(sql_posts.str().c_str());
    pthread_mutex_unlock(&databaseMutex);
    
    vector<vector<string> > track_posts_array;
    std::string track_posts_sql_last = "SELECT MAX(_id) FROM track_posts";
    pthread_mutex_lock(&databaseMutex);
    track_posts_array = db_select(track_posts_sql_last.c_str(), 1);
    pthread_mutex_unlock(&databaseMutex);
    int db_track_posts_id = atoi(track_posts_array.at(0).at(0).c_str());
    
    return db_track_posts_id;
}

void updateIntoPost (std::string id, std::string date, std::string modified)
{
    struct timeval tr;
    struct tm* ptmr;
    char time_rasp[40];
    gettimeofday (&tr, NULL);
    ptmr = localtime (&tr.tv_sec);
    strftime (time_rasp, sizeof (time_rasp), "%Y-%m-%d %H:%M:%S %z", ptmr);
    
    vector<vector<string> > track_posts_update_array;
    std::stringstream track_posts_update_count_sql;
    track_posts_update_count_sql << "SELECT count_update FROM track_posts WHERE id = " << id << ";";
    pthread_mutex_lock(&databaseMutex);
    track_posts_update_array = db_select(track_posts_update_count_sql.str().c_str(), 1);
    pthread_mutex_unlock(&databaseMutex);
    int db_track_posts_update_count = atoi(track_posts_update_array.at(0).at(0).c_str());
    
    db_track_posts_update_count++;
    
    stringstream sql_post_update;
    sql_post_update <<
    "UPDATE track_posts SET date = '" << date << "', modified = '" << modified << "', time_rasp = '" << time_rasp << "', count_update = " << db_track_posts_update_count << "  WHERE id = " << id << ";";
    pthread_mutex_lock(&databaseMutex);
    db_execute(sql_post_update.str().c_str());
    pthread_mutex_unlock(&databaseMutex);
    
}

vector<std::string> getTrackPostByType(std::string type)
{
    vector<std::string> info;
    
    std::stringstream typeinfo;
    typeinfo << "SELECT * FROM track_posts WHERE type ='" << type << "' AND _id = (SELECT MAX(_id) FROM track_posts WHERE type ='" << type << "')";
    pthread_mutex_lock(&databaseMutex);
    cout << "typeinfo: " << typeinfo.str() << endl;
    vector<vector<string> > typeinfo_array = db_select(typeinfo.str().c_str(), 13);
    pthread_mutex_unlock(&databaseMutex);
    
    if (typeinfo_array.size()>0)
    {
        info.push_back(typeinfo_array.at(0).at(0));     // _id
        info.push_back(typeinfo_array.at(0).at(1));     // id
        info.push_back(typeinfo_array.at(0).at(2));     // date
        info.push_back(typeinfo_array.at(0).at(3));     // modified
        info.push_back(typeinfo_array.at(0).at(4));     // slug
        info.push_back(typeinfo_array.at(0).at(5));     // type
        info.push_back(typeinfo_array.at(0).at(6));     // link
        info.push_back(typeinfo_array.at(0).at(7));     // api_link
        info.push_back(typeinfo_array.at(0).at(8));     // featured_image
        info.push_back(typeinfo_array.at(0).at(9));     // post_parent
        info.push_back(typeinfo_array.at(0).at(10));    // count_update
        info.push_back(typeinfo_array.at(0).at(11));    // db_local
        info.push_back(typeinfo_array.at(0).at(12));    // time_rasp
    } 
    return info;
}

vector<std::string> getTrackPostByTypeAndId(std::string type, int db_local)
{
    vector<std::string> info;
    
    std::stringstream typeinfo;
    typeinfo << "SELECT * FROM track_posts WHERE type ='" << type << "' "           << 
    "AND _id = (SELECT MAX(_id) FROM track_posts WHERE type ='" << type << "') "
    "AND db_local = " << db_local << ";";
    pthread_mutex_lock(&databaseMutex);
    cout << "typeinfo: " << typeinfo.str() << endl;
    vector<vector<string> > typeinfo_array = db_select(typeinfo.str().c_str(), 13);
    pthread_mutex_unlock(&databaseMutex);
    
    if (typeinfo_array.size()>0)
    {
        return typeinfo_array.at(0);
    } 
    return info;
}

vector<vector<string> > getTrackPosts(std::string type)
{
    vector<vector<string> > info;
    
    std::stringstream typeinfo;
    typeinfo << "SELECT * FROM track_posts WHERE type ='" << type << "';";
    pthread_mutex_lock(&databaseMutex);
    vector<vector<string> > typeinfo_array = db_select(typeinfo.str().c_str(), 11);
    pthread_mutex_unlock(&databaseMutex);
    
    if (typeinfo_array.size()>0)
    {
        return typeinfo_array;
    } 
    return info;
}

vector<vector<string> > getTrackPostChilds(int id)
{
    vector<vector<string> > child;
    
    std::stringstream typeinfo;
    typeinfo << 
    "SELECT "                   <<
    "RTPC.post "                <<
    "RTPC.post_url "            <<
    "FROM track_posts AS TP "   <<
    "JOIN rel_track_post_children AS RTPC ON TP._id_rel_track_post_children = RTPC._id_track_post " <<     
    "WHERE TP._id_track_post =" << id << ";";
    pthread_mutex_lock(&databaseMutex);
    vector<vector<string> > postchilds_array = db_select(typeinfo.str().c_str(), 11);
    pthread_mutex_unlock(&databaseMutex);
    
    if (postchilds_array.size()>0)
    {
        return postchilds_array;
    } 
    return child;
}

time_t getLastPostTime(std::string type)
{
    std::stringstream timeinfo;
    timeinfo << "SELECT time_rasp FROM track_posts WHERE type ='" << type << "';";
    pthread_mutex_lock(&databaseMutex);
    vector<vector<string> > timeinfo_array = db_select(timeinfo.str().c_str(), 1);
    pthread_mutex_unlock(&databaseMutex);
    
    if (timeinfo_array.size()>0)
    {
        const char *time_details = timeinfo_array.at(0).at(0).c_str();
        struct tm tm;
        strptime(time_details, "%Y-%m-%d %H:%M:%S %z", &tm);
        time_t lasttime = mktime(&tm); 
        return lasttime;
        
    } else 
    {
        return NULL;
    }
}


vector<std::string> getIpInfo()
{
    vector<std::string> info;
    
    std::stringstream ipinfo;
    ipinfo << "SELECT publicip, time FROM location;";
    pthread_mutex_lock(&databaseMutex);
    vector<vector<string> > ipinfo_array = db_select(ipinfo.str().c_str(), 2);
    pthread_mutex_unlock(&databaseMutex);
    
    if (ipinfo_array.size()>0)
    {
        info.push_back(ipinfo_array.at(0).at(0));
        info.push_back(ipinfo_array.at(0).at(1));
    } 
    
    return info;    
}

vector<std::string> getLocationInfo()
{
    vector<std::string> info_location;
    
    std::stringstream locinfo;
    locinfo << "SELECT _id, city, country, location FROM location;";
    pthread_mutex_lock(&databaseMutex);
    vector<vector<string> > locinfo_array = db_select(locinfo.str().c_str(), 4);
    pthread_mutex_unlock(&databaseMutex);
    
    if (locinfo_array.size()>0)
    {
        info_location.push_back(locinfo_array.at(0).at(0));
        info_location.push_back(locinfo_array.at(0).at(1));
        info_location.push_back(locinfo_array.at(0).at(2));
        info_location.push_back(locinfo_array.at(0).at(3));
    } 
    
    return info_location;    
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

vector<std::string> getTerminalInfo()
{
    
    vector<std::string> terminal;

    std::string sql_network = "SELECT * FROM network;";
    pthread_mutex_lock(&databaseMutex);
    vector<vector<string> > network_array = db_select(sql_network.c_str(), 4);
    pthread_mutex_unlock(&databaseMutex);
    
    if (network_array.size()>0)
    {
        std::string ipnumber = network_array.at(0).at(1);
        terminal.push_back(ipnumber);                       //ipnumber              0
        std::string ippublic = network_array.at(0).at(2);
        terminal.push_back(ippublic);                       //ippublic              1
        std::string macaddress = network_array.at(0).at(3);
        terminal.push_back(macaddress);                     //macaddress            2
    }
    
    std::stringstream sql_location;
    sql_location << "SELECT * " <<
    "FROM location WHERE _id = (SELECT MAX(_id) FROM location);";
    pthread_mutex_lock(&databaseMutex);
    vector<vector<string> > location_array = db_select(sql_location.str().c_str(), 9);
    pthread_mutex_unlock(&databaseMutex);
    
    if (location_array.size()>0)
    {
        terminal.push_back(location_array.at(0).at(2)); //hostname              3
        terminal.push_back(location_array.at(0).at(3)); //city                  4
        terminal.push_back(location_array.at(0).at(5)); //country               5
        terminal.push_back(location_array.at(0).at(6)); //location              6
        terminal.push_back(location_array.at(0).at(7)); //network_provider      7
    }
    
    std::string sql_status = "SELECT * FROM status;";
    pthread_mutex_lock(&databaseMutex);
    vector<vector<string> > status_array = db_select(sql_status.c_str(), 3);
    pthread_mutex_unlock(&databaseMutex);
    
    if (status_array.size()>0)
    {
        terminal.push_back(status_array.at(0).at(1)); //uptime              8
        terminal.push_back(status_array.at(0).at(2)); //starttime           9
    }
    
    std::string sql_terminal = "SELECT * FROM terminal;";
    pthread_mutex_lock(&databaseMutex);
    vector<vector<string> > terminal_array = db_select(sql_terminal.c_str(), 10);
    pthread_mutex_unlock(&databaseMutex);
    
    if (terminal_array.size()>0)
    {
        terminal.push_back(terminal_array.at(0).at(0)); //db_local              10
        terminal.push_back(terminal_array.at(0).at(1)); //model                 11
        terminal.push_back(terminal_array.at(0).at(2)); //hardware              12
        terminal.push_back(terminal_array.at(0).at(3)); //serial                13
        terminal.push_back(terminal_array.at(0).at(4)); //revision              14
        terminal.push_back(terminal_array.at(0).at(5)); //disktotal             15
        terminal.push_back(terminal_array.at(0).at(6)); //diskused              16
        terminal.push_back(terminal_array.at(0).at(7)); //diskavailable         17
        terminal.push_back(terminal_array.at(0).at(8)); //disk_percentage_used  18
        terminal.push_back(terminal_array.at(0).at(9)); //temperature           19
    }
   
    return terminal;
    
}

vector<std::string> getMatInfoFromId(int db_idmat)
{
    vector<std::string> matidarray;
    
    std::stringstream sql_mat;
    sql_mat << "SELECT * FROM mat WHERE _id = " << db_idmat << ";";
    pthread_mutex_lock(&databaseMutex);
    vector<vector<string> > mat_array = db_select(sql_mat.str().c_str(), 8);
    pthread_mutex_unlock(&databaseMutex);
    
    if (mat_array.size()>0)
    {
        std::stringstream screen;
        std::string matwidth = mat_array.at(0).at(4);
        std::string matheight = mat_array.at(0).at(5);
        screen << "[" << matwidth << "," << matheight << "]";                           //screen size
        matidarray.push_back(screen.str());                                             //screen             0
        matidarray.push_back(mat_array.at(0).at(6));                                    //matfile            1
    }
    return matidarray;
}

int getPostByIdAndType(int db_idpost)
{
    int id;
    std::stringstream sql_post ;
    sql_post << "SELECT * FROM track_posts WHERE _id = " << db_idpost << ";";
    pthread_mutex_lock(&databaseMutex);
    vector<vector<string> > post_array = db_select(sql_post.str().c_str(), 1);
    pthread_mutex_unlock(&databaseMutex);
    
    if (post_array.size()>0)
    {
        id = atoi(post_array.at(0).at(0).c_str());
    }
    return id;
}

vector<std::string> getServerInfo()
{
    vector<std::string> matidarray;
    
    std::stringstream sql_server;
    sql_server << "SELECT client_number, client_name, base_url FROM server;";
    pthread_mutex_lock(&databaseMutex);
    vector<vector<string> > server_array = db_select(sql_server.str().c_str(), 3);
    pthread_mutex_unlock(&databaseMutex);
    
    if (server_array.size()>0)
    {
        return server_array.at(0);
    }
    return matidarray;
}

void insertUpdateStatus(std::string uptime, vector<int> camsarray, int db_terminal_id)
{
    
    struct timeval tr;
    struct tm* ptmr;
    char time_rasp[40];
    gettimeofday (&tr, NULL);
    ptmr = localtime (&tr.tv_sec);
    strftime (time_rasp, sizeof (time_rasp), "%Y-%m-%d %H:%M:%S %z", ptmr);
    
    //Status
    stringstream sql_status;
    sql_status <<
    "INSERT INTO status (uptime, starttime) " <<
    "SELECT '"  << uptime         << "'"
    ", '"       << time_rasp        << "' "
    "WHERE NOT EXISTS (SELECT * FROM status WHERE uptime = '"<< uptime << "' " <<
    "AND starttime = '" << time_rasp << "');";
    cout << "sql_status: " << sql_status.str() << endl;
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
    cout << "sql_status_update: " << sql_status_update.str() << endl;
    db_execute(sql_status_update.str().c_str());
    
    cout << "Getting hard info." << endl;
    vector<int> camhard;
    for (int i=0; i< camsarray.size(); i++)
    {
        stringstream insert_camera_query;
        insert_camera_query <<
        "INSERT INTO rel_terminal_camera (_id_terminal, _id_camera) " <<
        "SELECT " << db_terminal_id << "," << camsarray.at(i) <<
        " WHERE NOT EXISTS (SELECT * FROM rel_terminal_camera WHERE _id_terminal = "
        << db_terminal_id << " AND _id_camera = " << camsarray.at(i) << ");";
        cout << "insert_camera_query: " << insert_camera_query.str() << endl;
        db_execute(insert_camera_query.str().c_str());
            
        std::string last_har_cam_id_query = "SELECT MAX(_id) FROM rel_terminal_camera";
        vector<vector<string> > camhard_array = db_select(last_har_cam_id_query.c_str(), 1);
        int db_cam_hard_id = atoi(camhard_array.at(0).at(0).c_str());
        cout << "db_cam_hard_id: " << db_cam_hard_id << endl;
            
        camhard.push_back(db_cam_hard_id);
        
    }
    
    //rel_terminal_camera_status
    for (int i=0; i< camhard.size(); i++)
    {
        stringstream insert_rel_hardcamsta_query;
        insert_rel_hardcamsta_query <<
        "INSERT INTO rel_terminal_camera_status (_id_terminal_camera, _id_status) " <<
        "SELECT " << camhard.at(i) << "," << db_status_id <<
        " WHERE NOT EXISTS (SELECT * FROM rel_terminal_camera_status WHERE _id_terminal_camera = "
        << camhard.at(i) << " AND _id_status = " << db_status_id << ");";
        cout << "insert_rel_hardcamsta_query: " << insert_rel_hardcamsta_query.str() << endl;
        db_execute(insert_rel_hardcamsta_query.str().c_str());
    }
    
    //Reset to recognizing = 0 to all jobs
    stringstream sql_update_recognizing;
    sql_update_recognizing <<
    "UPDATE recognition_setup SET recognizing = 0;";
    db_execute(sql_update_recognizing.str().c_str());
    
}

motion::Message saveRecognition(motion::Message m)
{
    
    motion::Message::MotionCamera * pcamera = m.mutable_motioncamera(0);
    motion::Message::MotionRec * prec = pcamera->mutable_motionrec(0);

    int sizec = m.motioncamera_size();

    cout << "sizec: " << sizec << endl;
    bool cameraexist = false;

    std::string rcoords;
    int db_coordnatesid; 
    if (prec->hasregion())
    {
        std::string rc = prec->coordinates(); 
        rcoords = base64_decode(rc);
        db_coordnatesid = insertRegionIntoDatabase(rcoords);   
        m.set_data(rcoords);
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

    google::protobuf::uint32 activecam = m.activecam();

    std::string recname = prec->recname();

    std::string xml_path = getXMLFilePathAndName(sourcepath, activecam, recname, str_day, XML_FILE);

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
    int db_recognitionsetup_id = insertIntoRecognitionSetup(prec, db_dayid, db_camera_id, db_coordnatesid, xml_path);
    prec->set_db_idrec(db_recognitionsetup_id);
    
    int db_camera_recognition_setupl_array = insertIntoRelCameraRecognitionSetup(time_rasp, db_recognitionsetup_id, db_camera_id);

    if (prec->hascron())
    {
        insertIntervalCrontabIntoDatabase(pcamera, prec, db_camera_recognition_setupl_array); 
    }   
    return m; 
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
    "V.path, "                  <<  //12
    "I.number "                 <<  //13
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
    vector<vector<string> > rcm_array = db_select(sql_monthstr.c_str(), 14);
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
                
                std::string number = rowi.at(13).c_str();
                google::protobuf::int32 instancenumber = atoi(number.c_str());
                minstance->set_number(instancenumber);

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

void updateRecognition(motion::Message m)
{

    motion::Message::MotionCamera * pcamera = m.mutable_motioncamera(0);   
    motion::Message::MotionMonth * pmonth = pcamera->mutable_motionmonth(0);
    motion::Message::MotionRec * prec = pcamera->mutable_motionrec(0);
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
    if (prec->hasregion())
    {
        std::string rc = prec->coordinates(); 
        rcoords = base64_decode(rc);
        updateRegionIntoDatabase(rcoords, prec->db_recognitionsetupid());
    }
     
    google::protobuf::uint32 activecam = R_PROTO.activecam();
    
    std::string xml_path = getXMLFilePathAndName(sourcepath, activecam, prec->recname(), str_day, XML_FILE);
    
     //time
    struct timeval tr;
    struct tm* ptmr;
    char time_rasp[40];
    gettimeofday (&tr, NULL);
    ptmr = localtime (&tr.tv_sec);
    strftime (time_rasp, sizeof (time_rasp), "%Y-%m-%d %H:%M:%S %z", ptmr);
    
    //insertIntervalCrontabIntoDatabase(pcamera);
    updateRecognitionSetup(pcamera->db_idcamera(), prec, pday);
    updateCameraMonth(time_rasp, prec->db_recognitionsetupid());
    
}

vector<std::string> getCamerasFromDB()
{
    vector<std::string> cameraarray;
    
    std::stringstream sql_camera;
    sql_camera << "SELECT number, name, created FROM cameras;";
    pthread_mutex_lock(&databaseMutex);
    vector<vector<string> > camera_array = db_select(sql_camera.str().c_str(), 3);
    pthread_mutex_unlock(&databaseMutex);
    
    if (camera_array.size()>0)
    {
        return camera_array.at(0);
    }
    return cameraarray;
}