/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Analize.cpp
 * Author: jose
 * 
 * Created on 24 de junio de 2016, 13:13
 */

#include <vector>

#include "../analysis/Analize.h"

#include "../database/database.h"
#include "../b64/base64.h"
#include "../utils/utils.h"
#include "../http/post.h"

using namespace std;

Analize::Analize() 
{
    
}

Analize::Analize(const Analize& orig) 
{
    
}

Analize::~Analize() 
{
    
}

bool process(vector<int> db_instance)
{
    
   for (int i=0; i<db_instance.size(); i++)
   {
       db_instance.at(i);
   }
    
   in runa =  pthread_create(&thread_echo, NULL, ThreadMain, (void *) clntSock);
}

