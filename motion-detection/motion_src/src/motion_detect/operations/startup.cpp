/*
 * File:   startup.cpp
 * Author: jose
 *
 * Created on Dec 12, 2015, 7:48 PM
 */



#include "../operations/startup.h"
#include "../utils/utils.h"
#include "../database/database.h"
#include "../operations/camera.h"
#include "../recognition/detection.h"
#include "../http/json.h"

#include <iostream>

#include <stdio.h>
#include <sys/time.h>
#include <vector>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <usb.h>
#include <time.h>

#include <pthread.h>
#include <signal.h>
#include <cerrno> 


int hardwareInfo()
{
 
    cout << "Getting hard info." << endl;
    int db_terminal_id = db_cpuinfo();
    cout << "db_terminal_id: " << db_terminal_id << endl;
    
    //Rasp Variables.
    cams = getCameras();
    if (cams.size()==0)
    {
        cout << "NO CAMERA IDENTIFIED!" << endl;
        cout << "QUITTING PROGRAM....." << endl;
        return 0;
    }
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
    
    //Clear recognitions start if rebooted.
    for (int i=0; i<cams.size(); i++)
    {
        int camnum = cams.at(i);
        updateRecStatusByCamera(0, camnum);
    }
    
     //Store into database.
    vector<int> camsarray = db_cams(cams);
    
    FILE *intime;
    char bufftime[512];
    if(!(intime = popen("uptime", "r")))
    {
        return 1;
    }

    std::string uptime;
    while(fgets(bufftime, sizeof(bufftime), intime)!=NULL)
    {
        stringstream bus;
        bus << bufftime;
        uptime = bus.str();
    }
    pclose(intime);
    
    insertUpdateStatus(uptime, camsarray, db_terminal_id);
    
    starttime = time_rasp;
    
    cout << "Start Time:: " << starttime << endl;
    
    return 0;
    
}

int netWorkInfo()
{
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
    
    vector<std::string> ipinfo;
    ipinfo = getIpInfo();
    bool callip = false;
    if (ipinfo.size()>0)
    {
        public_ip = ipinfo.at(0);
        
        const char *time_details = ipinfo.at(1).c_str();
        struct tm tm;
        strptime(time_details, "%Y-%m-%d %H:%M:%S %z", &tm);
        time_t last = mktime(&tm); 
        
        time_t now;
        time(&now); 
        
        double seconds_since_start = difftime(now, last);
        double timecount = 60 * 60 * 24;
        
        if (seconds_since_start>timecount)
        {
            callip = true;
        }
        
    } else 
    {
        callip = true;
    }
        
    if (callip)
    {    
        /*FILE *inip;
        char buffip[512];
        if(!(inip = popen("curl ifconfig.me", "r")))
        {
            return 1;
        }
        stringstream busip; 
        while(fgets(buffip, sizeof(buffip), inip)!=NULL)
        {
            busip << buffip;
            public_ip = busip.str();
        }
        
        cout << "ipnumber: " << NETWORK_IP << endl;
        cout << "publicip: " << public_ip << endl;*/
        
        FILE *inloc;
        char buffloc[512];
        std::string location = "curl ipinfo.io"; // 190.177.218.76";
        cout << "location: " << location << endl;
        if(!(inloc = popen(location.c_str(), "r")))
        {
           return 1;
        }
        stringstream busloc; 
        while(fgets(buffloc, sizeof(buffloc), inloc)!=NULL)
        {
            busloc << buffloc;
        }
        vector<std::string> locationp = parse_and_store_ipinfo_io(busloc.str().c_str());
        
        hostname = locationp.at(1);
        city = locationp.at(2);
        region = locationp.at(3);
        country = locationp.at(4);
        loc = locationp.at(5);        
        org = locationp.at(6);
                
        insertIntoLocation(locationp);
        
        pclose(inloc);
        
        std::string maccheck = "cat /sys/class/net/eth0/address";
        char *cestrmac = new char[maccheck.length() + 1];
        strcpy(cestrmac, maccheck.c_str());
        std::string resutl_mac = exec_command(cestrmac);
        resutl_mac.erase(std::remove(resutl_eth0.begin(), resutl_eth0.end(), '\n'), resutl_eth0.end());

         //Status
        stringstream sql_network;
        sql_network <<
        "INSERT INTO network (ipnumber, ippublic, macaddress) " <<
        "SELECT '"  << local_ip        << "'"
        ", '"       << public_ip       << "' " << 
        ", '"       << resutl_mac      << "' " << 
        "WHERE NOT EXISTS (SELECT * FROM network WHERE ipnumber = '"<< local_ip << "' " <<
        "AND ippublic = '"      << public_ip    << "' " <<
        "AND macaddress = '"    << resutl_mac   << "');";
        cout << "sql_network: " << sql_network.str() << endl;
        db_execute(sql_network.str().c_str());
        
    }
    return 0;
}

int createDirectories()
{
    
    std::string secdatafile = basepath + "data/data";
    directoryExistsOrCreate(secdatafile.c_str());
    std::string matdatafile = basepath + "data/mat";
    directoryExistsOrCreate(matdatafile.c_str());
    std::string trackdatafile = basepath + "data/tracking";
    directoryExistsOrCreate(trackdatafile.c_str());
    dumpinstancefolder = basepath + "data/instances";
    directoryExistsOrCreate(dumpinstancefolder.c_str());
    
    for (int k=0; k< cams.size(); k++)
    {
        std::stringstream dumpcamera;
        dumpcamera << dumpinstancefolder << "/camera" << cams.at(k);
        directoryExistsOrCreate(dumpcamera.str().c_str());
    }
    return 0;
}

void runJobsInterval(std::string timecompare, std::vector<pthread_t> threads_recognition)
{
    for (int t=0; t< cams.size(); t++)
    {
       
       char time_interval[8];    
       char *cstr = new char[timecompare.length() + 1];
       strcpy(cstr, timecompare.c_str());
     
       stringstream camstr;
       camstr << cams.at(t);
    
       int camnum = atoi(camstr.str().c_str());
       
       int running;
        
       int threadid = threads_recognizing_pids[camnum];
       
       if (threadid>0)
       {
            int response = pthread_kill(threadid, 0);
            if (response != 0) 
            {
                 running = 0;
            } else 
            {
                 running = 1;
            }
       } else 
       {
           running = 0;
       }
       
        updateRecStatusByCamera(running, camnum);
       
        vector<string> runvector = checkJobRunningQuery(camstr.str(), cstr);
        if (runvector.size()>0)
        {     
            std::string camer    = camstr.str();
            std::string recname  = runvector.at(1); 
            if (loadStartQuery(camer, recname))
            {
                startMainRecognition(camnum);
            } else 
            {
                cout << "No matching values for the current arguments." << endl; 
            }
            updateRecStatusByRecName(1, recname);
        }
    }
}

int startUpParams(int argc, const char **argv)
{
    // Run Params
    if ( argc >= 2 ) 
    {
        std::string doparam = argv[1];
        //std::string param = argv[2];
        if (doparam=="-status")
        {
            status();
            return 0;
        }
        else if (doparam=="-start")
        {
            std::string param_camera = argv[2];
            std::string param_name   = argv[3];
            
            bool load = loadStartQuery(param_camera, param_name); 
            if (load)
            {
                startMainRecognition(atoi(param_camera.c_str()));
            } else 
            {
                cout << "No matching values for the current arguments." << endl; 
            }
        } 
        else if (doparam=="-stop")
        {
            
        }
    }
   
    struct timeval tr;
    struct tm* ptmr;
    char time_rasp[40];
    gettimeofday (&tr, NULL);
    ptmr = localtime (&tr.tv_sec);
    strftime (time_rasp, sizeof (time_rasp), "%Y-%m-%d %H:%M:%S %z", ptmr);
    
    //Activity
    stringstream allparams;
    allparams << argv;
    stringstream sql_activity;
    sql_activity <<
    "INSERT into activity (params, run_time) VALUES ('"<< allparams.str() << "', '" << time_rasp << "');";
    db_execute(sql_activity.str().c_str());
    
    return 0;
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