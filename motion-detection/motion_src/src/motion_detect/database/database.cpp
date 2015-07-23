/*
 * File:   database.cpp
 * Author: jose
 *
 * Created on Julio 22, 2015, 11:23 AM
 */

#include "../database/database.h"

sqlite3 *db;
char *zErrMsg = 0;
int  rc;


static int callback(void *NotUsed, int argc, char **argv, char **azColName)
{
    int i;
    for(i=0; i<argc; i++){
        printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    }
    printf("\n");
    return 0;
}

void db_open()
{
    /* Open database */
    rc = sqlite3_open("database/motion.db", &db);
    if( rc ){
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
    }else{
        fprintf(stdout, "Opened database successfully\n");
    }

}


vector<vector<string> > db_select(const char *sql, int columns)
{
    
    db_open();
    
    vector<vector<string> > resutl;
    
    sqlite3_stmt *stmt;
    rc = sqlite3_prepare_v2(db, sql,
                            -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        cerr << "SELECT failed: " << sqlite3_errmsg(db) << endl;
        //return NULL; // or throw
    }
    
    for( int i = 0; i < columns; i++ )
        resutl.push_back(std::vector< std::string >());
    
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

void db_execute(const char *sql)
{
    db_open();
    
    rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);
    if( rc != SQLITE_OK )
    {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    } else
    {
        fprintf(stdout, "Records created successfully\n");
    }
    db_close();
}

void db_close()
{
    sqlite3_close(db);
}



void db_cpuinfo()
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
            
            cout << "line: " << line << endl;
            
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
        
        model       = splitString(model, ":").at(1);
        hardware    = splitString(hardware, ": ").at(1);
        revision    = splitString(revision, ": ").at(1);
        serial      = splitString(serial, ": ").at(1);
        
        /*std::string delimiter = " : ";
         model       = model.substr(model.find(delimiter), model.size());
         hardware    = hardware.substr(hardware.find(delimiter), hardware.size());
         revision    = revision.substr(revision.find(delimiter), revision.size());
         serial      = serial.substr(serial.find(delimiter), serial.size());*/
        
        //cout << "model: " << model << endl;
        //cout << "hardware: " << hardware << endl;
        //cout << "revision: " << revision << endl;
        //cout << "serial: " << serial << endl;
        
        stringstream sql;
        sql <<
        "INSERT INTO hardware (model,hardware,serial,revision) " <<
        "VALUES ('" << model    << "'"
        ", '"       << hardware << "'"
        ", '"       << revision << "'"
        ", '"       << serial   << "');" << endl;
        
        db_execute(sql.str().c_str());
    
}


void db_cams(std::vector<int> cams)
{
    for(int i=0; i<cams.size(); i++)
    {
        std::stringstream command;
        command << "/sys/class/video4linux/video" << cams.at(i) << "/name";
        std::string file = command.str();
        
        stringstream nfile;
        nfile << "data/camera_" << cams.at(i) << ".txt";
        string newfile = nfile.str();
        
        std::ifstream  src(file.c_str(), std::ios::binary);
        std::ofstream  dst(newfile.c_str(),  std::ios::binary);
        dst << src.rdbuf();
        dst.close();
        
        string camera = get_file_contents(newfile);
        
        stringstream insert_camera_query;
        insert_camera_query <<
        "INSERT INTO cameras (number, name) "       <<
        "SELECT " << cams.at(i) << ",'" << camera << "' "  <<
        "WHERE NOT EXISTS (SELECT * FROM cameras WHERE name = '" + camera + "');";
        
        db_execute(insert_camera_query.str().c_str());

    }
}


/*struct usb_bus *bus;
 struct usb_device *dev;
 usb_init();
 usb_find_busses();
 usb_find_devices();
 
 int i = 0;
 
 if (cams.size()>0)
 {
 for (bus = usb_busses; bus; bus = bus->next)
 {
 
 for (dev = bus->devices; dev; dev = dev->next)
 {
 
 char *vendor;
 size_t sv;
 
 char *product;
 size_t sp;
 
 cout << "dev->descriptor.idVendor: " <<  dev->descriptor.idVendor  << endl;
 cout << "dev->descriptor.idProduct: " <<  dev->descriptor.idProduct  << endl;
 
 cout << "..........." << endl;
 
 sv = snprintf(NULL, 0, "0x%04x", dev->descriptor.idVendor);
 vendor = (char *)malloc(sv + 1);
 
 sp = snprintf(NULL, 0, "0x%04x", dev->descriptor.idProduct);
 product = (char *)malloc(sp + 1);
 
 printf("Trying device %s/%s\n", bus->dirname, dev->filename);
 printf("\tID_VENDOR = 0x%04x\n", dev->descriptor.idVendor);
 printf("\tID_PRODUCT = 0x%04x\n", dev->descriptor.idProduct);
 
 
 
 i++;
 
 }
 }
 }
 sqlite3_close(db);*/
