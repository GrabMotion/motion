/*
 * File:   camera.cpp
 * Author: jose
 *
 * Created on Dec 12, 2015, 7:48 PM
 */

#include "../operations/camera.h"
#include "../b64/base64.h"
#include "../utils/utils.h"
#include "../database/database.h"

#include <iostream>
#include <sstream>
#include <stdio.h>
#include <ctime>
#include <sys/types.h>

using namespace std;
using namespace cv;

motion::Message::MotionCamera * takePictureToProto(int camera, motion::Message::MotionCamera * mcam)
{
    
    cout << "CAPTURING !!!!!!!!!!!!!" << endl;
    
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
    mcam->set_tempdata(oriencoded.c_str());
      
    std::time_t t = std::time(0);
    stringstream tst;
    tst << t;
    int activemat = atoi(tst.str().c_str());

    cout << "activemat::::: " << activemat << endl; 
    
    mcam->set_activemat(activemat);
    mcam->set_matrows(height_s);
    mcam->set_matcols(width_s);
    mcam->set_matheight(h);
    mcam->set_matwidth(w);
   
    // Initialize a stringstream and write the data
    //int size_init = m.ByteSize();
    
    cout << "mcam->db_idcamera::" << mcam->db_idcamera() << endl;
    
    //Write base64 to file for checking.
    std::string basefile = basepath + "data/mat/" + IntToString(activemat);
    
    std::ofstream out;
    out.open (basefile.c_str());
    out << mcam->tempdata() << "\n";
    out.close();
    
    stringstream insert_mats_query;
    insert_mats_query <<
    "INSERT INTO mat (matcols, matrows, matwidth, matheight, matfile, basefile, data) " <<
    "SELECT " << width_s << ", " << height_s << ", " << w << ", " << h << ", " << activemat << ",'" << basefile << "','" << mcam->tempdata() << "'"
    " WHERE NOT EXISTS (SELECT * FROM mat WHERE matfile = " << activemat << ");";
    db_execute(insert_mats_query.str().c_str());
    std::string last_mats_query = "SELECT MAX(_id) FROM mat";
    vector<vector<string> > mats_array = db_select(last_mats_query.c_str(), 1);
    int db_mats_id = atoi(mats_array.at(0).at(0).c_str());
    cout << "db_mats_id: " << db_mats_id << endl;
    
    mcam->set_db_idmat(db_mats_id);

    stringstream insert_rel_cameras_mats_query;
    insert_rel_cameras_mats_query <<
    "INSERT INTO rel_camera_mat (_id_camera, _id_mat) "     <<
    "SELECT " << mcam->db_idcamera() << "," << db_mats_id   <<
    " WHERE NOT EXISTS (SELECT * FROM rel_camera_mat WHERE _id_camera = "
    << mcam->db_idcamera() << " AND _id_mat = " << db_mats_id << ");";
    db_execute(insert_rel_cameras_mats_query.str().c_str());
    
    cvReleaseCapture(&capture);
    
    return mcam;
}


std::string takeThumbnailFromCamera(int camera)
{
    
    cout << "CAPTURING !!!!!!!!!!!!!" << endl;
    
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
     
    cout << "+++++++++++CREATING THUMBNAIL++++++++++++++" << endl;
    
    Size size(100,75);//the dst image size,e.g.100x100
    Mat dst;//dst image
    resize(mat,dst,size);//resize image

    std:stringstream thumbmat;
    thumbmat << dst << endl;

    std::string thumbnail = thumbmat.str();

    std::string thumbencoded = base64_encode(reinterpret_cast<const unsigned char*>(thumbnail.c_str()), thumbnail.length());
    
    cvReleaseCapture(&capture);
    
    return thumbencoded;
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
