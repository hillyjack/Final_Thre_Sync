//
// Created by ubuntu on 11/02/18.
//

#ifndef FINAL_THRE_SYNC_PRODUCER_H
#define FINAL_THRE_SYNC_PRODUCER_H

#include "includes.h"

char * files_list;

void createProducer(void *p);
void createPythonProducer();
void questionToShmArr (double num1, double num2, int op );

#endif //FINAL_THRE_SYNC_PRODUCER_H
