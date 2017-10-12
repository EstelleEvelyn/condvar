/******************************************************************************
*
* LAST REVISED: 10/6/17 Sherri Goings

Estelle Bayer
CS 332
******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <time.h>
#include "boat.h"


pthread_cond_t adultBoardOahu;
pthread_cond_t kidsBoardOahu;
pthread_cond_t onBoat;
pthread_cond_t onMolo;
int boatLoc;
int kidsOnBoard;
int adultsOnBoard;
int adultGoes;

void init() {
  /* Initialize mutex and condition variable objects */
  pthread_mutex_init(&lock, NULL);
  pthread_cond_init(&allReady, NULL);
  pthread_cond_init(&mayStart, NULL);
  pthread_cond_init(&allDone, NULL);
  pthread_cond_init(&adultBoardOahu, NULL);
  pthread_cond_init(&kidsBoardOahu, NULL);
  pthread_cond_init(&onBoat, NULL);
  pthread_cond_init(&onMolo, NULL);
  boatLoc = OAHU;
  kidsOnBoard = 0;
  adultsOnBoard = 0;
  adultGoes = 0;
}
 /*
Two children at a time cross from Oahu to Molokai. The first to board returns the
boat to Oahu and exits. The second to board either stays on Molokai, if no adults
are left on Oahu, or waits for an adult to cross and returns the boat to Oahu.
 */
void* childThread(void* args) {
  //Count the number of kids on oahu, signal main to check if all here now
  pthread_mutex_lock(&lock);
  kidsOahu++;
  pthread_cond_signal(&allReady);

  // wait until main signals that all here
  while (!start) {
    pthread_cond_wait(&mayStart, &lock);
  }
  //loop while there are children
  while(kidsOahu > 0) {
    //different algorithm while adults have yet to finish crossing
    while(adultsOahu > 0) {
      //can board if boat at Oahu, not an adult's turn, and not too many on board
      while(boatLoc == MOLO || kidsOnBoard > 1 || adultsOnBoard > 0 || adultGoes == 1) {
        pthread_cond_wait(&kidsBoardOahu, &lock);
      }
      boardBoat(KID, OAHU);
      kidsOnBoard++;
      kidsOahu--;
      //first kid waits to reach Molokai
      if(kidsOnBoard == 1) {
        while(boatLoc == OAHU) {
          pthread_cond_wait(&onBoat, &lock);
        }
        //return boat to Oahu
        boatCross(MOLO, OAHU);
        boatLoc = OAHU;
        leaveBoat(KID, OAHU);
        kidsOnBoard--;
        kidsOahu++;
        //signal an adult to board and cross
        adultGoes = 1;
        pthread_cond_signal(&adultBoardOahu);
      //second kid brings boat to Molokai
      } else {
        boatCross(OAHU, MOLO);
        boatLoc = MOLO;
        leaveBoat(KID, MOLO);
        kidsOnBoard--;
        //wake first kid to return boat
        pthread_cond_signal(&onBoat);
        //wait for adult to bring boat over
        while(boatLoc == OAHU || adultsOnBoard != 0 || kidsOnBoard != 0) {
          pthread_cond_wait(&onMolo, &lock);
        }
        //return boat
        boardBoat(KID, MOLO);
        kidsOnBoard++;
        boatCross(MOLO, OAHU);
        boatLoc = OAHU;
        leaveBoat(KID, OAHU);
        kidsOnBoard--;
        kidsOahu++;
        //allow kids to board
        pthread_cond_signal(&kidsBoardOahu);
      }
    }
    //no adults left: board if the boat is on Oahu and not full
    while(boatLoc == MOLO || kidsOnBoard > 1) {
      pthread_cond_wait(&kidsBoardOahu, &lock);
    }
    boardBoat(KID, OAHU);
    kidsOahu--;
    kidsOnBoard++;
    //first kid waits to reach Molokai
    if(kidsOnBoard == 1) {
      while(boatLoc == OAHU || kidsOnBoard == 2){
        pthread_cond_wait(&onBoat, &lock);
      }
      //if there are still kids left to cross, return boat to Oahu
      if(kidsOahu != 0) {
        boatCross(MOLO, OAHU);
        boatLoc = OAHU;
        leaveBoat(KID, OAHU);
        kidsOnBoard--;
        kidsOahu++;
        pthread_cond_signal(&kidsBoardOahu);
      //if no other kids left on Oahu, exit on Molokai
      } else {
        leaveBoat(KID, MOLO);
        kidsOnBoard--;
      }
      //second kid cross from Oahu to Molokai and gets off the boat
    } else {
      boatCross(OAHU, MOLO);
      boatLoc = MOLO;
      leaveBoat(KID, MOLO);
      kidsOnBoard--;
      pthread_cond_signal(&onBoat);
    }
  }
  // signals to wake main to check if everyone now across, you may choose to only do
  // this in one of the adult or child threads, as long as eventually both Oahu counts
  // go to 0 and you signal allDone somewhere!
  pthread_cond_signal(&allDone);
  pthread_mutex_unlock(&lock);

  return NULL;
}

/*An adult crosses when the last boat to have left Oahu contained two children.
*/
void* adultThread(void* args) {
  //Count the number of adults on oahu, signal main to check if all here now
  pthread_mutex_lock(&lock);
  adultsOahu++;
  pthread_cond_signal(&allReady);

  // wait until main signals that all here
  while (!start) {
    pthread_cond_wait(&mayStart, &lock);
  }

  //wait to board until boat is on Oahu, empty, and two kids took the last boat
  while(boatLoc == MOLO || kidsOnBoard > 0 || adultsOnBoard > 0 || adultGoes == 0) {
    pthread_cond_wait(&adultBoardOahu, &lock);
  }
  //go to Molokai, get off,  set next voyage to be two kids
  boardBoat(ADULT, OAHU);
  adultsOnBoard++;
  adultGoes = 0;
  adultsOahu--;
  boatCross(OAHU, MOLO);
  boatLoc = MOLO;
  leaveBoat(ADULT, MOLO);
  adultsOnBoard--;
  //signal child waiting on Molokai to return boat
  pthread_cond_signal(&onMolo);
  pthread_mutex_unlock(&lock);

  return NULL;
}
