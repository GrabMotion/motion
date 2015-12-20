/*
 * File:   observer.cpp
 * Author: jose
 *
 * Created on Dec 12, 2015, 7:48 PM
 */

#include "../operations/observer.h"

#include "opencv2/opencv.hpp"
#include "opencv2/highgui/highgui.hpp"

#include "../database/database.h"
#include "../b64/base64.h"
#include "../utils/utils.h"
#include "../http/post.h"


#include <iostream>
#include <string>
#include <sstream>
#include <stdio.h>
#include <dirent.h>

using namespace std;
using namespace cv;

void loadInstancesFromFile()
{
    struct dirent **entry_list_device;
    int count_device;
    struct stat stdev;

    count_device = scandir(dumpinstancefolder.c_str(), &entry_list_device, 0, versionsort);
    if (count_device < 0) 
    {
        return;
    }

    for (int i = 0; i < count_device; i++) 
    {
        struct dirent *entry_device;
        entry_device = entry_list_device[i];

        std::string name = entry_list_device[i]->d_name;

        if (entry_device->d_name[0] != '.' && entry_device->d_name[strlen(entry_device->d_name)-1] != '~') 
        {
            break;
        }

        std::string devicedir = dumpinstancefolder + "/" + entry_device->d_name;

        //std::string dumpfolder = devicedir + "/";

        struct dirent **entry_list_instances;
        int count_inst;
        struct stat stin;

        count_inst = scandir(devicedir.c_str(), &entry_list_instances, 0, versionsort);
        if (count_inst < 0) 
        {
            break;
        }

        for (int j = 0; j < count_inst; j++) 
        {
            struct dirent *entry_inst;
            entry_inst = entry_list_instances[j];

            std::string dumpfile = devicedir + "/" + entry_inst->d_name;
            lstat(dumpfile.c_str(), &stin);
            bool isdir = S_ISDIR(stin.st_mode);
            if (!isdir)
            {
                if(dumpfile.substr(dumpfile.find_last_of(".") + 1) == "dat") 
                {

                    std::string protodata = get_file_contents(dumpfile);
                    std::string oridecoded = base64_decode(protodata);

                    motion::Message::Instance pinstance;

                    pinstance.ParsePartialFromArray(oridecoded.c_str(), oridecoded.size());

                    std::string XMLFILE = pinstance.xmlfile(); 
                    if(!std::ifstream(XMLFILE.c_str()))
                    {
                        build_xml(XMLFILE.c_str());
                    }

                    time_t tbegin = pinstance.begintime();
                    time_t tinit = pinstance.inittime();
                    time_t tend = pinstance.endtime();

                    time_t begin_t = (tbegin - tinit) / CLOCKS_PER_SEC;
                    std::ostringstream begin;
                    begin << begin_t;
                    std::ostringstream end;
                    end << (tend - tinit) / CLOCKS_PER_SEC;
                    writeXMLInstance(XMLFILE, begin.str(), end.str(), pinstance.instance(), pinstance.instancecode());

                    pinstance.set_instancestart(begin.str());
                    pinstance.set_instanceend(end.str());

                    vector<int> images;
                    images.clear();

                    int imagesize = pinstance.image_size();

                    for (int j = 0; j < imagesize; j++)
                    {
                        //Image
                        int db_image_id;
                        const motion::Message::Image & img = pinstance.image(j);
                        db_image_id = insertIntoIMage(img);
                        const motion::Message::Crop & crop = pinstance.crop(j);                
                        insertIntoCrop(crop, db_image_id);
                        images.push_back(db_image_id);
                    } 

                    struct tm t_mend;
                    std::stringstream endt;
                    endt << pinstance.endtime();
                    const char * endtimechar = endt.str().c_str();

                    motion::Message::Video dvideo = pinstance.video();
                    int db_video_id = insertIntoVideo(dvideo);
                    int db_instance_id = insertIntoInstance(pinstance.instance(), &pinstance, endtimechar, db_video_id, images);

                    std::stringstream postcontent;
                    postcontent << 
                    "<p>Camera name:</p> " << pinstance.camera()            << "\n" << 
                    "<p>Camera number:</p> " << pinstance.cameranumber()    << "\n" << 
                    "<p>Recognition Job:</p> " << pinstance.recname()       << "\n" << 
                    "<p>Instance:</p> " << pinstance.instance()             << "\n\n" <<   
                    "<p>Begin time:</p> " << pinstance.begintime()          << "\n" <<           
                    "<p>End time:</p> " << pinstance.endtime()              << "\n";           

                    int post = postInstance(db_instance_id, postcontent.str());

                    if (post)
                    {

                        int removed = remove( dumpfile.c_str() );
                        if( removed != 0 )
                        {
                            cout << "Cannot delete instance dump data file: " << dumpfile << endl;
                            exit(0);
                        } else
                        {
                            cout << "instance data file removed: " << dumpfile << endl;
                        }

                        pinstance.Clear(); 
                    }
                }
            }
            free(entry_inst);
        }
        free(entry_device);          
        free(entry_list_instances);          
    }
    free(entry_list_device);   
}

motion::Message getLocalPtoro()
{
    motion::Message mlocal;
    std::string protofile = "data/data/localproto.txt";
    
    if (file_exists(protofile))
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