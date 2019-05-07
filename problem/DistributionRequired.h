//
// Created by Alexandre Lemos on 05/11/2018.
//

#ifndef PROJECT_DISTRIBUTIONREQUIRED_H
#define PROJECT_DISTRIBUTIONREQUIRED_H


#include "distribution.h"
#include "NoLimit.h"

class DistributionRequired: public distribution {

public:
    DistributionRequired(Constraint *type, std::vector<Class *> classes) : distribution(classes,
                                                                                        type) {

    }

    virtual int getPenalty() { return -1; }
};


#endif //PROJECT_DISTRIBUTIONREQUIRED_H
