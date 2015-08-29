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

void updateCameraDB(int status, char * time, int camera)
{
    stringstream sql_updatecameras;
    sql_updatecameras <<
    "UPDATE cameras set recognizing = " << status << ", since = '" << time << "' "  <<
    "WHERE number = " << camera << ";";
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
      
        stringstream insert_camera_query;
        insert_camera_query <<
        "INSERT INTO cameras (number, name, created, recognizing, since) " <<
        "SELECT " << cams.at(i) << ",'" << camera << "','" << time_rasp << "'," << 0 << "," << NULL << 
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
        
        updateCameraDB(0, time_rasp, cams.at(i));
        
        camsarray.push_back(db_camera_id);
        
    }
    
    return camsarray;
}

void status()
{
 
    stringstream sql_status;
    sql_status << "SELECT "     <<
    "RC.codename, "             << // 0
    "C.recognizing, "           << // 1
    "C.number, "                << // 2
    "C.name, "                  << // 3
    "I.timestart, "             << // 4 
    "I.timeend, "               << // 5
    "C.since "                  << // 6
    "FROM recognition_setup RC "<< 
    "JOIN interval AS I ON RC._id_interval = I._id " << 
    "JOIN cameras AS C ON RC._id_camera = C._id;";  
    std::string last_har_cam_id_query = "SELECT MAX(_id) FROM rel_hardware_camera";
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
    stringstream sql_load_recognition;
    sql_load_recognition << "SELECT " <<
    "RC.codename, "         << // 0
    "RC.has_region, "       << // 1
    "RC.runatstartup, "     << // 2
    "RC.delay, "            << // 3
    "RC.storeimage, "       << // 4
    "RC.storevideo, "       << // 5
    "RC._id_mat, "          << // 6
    "CO.coordinates, "      << // 7
    "C._id, "               << // 8
    "C.name, "              << // 9
    "C.number, "            << // 10
    "I.timestart, "         << // 11
    "I.timeend, "           << // 12
    "RC.name, "             << // 13
    "M.matcols, "           << // 14 
    "M.matrows, "           << // 15 
    "M.matwidth, "          << // 16
    "M.matheight, "         << // 17
    "I.number "             << // 18
    "FROM recognition_setup RC "                            << 
    "JOIN coordinates AS CO ON RC._id_coordinates = CO._id "<<
    "JOIN interval AS I ON RC._id_interval = I._id "        <<
    "JOIN cameras AS C ON RC._id_camera = C._id "           <<
    "JOIN mat AS M ON RC._id_mat = M._id "                  <<
    "WHERE C.number = " << camera << " AND RC.name = '" << recname << "';";
    cout << "sql_load_recognition: " << sql_load_recognition.str() << endl;
    pthread_mutex_lock(&databaseMutex);
    vector<vector<string> > start_array = db_select(sql_load_recognition.str().c_str(), 19);
    pthread_mutex_unlock(&databaseMutex);
    if (start_array.size()>0)
    {
        vector<string> rows = start_array.at(0);

        motion::Message m;
        
        google::protobuf::int32 activecam = atoi(rows.at(8).c_str());
        m.set_activecam(activecam);
        
        google::protobuf::int32 cam32 = atoi(camera.c_str());
        m.set_activecam(cam32);

        m.set_type(motion::Message_ActionType_REC_START);
        motion::Message::MotionCamera * mcamera = m.add_motioncamera();

        bool hasregion = to_bool(rows.at(1));
        mcamera->set_hasregion(hasregion);
        if (hasregion)
        {
            std::string res = rows.at(7);
            std::string resencoded = base64_encode(reinterpret_cast<const unsigned char*>(res.c_str()), res.length());
            mcamera->set_coordinates(resencoded.c_str());
        }
        
        std::string codename = rows.at(0);
        mcamera->set_codename(codename);
        
        mcamera->set_fromdatabase(true);
        
        mcamera->set_runatstartup(to_bool(rows.at(2)));
        
        google::protobuf::int32 delay = atoi(rows.at(3).c_str());
        mcamera->set_delay(delay);
        
        mcamera->set_storeimage(to_bool(rows.at(4)));
        
        mcamera->set_storevideo(to_bool(rows.at(5)));
        
        google::protobuf::int32 dbmat = atoi(rows.at(11).c_str());
        mcamera->set_db_idmat(dbmat);
        
        google::protobuf::int32 dbidcamera = atoi(rows.at(8).c_str());
        mcamera->set_db_idcamera(dbidcamera);

        mcamera->set_cameraname(rows.at(9));
        
        google::protobuf::int32 camnum = atoi(rows.at(10).c_str());
        mcamera->set_cameranumber(camnum);

        mcamera->set_timestart(rows.at(11));
        mcamera->set_timeend(rows.at(12));

        mcamera->set_name(rows.at(13));
        
        google::protobuf::int32 matcols = atoi(rows.at(13).c_str());
        mcamera->set_matcols(matcols);
        
        google::protobuf::int32 matrows = atoi(rows.at(14).c_str());
        mcamera->set_matrows(matrows);

        std::string _day = getCurrentDayLabel();
        m.set_currday(_day);

        std::string _month = getCurrentMonthLabel();
        m.set_currmonth(_month);
        
        mcamera->set_lastinstance(rows.at(18));
        
        PROTO.Clear();
        PROTO = m;
        
        return true;
    
    } 
    else 
    {
        return false;
    }
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
    "AND C.recognizing      = 0 "                   <<
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


