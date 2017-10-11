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


pthread_cond_t onOahu;
pthread_cond_t onBoat;
pthread_cond_t onMolo;
int boatLoc;
int kidsOnBoard;
int adultsOnBoard;
int lastCrossed;

void init() {
  /* Initialize mutex and condition variable objects */
  pthread_mutex_init(&lock, NULL);
  pthread_cond_init(&allReady, NULL);
  pthread_cond_init(&mayStart, NULL);
  pthread_cond_init(&allDone, NULL);
  pthread_cond_init(&onOahu, NULL);
  pthread_cond_init(&onBoat, NULL);
  pthread_cond_init(&onMolo, NULL);
  boatLoc = OAHU;
  lastCrossed = ADULT;
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
    while (boatLoc == MOLO || kidsOnBoard == 2 || adultsOnBoard == 1 || (lastCrossed == KID && adultsOahu != 0)) {
      pthread_cond_wait(&onOahu, &lock);
    }
    boardBoat(KID, OAHU);
    kidsOahu--;
    kidsOnBoard++;
    if (kidsOnBoard == 1) {
      while (boatLoc == OAHU) {
        pthread_cond_wait(&onBoat, &lock);
      }
      if (kidsOahu == 0 && adultsOahu == 0) {
        leaveBoat(KID, MOLO);
        kidsOnBoard--;
        pthread_cond_signal(&allDone);
        pthread_mutex_unlock(&lock);
      } else {
        boatCross(MOLO, OAHU);
        boatLoc = OAHU;
        if (adultsOahu != 0) {
          leaveBoat(KID, OAHU);
          kidsOnBoard--;
          kidsOahu++;
        }
        pthread_cond_signal(&onOahu);
      }
    } else {
      boatCross(OAHU, MOLO);
      lastCrossed = KID;
      boatLoc = MOLO;
      leaveBoat(KID, MOLO);
      kidsOnBoard--;
      pthread_cond_signal(&onBoat);
      if (kidsOahu == 0 && adultsOahu == 0) {
        pthread_cond_signal(&onOahu);
      }
      while (boatLoc == OAHU || lastCrossed == KID || adultsOnBoard != 0 || kidsOnBoard != 0) {
          pthread_cond_wait(&onMolo, &lock);
        }
      boardBoat(KID, MOLO);
      kidsOnBoard++;
      boatCross(MOLO, OAHU);
      boatLoc = OAHU;
      leaveBoat(KID, OAHU);
      kidsOnBoard--;
      kidsOahu++;
      pthread_cond_signal(&onOahu);
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

  while(boatLoc == MOLO || kidsOnBoard > 0 || adultsOnBoard > 0 || lastCrossed == ADULT) {
    pthread_cond_wait(&onOahu, &lock);
  }
  boardBoat(ADULT, OAHU);
  adultsOnBoard++;
  adultsOahu--;
  boatCross(OAHU, MOLO);
  boatLoc = MOLO;
  lastCrossed = ADULT;
  leaveBoat(ADULT, MOLO);
  adultsOnBoard--;
  pthread_cond_signal(&onMolo);
  pthread_mutex_unlock(&lock);

  return NULL;
}
