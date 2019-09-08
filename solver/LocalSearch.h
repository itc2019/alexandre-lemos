//
// Created by Alexandre Lemos on 15/03/2019.
//

#ifndef TIMETABLER_GRASP_H
#define TIMETABLER_GRASP_H


static const int sizeWindow = 2;

#include <fstream>
#include "../problem/Instance.h"
#include <climits>
#include <cfloat>
#include <random>
#include <chrono>
#include <ostream>
#include "LSDivided.h"
#include <algorithm>

extern void printTime();
extern double getTimeSpent();

class LocalSearch {
protected:
    int maxCons = 0;
    int MAX_ITERATIONS = 5;
    LSDivided *lsDivided;
    int time;
    int alreadyDone = 0;
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
    std::vector<int> problems;
    bool isRoomProblem = false;

public:

    bool ** getStu(){ return stu;}

    LocalSearch(Instance *instance): instance(instance){
        stu = new bool *[instance->getStudent().size()];
        stuSub = new bool *[instance->getStudent().size()];
        int i = 0;
        for (std::map<int, Student>::const_iterator it = instance->getStudent().begin();
             it != instance->getStudent().end(); ++it) {
            stu[i] = new bool[instance->getClasses().size()];
            stuSub[i] = new bool[it->second.getNumbSubpart()];
            for (int j = 0; j < instance->getClasses().size(); ++j) {
                stu[i][j] = 0;
            }
            for (int j = 0; j < it->second.getNumbSubpart(); ++j) {
                stuSub[i][j] = 0;
            }
            totalNassigment += it->second.getNumbSubpart();
            i++;
        }
    }
    LocalSearch(int MAX_ITERATIONS, double rcl, Instance *instance, int seconds);

    virtual void GRASP();

    virtual void LNS();

    void greedyStu();



protected:

    void allRandom();

    virtual void init();

    void tryMove(int i, int i1);

    std::vector<Solution *> findPossibleMove(int i);

    virtual bool eval();

    virtual void store();

    virtual void printStatus(int ite);

    virtual void printFinal();

    virtual void Greedy();

    virtual void Local();

    virtual void Local1();

    void Local2();


    virtual int
    isAllocable(int lectureID, std::string week, std::string day, int start, int duration, int roomID);


    virtual double cost(int lectureID, int roomID, int i, int i1);

    virtual unsigned int seedHandler();

    virtual int assign(Solution *);

    virtual int getGAP() const;


    virtual int tryswampLectures(int l1, const std::string &l2, std::string &d1, int t1, int le1, int i);

    virtual void
    swampLectures(int lect1, const std::string &lect2, std::string &day, int start, int lenght, int i);

    virtual int getGAPStored();

    virtual int checkUpdate(int maxCost, int id, int time, const std::pair<Room*, int> &room);

    virtual int checkUpdate(int maxCost, int id, int time) { return 0; };


    virtual int tryswampRoom(int lecture, int roomID);

    bool stringcompare(std::string s1, std::string s2, int size, bool orAND);

    int isFirst(std::string s1, std::string s2, int size);

    int stuCost(Class *c, Student s);


    void removeElement();

    bool stringcontains(std::string s1, std::string s2, int size);

    void conflict(int lecture);

    bool assignable(int id, int pInt[]);

    void Greedy1();
};


#endif //PROJECT_GRASP_H
