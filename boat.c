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

  while (boatLoc == MOLO || kidsOnBoard == 2 || adultsOnBoard == 1 || (lastCrossed == KID || adultsOahu != 0 )) {
    pthread_cond_wait(&onOahu, &lock);
  }
  boardBoat(KID, OAHU);
  kidsOahu--;
  kidsOnBoard++;
  if (kidsOnBoard == 1) {
    while (boatLoc == OAHU) {
      pthread_cond_wait(&onBoat, &lock);
    }
    boatCross(MOLO, OAHU);
    boatLoc = OAHU;
    leaveBoat(KID, OAHU);
    kidsOnBoard--;
    kidsOahu++;
    pthread_cond_broadcast(&onOahu);
    pthread_mutex_unlock(&lock);
  } else {
    boatCross(OAHU, MOLO);
    lastCrossed = KID;
    boatLoc = MOLO;
    pthread_cond_signal(&onBoat);
    leaveBoat(KID, MOLO);
    kidsOnBoard--;
    while (boatLoc == OAHU || lastCrossed == KID || adultsOnBoard != 0 || kidsOnBoard != 0 || adultsOahu == 0) {
        pthread_cond_wait(&onMolo, &lock);
      }
    boardBoat(KID, MOLO);
    kidsOnBoard++;
    boatCross(MOLO, OAHU);
    boatLoc = OAHU;
    leaveBoat(KID, OAHU);
    kidsOnBoard--;
    kidsOahu++;
    fflush(stdout);
    pthread_cond_broadcast(&onOahu);
    pthread_mutex_unlock(&lock);
  }
  /*
   * DUMMY CODE - Remove in final solution!
   * adult rows self to Molokai, boat magically returns (or there are infinite boats available)
   * updates Oahu count to show has crossed
   * KID, ADULT, OAHU, and MOLO are defined in the .h file and should be the only 4
   * possible values for the arguments to the action functions.
   */
  // boardBoat(KID, OAHU);
  // boatCross(OAHU, MOLO);
  // leaveBoat(KID, MOLO);
  // kidsOahu--;
  /*** end of dummy code ***/

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
  pthread_cond_broadcast(&onMolo);
  pthread_mutex_unlock(&lock);
  /*
   * DUMMY CODE - Remove in final solution!
   * adult rows self to Molokai, boat magically returns (or there are infinite boats available)
   * updates Oahu count to show has crossed
   * KID, ADULT, OAHU, and MOLO are defined in the .h file and should be the only 4
   * possible values for the arguments to the action functions.
   */
  // boardBoat(ADULT, OAHU);
  // boatCross(OAHU, MOLO);
  // leaveBoat(ADULT, MOLO);
  // adultsOahu--;
  /*** end of dummy code ***/

  // signals to wake main to check if everyone now across, you may choose to only do
  // this in one of the adult or child threads, as long as eventually both Oahu counts
  // go to 0 and you signal allDone somewhere!
  pthread_cond_signal(&allDone);
  pthread_mutex_unlock(&lock);

  return NULL;
}
