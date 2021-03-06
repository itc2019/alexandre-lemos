//
// Created by Alexandre Lemos on 05/11/2018.
//

#ifndef TIMETABLER_LECTURE_H
#define TIMETABLER_LECTURE_H

#include <string.h>
#include <ostream>

class Lecture {
    int cost;
    int orderID=-1;
    std::string days;
    int start;
    int lenght;
    int end;
    std::string weeks;
    int penalty;
    bool doubleL = false;
 public:
    Lecture(int lenght, int start, std::string weeks, std::string days, int penalty) : days(days), start(start),
                                                                                       lenght(lenght),
                                                                                       weeks(weeks),
                                                                                       penalty(penalty) {
        end = start + lenght;
    }


    Lecture(int lenght) : lenght(lenght) {}

    int getCost() { return cost; }

    void setCost(int c) { cost = c; }

    int getEnd() { return end; }

    std::string &getDays() {
        return days;
    }

    void setDays(const std::string &days) {
        Lecture::days = days;
    }

    int getNumbWeeks() {
        int value = 0;
        for (char &c : weeks) {
            if (c == '1')
                value++;

        }
        return value;
    }

    int getStart() const {
        return start;
    }

    int getPenalty() const {
        return penalty;
    }

    void setStart(int start) {
        Lecture::start = start;
    }

    int getLenght() {
        return lenght;
    }

    const std::string &getWeeks() const {
        return weeks;
    }

    void setWeeks(const std::string &weeks) {
        Lecture::weeks = weeks;
    }

    void setLenght(int lenght) {
        Lecture::lenght = lenght;
    }

    friend std::ostream &operator<<(std::ostream &os, const Lecture &lecture) {
        os << "days: " << lecture.days << " start: " << lecture.start << " lenght: " << lecture.lenght << " weeks: "
           << lecture.weeks << " penalty: " << lecture.penalty;
        return os;
    }

    bool isActive(int w) {
        if (weeks[w] == '1')
            return true;
        return false;
    }

    Lecture(int id, char *doubleLec) {
        if (strcmp(doubleLec, "yes") == 0)
            doubleL = true;
        lenght = 1;
    }

    int getOrderId() {
        return orderID;
    }
    void setOrderID(int i){
        orderID=i;
    }

    int getNumbDays(){
        int value = 0;
        for (char &c : days) {
            if (c == '1')
                value++;

        }
        return value;

    }
};


#endif //TIMETABLER_LECTURE_H
