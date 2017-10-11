/******************************************************************************
*
* LAST REVISED: 10/6/17 Sherri Goings
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
}

void* childThread(void* args) {
  //Count the number of kids on oahu, signal main to check if all here now
  pthread_mutex_lock(&lock);
  kidsOahu++;
  pthread_cond_signal(&allReady);

  // wait until main signals that all here
  while (!start) {
    pthread_cond_wait(&mayStart, &lock);
  }
  while(kidsOahu != 0) {
    while(adultsOahu != 0) {
      while(boatLoc == MOLO || kidsOnBoard > 1 || adultsOnBoard > 0) {
        pthread_cond_wait(&kidsBoardOahu, &lock);
      }
      boardBoat(KID, OAHU);
      kidsOnBoard++;
      kidsOahu--;
      if(kidsOnBoard == 1) {
        while(boatLoc == OAHU) {
          pthread_cond_wait(&onBoat, &lock);
        }
        boatCross(MOLO, OAHU);
        boatLoc = OAHU;
        leaveBoat(KID, OAHU);
        kidsOnBoard--;
        kidsOahu++;
        pthread_cond_signal(&adultBoardOahu);
        pthread_mutex_unlock(&lock);
      } else {
        boatCross(OAHU, MOLO);
        boatLoc = MOLO;
        leaveBoat(KID, MOLO);
        kidsOnBoard--;
        pthread_cond_signal(&onBoat);
        while(boatLoc == OAHU || adultsOnBoard != 0 || kidsOnBoard != 0) {
          pthread_cond_wait(&onMolo, &lock);
        }
        boardBoat(KID, MOLO);
        kidsOnBoard++;
        boatCross(MOLO, OAHU);
        boatLoc == OAHU;
        leaveBoat(KID, OAHU);
        kidsOnBoard--;
        kidsOahu++;
        pthread_cond_signal(&kidsBoardOahu);
        pthread_mutex_unlock(&lock);
      }
    }
    while(boatLoc == MOLO || kidsOnBoard > 1) {
      pthread_cond_wait(&kidsBoardOahu, &lock);
    }
    boardBoat(KID, OAHU);
    kidsOahu--;
    kidsOnBoard++;
    if(kidsOnBoard == 1) {
      while(boatLoc == OAHU){
        pthread_cond_wait(&onBoat, &lock);
      }
      if (kidsOahu != 0) {
        boatCross(MOLO, OAHU);
        boatLoc = OAHU;
        leaveBoat(KID, OAHU);
        kidsOnBoard--;
        kidsOahu++;
        pthread_cond_signal(&kidsBoardOahu);
        pthread_mutex_unlock(&lock);
      } else {
        leaveBoat(KID, MOLO);
        kidsOnBoard--;
        pthread_cond_signal(&allDone);
      }
    } else {
      leaveBoat(KID, MOLO);
      kidsOnBoard--;
      pthread_cond_signal(&onBoat);
      pthread_mutex_unlock(&lock);
    }
  }

  // signals to wake main to check if everyone now across, you may choose to only do
  // this in one of the adult or child threads, as long as eventually both Oahu counts
  // go to 0 and you signal allDone somewhere!
  pthread_cond_signal(&allDone);
  pthread_mutex_unlock(&lock);

  return NULL;
}

void* adultThread(void* args) {
  //Count the number of adults on oahu, signal main to check if all here now
  pthread_mutex_lock(&lock);
  adultsOahu++;
  pthread_cond_signal(&allReady);

  // wait until main signals that all here
  while (!start) {
    pthread_cond_wait(&mayStart, &lock);
  }

  while(boatLoc == MOLO || kidsOnBoard > 0 || adultsOnBoard > 0) {
    pthread_cond_wait(&adultBoardOahu, &lock);
  }
  boardBoat(ADULT, OAHU);
  adultsOnBoard++;
  adultsOahu--;
  boatCross(OAHU, MOLO);
  boatLoc = MOLO;
  leaveBoat(ADULT, MOLO);
  adultsOnBoard--;
  pthread_cond_signal(&onMolo);
  pthread_mutex_unlock(&lock);

  return NULL;
}
