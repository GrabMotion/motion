/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Analize.h
 * Author: jose
 *
 * Created on 24 de junio de 2016, 13:13
 */

#ifndef ANALIZE_H
#define ANALIZE_H

class Analize {
public:
    Analize();
    Analize(const Analize& orig);
    virtual ~Analize();
    
    bool process(vector<int> instances);
private:

};

#endif /* ANALIZE_H */

