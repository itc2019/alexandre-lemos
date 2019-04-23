//
// Created by Alexandre Lemos on 09/01/2019.
//

#ifndef PROJECT_BINARYMODELGUROBIEXECUTER_H
#define PROJECT_BINARYMODELGUROBIEXECUTER_H


#include "/Library/gurobi810/mac64/include/gurobi_c++.h"

#include <exception>
#include <stdlib.h>
#include "../problem/Instance.h"
#include "../solver/TwoVarGurobiExecuter.h"
#include "../solver/roomLectureGRB.h"
#include "../solver/roomLectureBool.h"


class BinaryModelGurobiExecuter : public TwoVarGurobiExecuter {
    GRBVar ***lectureTime;


public:

    void loadPreviousWeekSolution(int ***time, int **room) {
        for (int d = 0; d < instance->getNdays(); ++d) {
            for (int t = 0; t < instance->getSlotsperday(); ++t) {
                for (int i = 0; i < instance->getNumClasses(); ++i) {
                    model->addConstr(lectureTime[d][t][i] == time[d][t][i]);
                }
            }

        }
        roomLecture->loadPreviousWeekSolution(room);
    }

    BinaryModelGurobiExecuter(Instance *i) {
        setInstance(i);
        roomLecture = new roomLectureGRB(instance, currentW);
    }

    BinaryModelGurobiExecuter(bool isStatic, Instance *i) {
        setInstance(i);
        if (isStatic)
            roomLecture = new roomLectureBool(instance, currentW);
        else
            roomLecture = new roomLectureGRB(instance, currentW);
    }

    void definedAuxVar() {
        //Not yet used
        ;
    }

    void definedLectureTime() {
        lectureTime = new GRBVar **[instance->getNdays()];
        for (int i = 0; i < instance->getNdays(); i++) {
            lectureTime[i] = new GRBVar *[instance->getSlotsperday()];
            for (int k = 0; k < instance->getSlotsperday(); ++k) {
                lectureTime[i][k] = new GRBVar[instance->getNumClasses()];
                for (int j = 0; j < instance->getNumClasses(); ++j) {
                    std::string name = "A_" + itos(i) + "_" + itos(k) + "_" + itos(j);
                    lectureTime[i][k][j] = model->addVar(0.0, 1.0, 0.0, GRB_BINARY, name);
                }

            }

        }


    }

    /**
     * The lecture can only be scheduled in one slot
     */

    void oneLectureperSlot() {
        try {
            for (int j = 0; j <
                            instance->getNumClasses(); j++) {
                GRBLinExpr expr = 0;
                for (int d = 0; d < instance->getNdays(); d++) {
                    for (int t = 0; t < instance->getSlotsperday(); t++) {
                        expr += lectureTime[d][t][j];
                        for (int t1 = t + 1; t1 < instance->getSlotsperday(); t1++) {
                            for (int t2 = t1 + 1; t2 < instance->getSlotsperday(); t2++) {
                                GRBVar temp = model->addVar(0.0, 1.0, 0.0, GRB_BINARY);
                                model->addGenConstrIndicator(temp, 1,
                                                             (lectureTime[d][t][j] - lectureTime[d][t2][j]) == 0);
                                model->addConstr(lectureTime[d][t1][j] == temp);
                            }
                        }
                    }

                }
                model->addConstr(expr == instance->getClasses()[j]->getLenght());

            }
        } catch (GRBException e) {
            printError(e, "oneLectureperSlot");
        }

    }

    /**
     * Force lectures to be in slot n
     */


    void oneLectureSlot() {
        try {
            for (int j = 0; j <
                            instance->getNumClasses(); j++) {
                for (int i = 0; i < instance->getClasses()[j]->getLenght(); i++) {
                    int k = 0;
                    for (char &c :instance->getClasses()[j]->getDays()) {
                        if (c == '1')
                            model->addConstr(lectureTime[k][instance->getClasses()[j]->getStart() + i][j] == 1);
                        else if (c == '0')
                            model->addConstr(lectureTime[k][instance->getClasses()[j]->getStart() + i][j] == 0);
                        k++;
                    }
                }

            }
        } catch (GRBException e) {
            printError(e, "oneLectureSlot");
        }

    }


    /***
     * The room can only have one lecture per slot
     */

    void oneLectureRoomConflict() {
        try {
            for (int i = 0; i < instance->getRooms().size(); i++) {
                for (int d = 0; d < instance->getNdays(); d++) {
                    for (int k = 0; k < instance->getSlotsperday(); k++) {
                        GRBQuadExpr temp = 0;
                        for (int j = 0; j < instance->getNumClasses(); j++) {
                            if (instance->getClasses()[j]->containsRoom(instance->getRoom(i + 1))) {
                                if (roomLecture->isStatic())
                                    temp += (lectureTime[d][k][j] * roomLecture->getBool()[j][i]);
                                else
                                    temp += (lectureTime[d][k][j] * roomLecture->getGRB()[j][i]);
                            }
                            //lectureTime[Qui][12]["ACED"]*roomLecture["FA1"]["ACED"]+lectureTime[Qui][12]["LP"]*roomLecture["FA1"]["LP"]
                        }
                        model->addQConstr(temp <= 1);
                    }
                }
            }
        } catch (GRBException e) {
            printError(e, "oneLectureRoomConflict");
        }


    }


    /**
    * Ensure Room closed in a day cannot be used
    */
    void roomClosebyDay() {
        for (int d = 0; d < instance->getNdays(); ++d) {
            for (int t = 0; t < instance->getSlotsperday(); ++t) {
                for (int i = 0; i < instance->getNumRoom(); i++) {
                    for (int j = 0; j < instance->getNumClasses(); ++j) {
                        if (instance->isRoomBlockedbyDay(i, d) &&
                            instance->getClasses()[j]->containsRoom(instance->getRoom(i + 1))) {
                            if (roomLecture->isStatic())
                                model->addConstr(roomLecture->getBool()[j][i] * lectureTime[d][t][j] == 0);
                            else
                                model->addConstr(roomLecture->getGRB()[j][i] * lectureTime[d][t][j] == 0);

                        }
                    }
                }
            }

        }


    }


    /**
    * Ensure times lot in a day is closed cannot be used
    */
    void slotClose() {
        for (int i = 0; i < instance->getNumClasses(); i++) {
            for (int d = 0; d < instance->getNdays(); d++) {
                for (int t = 0; t < instance->getSlotsperday(); ++t) {
                    if (instance->isTimeUnavailable(i * t))
                        model->addConstr(lectureTime[d][t][i] == 0);

                }
            }

        }
    }

    /**
     * One assignment, is invalid and needs to be assigned
     * to a different room or to a different time slot
     */
    void assignmentInvalid() {
        for (int i = 0; i < instance->getNumClasses(); ++i) {
            if (instance->isIncorrectAssignment(i)) {
                for (int j = 0; j < instance->getNdays(); ++j) {
                    for (int k = 0; k < instance->getSlotsperday(); ++k) {
                        if (solutionTime[j][k][i] == 1) {
                            GRBLinExpr temp = 0;
                            temp = lectureTime[j][k][i];
                            for (int l = 0; l < instance->getNumRoom(); ++l) {
                                if (solutionRoom[l][i] == 1 &&
                                    instance->getClasses()[i]->containsRoom(instance->getRoom(l + 1))) {
                                    if (!roomLecture->isStatic())
                                        temp += roomLecture->getGRB()[i][l];
                                    model->addConstr(temp <= 1);
                                    break;
                                }

                            }
                        }
                    }
                }

            }

        }
    }

    /**Teacher's conflict*/
    void teacher() {
        for (std::map<std::string, Course *>::const_iterator it = instance->getCourses().begin();
             it != instance->getCourses().end(); it++) {
            for (std::map<int, std::vector<Subpart *>>::iterator sub = it->second->getConfiguratons().begin();
                 sub != it->second->getConfiguratons().end(); ++sub) {
                for (int i = 0; i < sub->second.size(); ++i) {
                    for (int d = 0; d < instance->getNdays(); ++d) {
                        for (int t = 0; t < instance->getSlotsperday(); ++t) {
                            GRBLinExpr conflict = 0;
                            for (int c = 0; c < sub->second[i]->getClasses().size(); c++) {
                                conflict += lectureTime[d][t][c];
                            }
                            model->addConstr(conflict <= sub->second[i]->getOverlap());
                        }

                    }
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
            for (int d = 0; d < instance->getNdays(); ++d) {
                for (int t = 0; t < instance->getSlotsperday(); ++t) {
                    GRBLinExpr conflict = 0;
                    for (int c = 0; c < it->second.getCourse().size(); ++c) {
                        conflict += lectureTime[d][t][c];
                    }
                    model->addConstr(conflict <= 1);
                }

            }
        }
    }

    /** Student conflicts hard constraint based on the original solution
     *
     */
    void studentConflictSolution() {
        for (std::map<int, Student>::const_iterator it = instance->getStudent().begin();
             it != instance->getStudent().end(); it++) {
            for (int d = 0; d < instance->getNdays(); ++d) {
                for (int t = 0; t < instance->getSlotsperday(); ++t) {
                    GRBLinExpr conflict = 0;
                    for (int c = 0; c < it->second.getClasses().size(); ++c) {
                        conflict += lectureTime[d][t][c];
                    }
                    model->addConstr(conflict <= 1);
                }

            }
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
                                            lectureTime[d][instance->getClasses()[l]->getStart() + i][l]
                                            * roomLecture->getBool()[l][j];
                                else
                                    temp += instance->getClasses()[l]->getLimit() *
                                            lectureTime[d][instance->getClasses()[l]->getStart() + i][l]
                                            * roomLecture->getGRB()[l][j];
                            } else if (instance->getClasses()[l]->containsRoom(instance->getRoom(j + 1))) {
                                if (roomLecture->isStatic())
                                    temp += it->second.getCapacity() *
                                            lectureTime[d][instance->getClasses()[l]->getStart() + i][l]
                                            * roomLecture->getBool()[l][j];
                                else
                                    temp += it->second.getCapacity() *
                                            lectureTime[d][instance->getClasses()[l]->getStart() + i][l]
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
                            if (instance->getClasses()[l]->containsRoom(instance->getRoom(j + 1)))
                                if (roomLecture->isStatic())
                                    temp += abs(it->second.getCapacity() - instance->getClasses()[l]->getLimit()) *
                                            lectureTime[d][instance->getClasses()[l]->getStart() + i][l]
                                            * roomLecture->getBool()[l][j];
                            if (roomLecture->isStatic())
                                temp += abs(it->second.getCapacity() - instance->getClasses()[l]->getLimit()) *
                                        lectureTime[d][instance->getClasses()[l]->getStart() + i][l]
                                        * roomLecture->getGRB()[l][j];
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

    GRBLinExpr gapStudentsTimetable() {
        GRBLinExpr min = 0;
        for (int i = 0; i < instance->getStudent().size(); ++i) {
            for (int j = 0; j < instance->getNdays(); ++j) {
                for (int k = 1; k < instance->getSlotsperday(); ++k) {
                    GRBLinExpr all = 0;
                    GRBVar t = model->addVar(0.0, std::numeric_limits<int>::max(), 0.0, GRB_INTEGER);
                    GRBVar tmin = model->addVar(0.0, 1.0, 0.0, GRB_BINARY);
                    for (int l = 0; l < instance->getNumClasses(); ++l) {
                        //std::cout << l << " " << instance->getStudent(i).isEnrolled(l) << std::endl;
                        if (instance->getStudent(i).isEnrolled(l))
                            all += lectureTime[j][k][l] + lectureTime[j][k - 1][l];
                    }
                    model->addConstr(t == all);
                    model->addGenConstrIndicator(tmin, 1, t == 1);

                    min += tmin;

                }
            }

        }
        return min;
        //std::cout << min << std::endl;
    }


public:


    /***
    * The current distance of the solution with the old solution
    * The distante is base on the weighted Hamming distance of the lectureTime variable (time slot atributions)
    * The weighted is baed on the number of students moved
    * @param oldTime
    * @param weighted
    * @return IloExpr
    */


    GRBQuadExpr distanceToSolutionLectures(int ***oldTime, bool weighted) {
        GRBQuadExpr temp = 0;
        for (int i = 0; i < instance->getNdays(); i++) {
            for (int t = 0; t < instance->getSlotsperday(); t++) {
                for (int j = 0; j < instance->getNumClasses(); ++j) {
                    GRBVar tempv = model->addVar(0.0, 1.0, 0.0, GRB_BINARY);
                    model->addGenConstrIndicator(tempv, 0, oldTime[i][t][j] == lectureTime[i][t][j]);
                    model->addGenConstrIndicator(tempv, 1, lectureTime[i][t][j] - oldTime[i][t][j] - 1 <= 0);
                    temp += instance->getClasses()[j]->getLimit() * tempv;
                }

            }
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
                    lectureTime[k][i][j].set(GRB_DoubleAttr_Start, solutionTime[k][i][j]);
                }
            }
        }
        for (int r = 0; r < instance->getRooms().size(); ++r) {
            for (int l = 0; l < instance->getNumClasses(); ++l) {
                if (instance->getClasses()[l]->containsRoom(instance->getRoom(r + 1))) {
                    if (roomLecture->isStatic())
                        roomLecture->getBool()[l][r] = solutionRoom[r][l];
                    else
                        roomLecture->getGRB()[l][r].set(GRB_DoubleAttr_Start, solutionRoom[r][l]);
                }

            }

        }

    }


    /**
     * Switch solution time
     * Updates the solution time structure with new data
     * @Requires delete the previous found solution
     *
     */

    void switchSolutionTime() {
        for (int i = 0; i < instance->getNumClasses(); i++) {
            for (int k = 0; k < instance->getNdays(); ++k) {
                for (int j = 0; j < instance->getSlotsperday(); ++j) {
                    solutionTime[k][j][i] = lectureTime[k][j][i].get(GRB_DoubleAttr_X);

                    if (solutionTime[k][j][i] != 0) {
                        instance->getClasses()[i]->setSolutionTime(j,
                                                                   strdup(std::to_string(k).c_str()));
                    }

                }

            }

        }

    }


};


#endif