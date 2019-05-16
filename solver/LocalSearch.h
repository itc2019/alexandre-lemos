//
// Created by Alexandre Lemos on 15/03/2019.
//

#ifndef TIMETABLER_GRASP_H
#define TIMETABLER_GRASP_H


#include <fstream>
#include "../problem/Instance.h"
#include <climits>
#include <cfloat>
#include <random>
#include <chrono>

extern double getTimeSpent();

class LocalSearch {
protected:
    int MAX_ITERATIONS = 5;
    Instance *instance;
    std::vector<Solution*> tabu;
    unsigned long long int best = ULLONG_MAX;
    unsigned long long int currentV = ULLONG_MAX;
    Solution** current;
    std::ofstream seedFile;
    int sizeRCL;
    bool **stu;
    bool **stuSub;
    int sizeRCLS;
    int totalNassigment;

public:
    LocalSearch(int MAX_ITERATIONS, double rcl, Instance *instance);

    virtual void GRASP();

    virtual void LNS();

    LocalSearch(Instance *pInstance);

protected:

    virtual void init();

    virtual bool eval();

    virtual void store();

    virtual void printStatus(int ite);

    virtual void printFinal();

    virtual void Greedy();

    virtual void Local();

    virtual unsigned long long int
    isAllocable(int lectureID, std::string week, std::string day, int start, int duration, int roomID);


    virtual double cost(int lectureID, int roomID, int i, int i1);

    virtual unsigned int seedHandler();

    virtual int assign(Solution *);

    virtual int getGAP() const;


    virtual bool tryswampLectures(int l1, const std::string &l2, std::string &d1, int t1, int le1);

    virtual void
    swampLectures(int lect1, const std::string &lect2, std::string &day, int start, int lenght);

    virtual int getGAPStored();

    virtual int checkUpdate(int maxCost, int id, int time, const std::pair<Room, int> &room);

    virtual int checkUpdate(int maxCost, int id, int time) {};


    bool tryswampLectures(int lecture, int roomID);
};


#endif //PROJECT_GRASP_H
