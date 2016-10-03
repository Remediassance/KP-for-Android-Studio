#include "AndroidKp.h"
#include "kp.h"
#include <android/log.h>


/**
 * @fn  JNIEXPORT jint JNICALL Java_petrsu_smartroom_android_srcli_KP_connectSmartSpace(JNIEnv *env, jobject obj, jstring hostname, jstring ip, jint port)
 *
 * @brief Function connects client to Smart Space
 *
 * @param env - pointer to JNI environment
 * @param obj - Java object
 * @param hostname - name of Smart Space
 * @param ip - SIB IP address
 * @param port - SIB port
 *
 * @return Returns 0 in success and -1 if failed
 */
JNIEXPORT jint JNICALL Java_petrsu_smartroom_android_srcli_KP_connectSmartSpace(
		JNIEnv *env, jobject obj, jstring hostname, jstring ip, jint port) {

	const char *hostname_ = (*env) -> GetStringUTFChars(env, hostname, NULL);
	const char *ip_ = (*env) -> GetStringUTFChars(env, ip, NULL);
	char *uuid = generateUuid("KP");

	if(sslog_ss_init_session_with_parameters(hostname_, ip_, (int)port) != 0)
		return -1;

	register_ontology();

	if (ss_join(sslog_get_ss_info(), uuid) == -1) {
		__android_log_print(ANDROID_LOG_ERROR, "connectionSmartSpace():",
				"Connection error");
		return -1;
	}

	if((*env)->GetJavaVM(env, &JVM) != 0)
		__android_log_print(ANDROID_LOG_ERROR, "class KP",
				"Get java VM failed");

	return 0;
}


/**
 * @fn  Java_petrsu_smartroom_android_srcli_KP_disconnectSmartSpace(JNIEnv *env, jobject obj)
 *
 * @brief Function disconnects client from Smart Space
 *
 * @param env - pointer to JNI environment
 * @param obj - Java object
 */
JNIEXPORT void JNICALL Java_petrsu_smartroom_android_srcli_KP_disconnectSmartSpace(
		JNIEnv *env, jobject obj) {

	logout();
	sslog_sbcr_unsubscribe_all(true);
	sslog_ss_leave_session(sslog_get_ss_info());
	sslog_repo_clean_all();
}


/**
 * @brief Logging out user from Smart Space
 */
void logout() {
	prop_val_t *person_prop = sslog_ss_get_property(personProfile,
			PROPERTY_PERSONINFORMATION);
	individual_t *person;
	char *online_status = "online";
	char *offline_status = "offline";

	if(person_prop == NULL)
		return;
	else
		person = (individual_t *)person_prop->prop_value;

	if(person == NULL)
		return;

	sslog_ss_update_property(person, PROPERTY_STATUS->name,
			(void *)online_status,
			(void *)offline_status);
}


/**
 * @fn  Java_petrsu_smartroom_android_srcli_KP_userRegistration(JNIEnv *env, jobject obj, jstring userName, jstring password)
 *
 * @brief Registers client in Smart Space
 *
 * Function takes `userName` and `password` and compares them
 * with corresponding individual properties
 *
 * @param env - pointer to JNI environment
 * @param obj - Java object
 * @param userName - user name
 * @param password - user password
 *
 * @return Returns 0 in success, 1 if username exists and -1 if failed
 */
JNIEXPORT jint JNICALL Java_petrsu_smartroom_android_srcli_KP_userRegistration(
		JNIEnv *env, jobject obj, jstring userName, jstring password) {

	list_t* profileList = sslog_ss_get_individual_by_class_all(CLASS_PROFILE);
	individual_t *profile = NULL;
	int personFounded = -1;

	/* Profiles exist in SmartSpace */
	if(profileList != NULL) {
		list_head_t* pos = NULL;
		list_for_each(pos, &profileList->links) {
			list_t* node = list_entry(pos, list_t, links);
			profile = (individual_t*)(node->data);
			sslog_ss_populate_individual(profile);

			if(profile != NULL) {
				personFounded = checkProfile(profile,
						(*env)->GetStringUTFChars(env, userName, NULL),
						(*env)->GetStringUTFChars(env, password, NULL));
			} else {
				return -1;
			}

			/* Person was found */
			if(personFounded == 0) {
				if(activatePerson(profile) != 0) {
					return -1;
				} else {
					return 0;
				}
			}
		}
	}

	return -1;
}

/**
 * @brief Counts person position in timeslot
 *
 * @return Time slot index if success and -1 otherwise
 */
JNIEXPORT jint JNICALL Java_petrsu_smartroom_android_srcli_KP_personTimeslotIndex(
		JNIEnv *env, jobject obj) {

	individual_t *timeslot = getFirstTimeslot();
	prop_val_t *person = sslog_ss_get_property(personProfile,
			PROPERTY_PERSONINFORMATION);
	individual_t *personInfo;
	int index = 0;		// Time slot index

	if(person != NULL) {
		personInfo = (individual_t *) person->prop_value;
	} else {
		return -1;
	}

	while(timeslot != NULL) {
		prop_val_t *timeslotData = sslog_ss_get_property(timeslot,
				PROPERTY_TIMESLOTPERSON);

		/* If person is not active */
		if(timeslotData == NULL) {
			index += 1;
			prop_val_t *nextTimeslot = sslog_ss_get_property(timeslot,
					PROPERTY_NEXTTIMESLOT);

			if(nextTimeslot != NULL) {
				timeslot = (individual_t *) nextTimeslot->prop_value;
			} else {
				return -1;
			}

			continue;
		}

		individual_t *personTimeslot = (individual_t *) timeslotData->prop_value;

		if(strcmp(personInfo->uuid, personTimeslot->uuid) == 0) {
			return index;
		} else {
			index += 1;
		}

		prop_val_t *nextTimeslot = sslog_ss_get_property(timeslot,
				PROPERTY_NEXTTIMESLOT);

		if(nextTimeslot == NULL)
			break;

		timeslot = (individual_t *) nextTimeslot->prop_value;
	}

	return -1;
}


/**
 * @brief Creates new profile in Smart Space
 * @param userName - user name
 * @param phone - user phone number
 * @param email - user email
 * @return 0 in success and -1 otherwise
 */
JNIEXPORT jint JNICALL Java_petrsu_smartroom_android_srcli_KP_registerGuest(
		JNIEnv *env, jclass clazz, jstring name, jstring phone, jstring email) {

	const char *p_name = (*env)->GetStringUTFChars(env, name, NULL);
	const char *p_phone = (*env)->GetStringUTFChars(env, phone, NULL);
	const char *p_email = (*env)->GetStringUTFChars(env, email, NULL);

	if(personExists(p_name))
		return 1;

	individual_t *person = createPerson(p_name, p_phone, p_email);

	if(person != NULL) {
		if(createProfile(person) == NULL) {
			return -1;
		}
	} else {
		return -1;
	}

	return 0;
}

/**
 * @brief gets place information based on its name
 * @param city - city title
 * @return photo url in success and NULL otherwise
 */
JNIEXPORT jstring JNICALL Java_petrsu_smartroom_android_srcli_KP_setPlaceInfo(
		JNIEnv *env, jclass clazz, jstring city, jstring uuid) {

	const char *p_city = (*env)->GetStringUTFChars(env, city, NULL);
	individual_t *place;

	if(!placeExists(p_city))
		place = createPlace(p_city);

	// Это теперь надо установить в свойство класса персон
	
	individual_t *person = (individual_t *)sslog_repo_get_individual_by_uuid(uuid);

	sslog_ss_add_property(person, PROPERTY_CITY, (void *)place);

	list_t *photosList = sslog_ss_get_individual_by_class_all(CLASS_PHOTO);

	if(photosList != NULL) {
		list_head_t* pos = NULL;
		list_for_each(pos, &photosList->links) {
			list_t* node = list_entry(pos, list_t, links);
			individual_t* photo = (individual_t*)(node->data);
			prop_val_t *p_name = sslog_ss_get_property(photo, PROPERTY_PHOTOURL);

			if(p_name != NULL) {
				return (char *)p_name->prop_value;
			}
		}
	}

	return NULL;
}


JNIEXPORT jint JNICALL Java_petrsu_smartroom_android_srcli_KP_loadParticipantsList(
		JNIEnv *env, jclass clazz, jobject obj) { 

	prop_val_t *propTimeslot = NULL;

	if(obj != NULL) {
			galleryClassObject = (jobject *)(*env)->NewGlobalRef(env, obj);
	} else {
		return -1;
	}

	propTimeslot = sslog_ss_get_property(getCurrentSection(), PROPERTY_FIRSTTIMESLOT);

	if(propTimeslot == NULL) {
		return -1;
	}

	while(propTimeslot != NULL) {
		individual_t *pTimeslot = (individual_t *) propTimeslot->prop_value;

		if(pTimeslot == NULL)
			return -1;

		addTimeslotToParticipantsList(env, pTimeslot, obj);
		propTimeslot = sslog_ss_get_property(pTimeslot, PROPERTY_NEXTTIMESLOT);
		//__android_log_print(ANDROID_LOG_INFO, "addTimeslotToParticipantsList()", "%i", fl);

	}

	return 0;
}



void addTimeslotToParticipantsList(JNIEnv *env, individual_t *timeslot, jobject obj) {
	
	jmethodID methodId = (*env)->GetMethodID(env, classGallery, "addTimeslotItemToList2","(Ljava/lang/String;Ljava/lang/String;)V");
	
	char *name;
	char *uuid = "NoUuid";

	prop_val_t *p_val_name = sslog_ss_get_property (timeslot, PROPERTY_TIMESLOTSPEAKERNAME);
	prop_val_t *p_val_person_link = sslog_ss_get_property (timeslot, PROPERTY_TIMESLOTPERSON);
	prop_val_t *p_val_pres_title = getPresentationTitleProp(timeslot);

	if(p_val_person_link != NULL){
		individual_t *person = (individual_t*)(p_val_person_link -> prop_value);
		uuid = person -> uuid;
		name = (p_val_name == NULL) ? "No name" : (char *)(p_val_name->prop_value);
	} 
	else __android_log_print(ANDROID_LOG_ERROR, "loadTimeSlot()", "PROPERTY_TIMESLOTPERSON is NULL");

		/*Calling CityGallery's addTimeslotItemToList Java method*/
	if(obj != NULL)
		(*env)->CallVoidMethod(env, obj, methodId, 
			(*env)->NewStringUTF(env, uuid),
			(*env)->NewStringUTF(env, name));
	else __android_log_print(ANDROID_LOG_ERROR, "loadTimeSlot()", "Object obj NULL");
}

/**
 * @brief Creates empty profile in SmartSpace
 *
 * @param person - person individual
 * @return Profile individual in success and NULL otherwise
 */
individual_t* createProfile(individual_t *person) {

	individual_t *profile = sslog_new_individual(CLASS_PROFILE);
	char *username = "";
	char *password = "";

	sslog_set_individual_uuid(profile,
			generateUuid("http://www.cs.karelia.ru/smartroom#Profile"));

	if(sslog_ss_add_property(profile, PROPERTY_USERNAME, (void *)username) == -1)
		return NULL;

	if(sslog_ss_add_property(profile, PROPERTY_PASSWORD, (void *)password) == -1)
		return NULL;

	if(sslog_ss_add_property(profile, PROPERTY_PERSONINFORMATION, person) == -1)
			return NULL;

	if(sslog_ss_insert_individual(profile) == -1)
		return NULL;

	personProfile = profile;

	return profile;
}

/**
 * @brief Gets place info based on individual from SmartSpace
 *
 * @param person - person individual
 * @return Place individual in success and NULL otherwise
 */
individual_t* createPlace(const char *city) {

 	char *placeTitle = "";
 	char *placeDescription = "";
 	char *placeFoundingDate = "";

 	individual_t *place = sslog_new_individual(CLASS_PLACE);

	//prop_val_t *p_val_placeTitle = sslog_ss_get_property(person, PROPERTY_CITY);

 	if(sslog_ss_add_property(place, PROPERTY_PLACETITLE, (void *)city) == -1)
		return NULL;

	if(sslog_ss_add_property(place, PROPERTY_PLACEDESCRIPTION, (void *)placeDescription) == -1)
		return NULL;

	if(sslog_ss_add_property(place, PROPERTY_PLACEFOUNDINGDATE, (void *)placeFoundingDate) == -1)
		return NULL;

	if(sslog_ss_insert_individual(place) == -1)
		return NULL;

	return place;
}

/**
 * @brief Creates person individual
 * @param name - user name
 * @param phone - user phone number
 * @param email - user email
 * @param city - user city
 * @return pointer to individual, NULL otherwise
 */
individual_t* createPerson(const char *name, const char *phone,
		const char *email) {

	individual_t *person = sslog_new_individual(CLASS_PERSON);

	if(person == NULL)
		return NULL;

	sslog_set_individual_uuid(person,
			generateUuid("http://xmlns.com/foaf/0.1/#Person"));

	if(sslog_ss_add_property(person, PROPERTY_NAME, (void *)name) == -1) {
		return NULL;
	}

	if(sslog_ss_add_property(person, PROPERTY_PHONE, (void *)phone) == -1) {
		return NULL;
	}

	if(sslog_ss_add_property(person, PROPERTY_MBOX, (void *)email) == -1) {
		return NULL;
	}

	/*if(sslog_ss_add_property(person, PROPERTY_CITY, (void *)city) == -1) {
		return NULL;
	}*/

	return person;
}


/**
 * @brief gets place information based on its name
 * @param city - city title
 * @return photo url in success and NULL otherwise
 */
JNIEXPORT jstring JNICALL Java_petrsu_smartroom_android_srcli_KP_getPlacePhoto(
		JNIEnv *env, jclass clazz, jstring city, jstring uuid) {

	individual_t *place;
	individual_t *person;

	jstring photoValue;
	jclass stringObject = (*env)->FindClass(env, "java/lang/String");

	const char *p_city = (*env)->GetStringUTFChars(env, city, NULL);
	const char *_uuid = (*env)->GetStringUTFChars(env, uuid, NULL);
	

	if(p_city != NULL){
		__android_log_print(ANDROID_LOG_INFO, "getolacePhoto():", "City name passed correctly");
	}
	else {
		__android_log_print(ANDROID_LOG_ERROR, "getPlacePhoto():", "No city name passed");
		return NULL;	
	}

	place = placeExists(p_city);
	
	if(!place){
		__android_log_print(ANDROID_LOG_INFO, "getPlacePhoto():", "Place does not exist!");
		place = createPlace(p_city);
	}
	

	// Это теперь надо установить в свойство класса персон
	
	person = (individual_t *)sslog_repo_get_individual_by_uuid(_uuid);
	if(person == NULL) {
		__android_log_print(ANDROID_LOG_ERROR, "getPlacePhoto():", "Person is NULL");
		return NULL;
	}
	else{
		__android_log_print(ANDROID_LOG_INFO, "getPlacePhoto():", "Fetched person from SmartSpace");
		__android_log_print(ANDROID_LOG_INFO, "getPlacePhoto():", person -> uuid);
		__android_log_print(ANDROID_LOG_INFO, "getPlacePhoto():", place -> uuid);
	}

	if(sslog_ss_add_property(person, PROPERTY_CITY, place) != 0){
		__android_log_print(ANDROID_LOG_ERROR, "getPlacePhoto():", "Cannot set property city to the person!");
		return NULL;
	}
	else{
		__android_log_print(ANDROID_LOG_INFO, "getPlacePhoto():", "Passed successfully3");
	}


	/*Только 1 фото на место*/
	prop_val_t* photo_prop = sslog_ss_get_property(place, PROPERTY_PLACEHASPHOTO); 
	individual_t *photo;

	if(photo_prop == NULL){
		__android_log_print(ANDROID_LOG_ERROR, "getPlacePhoto():", "%s", "PROPERTY_PLACEHASPHOTO is NULL");
		return NULL;
	}
	else 
		__android_log_print(ANDROID_LOG_INFO, "getPlacePhoto():", "PROPERTY_PLACEHASPHOTO is not NULL");

	photo = (individual_t*) (photo_prop -> prop_value);

	if(photo == NULL){
		__android_log_print(ANDROID_LOG_ERROR, "getPlacePhoto():", "%s", "Place has no photos");
		return NULL;
	}
	else
		__android_log_print(ANDROID_LOG_INFO, "getPlacePhoto():", "Place has photo with uuid %s", photo->uuid);

	prop_val_t *photoURL = sslog_ss_get_property(photo, PROPERTY_PHOTOURL);
	if(photoURL == NULL) {
		__android_log_print(ANDROID_LOG_ERROR, "getPlacePhoto():", "%s", "Error getting url of photo");
		return NULL;
	}
	else {		
		__android_log_print(ANDROID_LOG_INFO, "getPlacePhoto():", "got photo url! It is %s", (char*)(photoURL->prop_value));

		(*env)->ReleaseStringUTFChars(env, city, p_city);
		(*env)->ReleaseStringUTFChars(env, uuid, _uuid);

		photoValue = (*env)->NewStringUTF(env,(jstring)(photoURL->prop_value));
		return photoValue;
	}

	return NULL;

}


/**
 * @brief Gets the name of a persons city by his uuid
 * @param uuid - Uuid of a person
 * @return - Returns name of a city if one is set, NULL otherwise
 */
JNIEXPORT jstring JNICALL Java_petrsu_smartroom_android_srcli_KP_getCityByPersonUuid(
		JNIEnv *env, jclass clazz, jstring uuid) {

	jstring cityName;
	jclass stringObject = (*env)->FindClass(env, "java/lang/String");

	const char *_uuid = (*env)->GetStringUTFChars(env, uuid, NULL);

	individual_t* person = (individual_t *)sslog_repo_get_individual_by_uuid(_uuid);

	if(person == NULL){
		__android_log_print(ANDROID_LOG_ERROR, "getCityByPersonUuid():", "Cannot get person by uuid");
		return NULL;
	}
	else __android_log_print(ANDROID_LOG_INFO, "getCityByPersonUuid():", "Got person by uuid!");

	prop_val_t *city_prop = sslog_ss_get_property(person, PROPERTY_CITY);

	if(city_prop == NULL){
		__android_log_print(ANDROID_LOG_ERROR, "getCityByPersonUuid():", "Cannot get city property");
		return NULL;
	}
	else {
		__android_log_print(ANDROID_LOG_INFO, "getCityByPersonUuid():", "Person lives in some city ");	
		
		individual_t* city = (individual_t*) (city_prop -> prop_value);

		prop_val_t *cityTitle = sslog_ss_get_property(city, PROPERTY_PLACETITLE);
		
		if(cityTitle == NULL){
			__android_log_print(ANDROID_LOG_ERROR, "getCityByPersonUuid():", "City title property is not set");
			return NULL;
		}
		else __android_log_print(ANDROID_LOG_INFO, "getCityByPersonUuid():", "City title property is set");

		__android_log_print(ANDROID_LOG_INFO, "getCityByPersonUuid():", "City recieved from ss is %s", (char*)(cityTitle->prop_value));

		cityName = (*env)->NewStringUTF(env,(jstring)(cityTitle->prop_value));

		return cityName;
	}

	return NULL;

}


/**
 * @brief gets place information based on its name
 * @param city - city title
 * @return photo url in success and NULL otherwise
 */
JNIEXPORT jstring JNICALL Java_petrsu_smartroom_android_srcli_KP_getPlaceDescription(
		JNIEnv *env, jclass clazz, jstring city) {

	individual_t *place;
	individual_t *person;

	jstring placeDescription;
	jclass stringObject = (*env)->FindClass(env, "java/lang/String");

	const char *p_city = (*env)->GetStringUTFChars(env, city, NULL);
	//const char *_uuid = (*env)->GetStringUTFChars(env, uuid, NULL);
	

	if(p_city != NULL){
		__android_log_print(ANDROID_LOG_INFO, "getPlaceDescription():", "City name passed correctly");
	}
	else {
		__android_log_print(ANDROID_LOG_ERROR, "getPlaceDescription():", "No city name passed");
		return NULL;	
	}

	place = placeExists(p_city);
	
	if(!place){
		__android_log_print(ANDROID_LOG_INFO, "getPlaceDescription():", "Place does not exist!");
		return NULL;
	}
	else __android_log_print(ANDROID_LOG_INFO, "getPlaceDescription():","%s", place -> uuid);


	/*Только 1 описание на место*/
	prop_val_t* place_prop = sslog_ss_get_property(place, PROPERTY_PLACEDESCRIPTION); 

	if(place_prop == NULL){
		__android_log_print(ANDROID_LOG_ERROR, "getPlaceDescription():", "PROPERTY_PLACEDESCRIPTION is NULL");
		return NULL;
	}
	else {
		__android_log_print(ANDROID_LOG_INFO, "getPlaceDescription():", "PROPERTY_PLACEDESCRIPTION is not NULL");

		(*env)->ReleaseStringUTFChars(env, city, p_city);

		placeDescription = (*env)->NewStringUTF(env,(jstring)(place_prop->prop_value));
		return placeDescription;
	}

	return NULL;
}



/**
 * @brief Gets the name of a persons city by his uuid
 * @param uuid - title of a city
 * @return - Returns founding date of a city if one is set, NULL otherwise
 */
JNIEXPORT jstring JNICALL Java_petrsu_smartroom_android_srcli_KP_getPlaceFoundingDate(
		JNIEnv *env, jclass clazz, jstring city) {

	individual_t *place;

	jstring placeDescription;
	jclass stringObject = (*env)->FindClass(env, "java/lang/String");

	const char *p_city = (*env)->GetStringUTFChars(env, city, NULL);	

	if(p_city != NULL){
		__android_log_print(ANDROID_LOG_INFO, "getPlaceDescription():", "City name passed correctly");
	}
	else {
		__android_log_print(ANDROID_LOG_ERROR, "getPlaceDescription():", "No city name passed");
		return NULL;	
	}

	place = placeExists(p_city);
	
	if(!place){
		__android_log_print(ANDROID_LOG_INFO, "getPlaceDescription():", "Place does not exist!");
		return NULL;
	}
	else __android_log_print(ANDROID_LOG_INFO, "getPlaceDescription():","%s", place -> uuid);


	/*Только 1 описание на место*/
	prop_val_t* place_prop = sslog_ss_get_property(place, PROPERTY_PLACEFOUNDINGDATE); 

	if(place_prop == NULL){
		__android_log_print(ANDROID_LOG_ERROR, "getPlaceDescription():", "%s", "PROPERTY_PLACEFOUNDINGDATE is NULL");
		return NULL;
	}
	else {
		__android_log_print(ANDROID_LOG_INFO, "getPlaceDescription():", "PROPERTY_PLACEFOUNDINGDATE is not NULL");

		(*env)->ReleaseStringUTFChars(env, city, p_city);

		placeDescription = (*env)->NewStringUTF(env,(jstring)(place_prop->prop_value));
		return placeDescription;
	}

	return NULL;
}


/**
 * @brief sets sity property to individual
 * @param uuid - uuid of a user
 * @param city - name of a city to assign
 * @return 0 if success, -1 otherwise
 */
JNIEXPORT jint JNICALL Java_petrsu_smartroom_android_srcli_KP_setCity(JNIEnv *env, jclass clazz, jstring uuid, jstring city) {

	jclass stringObject = (*env)->FindClass(env, "java/lang/String");

	const char *_uuid = (*env)->GetStringUTFChars(env, uuid, NULL);
	const char *_city = (*env)->GetStringUTFChars(env, city, NULL);

	individual_t* newPlace;
	individual_t* oldPlace;
	individual_t* person;

	prop_val_t *op_prop;

	newPlace = (individual_t *)placeExists(_city);

	if(newPlace == NULL){
		__android_log_print(ANDROID_LOG_INFO, "setCity():", "Place does not exist!");
		newPlace = createPlace(_city);
	}
	else __android_log_print(ANDROID_LOG_INFO, "setCity():", "Place exists.");

	person = (individual_t *)sslog_repo_get_individual_by_uuid(_uuid);

	if(person == NULL){
		__android_log_print(ANDROID_LOG_ERROR, "setCity():", "Cannot get person by uuid");
		return -1;
	}
	else __android_log_print(ANDROID_LOG_INFO, "setCity():", "Got person by uuid!");

	oldPlace = sslog_ss_get_property(person, PROPERTY_CITY);
	//oldPlace = (individual_t*)(op_prop -> prop_value);

	/*
	* Do if city wasn't set before fot this individual
	*/
	if(oldPlace == NULL){
		__android_log_print(ANDROID_LOG_INFO, "setCity():", "PROPERTY_CITY was not set before!");

		if(sslog_add_property(person, PROPERTY_CITY, (void *)newPlace) == -1){
			__android_log_print(ANDROID_LOG_ERROR, "setCity():", "Cannot add PROPERTY_CITY to the person");
			return -1;
		} else __android_log_print(ANDROID_LOG_INFO, "setCity():", "Added property to the individual!");

		if(sslog_ss_insert_individual(person) == -1){
			__android_log_print(ANDROID_LOG_ERROR, "setCity():", "Cannot insert individual");
			return -1;
		} else __android_log_print(ANDROID_LOG_INFO, "setCity():", "Inserted the individual to the SS!");
	} 
	/*
	* Do this if city was set before for this individual
	*/
	else {
		__android_log_print(ANDROID_LOG_INFO, "setCity():", "PROPERTY_CITY was set before!");

		/*if(sslog_ss_update_property(person, PROPERTY_CITY, (void*)oldPlace, (void*)newPlace) == -1){
			__android_log_print(ANDROID_LOG_ERROR, "setCity():", "Error updating PROPERTY_CITY to a new value");
			return -1;
		} else __android_log_print(ANDROID_LOG_INFO, "setCity():", "Updated PROPERTY_CITY successfully!");*/

		sslog_ss_remove_property_all(person, PROPERTY_CITY);

		sslog_ss_add_property(person, PROPERTY_CITY, (void*)newPlace);

		/*if(sslog_ss_update_individual(person) == -1){
			__android_log_print(ANDROID_LOG_ERROR, "setCity():", "Error updating the individual");
			return -1;
		} else __android_log_print(ANDROID_LOG_INFO, "setCity():", "Updated individual successfully!");*/
	}

	(*env)->ReleaseStringUTFChars(env, city, _city);
	(*env)->ReleaseStringUTFChars(env, uuid, _uuid);

	return 0;
}

individual_t* createPlace(const char *city) {

 	char *placeTitle = "";
 	char *placeDescription = "";
 	char *placeFoundingDate = "";

 	individual_t *place = sslog_new_individual(CLASS_PLACE);
 	if(place == NULL)
 		__android_log_print(ANDROID_LOG_ERROR, "createPlace():", "Cannot instantiate place variable!");
 	else
 		__android_log_print(ANDROID_LOG_INFO, "createPlace():", "Instantiated place variable!");

 	sslog_set_individual_uuid(place,
			generateUuid("http://www.cs.karelia.ru/smartroom#Place"));

	//prop_val_t *p_val_placeTitle = sslog_ss_get_property(person, PROPERTY_CITY);

 	if(sslog_add_property(place, PROPERTY_PLACETITLE, (void *)city) == -1) {
 		__android_log_print(ANDROID_LOG_ERROR, "createPlace():", "Cannot add placeTitle property!");
		return NULL;
 	}
 	else __android_log_print(ANDROID_LOG_INFO, "createPlace():", "Added placeTitle property");

	if(sslog_add_property(place, PROPERTY_PLACEDESCRIPTION, (void *)placeDescription) == -1){
		__android_log_print(ANDROID_LOG_ERROR, "createPlace():", "Cannot add placeDescription property!");
		return NULL;
	}
	else __android_log_print(ANDROID_LOG_INFO, "createPlace():", "Added placeDescription property");

	if(sslog_add_property(place, PROPERTY_PLACEFOUNDINGDATE, (void *)placeFoundingDate) == -1){
		__android_log_print(ANDROID_LOG_ERROR, "createPlace():", "Cannot add placeFoundingDate property!");
		return NULL;
	}
	else __android_log_print(ANDROID_LOG_INFO, "createPlace():", "Added placeFoundingDate property!");

	if(sslog_ss_insert_individual(place) == -1) {
		__android_log_print(ANDROID_LOG_ERROR, "createPlace():", "Cannot insert individual!");
		return NULL;
	}
	else __android_log_print(ANDROID_LOG_INFO, "createPlace():", "Inserted place individual!");

	return place;
}

/**
 * @brief Checks whether username in Smart Space exists
 *
 * @param username - user name
 * @return TRUE if exists and FALSE otherwise
 */
bool personExists(const char *name) {
	list_t* personList = sslog_ss_get_individual_by_class_all(CLASS_PERSON);

	if(personList != NULL) {
		list_head_t* pos = NULL;
		list_for_each(pos, &personList->links) {
			list_t* node = list_entry(pos, list_t, links);
			individual_t* person = (individual_t*)(node->data);
			prop_val_t *p_name = sslog_ss_get_property(person, PROPERTY_NAME);

			if(p_name != NULL) {
				if(strcmp(name, (char *)p_name->prop_value) == 0) {
					return JNI_TRUE;
				}
			}
		}
	}

	return JNI_FALSE;
}

/**
 * @brief Checks whether place in Smart Space exists
 *
 * @param place - place title
 * @return TRUE if exists and FALSE otherwise
 */
individual_t* placeExists(const char *title) {
	list_t* placeList = sslog_ss_get_individual_by_class_all(CLASS_PLACE);

	if(placeList != NULL) {
		list_head_t* pos = NULL;
		list_for_each(pos, &placeList->links) {
			list_t* node = list_entry(pos, list_t, links);
			individual_t* city = (individual_t*)(node->data);
			prop_val_t *p_title = sslog_ss_get_property(city, PROPERTY_PLACETITLE);

			if(p_title != NULL) {
				__android_log_print(ANDROID_LOG_INFO, "placeExists()","%s vs %s",title,(char *)p_title->prop_value);
				if(strcmp(title, (char *)p_title->prop_value) == 0) {
					__android_log_print(ANDROID_LOG_INFO, "placeExists()","Found match!");
					return (individual_t*)city;
				}
			}
		}
	}
	else {
		__android_log_print(ANDROID_LOG_ERROR, "placeExists():", "Place list doesn't contain this place");
		return NULL;
	}
	return NULL;
}		


/**
 * @brief Searches person in Smart Space
 *
 * @param person - pointer to person individual
 * @param userName - registered user name
 * @param password - user password
 *
 * @return Returns 0 in success and -1 if failed
 */
int checkProfile(individual_t *profile, const char *userName, const char *password) {

	prop_val_t *p_val_username = sslog_ss_get_property (profile, PROPERTY_USERNAME);
	prop_val_t *p_val_password = sslog_ss_get_property (profile, PROPERTY_PASSWORD);

	if(p_val_username == NULL) {
		return -1;
	}

	if(p_val_password == NULL) {
		return -1;
	}

	if((strcmp(userName, (char *)p_val_username->prop_value) == 0) &&
			(strcmp(password, (char *)p_val_password->prop_value) == 0)) {
		personProfile = profile;

		return 0;
	}

	return -1;
}


/**
 * @brief Sets participant state to online/offline
 *
 * @param profile - user profile
 *
 * @return Returns 0 in success and -1 if failed
 */
int activatePerson(individual_t *profile) {

	char *online_status = "online";
	char *offline_status = "offline";

	prop_val_t *person_prop = sslog_ss_get_property(profile,
			PROPERTY_PERSONINFORMATION);
	individual_t *person;

	if(person_prop == NULL)
		return -1;

	person = (individual_t *)person_prop->prop_value;
	sslog_ss_populate_individual(person);

	if(person == NULL)
		return -1;

	/* If property does not exists */
	if(sslog_ss_get_property(person, PROPERTY_STATUS) == NULL) {

		if(sslog_ss_add_property(person, PROPERTY_STATUS,
				(void *)online_status) != 0) {

			__android_log_print(ANDROID_LOG_ERROR, "class KP",
					"%s", sslog_get_error_text());
			return -1;
		}

	} else {
		sslog_ss_update_property(person, PROPERTY_STATUS->name,
				(void *)offline_status,
				(void *)online_status);
	}

	return 0;
}


/**
 * @brief Gets TimeSlot individuals from Smart Space

 * @param env - pointer to JNI environment
 * @param clazz - class keeping loadTimeSlot() function
 * @param obj - Agenda class object
 *
 * @return Returns 0 in success and -1 if failed
 */
JNIEXPORT jint JNICALL Java_petrsu_smartroom_android_srcli_KP_loadTimeslotList(
		JNIEnv *env, jclass clazz, jobject obj) {

	if(obj != NULL) {
		agendaClassObject = (jobject *)(*env)->NewGlobalRef(env, obj);
	} else {
		return -1;
	}

	prop_val_t *propTimeslot = sslog_ss_get_property(getCurrentSection(),
			PROPERTY_FIRSTTIMESLOT);

	if(propTimeslot == NULL) {
		return -1;
	}

	while(propTimeslot != NULL) {
		individual_t *pTimeslot = (individual_t *) propTimeslot->prop_value;

		if(pTimeslot == NULL)
			return -1;

		addTimeslotToJavaList(env, pTimeslot, obj);
		propTimeslot = sslog_ss_get_property(pTimeslot, PROPERTY_NEXTTIMESLOT);
	}

	return 0;
}


/**
 * @brief Fills Agenda list by time slot properties values
 *
 * @param env - pointer to JNI environment
 * @param timeslot - timeslot individual
 * @param obj - Agenda class object
 */
void addTimeslotToJavaList(JNIEnv *env, individual_t *timeslot, jobject obj) {

	jmethodID methodId = (*env)->GetMethodID(env, classAgenda,
			"addTimeslotItemToList",
			"(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V");

	char *name, *title;
	char *status = "offline";
	char *imgLink = "absentImage";

	prop_val_t *p_val_name = sslog_ss_get_property (timeslot,
			PROPERTY_TIMESLOTSPEAKERNAME);
	prop_val_t *p_val_person_link = sslog_ss_get_property (timeslot,
			PROPERTY_TIMESLOTPERSON);
	prop_val_t *p_val_pres_title = getPresentationTitleProp(timeslot);

	name = (p_val_name == NULL) ?
			"No name" : (char *)(p_val_name->prop_value);

	title = (p_val_pres_title == NULL) ?
			"No title" : (char *)(p_val_pres_title->prop_value);

	if(p_val_person_link != NULL) {
		individual_t *person = (individual_t *)p_val_person_link->prop_value;
		prop_val_t *p_val_status = sslog_ss_get_property (person,
				PROPERTY_STATUS);

		status = (p_val_status != NULL) ?
				(char *) p_val_status->prop_value : "offline";

		prop_val_t *p_val_img = sslog_ss_get_property (person, PROPERTY_IMG);

		if(p_val_img != NULL)
			imgLink = (char *)p_val_img->prop_value;
		else if(strcmp(status, "online") == 0)
			imgLink = "noImage";
	}

	/* Calling Agenda's addTimeslotItemToList Java method */
	if(obj != NULL)
		(*env)->CallVoidMethod(env, obj, methodId,
				(*env)->NewStringUTF(env, name),
				(*env)->NewStringUTF(env, title),
				(*env)->NewStringUTF(env, imgLink),
				(*env)->NewStringUTF(env, status));
}


/**
 * @brief Extracts presentation title from Smart Space
 *
 * @param timeslot - time slot individual
 * @return Pointer to title property in success and NULL otherwise
 */
prop_val_t* getPresentationTitleProp(individual_t *timeslot) {
	prop_val_t *p_val_pres = sslog_ss_get_property (timeslot,
			PROPERTY_TIMESLOTPRESENTATION);

	if(p_val_pres == NULL)
		return NULL;

	individual_t *presentation = (individual_t *)p_val_pres->prop_value;

	return sslog_ss_get_property (presentation, PROPERTY_PRESENTATIONTITLE);
}


/**
 * @brief Generates uuid
 *
 * @param uuid - initial uuid value
 * @return Generated uuid
 */
char* generateUuid(char *uuid) {

	int rand_val = 0, rand_length = 1, i = 0, postfix_length = 4;
	char *result = (char*) malloc (
			sizeof(char) * strlen(uuid) + postfix_length + 2);

	for(; i < postfix_length; rand_length *= 10, i++);

	do {
		srand(time(NULL));
		rand_val = rand() % rand_length;
		sprintf(result, "%s-%d", uuid, rand_val);
	} while(sslog_ss_exists_uuid(result) == 1);

	return result;
}


/**
 * @brief Gets information about services by Classes
 *
 * @param menu - service menu object
 */
JNIEXPORT jint JNICALL Java_petrsu_smartroom_android_srcli_KP_getServicesInfo(
		JNIEnv *env, jclass clazz, jobject menu) {

	jclass class = getJClassObject(env, "ServicesMenu");
	jfieldID agendaStateField = getFieldID(env, class,
			"agendaServiceState", "I");
	jfieldID presentationStateField = getFieldID(env, class,
			"presentationServiceState", "I");
	individual_t *agenda = (individual_t *)
			sslog_ss_get_individual_by_class_all(CLASS_TIMESLOT);

	/* TODO: better to get service info by `hasService` property */

	/* If no CLASS_TIMESLOT in Smart Space */
	if(agenda == NULL) {
		__android_log_print(ANDROID_LOG_ERROR, "class KP",
				"Agenda service not found");
		(*env)->SetIntField(env, menu, agendaStateField, 0);
	} else {
		(*env)->SetIntField(env, menu, agendaStateField, 1);
	}

	individual_t *presentation = (individual_t *)
			sslog_ss_get_individual_by_class_all(CLASS_PRESENTATIONSERVICE);

	/* If no CLASS_PRESENTATIONSERVICE in Smart Space */
	if(presentation == NULL) {
		__android_log_print(ANDROID_LOG_ERROR, "class KP",
				"Projector service not found");
		(*env)->SetIntField(env, menu, presentationStateField, 0);
	} else {
		(*env)->SetIntField(env, menu, presentationStateField, 1);
	}

	return 0;
}


/**
 * @brief Short version for taking jclass object
 *
 * @param className - name of class
 * @return JClass in success and NULL otherwise
 */
jclass getJClassObject(JNIEnv *env, char *className) {

	char *classPath = (char *) malloc (sizeof(char) *
			strlen("petrsu/smartroom/android/srcli/") + strlen(className));

	strcpy(classPath, "petrsu/smartroom/android/srcli/");
	strcat(classPath, className);

	jclass class = (*env)->FindClass(env, classPath);

	if(class == NULL) {
		char *error = (char *) malloc (sizeof(char) * 30);
		strcpy(error, "Can't find ");
		strcat(error, className);
		strcat(error, " class");

		return NULL;
	}

	free(classPath);

	return class;
}


/**
 * @brief Gets Java class field ID
 *
 * @param fieldName - class field name
 * @param signature - type signature
 * @return Field ID in success and NULL otherwise
 */
jfieldID getFieldID(JNIEnv *env, jclass class, char *fieldName,
		char *signature) {

	jfieldID fieldID = (*env)->GetFieldID(env, class, fieldName, signature);
	char *err_msg = (char *) malloc (sizeof(char) * ERROR_MSG_LENGTH);

	strcpy(err_msg, "Can't get ");
	strcat(err_msg, fieldName);
	strcat(err_msg, " ID");

	if(fieldID == NULL)
		return NULL;

	free(err_msg);

	return fieldID;
}


/**
 * @brief Initializes subscription to conference activities
 *
 * @return 0 in success, -1 if fail
 */
JNIEXPORT jint JNICALL Java_petrsu_smartroom_android_srcli_KP_initSubscription(
		JNIEnv *env, jobject obj) {

	jclass *classAgendaObj = getJClassObject(env, "Agenda");
	jclass *classProjectorObj = getJClassObject(env, "Projector");

	classAgenda = (jclass *)(*env)->NewGlobalRef(env, classAgendaObj);
	classProjector = (jclass *)(*env)->NewGlobalRef(env, classProjectorObj);

	if(subscribeConferenceService() != 0)
		return -1;
	if(subscribePresentationService() != 0)
		return -1;

	return 0;
}


/**
 * @brief Subscribes to conference-service events and properties
 *
 * @return 0 in success and -1 otherwise
 */
int subscribeConferenceService() {
	extern void conferenceNotificationHandler(subscription_t *);
	void (*pConferenceHandler)(subscription_t *) =
			&conferenceNotificationHandler;

	individual_t *conferenceService;
	list_t *conferenceList =
			sslog_ss_get_individual_by_class_all(CLASS_CONFERENCESERVICE);
	list_t *listPropertiesConfService = list_get_new_list();
	list_t *listPropertiesSection = list_get_new_list();

	conferenceClassSubscriptionContainer = sslog_new_subscription(true);
	conferenceSubscriptionContainer = sslog_new_subscription(true);
	currentSection = getCurrentSection();

	if(conferenceList != NULL) {
		list_head_t* pos = NULL;

		list_for_each(pos, &conferenceList->links) {
			list_t* node = list_entry(pos, list_t, links);
			conferenceService = (individual_t *)(node->data);
			break;
		}

		list_add_data(PROPERTY_CURRENTSECTION, listPropertiesConfService);
		sslog_sbcr_add_individual(conferenceSubscriptionContainer,
				conferenceService, listPropertiesConfService);

		/* If subscribed to presentation service class */
		if(sslog_sbcr_is_active(conferenceClassSubscriptionContainer))
			sslog_sbcr_unsubscribe(conferenceClassSubscriptionContainer);

	} else {
		/* If conference service does not exist in SIB */
		sslog_sbcr_add_class(conferenceClassSubscriptionContainer,
				CLASS_CONFERENCESERVICE);
		sslog_sbcr_add_class(conferenceClassSubscriptionContainer,
				CLASS_AGENDANOTIFICATION);
		sslog_sbcr_set_changed_handler(conferenceClassSubscriptionContainer,
				pConferenceHandler);

		if(sslog_sbcr_subscribe(conferenceClassSubscriptionContainer)
				!= SSLOG_ERROR_NO) {

			__android_log_print(ANDROID_LOG_ERROR, "Conference service", "%s",
					sslog_get_error_text());
			return -1;
		}
	}

	list_add_data(PROPERTY_CURRENTTIMESLOT, listPropertiesSection);
	sslog_sbcr_add_individual(conferenceSubscriptionContainer, currentSection,
				listPropertiesSection);

	sslog_sbcr_set_changed_handler(conferenceSubscriptionContainer,
			pConferenceHandler);

	if(sslog_sbcr_subscribe(conferenceSubscriptionContainer) != SSLOG_ERROR_NO) {
		__android_log_print(ANDROID_LOG_ERROR, "Conference service", "%s",
				sslog_get_error_text());
		return -1;
	}

	return 0;
}


/**
 * @brief Subscribes to presentation-service events and properties
 *
 * @return 0 in success and -1 otherwise
 */
int subscribePresentationService() {
	extern void presentationNotificationHandler(subscription_t *);
	void (*pPresentationHandler)(subscription_t *) =
			&presentationNotificationHandler;

	list_t *presentationList =
			sslog_ss_get_individual_by_class_all(CLASS_PRESENTATIONSERVICE);
	list_t *listPropertiesPresService = list_get_new_list();
	list_t *listProperties = list_get_new_list();
	individual_t *presentationService = NULL;
	subscription_t *sbcrContainer = NULL;

	presentationClassSubscriptionContainer = sslog_new_subscription(true);
	presentationSubscriptionContainer = sslog_new_subscription(true);

	if(presentationList != NULL) {
		list_head_t* pos = NULL;

		list_for_each(pos, &presentationList->links) {
			list_t* node = list_entry(pos, list_t, links);
			presentationService = (individual_t *)(node->data);
			break;
		}

		list_add_data(PROPERTY_CURRENTSLIDENUM, listProperties);
		sslog_sbcr_add_individual(presentationSubscriptionContainer,
				presentationService, listProperties);

		/* If subscribed to presentation service class */
		if(sslog_sbcr_is_active(presentationClassSubscriptionContainer))
			sslog_sbcr_unsubscribe(presentationClassSubscriptionContainer);

		sbcrContainer = presentationSubscriptionContainer;

	} else {
		/* If presentation was not started */
		sslog_sbcr_add_class(
				presentationClassSubscriptionContainer,
				CLASS_PRESENTATIONSERVICE);

		sbcrContainer = presentationClassSubscriptionContainer;
	}

	if(sbcrContainer == NULL) {
		return -1;
	}

	sslog_sbcr_set_changed_handler(sbcrContainer, pPresentationHandler);

	if(sslog_sbcr_subscribe(sbcrContainer) != SSLOG_ERROR_NO) {
		__android_log_print(ANDROID_LOG_ERROR, "Presentation service", "%s",
				sslog_get_error_text());
		return -1;
	}

	return 0;
}


/**
 * @brief Checks whether subscriptions are active
 *
 * @return 0 in success, -1 or -2 if conference or presentation sbcr inactive
 */
JNIEXPORT jint JNICALL Java_petrsu_smartroom_android_srcli_KP_isActiveSubscriptions(
		JNIEnv *env, jclass clazz) {
	if(!sslog_sbcr_is_active(conferenceSubscriptionContainer))
		return -1;
	if(!sslog_sbcr_is_active(presentationSubscriptionContainer))
		return -2;

	return 0;
}

/**
 * obsolete
 */
JNIEXPORT jint JNICALL Java_petrsu_smartroom_android_srcli_KP_refreshConferenceSbcr(
		JNIEnv *env, jclass clazz) {
	sslog_free_subscription(conferenceSubscriptionContainer);
	return subscribeConferenceService();
}


/**
 * obsolete
 */
JNIEXPORT jint JNICALL Java_petrsu_smartroom_android_srcli_KP_refreshPresentationSbcr(
		JNIEnv *env, jclass clazz) {
	sslog_free_subscription(presentationSubscriptionContainer);
	return subscribePresentationService();
}


/**
 * @brief Handles conference subscription
 *
 * @param sbcr - subscriptions from Smart Space
 */
void conferenceNotificationHandler(subscription_t *sbcr) {
	/* Gets JNI environment pointer (env) */
	JNIEnv *env = NULL;
	bool attached = JNI_FALSE;

	/* Initialize `env` pointer in current thread */
	switch((*JVM)->GetEnv(JVM, (void **)&env, JNI_VERSION_1_6)) {
		case JNI_OK:
			break;

		case JNI_EDETACHED:
			(*JVM)->AttachCurrentThread(JVM, &env, NULL);
			attached = JNI_TRUE;
			break;

		case JNI_EVERSION:
			__android_log_print(ANDROID_LOG_ERROR, "conferenceHandler:",
					"invalid JNI version");
			break;
	}

	if(!sslog_sbcr_is_active(conferenceSubscriptionContainer))
			subscribeConferenceService();

	jmethodID updateAgenda = (*env)->GetMethodID(env, classAgenda,
			"updateAgenda", "()V");
	jmethodID updateCurTimeslot = (*env)->GetMethodID(env, classAgenda,
			"updateCurTimeslot", "()V");
	jfieldID conferenceCreatedField = (*env)->GetStaticFieldID(env, classAgenda,
			"agendaCreated", "I");

	/* Agenda activity is active now */
	int conferenceCreated = (*env)->GetStaticIntField(env,
			classAgenda, conferenceCreatedField);

	subscription_changes_data_t *changes = sslog_sbcr_get_changes_last(sbcr);
	list_t *list = sslog_sbcr_ch_get_individual_all(changes);

	if(list != NULL) {
		list_head_t *list_walker = NULL;

		list_for_each(list_walker, &list->links) {
			list_t *node = list_entry(list_walker, list_t, links);
			char *uuid = (char *) node->data;
			individual_t *individual = (individual_t *)
					sslog_repo_get_individual_by_uuid(uuid);

			/* Current time slot has been changed */
			prop_val_t *p_val_curtslot = sslog_ss_get_property (individual,
					PROPERTY_CURRENTTIMESLOT);
			if(p_val_curtslot != NULL) {
				if(conferenceCreated == 1)
					(*env)->CallVoidMethod(env, agendaClassObject,
							updateCurTimeslot);
			}

			/* Current section has been changed */
			prop_val_t *p_val_cursec = sslog_ss_get_property (individual,
					PROPERTY_CURRENTSECTION);
			if(p_val_cursec != NULL) {
					sslog_sbcr_unsubscribe(conferenceSubscriptionContainer);
					subscribeConferenceService();
					if(conferenceCreated == 1)
						(*env)->CallVoidMethod(env, agendaClassObject,
								updateAgenda);
			}

			/* Agenda has been updated */
			prop_val_t *p_val_upd_agenda = sslog_ss_get_property (individual,
					PROPERTY_UPDATEAGENDA);
			if(p_val_upd_agenda != NULL) {
				__android_log_print(ANDROID_LOG_ERROR, "Conference service",
						"updateAgenda");

				if(p_val_upd_agenda->property != NULL)
					__android_log_print(ANDROID_LOG_ERROR,
							"Conference service", "%s",
							p_val_upd_agenda->property->name);
				//if(conferenceCreated == 1)
					//(*env)->CallVoidMethod(env, agendaClassObject, updateAgenda);
			}
		}
	}

	list_free_with_nodes(list, NULL);

	if(attached)
		(*JVM)->DetachCurrentThread(JVM);
}


/**
 * @brief Handles presentation subscription
 *
 * @param sbcr - subscriptions from Smart Space
 */
void presentationNotificationHandler(subscription_t *sbcr) {
	/* Gets JNI environment pointer (env) */
	JNIEnv *env;
	bool attached = false;

	/* Initialize `env` pointer in current thread */
	switch((*JVM)->GetEnv(JVM, (void **)&env, JNI_VERSION_1_6)) {
		case JNI_OK:
			break;

		case JNI_EDETACHED:
			(*JVM)->AttachCurrentThread(JVM, &env, NULL);
			attached = true;
			break;

		case JNI_EVERSION:
			__android_log_print(ANDROID_LOG_ERROR, "presentationHandler:",
					"invalid JNI version");
			break;
	}

	jmethodID setSlideNumId = (*env)->GetMethodID(env, classProjector,
			"setSlideNumber", "(Ljava/lang/String;)V");
	jmethodID setSlideCountId = (*env)->GetMethodID(env, classProjector,
			"setSlideCount", "(Ljava/lang/String;)V");
	jmethodID setSlideImageId = (*env)->GetMethodID(env, classProjector,
			"setSlideImage", "(Ljava/lang/String;)V");
	jmethodID updateProjector = (*env)->GetMethodID(env, classProjector,
			"updateProjector", "()V");
	jfieldID presentationCreated = (*env)->GetStaticFieldID(env, classProjector,
			"presentationCreated", "I");
	int presentationIsCreated = (*env)->GetStaticIntField(env,
			classProjector, presentationCreated);

	if(!sslog_sbcr_is_active(presentationSubscriptionContainer))
		subscribePresentationService();

	/* Projector activity is active now */
	if(presentationIsCreated != 1) {
		if(attached)
			(*JVM)->DetachCurrentThread(JVM);
		return;
	}

	subscription_changes_data_t *changes = sslog_sbcr_get_changes_last(sbcr);
	list_t *list = sslog_sbcr_ch_get_individual_all(changes);

	if(list != NULL) {
		list_head_t *list_walker = NULL;
		list_for_each(list_walker, &list->links) {
			list_t *node = list_entry(list_walker, list_t, links);
			char *uuid = (char *) node->data;
			individual_t *individual = (individual_t *)
					sslog_repo_get_individual_by_uuid(uuid);

			/* Slide number has been changed */
			prop_val_t *p_val_slidenum = sslog_ss_get_property (individual,
						PROPERTY_CURRENTSLIDENUM);
			if(p_val_slidenum != NULL) {
				(*env)->CallVoidMethod(env, presentationClassObject,
						setSlideNumId,
						(*env)->NewStringUTF(
								env, (char *)p_val_slidenum->prop_value));
			}

			/* Slide count has been changed */
			prop_val_t *p_val_slidecount = sslog_ss_get_property (individual,
						PROPERTY_CURRENTSLIDECOUNT);
			if(p_val_slidecount != NULL) {
				(*env)->CallVoidMethod(env, presentationClassObject,
						setSlideCountId,
						(*env)->NewStringUTF(
								env, (char *)p_val_slidecount->prop_value));
			}

			/* Slide image has been changed */
			prop_val_t *p_val_slideimg = sslog_ss_get_property (individual,
						PROPERTY_CURRENTSLIDEIMG);
			if(p_val_slideimg != NULL) {
				(*env)->CallVoidMethod(env, presentationClassObject, setSlideImageId,
						(*env)->NewStringUTF(
								env, (char *)p_val_slideimg->prop_value));
			}
		}
	}

	(*env)->CallVoidMethod(env, presentationClassObject, updateProjector);
	list_free_with_nodes(list, NULL);

	if(attached)
		(*JVM)->DetachCurrentThread(JVM);
}


/**
 * @brief Gets current time slot index
 *
 * @return Current time slot index
 */
JNIEXPORT jint JNICALL Java_petrsu_smartroom_android_srcli_KP_getCurrentTimeslotIndex(
		JNIEnv *env, jclass clazz) {
	return calculateTimeslotIndex(NULL);
}


/**
 * @brief Checks whether section has been changed
 *
 * @return TRUE if section has been changed and FALSE otherwise
 */
JNIEXPORT jboolean JNICALL Java_petrsu_smartroom_android_srcli_KP_sectionChanged(
		JNIEnv *env, jclass class) {
	individual_t *currentSection_ = getCurrentSection();

	if(currentSection == NULL && currentSection_ != NULL) {
		return JNI_TRUE;
	}

	if(currentSection != NULL && currentSection_ != NULL) {
		if(strcmp(currentSection->uuid, currentSection_->uuid) != 0) {
			return JNI_TRUE;
		}
	}

	return JNI_FALSE;
}

/**
 * @brief Calculates current time slot index
 *
 * @param propTimeslot - property with current time slot
 * @return Time slot index and -1 otherwise
 */
int calculateTimeslotIndex(prop_val_t *propTimeslot) {

	individual_t *curTimeslot;
	individual_t *timeslot = getFirstTimeslot();
	currentTimeslotIndex = 1;

	if(propTimeslot != NULL) {
		curTimeslot = (individual_t *) propTimeslot->prop_value;
	} else {
		propTimeslot = sslog_ss_get_property (getCurrentSection(),
				PROPERTY_CURRENTTIMESLOT);

		if(propTimeslot != NULL)
			curTimeslot = (individual_t *) propTimeslot->prop_value;
		else
			return -1;
	}

	while(strcmp(curTimeslot->uuid, timeslot->uuid) != 0) {
		prop_val_t *val = sslog_ss_get_property(timeslot,
				PROPERTY_NEXTTIMESLOT);

		if(val != NULL) {
			timeslot = (individual_t *) val->prop_value;
			++currentTimeslotIndex;
		} else {
			return -1;
		}
	}

	return currentTimeslotIndex;
}

/**
 * @brief Function checks the fact that current
 * participant is a speaker
 *
 * @return True if participant is a speaker and
 * false otherwise
 */
JNIEXPORT jboolean JNICALL Java_petrsu_smartroom_android_srcli_KP_checkSpeakerState(
		JNIEnv *env, jclass clazz) {

	individual_t *timeslot;
	prop_val_t *curValue = sslog_ss_get_property(getCurrentSection(),
			PROPERTY_CURRENTTIMESLOT);

	if(curValue == NULL) {
		return JNI_FALSE;
	}

	timeslot = (individual_t *) curValue->prop_value;

	prop_val_t *personValue = sslog_ss_get_property(timeslot,
			PROPERTY_TIMESLOTPERSON);

	if(personValue == NULL) {
		return JNI_FALSE;
	}

	individual_t *curPerson = (individual_t *) personValue->prop_value;

	prop_val_t *person = sslog_ss_get_property(personProfile,
			PROPERTY_PERSONINFORMATION);

	if(person == NULL) {
		return JNI_FALSE;
	}

	individual_t *personInfo = (individual_t *) person->prop_value;

	if(strcmp(personInfo->uuid, curPerson->uuid) == 0) {
		return JNI_TRUE;
	} else {
		return JNI_FALSE;
	}
}


/**
 * @brief Loads presentation data from Smart Space
 *
 * @param presentationClassObj - Projector class object
 * @return 0 in success and -1 otherwise
 */
JNIEXPORT jint JNICALL Java_petrsu_smartroom_android_srcli_KP_loadPresentation(
		JNIEnv *env, jobject obj, jobject presentationClassObj) {

	prop_val_t *p_val_slideNum = NULL;
	prop_val_t *p_val_slideCount = NULL;
	prop_val_t *p_val_slideImg = NULL;
	individual_t *presentationService = NULL;

	if(presentationClassObj == NULL)
		return -1;

	presentationClassObject = (jobject *)(*env)->NewGlobalRef(env,
			presentationClassObj);

	__android_log_print(ANDROID_LOG_ERROR, "loadPresentation()", "start");

	jmethodID setSlideNumId = (*env)->GetMethodID(env, classProjector,
			"setSlideNumber", "(Ljava/lang/String;)V");
	jmethodID setSlideCountId = (*env)->GetMethodID(env, classProjector,
			"setSlideCount", "(Ljava/lang/String;)V");
	jmethodID setSlideImageId = (*env)->GetMethodID(env, classProjector,
			"setSlideImage", "(Ljava/lang/String;)V");
	list_t* presentationServiceList = sslog_ss_get_individual_by_class_all(
				CLASS_PRESENTATIONSERVICE);

	if(presentationServiceList != NULL) {
		list_head_t* pos = NULL;
		list_for_each(pos, &presentationServiceList->links) {
			list_t* node = list_entry(pos, list_t, links);
			presentationService = (individual_t*)(node->data);
			break;
		}

		/* Set slide image value */
		p_val_slideImg = sslog_ss_get_property (presentationService,
				PROPERTY_CURRENTSLIDEIMG);
		if(p_val_slideImg != NULL) {
			(*env)->CallVoidMethod(env, presentationClassObject,
					setSlideImageId,
					(*env)->NewStringUTF(env,
							(char *)p_val_slideImg->prop_value));
		}

		/* Set slide number value */
		p_val_slideNum = sslog_ss_get_property (presentationService,
				PROPERTY_CURRENTSLIDENUM);
		if(p_val_slideNum != NULL) {
			(*env)->CallVoidMethod(env, presentationClassObject,
					setSlideNumId,
					(*env)->NewStringUTF(env,
							(char *)p_val_slideNum->prop_value));
		}

		/* Set slide count value */
		p_val_slideCount = sslog_ss_get_property (presentationService,
				PROPERTY_CURRENTSLIDECOUNT);
		if(p_val_slideCount != NULL) {
			(*env)->CallVoidMethod(env, presentationClassObject,
					setSlideCountId,
					(*env)->NewStringUTF(env,
							(char *)p_val_slideCount->prop_value));
		}
	}

	__android_log_print(ANDROID_LOG_ERROR, "loadPresentation()", "DONE");

	return 0;
}


/**
 * @brief Starts conference
 *
 * @return 0 in success, -1 otherwise
 */
JNIEXPORT jint JNICALL Java_petrsu_smartroom_android_srcli_KP_startConference(
		JNIEnv *env, jobject obj) {

	individual_t *agendaNotif = sslog_new_individual(CLASS_AGENDANOTIFICATION);
	individual_t *confNotif = sslog_new_individual(CLASS_CONFERENCENOTIFICATION);

	if(agendaNotif == NULL || confNotif == NULL) {
		return -1;
	}

	sslog_set_individual_uuid(agendaNotif,
			generateUuid("http://www.cs.karelia.ru/smartroom#AgendaNotification"));
	sslog_set_individual_uuid(confNotif,
			generateUuid("http://www.cs.karelia.ru/smartroom#ConferenceNotification"));

	/* Prepare agenda notification */
	if(sslog_ss_add_property(agendaNotif, PROPERTY_STARTCONFERENCE,
			getCurrentSection()) != 0 ) {
		__android_log_print(ANDROID_LOG_ERROR,
				"startConference(): agendaNotif",
				"%s", sslog_get_error_text());
		return -1;
	}

	/* Prepare conference notification */
	if(sslog_ss_add_property(confNotif, PROPERTY_STARTCONFERENCE,
			getCurrentSection()) != 0 ) {
		__android_log_print(ANDROID_LOG_ERROR,
				"startConference(): conferenceNotif",
				"%s", sslog_get_error_text());
		return -1;
	}

	if(sslog_ss_insert_individual(agendaNotif) != 0) {
		__android_log_print(ANDROID_LOG_ERROR, "startConference()",
				"%s", sslog_get_error_text());
		return -1;
	}

	if(sslog_ss_insert_individual(confNotif) != 0) {
		__android_log_print(ANDROID_LOG_ERROR, "startConference()",
				"%s", sslog_get_error_text());
		return -1;
	}

	return 0;
}

/**
 * @brief Ends conference
 *
 * @return 0 in success, -1 otherwise
 */
JNIEXPORT jint JNICALL Java_petrsu_smartroom_android_srcli_KP_endConference(
		JNIEnv *env, jobject obj) {
	individual_t *confNotif = sslog_new_individual(CLASS_CONFERENCENOTIFICATION);

	if(confNotif == NULL) {
		return -1;
	}

	sslog_set_individual_uuid(confNotif,
			generateUuid("http://www.cs.karelia.ru/smartroom#ConferenceNotification"));

	/* Prepare conference notification */
	if(sslog_ss_add_property(confNotif, PROPERTY_ENDCONFERENCE,
			getCurrentSection()) != 0 ) {
		__android_log_print(ANDROID_LOG_ERROR, "endConference()",
				"%s", sslog_get_error_text());
		return -1;
	}

	if(sslog_ss_insert_individual(confNotif) != 0) {
		__android_log_print(ANDROID_LOG_ERROR, "endConference()",
				"%s", sslog_get_error_text());
		return -1;
	}

	return 0;
}

/**
 * @brief Sends notification to change presentation slide
 *
 * @param slideNumber - presentation slide number
 * @return 0 in success and -1 otherwise
 */
JNIEXPORT jint JNICALL Java_petrsu_smartroom_android_srcli_KP_showSlide(
		JNIEnv *env, jclass clazz, jint slideNumber) {

	int ret_val = 0;
	char number[] = "0";
	sprintf(number, "%d", (int)slideNumber);

	individual_t *individual = sslog_new_individual(
			CLASS_PRESENTATIONNOTIFICATION);

	if(individual == NULL) {
		return -1;
	}

	ret_val = sslog_set_individual_uuid(individual,
			generateUuid("http://www.cs.karelia.ru/smartroom#PresentationNotification"));

	if(ret_val != 1) {
		return -1;
	}

	/* Prepare presentation notification */
	if(sslog_ss_add_property(individual, PROPERTY_SHOWSLIDE,
			(void *)number) != 0 ) {
		__android_log_print(ANDROID_LOG_ERROR, "showSlide()",
				"%s", sslog_get_error_text());
		return -1;
	}

	if(sslog_ss_insert_individual(individual) != 0) {
		__android_log_print(ANDROID_LOG_ERROR, "showSlide()",
				"%s", sslog_get_error_text());
		return -1;
	}

	return 0;
}

/**
 * @brief Ends current presentation
 *
 * @return 0 in success and -1 otherwise
 */
JNIEXPORT jint JNICALL Java_petrsu_smartroom_android_srcli_KP_endPresentation(
		JNIEnv *env, jclass clazz) {
	prop_val_t *curPresentation = NULL;
	individual_t *presentationService = NULL;
	list_t* presentationServiceList = sslog_ss_get_individual_by_class_all(
			CLASS_PRESENTATIONSERVICE);

	if(presentationServiceList != NULL) {
		list_head_t* pos = NULL;
		list_for_each(pos, &presentationServiceList->links) {
			list_t* node = list_entry(pos, list_t, links);
			presentationService = (individual_t*)(node->data);
			break;
		}
	}

	if(presentationService != NULL) {
		curPresentation = sslog_ss_get_property(presentationService,
				PROPERTY_CURRENTPRESENTATION);
	} else {
		return -1;
	}

	if(curPresentation == NULL) {
		__android_log_print(ANDROID_LOG_ERROR, "endPresentation()",
				"current presentation prop = NULL");
		return -1;
	}

	individual_t *presentation = (individual_t *)curPresentation->prop_value;
	individual_t *presentationNotif = sslog_new_individual(
			CLASS_PRESENTATIONNOTIFICATION);

	sslog_set_individual_uuid(presentationNotif,
					generateUuid(
							"http://www.cs.karelia.ru/smartroom#PresentationNotification"));

	/* Prepare presentation notification */
	if(sslog_ss_add_property(presentationNotif, PROPERTY_ENDPRESENTATION,
			presentation) != 0 ) {
		__android_log_print(ANDROID_LOG_ERROR, "endPresentation()",
				"%s", sslog_get_error_text());
		return -1;
	}

	if(sslog_ss_insert_individual(presentationNotif) != 0) {
		__android_log_print(ANDROID_LOG_ERROR, "endPresentation()",
				"%s", sslog_get_error_text());
		return -1;
	}

	individual_t *conferenceNotif = sslog_new_individual(
			CLASS_CONFERENCENOTIFICATION);

	sslog_set_individual_uuid(conferenceNotif,
					generateUuid(
							"http://www.cs.karelia.ru/smartroom#ConferenceNotification"));

	/* Prepare conference notification */
	if(sslog_ss_add_property(conferenceNotif, PROPERTY_ENDPRESENTATION,
			presentation) != 0 ) {
		__android_log_print(ANDROID_LOG_ERROR, "endPresentation()",
				"%s", sslog_get_error_text());
		return -1;
	}

	if(sslog_ss_insert_individual(conferenceNotif) != 0) {
		__android_log_print(ANDROID_LOG_ERROR, "endPresentation()",
				"%s", sslog_get_error_text());
		return -1;
	}

	return 0;
}


/**
 * @brief Extracts microphone service IP address
 *
 * @return IP address in success and NULL otherwise
 */
JNIEXPORT jstring JNICALL Java_petrsu_smartroom_android_srcli_KP_getMicServiceIP(
		JNIEnv *env, jclass clazz) {

	list_t *list = sslog_ss_get_individual_by_class_all(CLASS_MICROPHONESERVICE);
	individual_t *individual;

	if(list != NULL) {
		list_head_t* pos = NULL;
		list_for_each(pos, &list->links) {
			list_t* node = list_entry(pos, list_t, links);
			individual = (individual_t*)(node->data);
			sslog_ss_populate_individual(individual);
		}
	} else {
		return NULL;
	}

	prop_val_t *ip_value = sslog_ss_get_property(individual, PROPERTY_IP);

	if(ip_value == NULL) {
		return NULL;
	}

	return (*env)->NewStringUTF(env, (char *)ip_value->prop_value);
}

/**
 * @brief Extracts Discussion service IP address
 *
 * @return IP address of the Discussion service if success, NULL otherwise
 */
JNIEXPORT jstring JNICALL Java_petrsu_smartroom_android_srcli_KP_getDiscussionServiceIP(
	JNIEnv *env, jclass clazz){

	list_t *list = sslog_ss_get_individual_by_class_all(CLASS_DISCUSSIONSERVICE);
	individual_t *individual;

	if(list != NULL) {
		list_head_t* pos = NULL;
		list_for_each(pos, &list->links) {
			list_t* node = list_entry(pos, list_t, links);
			individual = (individual_t*)(node->data);
			sslog_ss_populate_individual(individual);
		}
	} else {
		return NULL;
	}

	prop_val_t *ip_value = sslog_ss_get_property(individual, PROPERTY_HASCLIENTURL);

	if(ip_value == NULL) {
		return NULL;
	}

	return (*env)->NewStringUTF(env, (char *)ip_value->prop_value);
}

/**
 * @brief Extracts SocialProgram service IP address
 *
 * @return IP address of the SocialProgram service if success, NULL otherwise
 */
JNIEXPORT jstring JNICALL Java_petrsu_smartroom_android_srcli_KP_getSocialProgramServiceIP(
	JNIEnv *env, jclass clazz){

	list_t *list = sslog_ss_get_individual_by_class_all(CLASS_SOCIALPROGRAMSERVICE);
	individual_t *individual;

	if(list != NULL) {
		list_head_t* pos = NULL;
		list_for_each(pos, &list->links) {
			list_t* node = list_entry(pos, list_t, links);
			individual = (individual_t*)(node->data);
			sslog_ss_populate_individual(individual);
		}
	} else {
		return NULL;
	}

	prop_val_t *ip_value = sslog_ss_get_property(individual, PROPERTY_HASCLIENTURL);

	if(ip_value == NULL) {
		return NULL;
	}

	return (*env)->NewStringUTF(env, (char *)ip_value->prop_value);
}

/**
 * @brief Extracts microphone-service port
 *
 * @return Port in success and NULL otherwise
 */
JNIEXPORT jstring JNICALL Java_petrsu_smartroom_android_srcli_KP_getMicServicePort
  (JNIEnv *env, jclass clazz) {

	list_t *list = sslog_ss_get_individual_by_class_all(CLASS_MICROPHONESERVICE);
	individual_t *individual;

	if(list != NULL) {
		list_head_t* pos = NULL;
		list_for_each(pos, &list->links) {
			list_t* node = list_entry(pos, list_t, links);
			individual = (individual_t*)(node->data);
			sslog_ss_populate_individual(individual);
		}
	} else {
		return NULL;
	}

	prop_val_t *port_value = sslog_ss_get_property(individual, PROPERTY_PORT);

	if(port_value == NULL) {
		return NULL;
	}

	return (*env)->NewStringUTF(env, (char *)port_value->prop_value);
}


/**
 * @brief Gets current speaker name
 *
 * @return Speaker name in success and NULL otherwise
 */
JNIEXPORT jstring JNICALL Java_petrsu_smartroom_android_srcli_KP_getSpeakerName(
		JNIEnv *env, jclass clazz) {

	prop_val_t *curTimeslotProp = sslog_ss_get_property(getCurrentSection(),
			PROPERTY_CURRENTTIMESLOT);

	if(curTimeslotProp == NULL) {
		return NULL;
	}

	individual_t *curTimeslot = (individual_t *)curTimeslotProp->prop_value;
	prop_val_t *personLinkProp = sslog_ss_get_property(curTimeslot,
			PROPERTY_TIMESLOTPERSON);

	if(personLinkProp == NULL) {
		return NULL;
	}

	individual_t *person = (individual_t *)personLinkProp->prop_value;
	prop_val_t *personName = sslog_ss_get_property(person, PROPERTY_NAME);

	if(personName != NULL) {
		return (*env)->NewStringUTF(env, (char *)personName->prop_value);
	} else {
		return NULL;
	}
}


/**
 * @brief Checks whether connection established
 *
 * @return TRUE if connection established and FALSE otherwise
 */
JNIEXPORT jboolean JNICALL Java_petrsu_smartroom_android_srcli_KP_checkConnection(
		JNIEnv *env, jobject obj) {

	if(sslog_ss_exists_class(CLASS_SECTION) != 1) {
		return JNI_FALSE;
	} else {
		return JNI_TRUE;
	}
}


/**
 * @brief Gets presentation URL
 *
 * @param index - index of time slot
 * @return Presentation URL in success and NULL otherwise
 */
JNIEXPORT jstring JNICALL Java_petrsu_smartroom_android_srcli_KP_getPresentationLink(
		JNIEnv *env, jclass clazz, jint index) {

	individual_t *person = NULL;
	individual_t *presentation = NULL;
	individual_t *pTimeslot = NULL;

	pTimeslot = getTimeslot(index);

	if(pTimeslot == NULL)
		return NULL;

	prop_val_t *propPerson = sslog_ss_get_property(pTimeslot,
			PROPERTY_TIMESLOTPERSON);

	if(propPerson != NULL) {
		person = (individual_t *) propPerson->prop_value;
	} else {
		return NULL;
	}

	prop_val_t *propPresents = sslog_ss_get_property(person,
			PROPERTY_HASPRESENTATION);

	if(propPresents != NULL) {
		presentation = (individual_t *) propPresents->prop_value;
	} else {
		return NULL;
	}

	if(presentation != NULL) {
		prop_val_t *link = sslog_ss_get_property(presentation,
				PROPERTY_PRESENTATIONURL);

		if(link != NULL) {
			return (*env)->NewStringUTF(env, (char *)link->prop_value);
		}
	}

	return NULL;
}


/**
 * @brief Starts conference from selected time slot
 *
 * @param index - time slot index
 * @return 0 in success and -1 otherwise
 */
JNIEXPORT jint JNICALL Java_petrsu_smartroom_android_srcli_KP_startConferenceFrom
  (JNIEnv *env, jclass clazz, jint index) {

	int ret_val = 0;
	individual_t *timeslot = NULL;
	individual_t *individual = sslog_new_individual(CLASS_AGENDANOTIFICATION);

	if(individual == NULL)
		return -1;

	ret_val = sslog_set_individual_uuid(individual,
			generateUuid("http://www.cs.karelia.ru/smartroom#Notification"));

	if(ret_val != SSLOG_ERROR_NO)
		return -1;

	timeslot = getTimeslot(index);

	/* Prepare agenda notification */
	if(sslog_ss_add_property(individual, PROPERTY_STARTCONFERENCEFROM,
			timeslot) != 0 ) {
		__android_log_print(ANDROID_LOG_ERROR, "startConferenceFrom()",
				"%s", sslog_get_error_text());
		return -1;
	}

	if(sslog_ss_insert_individual(individual) != 0) {
		__android_log_print(ANDROID_LOG_ERROR, "startConferenceFrom()",
				"%s", sslog_get_error_text());
		return -1;
	}

	return 0;
}

/**
 * @brief Get timeslot's individual by index
 *
 * @param index - time slot index
 * @return Time slot individual in success and NULL otherwise
 */
individual_t* getTimeslot(int index) {

	prop_val_t *propTimeslot = sslog_ss_get_property(getCurrentSection(),
			PROPERTY_FIRSTTIMESLOT);
	individual_t *pTimeslot = NULL;
	int i = 0;

	if(propTimeslot == NULL) {
		propTimeslot = sslog_ss_get_property(getCurrentSection(),
				PROPERTY_FIRSTTIMESLOT);
	}

	for(; (i <= index) && (propTimeslot != NULL); i++) {
		pTimeslot = (individual_t *) propTimeslot->prop_value;

		if(pTimeslot != NULL) {
			propTimeslot = sslog_ss_get_property(pTimeslot,
					PROPERTY_NEXTTIMESLOT);
		}
	}

	return pTimeslot;
}

/**
 * @brief Loads chosen profile from agenda service
 *
 * @param obj - Profile class object
 * @param index - time slot index
 * @return Profile uuid in success and NULL otherwise
 */
JNIEXPORT jstring JNICALL Java_petrsu_smartroom_android_srcli_KP_loadProfile(
		JNIEnv *env, jclass clazz, jobject obj, jint index) {

	jclass *classProfile = getJClassObject(env, "Profile");
	individual_t *timeslot = getTimeslot(index);
	individual_t *person = NULL;

	if(obj == NULL || timeslot == NULL) {
		return NULL;
	}

	jmethodID setNameId = (*env)->GetMethodID(env, classProfile,
			"setName", "(Ljava/lang/String;)V");
	jmethodID setPhoneId = (*env)->GetMethodID(env, classProfile,
			"setPhone", "(Ljava/lang/String;)V");
	jmethodID setImageId = (*env)->GetMethodID(env, classProfile,
			"setImage", "(Ljava/lang/String;)V");

	prop_val_t *propPerson = sslog_ss_get_property(timeslot,
			PROPERTY_TIMESLOTPERSON);

	if(propPerson != NULL) {
		person = (individual_t *) propPerson->prop_value;
	} else {
		return NULL;
	}

	if(person == NULL)
		return NULL;

	/* Set name property */
	prop_val_t *propName = sslog_ss_get_property(person, PROPERTY_NAME);
	if(propName != NULL) {
		(*env)->CallVoidMethod(env, obj, setNameId,
				(*env)->NewStringUTF(env, (char *)propName->prop_value));
	}

	/* Set phone property */
	prop_val_t *propPhone = sslog_ss_get_property(person, PROPERTY_PHONE);
	if(propPhone != NULL) {
		(*env)->CallVoidMethod(env, obj, setPhoneId,
				(*env)->NewStringUTF(env, (char *)propPhone->prop_value));
	}

	/* Set image property */
	prop_val_t *propImage = sslog_ss_get_property(person, PROPERTY_IMG);
	if(propImage != NULL) {
		(*env)->CallVoidMethod(env, obj, setImageId,
				(*env)->NewStringUTF(env, (char *)propImage->prop_value));
	}

	return (*env)->NewStringUTF(env, (char *)person->uuid);
}

/**
 * @brief Saves profile to Smart Space
 *
 * @param name - person name
 * @param phone - person phone number
 * @return 0 in success and -1 otherwise
 */
JNIEXPORT jint JNICALL Java_petrsu_smartroom_android_srcli_KP_saveProfileChanges(
		JNIEnv *env, jclass clazz, jstring name, jstring phone) {

	prop_val_t *person_prop = sslog_ss_get_property(personProfile,
			PROPERTY_PERSONINFORMATION);
	individual_t *person;

	const char *new_name = (*env)->GetStringUTFChars(env, name, NULL);
	const char *new_phone = (*env)->GetStringUTFChars(env, phone, NULL);

	if(person_prop == NULL) {
		return -1;
	}

	person = (individual_t *)person_prop->prop_value;

	if(person == NULL) {
		return -1;
	}

	prop_val_t *person_name = sslog_ss_get_property(person, PROPERTY_NAME);
	char *p_name;

	if(person_name != NULL)
		p_name = (char *)person_name->prop_value;

	prop_val_t *person_phone = sslog_ss_get_property(person, PROPERTY_PHONE);
	char *p_phone;

	if(person_phone != NULL)
		p_phone = (char *)person_phone->prop_value;

	sslog_ss_update_property(person, PROPERTY_PHONE->name, (void *)p_phone,
					(void *)new_phone);
	sslog_ss_update_property(person, PROPERTY_NAME->name, (void *)p_name,
				(void *)new_name);

	return 0;
}


/**
 * @brief Gets person uuid
 *
 * @return Person uuid in success and NULL otherwise
 */
JNIEXPORT jstring JNICALL Java_petrsu_smartroom_android_srcli_KP_getPersonUuid
  (JNIEnv *env, jclass clazz) {

	prop_val_t *person_prop = sslog_ss_get_property(personProfile,
			PROPERTY_PERSONINFORMATION);
	individual_t *person;

	if(person_prop == NULL) {
		return NULL;
	}

	person = (individual_t *)person_prop->prop_value;

	if(person == NULL) {
		return NULL;
	}

	return (*env)->NewStringUTF(env, (char *)person->uuid);
}


/**
 * @brief Gets current section individual
 *
 * @return Individual of current section in success and NULL otherwise
 */
individual_t* getCurrentSection() {
	list_t *conferenceList = sslog_ss_get_individual_by_class_all(
			CLASS_CONFERENCESERVICE);
	individual_t *conference;

	// TODO: choose section according to user's choice

	if(conferenceList == NULL) {
		return getExistingSection();
	}

	list_head_t* pos = NULL;
	list_for_each(pos, &conferenceList->links) {
		list_t* node = list_entry(pos, list_t, links);
		conference = (individual_t *)(node->data);
		break;
	}

	prop_val_t *curSectionProp = sslog_ss_get_property(conference,
			PROPERTY_CURRENTSECTION);

	if(curSectionProp == NULL) {
		return getExistingSection();
	}

	return (individual_t *)(curSectionProp->prop_value);
}


/**
 * @brief Gets first existing section
 *
 * @return Section individual in success and NULL otherwise
 */
individual_t* getExistingSection() {
	list_t *sectionsList = sslog_ss_get_individual_by_class_all(CLASS_SECTION);
	individual_t *section;

	if(sectionsList == NULL) {
		return NULL;
	}

	list_head_t *pos = NULL;
	list_for_each(pos, &sectionsList->links) {
		list_t* node = list_entry(pos, list_t, links);
		section = (individual_t *)(node->data);
		break;
	}

	return section;
}


/**
 * @brief Gets first time slot individual
 *
 * @return First time slot individual in success and NULL otherwise
 */
individual_t* getFirstTimeslot() {
	individual_t *section = getCurrentSection();
	individual_t *timeslot;
	prop_val_t *firstTimeslotProp = sslog_ss_get_property(section,
			PROPERTY_FIRSTTIMESLOT);

	if(firstTimeslotProp != NULL) {
		timeslot = (individual_t *)firstTimeslotProp->prop_value;
	}

	return timeslot;
}


/**
 * @brief Extracts content-service URL
 *
 * @return Content service URL in success and NULL otherwise
 */
JNIEXPORT jstring JNICALL Java_petrsu_smartroom_android_srcli_KP_getContentUrl(
		JNIEnv *env, jclass clazz) {
	individual_t *contentService = getContentService();
	prop_val_t *urlProp = sslog_ss_get_property(contentService,
			PROPERTY_CONTENTURL);

	if(urlProp != NULL) {
		return (*env)->NewStringUTF(env, (char *)urlProp->prop_value);
	}

	return NULL;
}


/**
 * @brief Gets content service individual
 *
 * @return Content service individual in success and NULL otherwise
 */
individual_t* getContentService() {
	list_t *contentList = sslog_ss_get_individual_by_class_all(
			CLASS_CONTENTSERVICE);
	individual_t *contentService;

	if(contentList == NULL) {
		return NULL;
	}

	list_head_t* pos = NULL;
	list_for_each(pos, &contentList->links) {
		list_t* node = list_entry(pos, list_t, links);
		contentService = (individual_t *)(node->data);
		break;
	}

	return contentService;
}


/**
 * TODO
 */
JNIEXPORT jobjectArray JNICALL Java_petrsu_smartroom_android_srcli_KP_getCurrentSectionList(
		JNIEnv *env, jclass clazz) {
	list_t *conferenceList = sslog_ss_get_individual_by_class_all(CLASS_CONFERENCESERVICE);
	individual_t *conference;
	list_t *currentSectionsList;
	list_head_t* pos = NULL;
	const jint maxSections = 10;
	const jint sectionItems = 2; // {section title, section uuid}
	int index = 0;
	jstring titleValue;
	jclass stringObject = (*env)->FindClass(env, "java/lang/String");
	jobjectArray sectionsArray = (*env)->NewObjectArray(env, maxSections * sectionItems,
			stringObject, (*env)->NewStringUTF(env, NULL));

	if(conferenceList == NULL)
		return NULL;

	list_for_each(pos, &conferenceList->links) {
		list_t* node = list_entry(pos, list_t, links);
		conference = (individual_t *)(node->data);
		break; // takes first conference service individual
	}

	currentSectionsList = sslog_get_property_all(conference, PROPERTY_CURRENTSECTION);

	if(currentSectionsList == NULL) {
		__android_log_print(ANDROID_LOG_ERROR, "getCurrentSectionsList()",
				"get_all_props(): %s", sslog_get_error_text());
		return NULL;
	}

	list_for_each(pos, &currentSectionsList->links) {
		list_t* node = list_entry(pos, list_t, links);
		individual_t *section = (individual_t *)(node->data);
		prop_val_t *prop_title = sslog_ss_get_property(section, PROPERTY_SECTIONTITLE);

		if(index >= maxSections * sectionItems)
			break;

		if(prop_title != NULL)
			titleValue = (jstring)(prop_title->prop_value);

		(*env)->SetObjectArrayElement(env, sectionsArray, index,
				(*env)->NewStringUTF(env, titleValue));
		(*env)->SetObjectArrayElement(env, sectionsArray, ++index,
						(*env)->NewStringUTF(env, section->uuid));
		++index;
	}

	return sectionsArray;
}


/**
 * @brief Extracts videos of a user
 *
 * @return List of hasVideo properties
 */
list_t* getVideoList() {

	individual_t *person = NULL;
	prop_val_t *propPerson = sslog_ss_get_property(
			personProfile, PROPERTY_PERSONINFORMATION);

	if(propPerson != NULL) {
		person = (individual_t *) propPerson->prop_value;
	} else {
		return NULL;
	}

	return sslog_ss_get_property_max(person, PROPERTY_HASVIDEO,
			MAX_PROPERTIES);
}


/**
 * @brief Calculates list size
 *
 * @param list - input list
 * @return List size
 */
int getListSize(list_t *list) {

	int size = 0;
	list_t *tmpList = list;
	list_head_t* pos = NULL;

	if(tmpList == NULL)
		return size;

	list_for_each(pos, &tmpList->links) {
		list_t* node = list_entry(pos, list_t, links);
		++size;
	}

	return size;
}


/**
 * @brief Make an array of videos titles of the user
 *
 * @return Array of video titles
 */
JNIEXPORT jobjectArray JNICALL Java_petrsu_smartroom_android_srcli_KP_getVideoTitleList(
		JNIEnv *env, jclass clazz) {

	hasVideoPropList = getVideoList();
	hasVideoPropListLen = getListSize(hasVideoPropList);

	if(hasVideoPropListLen == 0)
		return NULL;

	jclass charSeqClass = (*env)->FindClass(env, "java/lang/CharSequence");
	jobjectArray titleArray = (*env)->NewObjectArray(env, hasVideoPropListLen,
			charSeqClass, NULL);

	list_head_t* pos = NULL;
	int index = 0;

	list_for_each(pos, &hasVideoPropList->links) {
		list_t* node = list_entry(pos, list_t, links);
		prop_val_t *hasVideoProp = (prop_val_t *)(node->data);
		individual_t *hasVideoInd = NULL;

		if(hasVideoProp != NULL) {
			hasVideoInd = (individual_t*) hasVideoProp->prop_value;
		} else {
			continue;
		}

		const prop_val_t *propTitle = sslog_ss_get_property(hasVideoInd,
				PROPERTY_VIDEOTITLE);
		char *charTitle;

		if(propTitle != NULL) {
			charTitle = (char*) propTitle->prop_value;
		} else {
			continue;
		}

		jstring stringTitle = (*env)->NewStringUTF(env, charTitle);

		(*env)->SetObjectArrayElement(env, titleArray, index, stringTitle);

		++index;
	}

	__android_log_print(ANDROID_LOG_ERROR, "getCurrentSectionsList()", "DONE");

	return titleArray;
}

/**
 * @brief Make an array of videos uuids of the user
 *
 * @return Array of video uuids
 */
JNIEXPORT jobjectArray JNICALL Java_petrsu_smartroom_android_srcli_KP_getVideoUuidList(
		JNIEnv *env, jclass clazz) {

	jclass charSeqClass = (*env)->FindClass(env, "java/lang/CharSequence");
	jobjectArray uuidArray = (*env)->NewObjectArray(env, hasVideoPropListLen,
			charSeqClass, NULL);

	list_head_t* pos = NULL;
	int index = 0;

	list_for_each(pos, &hasVideoPropList->links) {
		list_t* node = list_entry(pos, list_t, links);
		prop_val_t *hasVideoProp = (prop_val_t *)(node->data);
		individual_t *hasVideoInd = NULL;

		if(hasVideoProp != NULL) {
			hasVideoInd = (individual_t*) hasVideoProp->prop_value;
		} else {
			continue;
		}

		jstring stringUrl = (*env)->NewStringUTF(env, hasVideoInd->uuid);

		(*env)->SetObjectArrayElement(env, uuidArray, index, stringUrl);

		++index;
	}

	return uuidArray;
}

/**
 * @brief Sends notification to start selected video
 *
 * @param url - video uuid
 * @return o in success and -1 otherwise
 */
JNIEXPORT jint JNICALL Java_petrsu_smartroom_android_srcli_KP_startVideo(
		JNIEnv *env, jclass clazz, jstring url) {

	const char *videoUrl = (*env) -> GetStringUTFChars(env, url, NULL);
	individual_t *individual = sslog_new_individual(
			CLASS_PRESENTATIONNOTIFICATION);

	if(individual == NULL) {
		return -1;
	}

	sslog_set_individual_uuid(individual,
			generateUuid(
					"http://www.cs.karelia.ru/smartroom#PresentationNotification"));

	individual_t *indVideo = sslog_new_individual(CLASS_VIDEO);
	startedVideoUuid = (*env) -> GetStringUTFChars(env, url, NULL);

	sslog_set_individual_uuid(indVideo, startedVideoUuid);

	/* Prepare presentation notification */
	if(sslog_ss_add_property(individual, PROPERTY_STARTVIDEO,
			indVideo) != 0 ) {
		return -1;
	}

	if(sslog_ss_insert_individual(individual) != 0) {
		return -1;
	}

	return 0;
}

/**
 * @brief Sends notification to stop playing video
 */
JNIEXPORT void JNICALL Java_petrsu_smartroom_android_srcli_KP_stopVideo
  (JNIEnv *env, jobject obj) {

	individual_t *individual = sslog_new_individual(
			CLASS_PRESENTATIONNOTIFICATION);
	if(individual == NULL)
		return;

	int ret_val = sslog_set_individual_uuid(individual,
			generateUuid(
					"http://www.cs.karelia.ru/smartroom#PresentationNotification"));

	individual_t *indVideo = sslog_new_individual(CLASS_VIDEO);
	sslog_set_individual_uuid(indVideo, startedVideoUuid);

	if(sslog_ss_add_property(individual, PROPERTY_STOPVIDEO, indVideo) != 0 ) {
		return;
	}

	if(sslog_ss_insert_individual(individual) != 0) {
		return;
	}
}
