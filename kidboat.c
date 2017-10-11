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
    while(boatLoc == MOLO || kidsOnBoard > 1) {
      pthread_cond_wait(&kidsBoardOahu, &lock);
    }
    printf("I think you're on %i with at most 1 on board!\n", boatLoc );
    fflush(stdout);
    boardBoat(KID, OAHU);
    kidsOahu--;
    kidsOnBoard++;
    printf("1current kids on board: %i\n", kidsOnBoard);
    fflush(stdout);
    if(kidsOnBoard == 1) {
      while(boatLoc == OAHU || kidsOnBoard == 2){
        pthread_cond_wait(&onBoat, &lock);
      }
      if (kidsOahu > 0) {
        boatCross(MOLO, OAHU);
        boatLoc = OAHU;
        leaveBoat(KID, OAHU);
        kidsOnBoard--;
        printf("2current kids on board: %i\n", kidsOnBoard);
        fflush(stdout);
        kidsOahu++;
        pthread_cond_signal(&kidsBoardOahu);
        pthread_mutex_unlock(&lock);
      } else {
        printf("gonna leave now");
        fflush(stdout);
        leaveBoat(KID, MOLO);
        kidsOnBoard--;
        pthread_cond_signal(&allDone);
        pthread_mutex_unlock(&lock);
      }
    } else {
      boatLoc = MOLO;
      boatCross(OAHU, MOLO);
      boatLoc = MOLO;
      leaveBoat(KID, MOLO);
      kidsOnBoard--;
      boatLoc = MOLO;
      printf("3current kids on board: %i\n", kidsOnBoard);
      fflush(stdout);
      pthread_cond_signal(&onBoat);
      pthread_mutex_unlock(&lock);
    }
  }


  return NULL;
}

void *adultThread(void* args) {
  return NULL;
}
