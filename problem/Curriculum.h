//
// Created by Alexandre Lemos on 10/08/2019.
//

#ifndef PROJECT_CURRICULUM_H
#define PROJECT_CURRICULUM_H


#include <set>
#include "Room.h"
#include "Class.h"
#include "ConstraintShort.h"

class Curriculum {
    int id;
    std::set<ClusterbyRoom*> pClass;
    std::vector<ConstraintShort*>* range;



public:
    Curriculum(ClusterbyRoom *c, std::vector<ConstraintShort*>* ra) :range(ra){pClass.insert(c); }
    Curriculum(std::set<ClusterbyRoom*> pClass,std::vector<ConstraintShort*>*ra) :range(ra),pClass(pClass) {}

    void setRange(std::vector<ConstraintShort*>* newRange){ range=newRange;}

    std::vector<ConstraintShort*>* getRange(){ return range;}

    void addClass(ClusterbyRoom * c){
        pClass.insert(c);
    }


    bool operator==(const Curriculum &rhs) const {
        return id == rhs.id &&
               pClass == rhs.pClass;
    }

    bool operator!=(const Curriculum &rhs) const {
        return !(rhs == *this);
    }

     std::set<ClusterbyRoom*> getPClass()  {
        return pClass;
    }



};


#endif //PROJECT_CURRICULUM_H