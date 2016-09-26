#ifndef _KP_H_
#define _KP_H_

#include "ontology.h"
#include <stdlib.h>
#include <errno.h>
#include <jni.h>
#include <string.h>

#define MAX_CLASS_NAME_LENGTH 20
#define ERROR_MSG_LENGTH 40
#define MAX_PROPERTIES 10

JavaVM* JVM;

individual_t *personProfile;
individual_t *currentSection;

list_t *hasVideoPropList;

jclass *classAgenda;
jclass *classProjector;
jclass *classGallery;
jclass *classKP;

jobject *agendaClassObject;
jobject *presentationClassObject;
jobject *galleryClassObject;

subscription_t *conferenceSubscriptionContainer;
subscription_t *meetingSubscriptionContainer;
subscription_t *presentationSubscriptionContainer;

subscription_t *conferenceClassSubscriptionContainer;
subscription_t *meetingClassSubscriptionContainer;
subscription_t *presentationClassSubscriptionContainer;

char *startedVideoUuid;

int currentTimeslotIndex;
int hasVideoPropListLen;

void addTimeslotToJavaList(JNIEnv *, individual_t *, jobject, jboolean);
void subscriptionHandler(subscription_t *);
void projectorNotificationHandler(subscription_t *);
void agendaNotificationHandler(subscription_t *);
void conferenceNotificationHandler(subscription_t *);
void meetingNotificationHandler(subscription_t *);
void logout();
int sendNewMeetingSpeechNotification(const char *);
int getListSize(list_t*);
int subscribeConferenceService();
int subscribePresentationService();
int subscribeMeetingService();
int calculateTimeslotIndex(prop_val_t *, bool);
int searchPerson(individual_t *, const char *, const char *);
int activatePerson(individual_t *);
char* generateUuid(char*);
bool personExists(const char *);
individual_t* placeExists(const char *); 

jclass getJClassObject(JNIEnv *, char *);
jfieldID getFieldID(JNIEnv *, jclass, char *, char *);
prop_val_t* initNullProperty();
prop_val_t* getPresentationTitleProp(individual_t*);
individual_t* createProfile(individual_t *);
individual_t* createPerson(const char *, const char *, const char*, const char*);
individual_t* getTimeslot(int, bool);
individual_t* getCurrentSection();
individual_t* getCurrentMeetingSection();
individual_t* getFirstTimeslot(bool ismeeting);
individual_t* getContentService();
individual_t* getExistingSection();
individual_t* createPlace(const char *);
list_t* getVideoList();


#endif
