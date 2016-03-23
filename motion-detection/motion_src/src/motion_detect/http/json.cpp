/*
 * File:   json.cpp
 * Author: jose
 *
 * Created on Dec 12, 2015, 7:48 PM
 */

#include "../http/json.h"
#include "../database/database.h"

#include <sstream>
#include <stdio.h>
#include <string.h>

std::string parse_json_array( json_object* j, char* key ) 
{
    std::string val;
    enum json_type type;
    json_object* a = j;
    if ( key ) 
    {
        a = json_object_object_get( j, key );
    }
  
    int l = json_object_array_length( a );
    int i;
    json_object* v;
    for ( i = 0; i < l; i++ ) 
    {
        v = json_object_array_get_idx( a, i );
        type = json_object_get_type( v );
        if ( type == json_type_array ) 
        {
            parse_json_array( v, NULL );
        } else if ( type =json_type_string )
        {
            val = json_object_get_string(v);
        } else 
        {
            val = parse_json( v );
        }
    }
     
    return val;
}

std::string parse_json ( json_object* j )
{
    
    std::string val;
    
    enum json_type type;
     
    json_object_object_foreach( j, key, v ) {
         
        type = json_object_get_type( v );
         
        std::stringstream ss;
        ss << key;
        std::string strkey = ss.str();
         
        //std::cout << std::endl;
        //std::cout << "json key: " << key << std::endl;
        //std::cout << std::endl;
         
        switch ( type ) {
                 
            case json_type_boolean:
            case json_type_double:
            case json_type_int: 
                if ( strcmp( key, "child_id" ) == 0 )
                {
                    val = json_object_get_int(v);
                    return val;
                }
            case json_type_string:
                if ( strcmp( key, "href" ) == 0 )
                {
                    val = json_object_get_string(v);
                    return val;
                }
                else if ( strcmp( key, "child_url" ) == 0 )
                {
                    val = json_object_get_string(v);
                    return val;
                }
                else if ( strcmp( key, "child_api" ) == 0 )
                {
                    val = json_object_get_string(v);
                    return val;
                }
            //case json_type_object:
            //    j = json_object_object_get( j, key );
            //    val = parse_json( j );
            //    break;
                 
            case json_type_array:
                if ( strcmp( key, "self" ) == 0 ) 
                {
                   val = parse_json_array( j, key);
                }
                break;
        }
    }
     
    //std::cout << "RETURN" << std::endl;
     
    return val;
}


vector<std::string> parse_and_store_ipinfo_io(const char *cstr)
{
    
    vector<std::string> location;
            
    json_object * jobj = json_tokener_parse(cstr);
    
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
        
    enum json_type type;
    json_object_object_foreach(jobj, key, val)
    {
        printf("type: ",type);
        type = json_object_get_type(val);
        
        if ( strcmp( key, "as" ) == 0 )
        {
            as = json_object_get_string(val);
        }
        else if ( strcmp( key, "city" ) == 0 )
        {
            city = json_object_get_string(val);
        }
        else if ( strcmp( key, "county" ) == 0 )
        {
            country = json_object_get_string(val);
        }
        else if ( strcmp( key, "countryCode" ) == 0 )
        {
            countryCode = json_object_get_string(val);
        }
        else if ( strcmp( key, "isp" ) == 0 )
        {
            isp = json_object_get_string(val);
        }
        else if ( strcmp( key, "lat" ) == 0 )
        {
            lat = json_object_get_string(val);
        }
        else if ( strcmp( key, "lon" ) == 0 )
        {
            lon = json_object_get_string(val);
        }
        else if ( strcmp( key, "query" ) == 0 )
        {
            queryIp = json_object_get_string(val);
        }       
        else if ( strcmp( key, "region" ) == 0 )
        {
            region = json_object_get_string(val);
        }
        else if ( strcmp( key, "regionName" ) == 0 )
        {
            regionName = json_object_get_string(val);
        }        
        else if ( strcmp( key, "timezone" ) == 0 )
        {
            time_zone = json_object_get_string(val);
        }
        else if ( strcmp( key, "zip" ) == 0 )
        {
            zip = json_object_get_string(val);
        }
    }
    
    location.push_back(as);             // 0
    location.push_back(city);           // 1
    location.push_back(country);        // 2
    location.push_back(countryCode);    // 3
    location.push_back(isp);            // 4
    location.push_back(lat);            // 5
    location.push_back(lon);            // 6
    location.push_back(queryIp);        // 7
    location.push_back(region);         // 8
    location.push_back(regionName);     // 9
    location.push_back(time_zone);      // 10
    location.push_back(zip);            // 11
       
    return location;pi
            
}