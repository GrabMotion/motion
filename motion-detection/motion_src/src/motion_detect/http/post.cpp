/*
 * File:   post.cpp
 * Author: jose
 *
 * Created on Dec 12, 2015, 7:48 PM
 */

#include "../http/post.h"
#include "../utils/utils.h"
#include "../database/database.h"
#include "../http/json.h"
#include <stdio.h>

//Location
#define pi 3.14159265358979323846
#define earthRadiusKm 6371.0
// This function converts decimal degrees to radians

double deg2rad(double deg) {
  return (deg * pi / 180);
}

/**
 * Returns the distance between two points on the Earth.
 * Direct translation from http://en.wikipedia.org/wiki/Haversine_formula
 * @param lat1d Latitude of the first point in degrees
 * @param lon1d Longitude of the first point in degrees
 * @param lat2d Latitude of the second point in degrees
 * @param lon2d Longitude of the second point in degrees
 * @return The distance between the two points in kilometers
 */
double distanceEarth(double lat1d, double lon1d, double lat2d, double lon2d) {
  double lat1r, lon1r, lat2r, lon2r, u, v;
  lat1r = deg2rad(lat1d);
  lon1r = deg2rad(lon1d);
  lat2r = deg2rad(lat2d);
  lon2r = deg2rad(lon2d);
  u = sin((lat2r - lat1r)/2);
  v = sin((lon2r - lon1r)/2);
  return 2.0 * earthRadiusKm * asin(sqrt(u * u + cos(lat1r) * cos(lat2r) * v * v));
}


motion::Message postRecognition(motion::Message m)
{
    // POSTING

    motion::Message::MotionCamera * pcamera = m.mutable_motioncamera(0);
    motion::Message::MotionRec * prec = pcamera->mutable_motionrec(0);
    
    std::string recname = prec->recname();
     //time
    struct timeval tr;
    struct tm* ptmr;
    char time_rasp[40];
    gettimeofday (&tr, NULL);
    ptmr = localtime (&tr.tv_sec);
    strftime (time_rasp, sizeof (time_rasp), "%Y-%m-%d %H:%M:%S %z", ptmr);
    
    vector<std::string> mat_array = getMatInfoFromId(prec->db_idmat());
    std::string path = mat_array.at(1);
    bool matexist = file_exists(mat_array.at(1));
    if (matexist)
    {
        string loadedmat = get_file_contents(path);

        cv::Mat mat = extractMat(loadedmat);

        cv::Scalar red(255,0,0);
        
        std::string rcoords = m.datafile();

        mat = drawRectFromCoordinate(rcoords, mat, red);

        std::string trackdatafile = basepath + "data/tracking"; 
        directoryExistsOrCreate(trackdatafile.c_str());
        std::string recognitions = trackdatafile + "/recognitions";
        directoryExistsOrCreate(recognitions.c_str());

        std::string fineandextension = recname + ".jpg";
        std::string maximagepath = recognitions + "/" + fineandextension;
        imwrite(maximagepath, mat);

        cout << "posting: " << fineandextension << endl;

        bool jpgexist = file_exists(maximagepath);
        if (jpgexist)
        {
            //int id = getPostByIdAndType(idmedia);

            vector<std::string> camerainfo = getTrackPostByTypeAndIdLocal("camera", pcamera->db_idcamera());
            std::string post_parent;
            if (camerainfo.size()>0)
            {
               post_parent = camerainfo.at(1);
            }

            vector<std::string> recinfo = getTrackPostByTypeAndIdLocal("recognition", prec->db_idrec());
            
            std::string postlink;
            std::string api_url;
            bool rec_update = false;
            int idmedia;
            
            if (recinfo.size()>0)
            {
               postlink = recinfo.at(6);
               api_url  = recinfo.at(7);
               idmedia  = atoi(recinfo.at(8).c_str());
               rec_update = true;
               
            } else 
            {
                std::stringstream media;
                media << "curl --user " << WP_USER << ":" << WP_PASS << " -X POST -H 'Content-Disposition: filename=" << fineandextension << 
                "' --data-binary @'"<< maximagepath << "' -d title='" << recname << "' -H \"Expect: \" " << SERVER_BASE_URL << "media";

                idmedia = post_command_to_wp(false, media.str(), prec->db_idmat());
            }
            
            std::string media_url;
            vector<std::string> media_attahced = getTrackPostByTypeAndIdRemote("attachment", idmedia);
            if (media_attahced.size()>0)
            {
                media_url  = media_attahced.at(6);
            }

            std::string posttype;
            std::string url;
            if (rec_update)
            {
                posttype = "PUT";
                url = api_url;
            } else
            {
                posttype = "POST";
                url = SERVER_BASE_URL + "recognition";
            }

            std::string codename            = prec->codename();
            std::string region              = rcoords;
            int delay                       = prec->delay();
            bool runatstartup               = prec->runatstartup();
            bool hascron                    = prec->hascron();
            std::string screen              = mat_array.at(0);
            
            std::string coordsnb = region;    
            coordsnb.erase(std::remove(coordsnb.begin(), coordsnb.end(), '\n'), coordsnb.end());

            bool running = getRecRunningByName(recname);
            
            std::string intervals = getIntervalByIdRecognitionSetupId(prec->db_idrec());
            
            std::stringstream recognition_post;
            recognition_post << "curl --user " << WP_USER << ":" << WP_PASS << " -X " << posttype << " -d " << 
            "'{\"title\":\""            << recname     <<  "\","    <<
            "\"content_raw\":\"Content\",\"content\":\" \","        <<
            "\"excerpt_raw\":\"Excerpt\",\"status\":\"publish\","   <<
            "\"featured_image\":\""             << idmedia          <<  "\","   <<                
            "\"recognition_name\":\""           << recname          <<  "\","   <<
            "\"recognition_codename\":\""       << prec->codename() <<  "\","   <<                
            "\"recognition_region\":\""         << escape(region)   <<  "\","   <<                        
            "\"recognition_delay\":\""          << prec->delay()    <<  "\","   <<                        
            "\"recognition_runatstartup\":\""   << prec->runatstartup()         <<  "\","   <<                        
            "\"recognition_hascron\":\""        << prec->hascron()              <<  "\","   <<                                               
            "\"recognition_interval\":\""       << intervals                    <<  "\","   <<                                                       
            "\"recognition_screen\":\""         << screen                       <<  "\","   <<                                   
            "\"recognition_running\":\""        << running                      <<  "\","   <<                                   
            "\"recognition_created\":\""        << prec->created()              <<  "\","   <<                                   
            "\"post_parent\":\""                << post_parent                  <<  "\","   << 
            "\"recognition_media_url\":\""      << escape(media_url)            <<  "\","    << 
            "\"recognition_keepalive_time\":\"" << time_rasp                    <<  "\"}'"      <<   
            " -H \"Content-Type:application/json\" -H \"Expect: \""             <<
            " " << url;

            cout << "recognition_post: " << recognition_post.str() << endl;

            int post = post_command_to_wp(false, recognition_post.str(), prec->db_idrec());
            
            if (post)
            {
                //Reset to recognizing = 0 to all jobs
                updateRecStatusByRecName(0, recname);
            }
            
        } 
    }
    
    return m;
}

int instancePost(motion::Message::Instance pinstance, int db_instance_id, int post_parent)
{
    int post;
    vector<string> max_image = getMaxImageByPath(db_instance_id);
    if (max_image.size()==0)
        return 0;
    
    int db_local        = atoi(max_image.at(0). c_str());
    std::string maxpath = max_image.at(1);
    std::string maxname = max_image.at(2);
    std::string maxday  = max_image.at(3);
    std::string maxrec  = max_image.at(4);
    std::string maxtime = max_image.at(5);
    std::string coords  = max_image.at(6);

    cv::Mat mat = getImageWithTextByPath(maxpath);
    cv::Scalar yellow(0,255,255);
    mat = drawRectFromCoordinate(coords, mat, yellow);
    
    std::string trackdatafile = basepath + "data/tracking"; 
    directoryExistsOrCreate(trackdatafile.c_str());
    std::string dayname = trackdatafile + "/" + maxday;
    directoryExistsOrCreate(dayname.c_str());
    std::string recname = dayname + "/" + maxrec;
    directoryExistsOrCreate(recname.c_str());
    
    cout << "files and forlders created" << endl;
    
    vector<string> name_vector;
    split(maxname.c_str(), '/', name_vector);
    
    std::string fineandextension = name_vector.at(2) + ".jpg";
    std::string maximagepath = recname + "/" + fineandextension;
    imwrite(maximagepath, mat);

    cout << "posting: " << fineandextension << endl;
    
    std::stringstream media;
    media << "curl --user " << WP_USER << ":" << WP_PASS << " -X POST -H 'Content-Disposition: filename=" << 
    fineandextension << "' --data-binary @'"    << 
    maximagepath << "' -d title='"              << 
    maxtime << "' -H \"Expect: \" "             << 
    SERVER_BASE_URL << "media";

    int idmedia = post_command_to_wp(false, media.str(), db_local);
    
    std::string media_url;
    vector<std::string> media_attahced = getTrackPostByTypeAndIdRemote("attachment", idmedia);
    if (media_attahced.size()>0)
    {
        media_url  = media_attahced.at(6);
    }
    
    if (idmedia)
    {
        
        int id_featured_image = getPostByIdAndType(idmedia);
        
        std::string url = SERVER_BASE_URL + "instance";
        
        std::stringstream instance_post;
        instance_post << "curl --user " << WP_USER << ":" << WP_PASS << " -X POST -d " << 
        "'{\"title\":\""            << maxtime                  <<  "\","       <<
        "\"content_raw\":\"Content\",\"content\":\" \","        <<
        "\"excerpt_raw\":\"Excerpt\",\"status\":\"publish\","   <<
        "\"post_parent\":\""            << post_parent          <<  "\","   <<                
        "\"featured_image\":\""         << id_featured_image            <<  "\","   <<                
        "\"instance_number\":\""        << pinstance.instance()         <<  "\","   <<
        "\"instance_begintime\":\""     << pinstance.instancestart()    <<  "\","   <<
        "\"instance_endtime\":\""       << pinstance.instanceend()      <<  "\","   <<
        "\"instance_code\":\""          << pinstance.instancecode()     <<  "\","   <<
        "\"instance_media_url\":\""     << media_url                    <<  "\"}'"  <<    
        " -H \"Content-Type:application/json\" -H \"Expect: \"" <<
        " " << url;
        
        cout << "instance_post: " << instance_post.str() << endl;

        post = post_command_to_wp(false, instance_post.str(), db_instance_id);
       
        //PUSH NOTIFICATION.
        
    
    } else 
    {
        return 0;
    }
    
    return post;

}

int insertUpdatePostInfo(bool update, std::string message, int db_local)
{
    
    cout << "post_response: " <<  message << endl;
    
    vector<std::string> resutl_post = parsePost(message, 0);
            
    string id                   = resutl_post.at(0);
    std::string date            = resutl_post.at(1);
    std::string modified        = resutl_post.at(2);
    std::string slug            = resutl_post.at(3);
    std::string post_type       = resutl_post.at(4);
    std::string link            = resutl_post.at(5);
    std::string api_link        = resutl_post.at(6);
    std::string featured_image  = resutl_post.at(7);   
    std::string post_parent     = resutl_post.at(8);   
    
    if (!update)
    {
        //Update Time from Server
        const char *timestr = modified.c_str();
        struct tm server_time;
        strptime(timestr, "%Y-%m-%dT%H:%M:%SZ", &server_time);
        printf("The system time is set to %s\n", setTimeToRaspBerry(server_time, -3));
        insertIntoPosts(id, date, modified, slug, post_type, link, api_link, featured_image, post_parent, db_local);  

    } else if (update)
    {
        updateIntoPost(id, date, modified);
    } 
  
    return atoi(id.c_str());
    
}

int post_command_to_wp(bool update, std::string command, int db_local)
{
    int id = 0;
    
    cout << "post_command: " << command << endl;
    
    FILE* pipe = popen(command.c_str(), "r");
    int size = sizeof(pipe);
    char buffer[128];
    std::string result = "";
    std::string error;
    
    std::stringstream post_response;
    while(!feof(pipe)) 
    {
    	if(fgets(buffer, sizeof(buffer), pipe) != NULL)
        {
            post_response << buffer; 
        }       
    }
    pclose(pipe);
    char *s;
    s = strstr(buffer, "<?xml");      
    if (s != NULL)                     
    {
        TiXmlDocument doc;
        doc.Parse(post_response.str().c_str());
        TiXmlNode * titlenode = doc.RootElement()->FirstChild( "head" )->FirstChild( "title" ); //Tree root
        error = titlenode->Value();

    } else 
    {
        id = insertUpdatePostInfo(update, post_response.str(), db_local);
    }
    return id;
}

std::string get_command_to_wp(std::string command)
{
    FILE* pipe = popen(command.c_str(), "r");
    int size = sizeof(pipe);
    char buffer[128];
    std::string result = "";
    std::string error;
    
    std::stringstream post_response;
    while(!feof(pipe)) 
    {
    	if(fgets(buffer, sizeof(buffer), pipe) != NULL)
        {
            post_response << buffer; 
        }       
    }
    pclose(pipe);
    return post_response.str();
}

vector<std::string> get_command_to_array_wp(std::string url)
{
    int id = 0;
    
    std::string command = "curl " + url;
    cout << "get_command: " << command << endl;
    
    FILE* pipe = popen(command.c_str(), "r");
    int size = sizeof(pipe);
    char buffer[128];
    std::string result = "";
    std::string error;
    
    std::stringstream post_response;
    while(!feof(pipe)) 
    {
    	if(fgets(buffer, sizeof(buffer), pipe) != NULL)
        {
            post_response << buffer; 
        }       
    }
    pclose(pipe);
    char *s;
    s = strstr(buffer, "<?xml");      
   
    return parsePost(post_response.str(), 0);
}



void locationPost(bool update,  vector<std::string> locationinfo )
{
    
    int db_local;
    
    std::string as; 
    std::string city;
    std::string country;
    std::string countryCode;
    std::string isp;
    std::string lat;
    std::string lon;
    std::string queryIp;
    std::string region;
    std::string regionName;
    std::string time_zone;
    std::string zip;
    
    db_local    = atoi(locationinfo.at(0).c_str());      
    
    as          = locationinfo.at(1);
    city        = locationinfo.at(2);        
    country     = locationinfo.at(3);        
    countryCode = locationinfo.at(4);            
    isp         = locationinfo.at(5);    
    lat         = locationinfo.at(6);    
    lon         = locationinfo.at(7);    
    queryIp     = locationinfo.at(8);        
    region      = locationinfo.at(9);        
    regionName  = locationinfo.at(10);            
    time_zone   = locationinfo.at(11);            
    zip         = locationinfo.at(12);

    std::stringstream clientapi;
    clientapi << SERVER_BASE_URL << "client/" <<  CLIENT_ID;
    std::string client_api = escape(clientapi.str());
    
    bool post_location      = false;
    
    std::string posttype;
    std::string url_location;
    
    if (update)
    {
        posttype = "PUT";
        url_location = locationinfo.at(3);
        post_location = false;
    } else 
    {
        posttype = "POST";
        url_location = SERVER_BASE_URL + "location";
        post_location = true;
    }
   
    if (post_location)
    {
        std::stringstream location_post;
        location_post << "curl --user " << WP_USER << ":" << WP_PASS << " -X " << posttype << " -d "   << 
        "'{\"title\":\""            << city                     <<  "\","       <<
        "\"content_raw\":\"Content\",\"content\":\" \","        <<
        "\"excerpt_raw\":\"Excerpt\",\"status\":\"publish\","   <<
        "\"post_parent\":\""                << CLIENT_ID        <<  "\","   <<                
        "\"locaton_as\":\""                 << as               <<  "\","   <<                
        "\"locaton_city\":\""               << city             <<  "\","   <<        
        "\"locaton_country\":\""            << country          <<  "\","   <<                
        "\"locaton_country_code\":\""       << countryCode      <<  "\","   <<                
        "\"locaton_isp\":\""                << isp              <<  "\","   <<                                       
        "\"locaton_longitude\":\""          << lon              <<  "\","   <<        
        "\"locaton_latitude\":\""           << lat              <<  "\","   <<        
        "\"locaton_region\":\""             << region           <<  "\","   <<                
        "\"locaton_region_name\":\""        << regionName       <<  "\","   <<                        
        "\"locaton_region_time_zone\":\""   << time_zone        <<  "\","   <<                            
         "\"locaton_zip\":\""               << zip              <<  "\"}'"  <<                     
        " -H \"Content-Type:application/json\" -H \"Expect: \"" <<
        " " << url_location;
        
        std::string path =  basepath + "/curl_location.txt";
        std::ofstream outxml;
        outxml.open (path.c_str());
        outxml << location_post.str() << "\n";
        outxml.close();
        
        cout << "location_post: " << location_post.str() << endl;

        post_command_to_wp(false, location_post.str(), db_local);
        
    }
    
    /*std::stringstream url_client;
    url_client << " curl " << SERVER_BASE_URL << "client/" << CLIENT_ID;
    std::string locations_relationship_response = get_command_from_wp(url_client.str());
    vector<string> locations_relationship = parsePost(locations_relationship_response, 1, "client_locations");
    std::string response_locations;
    if (locations_relationship.size()>8)
    {
        response_locations = locations_relationship.at(8);
        
        vector<std::string> locations_restul;
        vector<string> locationsrel;
        split(response_locations, ';', locationsrel);
        if (locationsrel.size()>0)
        {
            for (int t=0; t<locationsrel.size(); t++)
            {
                vector<string> locations_remote;
                split(locationsrel.at(t), ',', locations_remote);
                
                if ( atoi(locations_remote.at(0).c_str()) == atoi(local_location.at(0).at(1).c_str()) )
                {
                    std::stringstream url_client;
                    url_client << " curl " << locations_remote.at(1);
                    std::string location_coordinates_response = get_command_from_wp(url_client.str());
                    std::string response_location_coordinates = parsePost(location_coordinates_response, 2, "locaton_latitude", "locaton_longitude").at(8);    

                    vector<string> locations;
                    split(response_location_coordinates, ',', locations);
                    if (locations.size()>0)
                    {
                        double distance = distanceEarth( atol(latitude.c_str()), atol(locations.at(0).c_str()), atol(longitude.c_str()), atol(locations.at(0).c_str()));
                        if (distance>10)
                        {
                            post_location = true;
                        }
                    }
                }
            }
        }
    } else 
    {
        
    }*/
   
    
}

int dayPost(int db_recid, int db_dayid, std::string label, std::string xml)
{
    int idday;
    
    vector<std::string> recinfo = getTrackPostByTypeAndIdLocal("recognition", db_recid);
    std::string post_parent;
    if (recinfo.size()>0)
    {
        post_parent = recinfo.at(1);
    }

    bool update_day = false;
    std::string day_api_url; 
    vector<std::string> dayinfo = getTrackPostByTypeAndIdParent("day", atoi(post_parent.c_str()));
    if (dayinfo.size()>0)
    {
        update_day= true;
    }
   
    struct timeval tr;
    struct tm* ptmr;
    char time_rasp[40];
    gettimeofday (&tr, NULL);
    ptmr = localtime (&tr.tv_sec);
    strftime (time_rasp, sizeof (time_rasp), "%Y-%m-%d %H:%M:%S %z", ptmr);
    
    std::string posttype;
    std::string url;
    
    if (update_day)
    {
        posttype = "PUT";
        url = dayinfo.at(7);
    } else 
    {
        posttype = "POST";
        url = SERVER_BASE_URL + "day";
    }
 
    std::string day_created = getDayCreatedById(db_dayid);
    
    std::stringstream day_post;
    day_post << "curl --user " << WP_USER << ":" << WP_PASS << " -X " << posttype << " -d "   << 
    "'{\"title\":\""            << label                    <<  "\","       <<
    "\"content_raw\":\"Content\",\"content\":\" \","        <<
    "\"excerpt_raw\":\"Excerpt\",\"status\":\"publish\","   <<
    "\"post_parent\":\""            << post_parent          <<  "\","   <<                
    "\"day_label\":\""              << label                <<  "\","   <<                
    "\"day_xml\":\""                << xml                  <<  "\","   <<
    "\"day_created\":\""            << day_created          <<  "\","   <<        
    "\"day_keepalive_time\":\""     << time_rasp            <<  "\"}'"  <<     
    " -H \"Content-Type:application/json\" -H \"Expect: \""     <<
    " " << url;

    cout << "day_post: " << day_post.str() << endl;

    idday = post_command_to_wp(false, day_post.str(), db_dayid);

    return idday;
}

vector<std::string> parsePost(std::string message, int numArgs, ...)
{
    
    cout << "message: " << endl;
    va_list args;             // define argument list variable
    va_start (args,numArgs);  // init list; point to last
                            //   defined argument
    vector<string> rel_args;
    char resutl[50];
    for (int i=0; i < numArgs; i++) 
    {
        char *str = va_arg (args,char *); // get next argument
        strcat (resutl,str);
        std::stringstream restr;
        restr << resutl;
        std::string res = restr.str();
        rel_args.push_back(res); 
    }

    va_end (args); // cleanup the system stack
  
    vector<std::string> result_post;
    
    int id; 
    std::string date;
    std::string modified;
    std::string slug;
    std::string post_type;
    std::string link;
    std::string api_link;
    std::string featured_image; 
    std::string post_parent; 
    
    std::stringstream _id;
    
    vector<std::string> relation_array;
    std::stringstream customs;
    
    std::string location_latitude;
    std::string location_longitude;
    
    char *cstr = new char[message.length() + 1];
    strcpy(cstr, message.c_str());
    // do stuff
    json_object * jobj = json_tokener_parse(cstr);
     
    enum json_type type;
    json_object_object_foreach(jobj, key, val)
    {
        type = json_object_get_type(val);
         
        std::stringstream ss;
        ss << key;
        std::string strkey = ss.str();
         
        switch (type)
        {
            case json_type_int:
            case json_type_boolean:
            case json_type_double:
            case json_type_string:
                 
               if ( strcmp( key, "id" ) == 0 )
                {
                    id = json_object_get_int(val);
                }
                else if ( strcmp( key, "date" ) == 0 )
                {
                    date = json_object_get_string(val);
                }
                else if ( strcmp( key, "modified" ) == 0 )
                {
                    modified = json_object_get_string(val);
                }
                else if ( strcmp( key, "slug" ) == 0 )
                {
                    slug = json_object_get_string(val);
                }
                else if ( strcmp( key, "type" ) == 0 )
                {
                    post_type = json_object_get_string(val);
                }
                else if ( strcmp( key, "featured_image" ) == 0 )
                {
                    featured_image = json_object_get_string(val);
                }
                else if ( strcmp( key, "link" ) == 0 )
                {
                    link = json_object_get_string(val);
                }
                break;
                    
            case json_type_array:
                if ( strcmp( key, "post_parent" ) == 0 )
                {
                    post_parent = parse_json_array(jobj, key); 
                } else if ( strcmp( key, "location_latitude" ) == 0 )
                {
                    location_latitude = parse_json_array(jobj, key); 
                } else if ( strcmp( key, "location_longitude" ) == 0 )
                {
                    location_longitude = parse_json_array(jobj, key); 
                }          
                break;
        }
    }
     
    delete [] cstr;
    
    std::stringstream apilink;
    apilink << SERVER_BASE_URL << "" << post_type << "/" << id ;
  
     api_link = apilink.str();
     
    _id << id;                                  
    result_post.push_back(_id.str());           // 0 
    result_post.push_back(date);                // 1
    result_post.push_back(modified);            // 2
    result_post.push_back(slug);                // 3
    result_post.push_back(post_type);           // 4
    result_post.push_back(link);                // 5
    result_post.push_back(api_link);            // 6
    result_post.push_back(featured_image);      // 7
    result_post.push_back(post_parent);         // 8
    
    if (relation_array.size()>0)    
    {
        result_post.push_back(customs.str());   // 9
    }
    
    result_post.push_back(location_latitude);         // 10
    result_post.push_back(location_longitude);        // 11
    
    return result_post;
}

void postTerminalStatus()
{
    
    vector<std::string> locationinfo = getTrackPostByType("location");
    std::string post_parent;
    if (locationinfo.size()>0)
    {
        post_parent = locationinfo.at(1);
    }
    
    vector<std::string> typeinfo = getTrackPostByType("terminal");
    bool update = false;
    int count_update;
    std::string api_url;
    if (typeinfo.size()>0)
    {
       api_url = typeinfo.at(7);
       update = true; 
       count_update = atoi(typeinfo.at(10).c_str());
    }

    vector<std::string> terminal_info = getTerminalInfo();

    std::string ipnumber            = terminal_info.at(0);
    std::string ippublic            = terminal_info.at(1);
    std::string macaddress          = escape(terminal_info.at(2));
    std::string hostname            = escape(terminal_info.at(3));
    std::string city                = terminal_info.at(4);
    std::string country             = terminal_info.at(5);
    std::string location            = terminal_info.at(6);
    std::string network_provider    = terminal_info.at(7);
    std::string uptime              = escape(terminal_info.at(8));
    std::string starttime           = terminal_info.at(9);
    int db_local                    = atoi(terminal_info.at(10).c_str());
    std::string model               = escape(terminal_info.at(11));
    std::string hardware            = terminal_info.at(12);
    std::string serial              = terminal_info.at(13);
    std::string revision            = terminal_info.at(14);
    std::string disktotal           = terminal_info.at(15);
    std::string diskused            = terminal_info.at(15);
    std::string diskavailable       = terminal_info.at(16);
    std::string disk_percentage_used= terminal_info.at(17);
    std::string temperature         = terminal_info.at(19);
    std::string created             = terminal_info.at(20);

    struct timeval tr;
    struct tm* ptmr;
    char time_rasp[40];
    gettimeofday (&tr, NULL);
    ptmr = localtime (&tr.tv_sec);
    strftime (time_rasp, sizeof (time_rasp), "%Y-%m-%d %H:%M:%S %z", ptmr);

    std::string posttype;
    std::string url;
    if (update)
    {
        posttype = "PUT";
        url = api_url;
    } else
    {
        posttype = "POST";
        url = SERVER_BASE_URL + "terminal";
    }

    std::stringstream terminal_post;
    terminal_post << "curl --user " << WP_USER << ":" << WP_PASS << " -X "         << posttype << " -d " << 
    "'{\"title\":\""            << hardware     <<  "\","   <<
    "\"content_raw\":\"Content\",\"content\":\"\","     <<
    "\"excerpt_raw\":\"Excerpt\",\"status\":\"publish\","   <<
    "\"terminal_ipnumber\":\""           << ipnumber     <<  "\","   <<
    "\"terminal_public_ipnumber\":\""    << ippublic     <<  "\","   <<        
    "\"terminal_macaddress\":\""         << macaddress   <<  "\","   <<        
    "\"terminal_hostname\":\""           << hostname     <<  "\","   <<                
    "\"terminal_network_provider\":\""   << network_provider         <<  "\","  <<                        
    "\"terminal_uptime\":\""             << uptime       <<  "\","   <<                        
    "\"terminal_starttime\":\""          << starttime    <<  "\","   <<                        
    "\"terminal_model\":\""              << model        <<  "\","   <<                        
    "\"terminal_hardware\":\""           << hardware     <<  "\","   <<                        
    "\"terminal_serial\":\""             << serial       <<  "\","   <<
    "\"terminal_revision\":\""           << revision       <<  "\"," <<       
    "\"terminal_disk_total\":\""         << disktotal    <<  "\","   <<                        
    "\"terminal_disk_used\":\""                 << diskused     <<  "\","   <<                        
    "\"terminal_disk_available\":\""            << diskavailable<<  "\","   <<                        
    "\"terminal_disk_percentage_used\":\""      << disk_percentage_used     <<  "\","   << 
    "\"terminal_temperature\":\""               << temperature              <<  "\","   <<    
    "\"post_parent\":\""                        << post_parent              <<  "\","   <<         
    "\"terminal_temperature\":\""               << temperature              <<  "\","   <<
    "\"terminal_created\":\""                   << created                  <<  "\","   <<
    "\"terminal_keepalive_time\":\""            << time_rasp                <<  "\"}'"  <<   
    " -H \"Content-Type:application/json\" -H \"Expect: \""     <<
    " " << url << ";" ;

    cout << "terminal_post: " << terminal_post.str() << endl;

    post_command_to_wp(update, terminal_post.str(), db_local);
    
}

void postCameraStatus()
{
    
    vector<std::string> terminalinfo = getTrackPostByType("terminal");
    if (terminalinfo.size()>0)
    {
        std::string post_parent = terminalinfo.at(1);
        
        stringstream sql_cams;
        sql_cams   <<
        "SELECT _id, number, name, created FROM cameras;";
        cout << "sql_active_cam: " << sql_cams.str() << endl;
        pthread_mutex_lock(&databaseMutex);
        vector<vector<string> > array_cams = db_select(sql_cams.str().c_str(),4);
        pthread_mutex_unlock(&databaseMutex);

        struct timeval tr;
        struct tm* ptmr;
        char time_rasp[40];
        gettimeofday (&tr, NULL);
        ptmr = localtime (&tr.tv_sec);
        strftime (time_rasp, sizeof (time_rasp), "%Y-%m-%d %H:%M:%S %z", ptmr);

        for (int i=0; i<array_cams.size(); i++)
        {
            
            int db_local_cam = atoi(array_cams.at(i).at(0).c_str());
            std::string cameranumber    = array_cams.at(i).at(1);
            std::string cameraname      = array_cams.at(i).at(2);
            std::string cameracreated   = array_cams.at(i).at(3);
            
            vector<std::string> locationbyid = getTrackPostByTypeAndIdLocal("camera", db_local_cam);
            
            std::string url;
            std::string posttype;
            bool update_camera = false;
            
            if (locationbyid.size()>0)
            {
                url = locationbyid.at(7);
                posttype = "PUT";
                update_camera = true;
            } else
            {
                posttype = "POST";
                url = SERVER_BASE_URL + "camera";
            }
               
            std::stringstream camera_post;
            camera_post << "curl --user " << WP_USER << ":" << WP_PASS << " -X "         << posttype << " -d " << 
            "'{\"title\":\""            << cameraname     <<  "\","   <<
            "\"content_raw\":\"Content\",\"content\":\"\","     <<
            "\"excerpt_raw\":\"Excerpt\",\"status\":\"publish\","           <<
            "\"camera_name\":\""            << cameraname       <<  "\","   <<
            "\"camera_number\":\""          << cameranumber     <<  "\","   <<        
            "\"post_parent\":\""            << post_parent      <<  "\","   <<                             
            "\"camera_created\":\""         << cameracreated    <<  "\","   <<                             
            "\"camera_keepalive_time\":\""  << time_rasp    <<  "\"}'"      <<   
            " -H \"Content-Type:application/json\" -H \"Expect: \""         <<
            " " << url << ";" ;

            cout << "camera_post: " << camera_post.str() << endl;

            post_command_to_wp(update_camera, camera_post.str(), db_local_cam);
            
        }
    }
}

void terminalPost(double timecount)
{
    bool post_terminal = false;
    time_t post_time = getLastPostTime("terminal");
    if (post_time!=NULL)
    {
        time_t now;
        time(&now); 

        double posted = difftime(now, post_time);
       
        if ( posted > timecount ) {
            post_terminal = true;
        }         
    } else {
        post_terminal = true;
    }

    if (post_terminal)
        postTerminalStatus();
}

void locationPost(double timecount)
{
    
    bool post_location = false;
    time_t post_time = getLastPostTime("location");
    if (post_time!=NULL)
    {
        time_t now;
        time(&now); 

        double posted = difftime(now, post_time);
       
        if ( posted > timecount ) 
        {
            post_location = true;
        }         
    } else {
        post_location = true;
    }
    
    if (post_location)
    {    
        //Location post
        vector<std::string> haslocation = getTrackPostByType("location");
        vector<std::string> locationinfo = getLocationInfo();
        
        if (haslocation.size()==0)
        {
            
            locationPost(false, locationinfo);
            
        } else {
            
            std::string location =  locationinfo.at(3);
            vector<string> location_vector;
            split(location.c_str(), ',', location_vector);
        
            double stored_local_latitude = atof(location_vector.at(0).c_str());
            double stored_local_longitude = atof(location_vector.at(1).c_str());
            
            vector<std::string> location_from_server = get_command_to_array_wp(haslocation.at(7));
            
            double stored_server_latitude = atof(location_from_server.at(10).c_str());
            double stored_server_longitude = atof(location_from_server.at(11).c_str());
      
            double measure_distance = distanceEarth(stored_local_latitude, stored_local_longitude, stored_server_latitude, stored_server_longitude);
            
            if(measure_distance>1000)
            {
                locationinfo.push_back(haslocation.at(7));
                locationPost(true, locationinfo);
            }
        }           
    }
}

void cameraPost(double timecount)
{   
    bool post_camera = false;
    time_t post_time = getLastPostTime("camera");
    if (post_time!=NULL)
    {
        time_t now;
        time(&now); 

        double posted = difftime(now, post_time);
       
        if ( posted > timecount ) {
            post_camera = true;
        }         
    } else {
        post_camera = true;
    }
    
    if (post_camera)
        postCameraStatus();          
    
}