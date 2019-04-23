//
// Created by Alexandre Lemos on 09/01/2019.
//

#ifndef PROJECT_MIXEDMODELGUROBIEXECUTER_H
#define PROJECT_MIXEDMODELGUROBIEXECUTER_H

#include <exception>
#include <stdlib.h>
#include "../problem/Instance.h"
#include "../solver/TwoVarGurobiExecuter.h"
#include "../solver/roomLectureGRB.h"
#include "../solver/roomLectureBool.h"

class MixedModelGurobiExecuter : public TwoVarGurobiExecuter {
    GRBVar *lectureTime;
    GRBVar **day;
    GRBVar **sameday;
    GRBVar **order;
    GRBVar **gap;


public:

    virtual void save() {
        /*for (int j = 0; j < instance->getClassesWeek(currentW).size(); j++) {
            int day = lectureTime[j].get(GRB_DoubleAttr_X) / instance->getSlotsperday();
            int slot = lectureTime[j].get(GRB_DoubleAttr_X) - day * instance->getSlotsperday();
            solutionTime[day][slot][j] = 1;
        }
        for (int i = 0; i < instance->getRooms().size(); i++) {
            for (int j = 0; j < instance->getClassesWeek(currentW).size(); ++j) {
                if (instance->getClasses()[j]->containsRoom(instance->getRoom(i + 1))) {
                    if (!roomLecture->isStatic())
                        solutionRoom[i][j] = roomLecture->getGRB()[j][i].get(GRB_DoubleAttr_X);
                }
            }
        }*/
        switchSolutionTime();
        switchSolutionRoom();


    }

    void loadPreviousWeekSolution(int ***time, int **room) {
        for (int d = 0; d < instance->getNdays(); ++d) {
            for (int t = 0; t < instance->getSlotsperday(); ++t) {
                for (int i = 0; i < instance->getClassesWeek(currentW).size(); ++i) {
                    if (time[d][t][instance->getClassesWeek(currentW)[i]->getOrderID()]) {
                        model->addConstr(lectureTime[i] == (d * instance->getSlotsperday()) + t);
                    }
                }
            }

        }
        roomLecture->loadPreviousWeekSolution(room);
    }


    MixedModelGurobiExecuter(Instance *i) {
        setInstance(i);
        roomLecture = new roomLectureGRB(instance, currentW);
    }

    void setCurrrentW(int currrentW) {
        ILPExecuter::currentW = currrentW;
        roomLecture->setCurrrentW(currrentW);
    }

    MixedModelGurobiExecuter(bool isStatic, Instance *i) {
        setInstance(i);
        if (isStatic)
            roomLecture = new roomLectureBool(instance, currentW);
        else
            roomLecture = new roomLectureGRB(instance, currentW);
    }

    void definedAuxVar() {

    }

    void printConfiguration() {
        std::cout << "Three variable: type int for time" << std::endl;
        std::cout << "            : type bool for days" << std::endl;
        std::cout << "            : type bool for rooms" << std::endl;

    }

    void dayConst() {
        try {
            for (int k = 0; k < instance->getClassesWeek(currentW).size(); ++k) {
                for (int i = 0; i < instance->getClassesWeek(currentW)[k]->getLectures().size(); ++i) {
                    int j = 0;
                    for (char &c : instance->getClassesWeek(currentW)[k]->getLectures()[i]->getDays()) {
                        if (c == '1') {
                            model->addGenConstrIndicator(day[instance->getClassesWeek(currentW)[k]->getOrderID()][j], 1,
                                                         lectureTime[instance->getClassesWeek(
                                                                 currentW)[k]->getOrderID()] ==
                                                                       instance->getClassesWeek(
                                                                               currentW)[k]->getLectures()[i]->getStart());
                        }
                        j++;
                    }

                }
            }

        } catch (GRBException e) {
            printError(e, "day");
        }
    }


    void definedLectureTime() {
        lectureTime = new GRBVar[instance->getClassesWeek(currentW).size()];
        day = new GRBVar *[instance->getClassesWeek(currentW).size()];

        order = new GRBVar *[instance->getClassesWeek(currentW).size()];
        sameday = new GRBVar *[instance->getClassesWeek(currentW).size() - 1];
        int kn = 0;
        for (Class *k :instance->getClassesWeek(currentW)) {
            order[k->getOrderID()] = new GRBVar[instance->getClassesWeek(currentW).size()];
            sameday[k->getOrderID()] = new GRBVar[instance->getClassesWeek(currentW).size()];

            day[k->getOrderID()] = new GRBVar[instance->getNdays()];
            for (int i = 0; i < instance->getNdays(); ++i) {
                day[k->getOrderID()][i] = model->addVar(0.0, 1.0, 0.0, GRB_BINARY,
                                                        "day" + itos(k->getOrderID()) + "_" + itos(i));
            }
            for (Class *j1 : instance->getClassesWeek(currentW)) {
                if (kn == 0) {
                    std::string name = "A_" + itos(k->getOrderID());

                    lectureTime[j1->getOrderID()] = model->addVar(0.0, instance->getSlotsperday(), 0.0,
                                                                  GRB_INTEGER,
                                                                  name);//instance->getClassesWeek(currentW)[k]->getLectures().size()
                    model->addConstr(
                            lectureTime[j1->getOrderID()] + instance->getClasses()[j1->getOrderID()]->getLenght() <=
                            (((lectureTime[j1->getOrderID()] / instance->getSlotsperday()) + 1) *
                                      instance->getSlotsperday()) -
                                     1);
                }

                order[k->getOrderID()][j1->getOrderID()] = model->addVar(0.0, 1.0, 0.0, GRB_BINARY,
                                                                         "order" + itos(k->getOrderID()) + "_" +
                                                                         itos(j1->getOrderID()));
                model->addGenConstrIndicator(order[k->getOrderID()][j1->getOrderID()], 1,
                                             (lectureTime[k->getOrderID()]
                                              + instance->getClasses()[k->getOrderID()]->getLenght()) <=
                                             lectureTime[j1->getOrderID()]);
                model->addGenConstrIndicator(order[k->getOrderID()][j1->getOrderID()], 0,
                                             (lectureTime[k->getOrderID()] +
                                              instance->getClasses()[k->getOrderID()]->getLenght() - 1) >=
                                             lectureTime[j1->getOrderID()]);

            }
            kn++;
        }
        for (int k = 0; k < instance->getClassesWeek(currentW).size(); ++k) {
            int kID = instance->getClassesWeek(currentW)[k]->getOrderID();
            for (int j1 = 0; j1 < instance->getClassesWeek(currentW).size(); j1++) {
                int jID = instance->getClassesWeek(currentW)[j1]->getOrderID();
                sameday[kID][jID] = model->addVar(0.0, 1.0, 0.0, GRB_BINARY,
                                                  "sameday" + itos(kID) + "_" + itos(jID));
                for (int i = 0; i < instance->getNdays(); ++i) {
                    model->addGenConstrIndicator(sameday[kID][jID], 1,
                                                 day[kID][i]
                                                 == day[jID][i]);
                    model->addGenConstrIndicator(sameday[kID][jID], 0,
                                                 day[kID][i] + day[jID][i] <= 1);

                }


            }
        }


    }

    /**
     * The lecture can only be scheduled in one slot
     */

    void oneLectureperSlot() {
        //Deleted constraint

    }

    /**
     * Force lectures to be in slot n
     */


    void oneLectureSlot() {
        //Not actually needed

    }



    /***
     * The room can only have one lecture per slot without and quadratic expressions
     */

    void oneLectureRoomConflict() {
        roomLecture->oneLectureRoomConflict(order, sameday);

    }


    /** TODO::
    * Ensure Room closed in a day cannot be used
    */
    void roomClosebyDay() {
        for (int d = 0; d < instance->getNdays(); ++d) {
            for (int j = 0; j < instance->getClassesWeek(currentW).size(); ++j) {
                for (int i = 0; i < instance->getClassesWeek(currentW)[j]->getPossibleRooms().size(); i++) {
                    if (instance->isRoomBlockedbyDay(i + 1, d) && !roomLecture->isStatic()) {
                        GRBVar roomC = model->addVar(0.0, 1.0, 0.0, GRB_BINARY,
                                                     "roomC_" + itos(d) + "_" + itos(i) + "_" +
                                                     itos(instance->getClassesWeek(currentW)[j]->getOrderID()));
                        model->addGenConstrIndicator(roomC, 1,
                                                     lectureTime[instance->getClassesWeek(currentW)[j]->getOrderID()]
                                                     / instance->getSlotsperday() == d);
                        //model->addGenConstrIndicator(roomC, 0, lectureTime[j] / instance->getSlotsperday() <= (d-1));
                        //model->addGenConstrIndicator(roomC, 0, lectureTime[j] / instance->getSlotsperday() >= (d+1));
                        if (!roomLecture->isStatic())
                            model->addConstr(
                                    roomLecture->getGRB()[j][i] * roomC == 0);

                    }
                }
            }


        }


    }


    /**
    * Ensure times lot in a day is closed cannot be used
    */
    void slotClose() {
        try {
            for (int i = 0; i < instance->getClassesWeek(currentW).size(); i++) {
                for (int t = 0; t < instance->getNdays() * instance->getSlotsperday(); ++t) {
                    if (instance->isTimeUnavailable(t)) {
                        GRBVar temp = model->addVar(0, 1.0, 0.0, GRB_BINARY);
                        model->addGenConstrIndicator(temp, 0,
                                                     lectureTime[instance->getClassesWeek(currentW)[i]->getOrderID()] >=
                                                     t + 1);
                        model->addGenConstrIndicator(temp, 1,
                                                     lectureTime[instance->getClassesWeek(currentW)[i]->getOrderID()] <=
                                                     t);
                        GRBVar temp1 = model->addVar(0, 1.0, 0.0, GRB_BINARY);
                        model->addGenConstrIndicator(temp1, 1,
                                                     lectureTime[instance->getClassesWeek(currentW)[i]->getOrderID()] >=
                                                     t);
                        model->addGenConstrIndicator(temp1, 0,
                                                     lectureTime[instance->getClassesWeek(currentW)[i]->getOrderID()] <=
                                                     t - 1);
                        model->addConstr((temp1 + temp) <= 1);
                    }

                }


            }
        } catch (GRBException e) {
            printError(e, "slotclose");
        }
    }

    /**TODO::
     * One assignment, is invalid and needs to be assigned
     * to a different room or to a different time slot
     */
    void assignmentInvalid() {
        int value = 0;
        for (int j = 0; j < instance->getNdays(); ++j) {
            for (int i = 0; i < instance->getSlotsperday(); ++i) {
                for (int k = 0; k < instance->getClassesWeek(currentW).size(); ++k) {
                    if (solutionTime[j][i][k] == 1) {
                        value = j + i;
                        goto label;
                    }

                }

            }

        }
        label:
        for (int i = 0; i < instance->getClassesWeek(currentW).size(); ++i) {
            if (instance->isIncorrectAssignment(i)) {
                GRBLinExpr temp = 0;
                for (int l = 0; l < instance->getNumRoom(); ++l) {
                    if (solutionRoom[l][i] == 1) {
                        GRBVar t = model->addVar(0.0, 1.0, 0.0, GRB_BINARY);
                        model->addGenConstrIndicator(t, 1,
                                                     lectureTime[instance->getClassesWeek(currentW)[i]->getOrderID()] ==
                                                     value);
                        // model->addGenConstrIndicator(t, 0, lectureTime[i] <= value - 1);
                        temp = t;// + roomLecture[l][i];
                        model->addConstr(temp <= 1);
                        break;
                    }

                }
            }

        }
    }


    /**TODO::Teacher's conflict*/
    void teacher() {
        for (std::map<std::string, Course *>::const_iterator it = instance->getCourses().begin();
             it != instance->getCourses().end(); it++) {
            for (std::map<int, std::vector<Subpart *>>::iterator sub = it->second->getConfiguratons().begin();
                 sub != it->second->getConfiguratons().end(); ++sub) {
                for (int i = 0; i < sub->second.size(); ++i) {
                    GRBLinExpr conflict = 0;
                    for (int c = 0; c < sub->second[i]->getClasses().size(); c++) {
                        for (int c1 = 0; c1 < sub->second[i]->getClasses().size(); c1++) {
                            if (c1 != c && sub->second[i]->getClasses()[c]->isActive(currentW)
                                && sub->second[i]->getClasses()[c1]->isActive(currentW)) {

                                GRBVar tempT = model->addVar(0.0, 1.0, 0.0, GRB_BINARY,
                                                             "tempT" + it->first + "_" + itos(i) + "_" +
                                                             itos(c) +
                                                             "_" + itos(c1));

                                model->addGenConstrIndicator(tempT, 1, order[c][c1] + order[c1][c] >= 2);
                                model->addGenConstrIndicator(tempT, 0, order[c][c1] + order[c1][c] <= 1);


                                conflict += tempT;
                            }
                        }
                    }
                    model->addConstr(conflict <= sub->second[i]->getOverlap());
                }


            }
        }
    }


    /** Student conflicts hard constraint based on the input model
     *
     */
    void studentConflict() {
        for (std::map<int, Student>::const_iterator it = instance->getStudent().begin();
             it != instance->getStudent().end(); it++) {
            for (int c = 0; c < it->second.getClasses().size(); ++c) {
                for (int j1 = 1; j1 < it->second.getClasses().size(); ++j1) {
                    GRBVar orv[2];
                    GRBVar tempX2 = model->addVar(0.0, 1.0, 0.0, GRB_BINARY);
                    orv[0] = order[c][j1];
                    orv[01] = order[j1][c];
                    model->addGenConstrOr(tempX2, orv, 2);
                    model->addConstr(tempX2 == 1);

                }

            }


        }
    }

    /** Student conflicts hard constraint based on the original solution
     *
     */
    void studentConflictSolution() {
        try {
            for (std::map<int, Student>::const_iterator it = instance->getStudent().begin();
                 it != instance->getStudent().end(); it++) {
                for (int c = 0; c < it->second.getClasses().size(); ++c) {
                    for (int j1 = 1; j1 < it->second.getClasses().size(); ++j1) {
                        model->addConstr(order[c][j1] + order[j1][c] <= 1);
                    }
                }
            }
        } catch (GRBException e) {
            printError(e, "studentConflictSolution");
        }
    }


private:
//Number of seated students for optimization or constraint
    GRBQuadExpr numberSeatedStudents() {
        GRBQuadExpr temp = 0;
        for (int l = 0; l < instance->getNumClasses(); l++) {
            int j = 0;
            for (std::map<int, Room>::const_iterator it = instance->getRooms().begin();
                 it != instance->getRooms().end(); it++) {
                int d = 0;
                for (char &c :instance->getClasses()[l]->getDays()) {
                    if (c != '0') {
                        for (int i = 0; i < instance->getClasses()[l]->getLenght(); i++) {
                            if (it->second.getCapacity() >= instance->getClasses()[l]->getLimit() &&
                                instance->getClasses()[l]->containsRoom(instance->getRoom(j + 1))) {
                                if (roomLecture->isStatic())
                                    temp += instance->getClasses()[l]->getLimit() *
                                            lectureTime[l]
                                            * roomLecture->getBool()[l][j];
                                else
                                    temp += instance->getClasses()[l]->getLimit() *
                                            lectureTime[l]
                                            * roomLecture->getGRB()[l][j];
                            } else if (instance->getClasses()[l]->containsRoom(instance->getRoom(j + 1))) {
                                if (roomLecture->isStatic())
                                    temp += it->second.getCapacity() *
                                            lectureTime[l]
                                            * roomLecture->getBool()[l][j];
                                else
                                    temp += it->second.getCapacity() *
                                            lectureTime[l]
                                            * roomLecture->getGRB()[l][j];
                            }
                        }
                    }
                    d++;
                }
                j++;
            }
        }
        //std::cout<<temp<<std::endl;

        return temp;
    }

    GRBQuadExpr usage() {
        GRBQuadExpr temp = 0;
        for (int l = 0; l < instance->getNumClasses(); l++) {
            int j = 0;
            for (std::map<int, Room>::const_iterator it = instance->getRooms().begin();
                 it != instance->getRooms().end(); it++) {
                int d = 0;
                for (char &c :instance->getClasses()[l]->getDays()) {
                    if (c != '0') {
                        for (int i = 0; i < instance->getClasses()[l]->getLenght(); i++) {
                            if (instance->getClasses()[l]->containsRoom(instance->getRoom(j + 1))) {
                                if (roomLecture->isStatic())
                                    temp += abs(it->second.getCapacity() - instance->getClasses()[l]->getLimit()) *
                                            lectureTime[l]
                                            * roomLecture->getBool()[l][j];
                                else
                                    temp += abs(it->second.getCapacity() - instance->getClasses()[l]->getLimit()) *
                                            lectureTime[l]
                                            * roomLecture->getGRB()[l][j];
                            }
                        }
                    }

                    d++;
                }
                j++;
            }
        }
        //std::cout<<temp<<std::endl;

        return temp;

    }

    void gapStudentsTimetableVar() {
        gap = new GRBVar *[instance->getNumClasses()];
        for (int j = 0; j < instance->getNumClasses(); j++) {
            gap[j] = new GRBVar[instance->getNumClasses()];
            for (int j1 = 0; j1 < instance->getNumClasses(); j1++) {
                if (j1 != j)
                    gap[j][j1] = model->addVar(0.0, 1.0, 0.0, GRB_BINARY,
                                               "gap" + itos(j) + "_" + itos(j1)); //TODO
            }
        }
        for (int l = 0; l < instance->getNumClasses(); ++l) {
            instance->getClasses()[l]->setOrderID(l);
            for (int l1 = 1; l1 < instance->getNumClasses(); ++l1) {
                if (l != l1) {
                    model->addGenConstrIndicator(gap[l][l1], 1,
                                                 (lectureTime[l] + instance->getClasses()[l]->getLenght()) ==
                                                 (lectureTime[l1]));
                    model->addGenConstrIndicator(gap[l][l1], 0, order[l][l1] + gap[l][l1] <= 1);
                    model->addGenConstrIndicator(gap[l1][l], 1,
                                                 (lectureTime[l1] + instance->getClasses()[l1]->getLenght()) ==
                                                 (lectureTime[l]));
                    model->addGenConstrIndicator(gap[l1][l], 0, order[l1][l] + gap[l1][l] <= 1);
                }

            }
        }

    }


    GRBLinExpr gapStudentsTimetable() {
        gapStudentsTimetableVar();
        std::cout << "Aux Var: Done" << std::endl;
        GRBLinExpr min = 0;
        for (int i = 0; i < instance->getStudent().size(); ++i) {
            for (int l = 0; l < instance->getStudent(i + 1).getClasses().size(); ++l) {
                GRBLinExpr all = 0;
                for (int l1 = 1; l1 < instance->getStudent(i + 1).getClasses().size(); ++l1) {
                    if (l != l1) {
                        all += gap[instance->getStudent(i + 1).getClass(l)->getOrderID()][instance->getStudent(
                                i + 1).getClass(l1)->getOrderID()]
                               + gap[instance->getStudent(i + 1).getClass(l1)->getOrderID()][instance->getStudent(
                                i + 1).getClass(l)->getOrderID()];
                    }

                }
                min += (2 - all);

            }


        }

        return min;
        //std::cout << min << std::endl;
    }


public:
    void printdistanceToSolutionLectures(bool w) {
        int temp = 0;

        for (int j = 0; j < instance->getClassesWeek(currentW).size(); j++) {
            int day = lectureTime[instance->getClassesWeek(currentW)[j]->getOrderID()].get(GRB_DoubleAttr_X) /
                      instance->getSlotsperday();
            int slot = lectureTime[instance->getClassesWeek(currentW)[j]->getOrderID()].get(GRB_DoubleAttr_X) -
                       day * instance->getSlotsperday();
            temp += (w ? instance->getClasses()[j]->getLimit() : 1) * (solutionTime[day][slot][j] != 1);
        }
        std::cout << temp << std::endl;


    }

    /***
    * The current distance of the solution with the old solution
    * The distante is base on the weighted Hamming distance of the lectureTime variable (time slot atributions)
    * The weighted is baed on the number of students moved
    * @param oldTime
    * @param weighted
    * @return IloExpr
    */

    //int slot=lectureTime[i].get(GRB_DoubleAttr_X)-day*instance->getSlotsperday();
    GRBQuadExpr distanceToSolutionLectures(int ***oldTime, bool weighted) {
        GRBQuadExpr temp = 0;

        for (int j = 0; j < instance->getNumClasses(); ++j) {
            GRBVar tempT = model->addVar(0.0, 1.0, 0.0, GRB_BINARY);
            GRBVar tempT1 = model->addVar(0.0, 1.0, 0.0, GRB_BINARY);
            model->addGenConstrIndicator(tempT, 0, lectureTime[j] ==
                                                   (instance->getClasses()[j]->getSolStart() +
                                                    instance->getClasses()[j]->getSolDay() *
                                                    instance->getSlotsperday()));
            /* model->addGenConstrIndicator(tempT, 1, lectureTime[j] >=
                                                    (1+instance->getClasses()[j]->getSolStart() +
                                                     instance->getClasses()[j]->getSolDay() *
                                                     instance->getSlotsperday()));*/

            model->addGenConstrIndicator(tempT1, 0, lectureTime[j] ==
                                                    (instance->getClasses()[j]->getSolStart() +
                                                     instance->getClasses()[j]->getSolDay() *
                                                     instance->getSlotsperday()));
            /*model->addGenConstrIndicator(tempT1, 1, lectureTime[j] <=
                                                   (1-instance->getClasses()[j]->getSolStart() +
                                                    instance->getClasses()[j]->getSolDay() *
                                                    instance->getSlotsperday()));*/
            temp += (weighted ? instance->getClasses()[j]->getLimit() : 1) * (tempT + tempT1);
        }


        return temp;
    }


private:
    /**
     * Warm starting procedure with the solution found before
     * Used the class atributes: solutionTime and roomLecture
     */

    void warmStart() {
        for (int k = 0; k < instance->getNdays(); k++) {
            for (int i = 0; i < instance->getSlotsperday(); i++) {
                for (int j = 0; j < instance->getNumClasses(); j++) {
                    if (solutionTime[k][i][j] && !instance->isTimeUnavailable(k * i))
                        lectureTime[j].set(GRB_DoubleAttr_Start, k * i);
                }
            }
        }
        roomLecture->warmStart(solutionRoom);
    }


    /**
     * Switch solution time
     * Updates the solution time structure with new data
     * @Requires delete the previous found solution
     *
     */

    void switchSolutionTime() {
        for (int i = 0; i < instance->getClassesWeek(currentW).size(); i++) {
            int day = lectureTime[instance->getClassesWeek(currentW)[i]->getOrderID()].get(GRB_DoubleAttr_X)
                      / instance->getSlotsperday();
            int slot = lectureTime[instance->getClassesWeek(currentW)[i]->getOrderID()].get(GRB_DoubleAttr_X)
                       - day * instance->getSlotsperday();
            //std::cerr<<day<<" "<<slot<<std::endl;

            solutionTime[day][slot][i] = 1;
            instance->getClassesWeek(currentW)[i]->setSolutionTime(slot,
                                                                   strdup(std::to_string(day).c_str()));


        }

    }

    void force() {
        roomLecture->force(solutionRoom, lectureTime, solutionTime);
    }

    /*
     * for (int l = 0; l < instance->getClassesWeek(currentW).size(); ++l) {
            GRBLinExpr orL=0;
            for (int temp: instance->getClassesWeek(currentW)[l]->getSlots()) {
                GRBVar t = model->addVar(0,1,0,GRB_BINARY,"TempG_"+itos(l)+itos(temp));
                model->addGenConstrIndicator(t,0,lectureTime[l] >= temp);
                model->addGenConstrIndicator(t,1,lectureTime[l] <= temp-1);//Not acceptable
                GRBVar t1 = model->addVar(0,1,0,GRB_BINARY,"TempG1_"+itos(l)+itos(temp));
                model->addGenConstrIndicator(t1,1,lectureTime[l] >= temp+1);//Not acceptable
                model->addGenConstrIndicator(t1,0,lectureTime[l] <= temp);
                GRBVar t2= model->addVar(0,1,0,GRB_BINARY,"TempOR_"+itos(l)+itos(temp));
                model->addGenConstrIndicator(t2,1,lectureTime[l] ==temp);
                model->addGenConstrIndicator(t2,0,t1+t==1);//Not acceptable OR
                orL+=t2;
            }
            model->addConstr(orL==1);
        }
     */
    virtual void block() {
        try {
        for (int l = 0; l < instance->getClassesWeek(currentW).size(); ++l) {
            for (int temp = 0; temp < instance->getSlotsperday(); temp++) {
                if (*instance->getClassesWeek(currentW)[l]->getSlots(instance->minTimeSlot()).find(temp) != temp) {
                    GRBVar t = model->addVar(0, 1, 0, GRB_BINARY, "TempG_" + itos(l) + itos(temp));
                    model->addGenConstrIndicator(t, 0, lectureTime[l] >= temp);
                    model->addGenConstrIndicator(t, 1, lectureTime[l] <= temp - 1);//Not acceptable
                    GRBVar t1 = model->addVar(0, 1, 0, GRB_BINARY, "TempG1_" + itos(l) + itos(temp));
                    model->addGenConstrIndicator(t1, 1, lectureTime[l] >= temp + 1);//Not acceptable
                    model->addGenConstrIndicator(t1, 0, lectureTime[l] <= temp);
                    model->addConstr(t1 + t >= 1);
                }
            }
        }
        } catch (GRBException e) {
            printError(e, "block");
        }

    }

    /**
     * Given classes cannot overlap in time, and if they are placedon overlapping days of week and weeks, they must
     * be placed far enough so that the attendees can travel between the two classes. This means that
     * (Ci.end + Ci.room.travel[Cj .room] ≤ Cj .start) ∨ (Cj .end + Cj .room.travel[Ci.room] ≤ Ci.start) ∨
     * ((Ci.days and Cj .days) = 0) ∨ ((Ci.weeks and Cj .weeks) = 0) for any two classes Ci and Cj from the constraint;
     * Ci.room.travel[Cj .room] is the travel time between the assigned rooms of Ci and Cj .
     * @param c
     * @param pen
     * @return
     */

    virtual GRBLinExpr travel(std::vector<Class *> c, int pen) {
        try {
            return roomLecture->travel(c, lectureTime, sameday, pen);
        } catch (GRBException e) {
            printError(e, "travel");
        }

    }

    /**
      * Precedence Given classes must be one after the other in the order provided in
      *  the constraint definition. For classes that have multiple meetings in a week or
      *  that are on different weeks, the constraint only cares about the first meeting
      *  of the class. That is,
      *      – the first class starts on an earlier week or
      *      – they start on the same week and the first class starts on an earlier day of
      *      the week or
      *      – they start on the same week and day of the week and the first class is
      *      earlier in the day.
      * @param vector
      * @param penalty
     **/


    virtual GRBLinExpr precedence(const std::vector<Class *, std::allocator<Class *>> &vector, int penalty) {
        try {
            GRBLinExpr result = 0;
            for (int c1 = 1; c1 < vector.size(); c1++) {
                if (vector[c1 - 1]->isActive(currentW) && vector[c1]->isActive(currentW)) {
                        GRBLinExpr t = 0;
                        GRBLinExpr prev = 0;
                        GRBLinExpr curre = 0;
                        for (int d = 0; d < instance->getNdays(); ++d) {
                            curre += day[vector[c1]->getOrderID()][d];
                            GRBVar sameD = model->addVar(0, 1, 0, GRB_BINARY);
                            model->addGenConstrIndicator(sameD, 0,
                                                         curre == 0);
                            model->addGenConstrIndicator(sameD, 0,
                                                         prev == 0);
                            model->addGenConstrIndicator(sameD, 0,
                                                         day[vector[c1 - 1]->getOrderID()][d] == 1);
                            model->addGenConstrIndicator(sameD, 1,
                                                         curre == 0);
                            model->addGenConstrIndicator(sameD, 0,
                                                         prev >= 1);
                            model->addGenConstrIndicator(sameD, 0,
                                                         day[vector[c1 - 1]->getOrderID()][d] == 1);

                            prev += day[vector[c1 - 1]->getOrderID()][d];
                            t += sameD;

                        }
                        if (penalty == -1)
                            model->addConstr(order[vector[c1 - 1]->getOrderID()][vector[c1]->getOrderID()] + t >= 1);
                        else {
                            GRBVar pen = model->addVar(0, 1, 0, GRB_BINARY);
                            model->addGenConstrIndicator(pen, 1,
                                                         order[vector[c1 - 1]->getOrderID()][vector[c1]->getOrderID()] +
                                                         t >= 1);
                            model->addGenConstrIndicator(pen, 0,
                                                         order[vector[c1 - 1]->getOrderID()][vector[c1]->getOrderID()] +
                                                         t == 0);
                            result += pen * penalty;

                        }
                    }


            }
            return result;
        } catch (GRBException e) {
            printError(e, "precedence");
        }
    }

    /**
     * Given classes must start at the same time slot, regardless of their
     * days of week or weeks. This means that Ci.start = Cj .start for any two
     * classes Ci and Cj from the constraint; Ci.start is the assigned start time slot
     * of a class Ci.
     * @param vector
     * @param penalty
     * @return
     */

    GRBLinExpr sameStart(const std::vector<Class *, std::allocator<Class *>> &vector, int penalty) {
        try {
            GRBLinExpr result = 0;
            for (int c1 = 0; c1 < vector.size(); c1++) {
                for (int c2 = c1 + 1; c2 < vector.size(); ++c2) {
                    if (vector[c1]->isActive(currentW) && vector[c2]->isActive(currentW)) {
                        if (penalty == -1) {
                            model->addConstr(
                                    lectureTime[vector[c1]->getOrderID()] == lectureTime[vector[c1]->getOrderID()]);
                        } else {
                            GRBVar temp = model->addVar(0, 1, 0, GRB_BINARY);
                            model->addGenConstrIndicator(temp, 1, lectureTime[vector[c1]->getOrderID()] >=
                                                                  lectureTime[vector[c1]->getOrderID()] + 1);
                            model->addGenConstrIndicator(temp, 0, lectureTime[vector[c1]->getOrderID()] <=
                                                                  lectureTime[vector[c1]->getOrderID()]);
                            GRBVar temp1 = model->addVar(0, 1, 0, GRB_BINARY);
                            model->addGenConstrIndicator(temp1, 1, lectureTime[vector[c1]->getOrderID()] <=
                                                                   lectureTime[vector[c1]->getOrderID()] - 1);
                            model->addGenConstrIndicator(temp1, 0, lectureTime[vector[c1]->getOrderID()] >=
                                                                   lectureTime[vector[c1]->getOrderID()]);
                            result += (temp1 + temp) * penalty;
                        }

                    }
                }
            }
            return penalty;
        } catch (GRBException e) {
            printError(e, "DifferentDays");
        }
    }

    /**
     * Given classes must be taught on different days of the week,
     * regardless of their start time slots and weeks. This means that
     * (Ci.days and Cj .days) = 0
     * for any two classes Ci and Cj from the constraint; doing binary ,,and” between
     * the bit strings representing the assigned days.
     * @param vector
     * @param penalty
     * @param b
     * @return
     */

    virtual GRBLinExpr differentDay(const std::vector<Class *, std::allocator<Class *>> &vector, int penalty, bool b) {
        try {
            GRBLinExpr result = 0;
            for (int c1 = 0; c1 < vector.size(); c1++) {
                for (int c2 = c1 + 1; c2 < vector.size(); ++c2) {
                    if (vector[c1]->isActive(currentW) && vector[c2]->isActive(currentW)) {
                        for (int d = 0; d < instance->getNdays(); ++d) {
                            if (penalty == -1) {
                                if (b) {
                                    model->addConstr(
                                            day[vector[c1]->getOrderID()][d] + day[vector[c2]->getOrderID()][d] <= 1);
                                } else {
                                    model->addConstr(
                                            day[vector[c1]->getOrderID()][d] + day[vector[c2]->getOrderID()][d] == 2);
                                }
                            } else {
                                GRBVar tempDay = model->addVar(0, 1, 0, GRB_BINARY);
                                model->addGenConstrIndicator(tempDay, (b ? 1 : 0),
                                                             day[vector[c1]->getOrderID()][d] +
                                                             day[vector[c2]->getOrderID()][d] <= 1);
                                model->addGenConstrIndicator(tempDay, (b ? 0 : 1),
                                                             day[vector[c1]->getOrderID()][d] +
                                                             day[vector[c2]->getOrderID()][d] == 2);
                                result += tempDay;

                            }

                        }
                    }
                }
            }
            return result;
        } catch (GRBException e) {
            printError(e, "DifferentDays");
        }
    }

    /**
     * Given classes overlap in time. Two classes overlap in time when they
     * overlap in time of day, days of the week, as well as weeks. This means that
     * (Cj .start < Ci.end) ∧ (Ci.start < Cj .end) ∧ ((Ci.days and Cj .days) 6= 0) ∧
     * ((Ci.weeks and Cj .weeks) 6= 0)
     * for any two classes Ci and Cj from the constraint, doing binary ,,and” between
     * days and weeks of Ci and Cj .
     * @param vector
     * @param penalty
     * @param b
     * @return
     */

    virtual GRBLinExpr overlap(const std::vector<Class *, std::allocator<Class *>> &vector, int penalty, bool b) {
        try {
            GRBLinExpr result = 0;
            for (int c1 = 0; c1 < vector.size(); c1++) {
                for (int c2 = c1 + 1; c2 < vector.size(); ++c2) {
                    if (vector[c1]->isActive(currentW) && vector[c2]->isActive(currentW)) {
                        if (b) {
                            //Same time
                            GRBVar t = model->addVar(0, 1, 0, GRB_BINARY);
                            model->addGenConstrIndicator(t, 1, order[vector[c1]->getOrderID()][vector[c2]->getOrderID()]
                                                               +
                                                               order[vector[c2]->getOrderID()][vector[c1]->getOrderID()] <=
                                                               1);
                            model->addGenConstrIndicator(t, 0, order[vector[c1]->getOrderID()][vector[c2]->getOrderID()]
                                                               +
                                                               order[vector[c2]->getOrderID()][vector[c1]->getOrderID()] ==
                                                               2);
                            //Same day
                            GRBVar d = model->addVar(0, 1, 0, GRB_BINARY);
                            model->addGenConstrIndicator(d, 0,
                                                         sameday[vector[c1]->getOrderID()][vector[c2]->getOrderID()] ==
                                                         1);
                            model->addGenConstrIndicator(d, 1,
                                                         sameday[vector[c1]->getOrderID()][vector[c2]->getOrderID()] ==
                                                         0);
                            if (penalty == -1)
                                model->addConstr(d + t >= 1);
                            else {
                                GRBVar temp = model->addVar(0, 1, 0, GRB_BINARY);
                                model->addConstr(temp, 0, d + t >= 1);
                                model->addConstr(temp, 1, d + t == 0);
                                result += penalty * temp;
                            }

                        } else {
                            if (penalty == -1) {
                                //Same time
                                model->addConstr(order[vector[c1]->getOrderID()][vector[c2]->getOrderID()]
                                                 + order[vector[c2]->getOrderID()][vector[c1]->getOrderID()] == 2);
                                //Same day
                                model->addConstr(sameday[vector[c1]->getOrderID()][vector[c2]->getOrderID()] == 1);
                                //Same week!
                            } else {
                                GRBVar temp = model->addVar(0, 1, 0, GRB_BINARY);
                                model->addConstr(temp, 0, order[vector[c1]->getOrderID()][vector[c2]->getOrderID()]
                                                          + order[vector[c2]->getOrderID()][vector[c1]->getOrderID()] ==
                                                          2);
                                model->addConstr(temp, 1, order[vector[c1]->getOrderID()][vector[c2]->getOrderID()]
                                                          + order[vector[c2]->getOrderID()][vector[c1]->getOrderID()] <=
                                                          1);
                                GRBVar temp1 = model->addVar(0, 1, 0, GRB_BINARY);
                                model->addConstr(temp1, 0,
                                                 sameday[vector[c1]->getOrderID()][vector[c2]->getOrderID()] == 1);
                                model->addConstr(temp1, 1,
                                                 sameday[vector[c1]->getOrderID()][vector[c2]->getOrderID()] == 0);
                                GRBVar temp2 = model->addVar(0, 1, 0, GRB_BINARY);
                                model->addConstr(temp1, 0, temp1 + temp <= 1);
                                model->addConstr(temp1, 1, temp1 + temp == 2);
                                result += penalty * temp2;
                            }
                        }
                    }

                }
            }
            return result;
        } catch (GRBException e) {
            printError(e, "overlap");
        }

    }

    /**
     * Given classes should be placed in the same room. This means that
     *(Ci.room = Cj .room) for any two classes Ci and Cj from the constraint;
     *Ci.room is the assigned room of Ci
     * @param vector
     * @param penalty
     * @param b
     * @return
     */

    GRBLinExpr differentRoom(const std::vector<Class *, std::allocator<Class *>> &vector, int penalty, bool b) {
        roomLecture->differentRoom(vector, penalty, b);

    }
};


#endif //PROJECT_ILPEXECUTER_H