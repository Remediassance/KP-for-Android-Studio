/**
 * @file subscription.c
 * @author Aleksandr A. Lomov <lomov@cs.karelia.ru>
 * @date   05 DEcember, 2009
 * @brief  Functions to work with the subscription.
 *
 *
 * @section LICENSE
 *
 * SmartSlog KP Library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * SmartSlog KP Library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with SmartSlog KP Library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA  02110-1301  USA
 *
 *
 * @section DESCRIPTION
 *
 * subscription.c - <b>Functions to work with the subscribtion.</b> <br />
 * This file is part of SmartSlog KP Library.
 *
 * <b>Copyright (C) SmartSlog Team (Aleksandr A. Lomov).
 * All rights reserved.</b> <br />
 * Mail-list: smartslog@cs.karelia.ru
 */

#include <string.h>
#include <stdlib.h>

#ifdef MTENABLE
#include <pthread.h>
#endif

#include "ss_func.h"
#include "classes.h"
#include "structures.h"
#include "kpi_interface.h"
#include "repository.h"

#include "utils/kp_debug.h"
#include "utils/util_func.h"
#include "utils/check_func.h"

#include "subscription_internal.h"
#include "subscription_changes_internal.h"


#if defined(WIN32) || defined (WINCE)
#include <winsock2.h>

void usleep(unsigned int usecs)
{
  unsigned int msecs = usecs / 1000;
  if (msecs < 1)
    Sleep (1);
  else
    Sleep (msecs);
}

#else
#include <unistd.h> /* usleep() */
#endif



/******************************************************************************/
/****************************** Structures list *******************************/
/// @cond INTERNAL_STRUCTURES

/** @brief The state of the tread. */
typedef enum thread_state_s {
    THREAD_UNINITIALIZED = -1, /**< Thread is not created. */
    THREAD_RUNNING = 1, /**< Thread is working. */
    THREAD_ENDED = 0 /**< Thread is ended. */
} thread_state;

/** @brief Contains data for asynchronous subscription thread. */
typedef struct async_sbcr_thread_s {
#ifdef MTENABLE
    pthread_t thread; /**< Thread desctiptor. */
#endif

    int status; /**< State of the thraed. */
} async_sbcr_thread_t;

/// @endcond
/******************************************************************************/



/******************************************************************************/
/************************ Static and global entities **************************/
/// @cond INTERNAL_STRUCTURES

/** @brief List for synchronous subscriptions.  */
static list_t g_sync_subscriptions = {NULL, LIST_HEAD_INIT(g_sync_subscriptions.links)};

//// For asynchronous sybscription
#ifdef MTENABLE
/** @brief List for asyncronous subscriptions.  */
static list_t g_async_subscriptions = {NULL, LIST_HEAD_INIT(g_async_subscriptions.links)};

/** @brief Mutex to manage the work with asynchronous subscription. */
static pthread_mutex_t g_async_subscription_mutex = PTHREAD_MUTEX_INITIALIZER;

/** @brief The flag to stop thread that works with asynchronous subscriptions. */
static bool g_is_async_process_need_to_stopped = false;

/** 
 * @brief The flag to indicate what is need to start working with first
 * asynchronous subscripton.
 *
 * When the asynchronous subscription is removed from the global list,
 * then it needs to continue processing from the first element in the list.
 * Otherwise it is possible, that the asynchronous process will be work
 * with the incorrect node.
 */
static bool g_to_first_async_subscription = false;

/**
 *  @brief Information about asynchronus subscription thread.
 *
 * This thread starts working after first asynchronous subscription will
 * be added to list with asynchronous subscriptions.
 *
 * It works while there are asynchronous subscriptions in the list or
 * while #g_is_async_process_need_to_stopped is false.
 */
static async_sbcr_thread_t async_sbcr_thread_info = {0, -1};
#endif

/// @endcond
/******************************************************************************/



/******************************************************************************/
/*************************** Static functions list ****************************/

static void add_subscription(subscription_t *container);
static void remove_subscription(subscription_t *subscription);
static int process_subscription(subscription_t *subscription, int timeout);
static int wait_unsubscription(subscription_t *subscription,
        int attempts_number);

static void update_subscription(subscription_t *subscription,
        ss_triple_t *old_triples, ss_triple_t *new_triples);
static void update_classes(ss_triple_t *old_triples, ss_triple_t *new_triples,
        subscription_changes_data_t *changes);
static void update_individual(individual_t *individual,
        ss_triple_t *old_triples, ss_triple_t *new_triples,
        subscription_changes_data_t *changes);

static ss_triple_t *classes_to_triples(list_t* classes);
static list_t* convert_triples_to_list_by_filter(ss_triple_t *triples,
        char *subject, char *predicate, char *object);
static action_type next_triples(list_t *new_triples, list_t *old_triples,
        ss_triple_t **new_triple, ss_triple_t **old_triple);

static action_type get_action_type(ss_triple_t *new_triple, ss_triple_t *old_triple);
static individual_t *get_or_create_individual_with_uuid(char *uuid);

static void copy_ss_info(ss_info_t *destination, const ss_info_t *source);

//// For asynchronous sybscription
#ifdef MTENABLE
static void start_async_sbrc_process();
static void stop_async_sbrc_process();
static bool is_async_sbrc_propcess_need_to_stoped();
static void *propcess_async_subscription(void *data);
#endif



/******************************************************************************/
/***************************** External functions *****************************/
/// @cond EXTERNAL_FUNCTIONS

/**
 * @brief Creates a new subscription.
 *
 * The subscriptioon can be synhronous or asynchronous. In the first case
 * you need to call #sbcr_wait to start checking notification from
 * the smart space.
 * In the second case, all operations are made in the background thread.
 *
 * This function sets a global error if it can't perfome operation,
 * you can get an error code using #sslog_get_error_code function.
 *
 * @param[in] is_asynchronous flag of the asynchronous type.
 *
 * @return new created subscription on success or NULL otherwise.
 */
SSLOG_EXTERN subscription_t* sslog_new_subscription(bool is_asynchronous)
{
    reset_error();

    subscription_t *container =
            (subscription_t *) malloc(sizeof (subscription_t));

    if (container == NULL) {
        KPLIB_DEBUG_PRINT("\n%s new_subscription END: out of memory.",                            \
                    KPLIB_DEBUG_SS_SBCR_PREFIX);
        set_error(SSLOG_ERROR_OUT_OF_MEMORY);
        return NULL;
    }

    container->info.socket = 0;
    container->info.id[0] = '\0';

    container->rtti = RTTI_SUBSCRIPTION;

    INIT_LIST_HEAD(&container->sbrc_data.links);
    INIT_LIST_HEAD(&container->sbrc_classes.links);

    container->is_asynchronous = is_asynchronous;
    container->is_active = false;
    container->is_stopped = false;

    container->changed_handler = NULL;
    container->unsubscription_handler = NULL;

    container->last_changes = new_subscription_changes();

    if (container->last_changes == NULL) {
        KPLIB_DEBUG_PRINT("\n%s new_subscription END: out of memory.",                            \
                    KPLIB_DEBUG_SS_SBCR_PREFIX);
        set_error(SSLOG_ERROR_OUT_OF_MEMORY);
        sslog_free_subscription(container);
        return NULL;
    }

    return container;
}

/**
 * @brief Free a subscription.
 *
 * This function does't release active subscriptions.
 *
 * This function sets a global error if it can't perfome operation,
 * you can get an error code using #sslog_get_error_code function.
 *
 * @param[in] subscription subscription container for free.
 */
SSLOG_EXTERN void sslog_free_subscription(subscription_t *subscription)
{
    int error_code = verify_subscription(subscription);

    if (error_code != SSLOG_ERROR_NO) {
        set_error(SSLOG_ERROR_INCORRECT_SUBSCRIPTION);
        return;
    }

    if (sslog_sbcr_is_active(subscription) == true) {
        set_error(SSLOG_ERROR_SUBSCRIPTION_IS_ACTIVE);
        return;
    }

    subscription->rtti = RTTI_MIN_VALUE;

    list_head_t *list_walker = NULL;

    list_for_each(list_walker, &subscription->sbrc_data.links)
    {
        list_t *list = list_entry(list_walker, list_t, links);
        subscription_data_t *sbcr_data = (subscription_data_t *) list->data;

        free_subscription_data(sbcr_data, true);
    }

    //list_free_with_nodes(subscription->sbrc_data, NULL);
    list_del_and_free_nodes(&subscription->sbrc_data, NULL);
    //list_free_with_nodes(subscription->sbrc_classes, NULL);
    list_del_and_free_nodes(&subscription->sbrc_classes, NULL);

    subscription->changed_handler = NULL;
    subscription->unsubscription_handler = NULL;


    free_subscription_changes(subscription->last_changes);
    subscription->last_changes = NULL;

    free(subscription);
}

/**
 * @brief Checks an active state of the subscription.
 *
 * Subscription is active, when it was successfully subscribed.
 * You can't change type of the subscription (synchronous or asynchronous) or
 * add entities to the subcription when it has active state.
 * When you unsubscribe the subscription or from the smart space receives
 * unsubscribe indication, then the subscription changes the state to inactive.
 *
 * This function set global error if it can't perfome operation,
 * you can get a error code using #sslog_get_error_code function.
 *
 * @param[in] subscription subcription to check the state.
 *
 * @return true if the subscription has active state or false otherwise (also if
 * argument is incorrect).
 */
SSLOG_EXTERN bool sslog_sbcr_is_active(subscription_t *subscription)
{
    reset_error();

    int error_code = verify_subscription(subscription);

    if (error_code != SSLOG_ERROR_NO) {
        set_error(error_code);
        return false;
    }

    return subscription->is_active;
}

/**
 * @brief Gets a subscription identifier.
 *
 * When subscription is in active state (it is subscribed), then it has an
 * identifier. While subscription is in inactive state, then the identifier is
 * empty string.<br />
 * <b>Don't free or modify returned string.</b>
 *
 * This function sets a global error if it can't perfome operation,
 * you can get an error code using #sslog_get_error_code function.
 *
 * @param[in] subscription subscription to get the indentifier.
 *
 * @return Subscription identifier or NULL if function can't perfome operation.
 */
SSLOG_EXTERN const char* sslog_sbcr_get_id(subscription_t *subscription)
{
    reset_error();

    int error_code = verify_subscription(subscription);

    if (error_code != SSLOG_ERROR_NO) {
        set_error(error_code);
        return NULL;
    }

    if (subscription->is_active == false) {
        return NULL;
    }

    return subscription->info.id;
}

/**
 * @brief Gets a subscription's last changes.
 *
 * When subscribed data are changed, then information about last changes is
 * updated. You can use this information to track what data have been chenged 
 * after last notification.
 *
 * <b>Be careful using the asynchronous subscription</b>.<br />
 * If you get the changes not in the handler, then it possible what they
 * will be freed if the new notification for current subscription will be
 * received while you are working with them. <br />
 * It's better to make a copy of data or make all needed actions with changes
 * in the handler.
 *
 * This function sets a global error if it can't perfome operation,
 * you can get an error code using #sslog_get_error_code function.
 *
 * @param[in] subscription subscription to get last changes.
 *
 * @return information about last changes on success or NULL otherwise.
 */
SSLOG_EXTERN subscription_changes_data_t *sslog_sbcr_get_changes_last(subscription_t *subscription)
{
    reset_error();

    int error_code = verify_subscription(subscription);

    if (error_code != SSLOG_ERROR_NO) {
        set_error(error_code);

        return NULL;
    }

    return subscription->last_changes;
}

/**
 * @brief Sets a handler that will be called after changing of subscribed data.
 *
 * Subscribed data could be changed after indications, after updating local data
 * this handler will be called. You can get last changes using
 * #sbcr_get_last_changes.
 *
 * You can't change the handler when the subscription is active.
 *
 * The handler is called synchronously.
 * It is important for asynchronous subscription, because the calling of
 * the handler stops checking indications for other subscription.
 *
 * This function sets a global error if it can't perfome operation,
 * you can get an error code using #sslog_get_error_code function.
 *
 * @param[in] subscription subscription to set handler.
 * @param[in] handler pointer to hadler or NULL to reset/disable it.
 */
SSLOG_EXTERN void sslog_sbcr_set_changed_handler(subscription_t *subscription,
        void (*handler)(subscription_t *))
{
    reset_error();

    int error_code = verify_subscription(subscription);

    if (error_code != SSLOG_ERROR_NO) {
        set_error(error_code);
        return;
    }

    if (sslog_sbcr_is_active(subscription) == true) {
        set_error(SSLOG_ERROR_SUBSCRIPTION_IS_ACTIVE);
        return;
    }

    subscription->changed_handler = handler;
}



/**
 * @brief Sets a handler that will be called after changing of subscribed data.
 *
 * Subscribed data could be changed after indications, after updating local data
 * this handler will be called. You can get last changes using
 * #sbcr_get_last_changes.
 *
 * You can't change the handler when the subscription is active.
 *
 * The handler is called synchronously.
 * It is important for asynchronous subscription, because the calling of
 * the handler stops checking indications for other subscription.
 *
 * This function sets a global error if it can't perfome operation,
 * you can get an error code using #sslog_get_error_code function.
 *
 * @param[in] subscription subscription to set handler.
 * @param[in] handler pointer to hadler or NULL to reset/disable it.
 */
SSLOG_EXTERN void sslog_sbcr_set_error_handler(subscription_t *subscription,
        void (*handler)(subscription_t *, int sslog_errno))
{
    reset_error();

    int error_code = verify_subscription(subscription);

    if (error_code != SSLOG_ERROR_NO) {
        set_error(error_code);
        return;
    }

    if (sslog_sbcr_is_active(subscription) == true) {
        set_error(SSLOG_ERROR_SUBSCRIPTION_IS_ACTIVE);
        return;
    }

	subscription->onerror_handler = handler;
}



/**
 * @brief Sets handler that will be called after receiving an unsubscription
 * indication.
 *
 * The smart space can unsubscribe a subscription. It sends the unsubscription
 * indication to an agent, after receiving this indication and processed it this
 * hendler will be called.
 *
 * The state of the subscription changes to inactive after unsubscribing
 * indication and you can subscribe it again.
 *
 * This handler did not called if you unsubscribe the subscription by youself
 * with function #sbcr_unsubscribe.
 *
 * The handler is called synchronously.
 * It is important for asynchronous subscription, because the calling of
 * the handler stops checking indications for other subscription.
 *
 * You can't change the handler when the subscription is active.
 *
 * This function sets a global error if it can't perfome operation,
 * you can get an error code using #sslog_get_error_code function.
 *
 * @param[in] subscription subscription to set handler.
 * @param[in] handler pointer to hadler or NULL to reset/disable it.
 */
SSLOG_EXTERN void sslog_sbcr_set_unsubscription_handler(subscription_t *subscription,
        void (*handler)(subscription_t *))
{
    reset_error();

    int error_code = verify_subscription(subscription);

    if (error_code != SSLOG_ERROR_NO) {
        set_error(error_code);
        return;
    }

    if (sslog_sbcr_is_active(subscription) == true) {
        set_error(SSLOG_ERROR_SUBSCRIPTION_IS_ACTIVE);
        return;
    }

    subscription->unsubscription_handler = handler;
}

/**
 * @brief Adds a individual-properties pair to the subscription.
 *
 * You can subscribe to properties of the individual. To make this subscription
 * you need the individual and a list with properties.
 * You can add many pairs to the one subscription. <br />
 * If you pass NULL instead the properties list, then all
 * properties of the individual's parent class will be used for subscribing.
 *
 * This function sets a global error if it can't perfome operation,
 * you can get an error code using #sslog_get_error_code function.
 *
 * @param[in] subscription subscription to add new data for subscribing.
 * @param[in] individual individual to add.
 * @param[in] properties list of properties to subscribe or NULL for all properties.
 *
 * @return SSLOG_ERROR_NO on success or error code otherwise.
 */
SSLOG_EXTERN int sslog_sbcr_add_individual(subscription_t *subscription,
        individual_t *individual, list_t *properties)
{
    reset_error();

    int error_code = verify_subscription(subscription);

    if (error_code != SSLOG_ERROR_NO) {
        KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_BMED) \
                ("\n%s sbcr_add_individual END: incorrect subscription (errno = %i).",                            \
                KPLIB_DEBUG_SS_SBCR_PREFIX, error_code);
        return set_error(error_code);
    }

    if (sslog_sbcr_is_active(subscription) == true) {
        KPLIB_DEBUG_PRINT \
                ("\n%s sbcr_add_individual END: subscription is active.",                            \
                KPLIB_DEBUG_SS_SBCR_PREFIX);
        return set_error(SSLOG_ERROR_SUBSCRIPTION_IS_ACTIVE);
    }

    error_code = verify_individual(individual);

    if (error_code != SSLOG_ERROR_NO) {
        KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_BMED) \
                ("\n%s sbcr_add_individual END: incorrect individual (errno = %i)",                            \
                KPLIB_DEBUG_SS_SBCR_PREFIX, error_code);
        return set_error(error_code);
    }

    subscription_data_t *data = new_subscription_data(individual, properties);

    if (data == NULL) {
        return set_error(SSLOG_ERROR_OUT_OF_MEMORY);
    }

    list_add_data(data, &subscription->sbrc_data);

    return SSLOG_ERROR_NO;
}

/**
 * @brief Adds a class to the subscription.
 *
 * You can subscribe to the class of individuals. This means that you will get
 * notifications evry time when RDF-triple that describes a type of an individual
 * (uuid - rdf:type - classtype) will be inseted or removed to/from
 * the smart space.
 * New individuals will be created automatically using indication data, but
 * the subscripton doesn't remove individuals when they are removed from the
 * smart space.
 *
 * This function sets a global error if it can't perfome operation,
 * you can get an error code using #sslog_get_error_code function.
 *
 * @param[in] subscription subscription to add new data for subscribing.
 * @param[in] ont_class class to add.
 *
 * @return SSLOG_ERROR_NO on success or error code otherwise.
 */
SSLOG_EXTERN int sslog_sbcr_add_class(subscription_t *subscription, class_t *ont_class)
{
    reset_error();

    int error_code = verify_subscription(subscription);

    if (error_code != SSLOG_ERROR_NO) {
        KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_BMED) \
                ("\n%s sbcr_add_class END: subscription is incorrect (errno = %i).",                                                            \
                KPLIB_DEBUG_SS_SBCR_PREFIX, error_code);
        return set_error(error_code);
    }

    if (sslog_sbcr_is_active(subscription) == true) {
        KPLIB_DEBUG_PRINT \
                ("\n%s sbcr_add_class END: subscription is active.",                            \
                KPLIB_DEBUG_SS_SBCR_PREFIX);
        return set_error(SSLOG_ERROR_SUBSCRIPTION_IS_ACTIVE);
    }

    error_code = verify_class(ont_class);

    if (error_code != SSLOG_ERROR_NO) {
        KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_BMED) \
            ("\n%s sbcr_add_class END: class is incorrect (errno = %i)",                                                            \
            KPLIB_DEBUG_SS_SBCR_PREFIX, error_code);
        return set_error(error_code);
    }

    list_add_data(ont_class, &subscription->sbrc_classes);

    return SSLOG_ERROR_NO;
}

/**
 * @brief Subscribes a subscription.
 *
 * On success, the subscription changes status to active.
 * After this you can't change the subscription:  add new data for subscribing,
 * change a type (synchronous or asynchronous) and etc.
 * Also this function performs a first synchronization:
 * the smart space sends all data that are corresponds to subscribed data.
 * This data is used to set a starting state of the local data to track
 * further changes.
 *
 * To start checking notification from the smart space for
 * the synchronous subscription you need to call the #sbcr_wait function.
 *
 * You can track local data chnages using callback function. This callback will
 * be called evry time then local data is changed.
 * Use #sbcr_set_changed_handler function to set this callback.
 *
 * You can manually unsubscribe the subscription using #sbcr_unsubscribe
 * function, but sometimes the smart space can unsubscribe it. In this case
 * will be called the unsubscription callback.
 * Use #sbcr_set_unsubscription_handler function to set this callback.
 *
 * This function sets a global error if it can't perfome operation,
 * you can get an error code using #sslog_get_error_code function.
 *
 * @param[in] subscription subscription to subscribe.
 *
 * @return SSLOG_ERROR_NO on seccess or error code otherwise.
 */
SSLOG_EXTERN int sslog_sbcr_subscribe(subscription_t *subscription)
{
    int error_code = verify_subscription(subscription);

    if (error_code != SSLOG_ERROR_NO) {
        KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_HIGH)("\n%s sbcr_subscribe END: " \
            "Can't subscribe: invalid subscription (errno %i)",                                \
             KPLIB_DEBUG_SS_SBCR_PREFIX, error_code);
        return set_error(error_code);
    }

    if (sslog_sbcr_is_active(subscription) == true) {
        KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_HIGH)("\n%s sbcr_subscribe END: " \
            "Can't subscribe: subscription is active.",                               \
             KPLIB_DEBUG_SS_SBCR_PREFIX);
        return set_error(SSLOG_ERROR_SUBSCRIPTION_IS_ACTIVE);
    }

#ifndef MTENABLE
    if (subscription->is_asynchronous == true) {
        KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_HIGH)("\n%s sbcr_subscribe END: " \
		  "Asynchronous subscription doesn't support (threads are disabled).",                             \
           KPLIB_DEBUG_SS_SBCR_PREFIX);
        return set_error(SSLOG_ERROR_INCORRECT_SUBSCRIPTION_TYPE);
    }
#endif

    /* save copy of the current ss_info */
    ss_info_t *info = sslog_get_ss_info();
    //ss_info_t *ss_info = (ss_info_t *) malloc(sizeof(ss_info_t));
    //subscription->ss_info = sslog_get_ss_info();
    
    if (info == NULL) {
        KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_HIGH)("\n%s sbcr_subscribe END: " \
            "Can't subscribe: invalid ss info.", KPLIB_DEBUG_SS_SBCR_PREFIX);

        return set_error(SSLOG_ERROR_CANT_SUBSCRIBE);
    }

    copy_ss_info(&subscription->ss_info, info);
    
    ss_triple_t *triples = NULL;
    list_head_t *list_walker = NULL;

    // Transform all individuals and their properties to triples.

    list_for_each(list_walker, &subscription->sbrc_data.links)
    {
        list_t *node = list_entry(list_walker, list_t, links);
        subscription_data_t *sbrc_data = (subscription_data_t *) node->data;

        ss_triple_t *new_triples =
                individual_to_triples_by_properties_any(sbrc_data->ind, sbrc_data->properties);

        triples = concat_triplets(triples, new_triples);
    }

    triples = concat_triplets(triples,
            classes_to_triples(&subscription->sbrc_classes));

    ss_triple_t *subsc_triples = NULL;

    if (ss_subscribe(&subscription->ss_info,
            &subscription->info, triples, &subsc_triples) < 0) {
        ss_delete_triples(triples);
        KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_HIGH)("\n%s sbcr_subscribe END: " \
                "KPI can't subscribe (errno = %i).",                             \
            KPLIB_DEBUG_SS_SBCR_PREFIX, subscription->ss_info.ss_errno);

        return set_error(SSLOG_ERROR_CANT_SUBSCRIBE);
    }

    ss_delete_triples(triples);

    subscription->is_active = true;

    // This step called first synchronization:
    // all triples that were receiveved after subscription are set as new.
    update_subscription(subscription, NULL, subsc_triples);
    ss_delete_triples(subsc_triples);

    // Add container to one of two lists depends from subscription's type.
    add_subscription(subscription);
    if (subscription->is_asynchronous == true) {
        //add_async_subscribe(subscription);
        //start_async_sbrc_process();
    } else {
        //add_sync_subscribe(subscription);
    }

    KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_HIGH)("\n%s sbcr_subscribe END: " \
            "subcription is subscribed.", KPLIB_DEBUG_SS_SBCR_PREFIX);

    return set_error(SSLOG_ERROR_NO);
}

/**
 * @brief Unsubscribes a subscription.
 *
 * The function sends a unsubscribe message to the smart space and
 * waits a unsubscribing confirmation from the smart space.
 * This operation may take some time if a link with the smart space is bad or
 * the smart space doesn't send the confirmation message.
 * If the confirmation message is received or a timeout is ended,
 * then the subscription is considered as unsubscribed and
 * it's status is changed to inactive.
 *
 * It is possible that before the unsubscription message will be received
 * one or more indications, this indications will be handled as usual:
 * with updating local data and with calling handlers.
 *
 * <b>Remember: </b> if you unsubscribe from the subscription,
 * then you need to free it by yourself using #free_subscription
 * if it does not needed.
 *
 * This function sets a global error if it can't perfome operation,
 * you can get an error code using #sslog_get_error_code function.
 *
 * @param[in] subscription subscription to unsubscribe.
 *
 * @return SSLOG_ERROR_NO on seccess or error code otherwise.
 */
SSLOG_EXTERN int sslog_sbcr_unsubscribe(subscription_t *subscription)
{
    KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_LOW) ("\n%s sbcr_unsubscribe START: ", \
        KPLIB_DEBUG_SS_SBCR_PREFIX);

    reset_error();

    int error_code = verify_subscription(subscription);

    if (error_code != SSLOG_ERROR_NO) {
        KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_LOW) ("\n%s sbcr_unsubscribe END: " \
                "incorrect subscription (errno = %i).",                             \
                KPLIB_DEBUG_SS_SBCR_PREFIX, error_code);
        return set_error(error_code);
    }

    if (sslog_sbcr_is_active(subscription) == false) {
        KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_LOW) ("\n%s sbcr_unsubscribe END: " \
                "subscription is not active.",                             \
                KPLIB_DEBUG_SS_SBCR_PREFIX);
        return set_error(SSLOG_ERROR_SUBSCRIPTION_IS_INACTIVE);
    }

    remove_subscription(subscription);

	if (ss_unsubscribe(&subscription->ss_info, &subscription->info) == 0) {
		error_code = wait_unsubscription(subscription, KPLIB_SBCR_UNSUBSCRIBE_ATTEMPTS);
    } else {
		KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_LOW) ("\n\t%s sbcr_unsubscribe: " \
            "kpi operation fails for subscription: %s",                             \
            KPLIB_DEBUG_SS_SBCR_PREFIX, subscription->info.id);

        error_code = SSLOG_ERROR_CANT_UNSUBSCRIBE;
    }

    subscription->is_active = false;

    KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_LOW) ("\n%s sbcr_unsubscribe END: " \
            "unsubscription for %s (errno = %i).",                             \
            KPLIB_DEBUG_SS_SBCR_PREFIX, subscription->info.id, error_code);

    return set_error(error_code);
}

/**
 * @brief Unsubscribes all subscriptions (synchronous and asynchronous).
 *
 * This function is similar to #sbcr_unsubscribe, but works with all
 * subscriptions and can free subscriptions while working.
 *
 * <b>Remembe:</b> if you pass flag to the function as 'true',
 * then you do not need to 'free' subscription by youself.
 * Also do not use this function if you are waiting synchronous subscription
 * with #sbcr_wait function. You need to stop (#sslog_sbcr_stop) synchronous
 * sybscription before calling this function.
 *
 * @param with_free flag to indicate a need to free subscriptions.
 */
SSLOG_EXTERN void sslog_sbcr_unsubscribe_all(bool with_free)
{
#ifdef MTENABLE
	stop_async_sbrc_process();
    while (true) {

        list_t *node = list_get_first_prev_node(&g_async_subscriptions);
        if (node == NULL) {
            break;
        }

        subscription_t *subscription = (subscription_t *) node->data;

        sslog_sbcr_unsubscribe(subscription);

        if (with_free == true) {
            sslog_free_subscription(subscription);
        }
    }
    list_del_and_free_nodes(&g_async_subscriptions,
            (void (*)(void*))sslog_free_subscription);
#endif

    while (true) {
        list_t *node = list_get_first_prev_node(&g_sync_subscriptions);
        if (node == NULL) {
            break;
        }

        subscription_t *subscription = (subscription_t *) node->data;

        sslog_sbcr_unsubscribe(subscription);

        if (with_free == true) {
            sslog_free_subscription(subscription);
        }
    }

    list_del_and_free_nodes(&g_sync_subscriptions,
            (void (*)(void*)) sslog_free_subscription);
}

/**
 * @brief Waits an indication for the subscription from the smart space.
 *
 * This function works only with synchronous subscriptions. It checks
 * is there any notifications from the smart space for the current subscription.
 * If the notification is received then it updates subscribed data and ends.
 *
 * If the smart space sends a unsubscription message, then it changes
 * the subscription state to inactive and ends.
 *
 * With synchronous subscription you also can use callbacks function to
 * track the changing subscribed data and the unsubscription.
 *
 * To stop waiting you can use #sbcr_stop function.
 *
 * <b>Remember:</b> do not use #sbcr_unsubscribe or #sbcr_unsubscribe_all
 * functions while you are waiting. Call the #sbcr_stop function and when
 * the #sbcr_wait ends working, then you can unsubscribe subscription.
 *
 * This function sets a global error if it can't perfome operation,
 * you can get an error code using #sslog_get_error_code function.
 *
 * @param[in] subscription subscription to wait notifications.
 *
 * @return SSLOG_ERROR_NO on seccess (notification, unsubscribe message is received
 * or subscription was stopped with #sslog_sbcr_stop) or error code otherwise.
 */
SSLOG_EXTERN int sslog_sbcr_wait(subscription_t *subscription)
{
    KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_HIGH) ("\n%s sbcr_wait START", KPLIB_DEBUG_SS_SBCR_PREFIX);

    int error_code = verify_subscription(subscription);

    if (error_code != SSLOG_ERROR_NO) {
        KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_LOW) ("\n%s sbcr_wait END: " \
            "incorrect subscription (error code = %i).", KPLIB_DEBUG_SS_SBCR_PREFIX, error_code);
        return set_error(error_code);
    }

    if (subscription->is_asynchronous == true) {
        KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_HIGH) ("\n%s sbcr_wait END: " \
                "incorrect type (asynchronous).", KPLIB_DEBUG_SS_SBCR_PREFIX);
        return set_error(SSLOG_ERROR_INCORRECT_SUBSCRIPTION_TYPE);
    }

    if (sslog_sbcr_is_active(subscription) == false) {
        KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_HIGH) ("\n%s sbcr_wait END: " \
                "subscription is not active.", KPLIB_DEBUG_SS_SBCR_PREFIX);
        return set_error(SSLOG_ERROR_SUBSCRIPTION_IS_INACTIVE);
    }

    subscription->is_stopped = false;

    while (sslog_sbcr_is_active(subscription) == true
            && subscription->is_stopped == false) {

        int status = process_subscription(subscription,
                KPLIB_SBCR_SYNC_WAITING_TIMEOUT);

        if (status == 0) { // Timeout
            continue;
        } else if (status < 0) { // Error
            KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_HIGH) ("\n%s wait_subscribe END: " \
                    "Can't perfome operation.", KPLIB_DEBUG_SS_SBCR_PREFIX);
			if (subscription->onerror_handler != NULL) {
				subscription->onerror_handler(subscription, subscription->ss_info.ss_errno);
			}

            return set_error(SSLOG_ERROR_UNKNOWN);
        } else { // Unsubscription or indication
            break;
        }
    }

    KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_HIGH) (
            "\n%s wait_subscribe END", KPLIB_DEBUG_SS_SBCR_PREFIX);

    return SSLOG_ERROR_NO;
}

/**
 * @brief Indicates what checking of the subscription must be stopped.
 *
 * This function works in pair with #sbcr_wait and works only with synchronous
 * subscriptions.<br />
 * If you want to stop waiting, then you call this function.
 * It only indicates, but not really stops checking, you need to wait while
 * the #sbcr_wait ends, and then make other actions under subscription.
 *
 * This function sets a global error if it can't perfome operation,
 * you can get an error code using #sslog_get_error_code function.
 *
 * @param[in] subscription subscription to stop.
 */
SSLOG_EXTERN void sslog_sbcr_stop(subscription_t *subscription)
{
    reset_error();

    int error_code = verify_subscription(subscription);

    if (error_code != SSLOG_ERROR_NO) {
        set_error(error_code);
        return;
    }

    if (subscription->is_asynchronous == true) {
        set_error(SSLOG_ERROR_INCORRECT_SUBSCRIPTION_TYPE);
        return;
    }

    if (sslog_sbcr_is_active(subscription) == false) {
        set_error(SSLOG_ERROR_SUBSCRIPTION_IS_INACTIVE);
        return;
    }

    subscription->is_stopped = true;
}

/**
 * @brief Indicates what synchronous checking of subscriptions must be stopped.
 *
 * This function works in pair with #sbcr_wait and it is similar with
 * #sbcr_stop but indicates stop checking for all synchrnous subscriptions.
 *
 * If you want to stop waiting, then you call this function.
 * It only indicates, but not really stops checking, you need to wait while
 * the #sbcr_wait ends, and then make other actions under subscriptions.
 *
 * This function sets a global error if it can't perfome operation,
 * you can get an error code using #sslog_get_error_code function.
 */
SSLOG_EXTERN void sslog_sbcr_stop_all()
{
    reset_error();

    list_head_t *list_walker = NULL;

    list_for_each(list_walker, &g_sync_subscriptions.links)
    {
        list_t *node = list_entry(list_walker, list_t, links);

        if (node->data == NULL) {
            continue;
        }

        subscription_t *subscription = (subscription_t *) node->data;
        subscription->is_stopped = true;
    }

}

/// @endcond
/******************************************************************************/



/******************************************************************************/
/***************************** Internal functions *****************************/
/// @cond INTERNAL_FUNCTIONS

/**
 * @brief Create a new data for the subscription.
 *
 * Subscription data contains individual and list with properties
 * or NULL for all individual's properties.
 *
 * @param individual individual.
 * @param properties list with properties for subscribe
 * or NULL for all properties.
 *
 * @return new subscription data on success or NULL otherwise.
 */
subscription_data_t* new_subscription_data(individual_t *individual, list_t *properties)
{
    if (verify_individual(individual) != 0) {
        return NULL;
    }

    subscription_data_t *data =
            (subscription_data_t *) malloc(sizeof (subscription_data_t));

    if (data == NULL) {
        return NULL;
    }

    data->ind = individual;
    data->properties = properties;

    return data;
}

/**
 * @brief Free subscription data.
 *
 * Free subscription data structure, individual and properties are not freed.
 * You can free properties list (only list structure will be freed, not properties),
 * using parameter free_prop_list.
 *
 * @param data subscription data to free.
 * @param free_prop_list if it is 'true' - list structure will be freed.
 */
void free_subscription_data(subscription_data_t *data, bool free_prop_list)
{
    if (data == NULL) {
        return;
    }

    if (free_prop_list == true) {
        list_free_with_nodes(data->properties, NULL);
    }
    data->properties = NULL;

    free(data);
}

/// @endcond
/******************************************************************************/



/******************************************************************************/
/***************************** Static functions *******************************/

/**
 * @brief Adds a subscription to the list.
 *
 * Depending of the subscription type adds the subscription to list and
 * starts asynchronous subscription process if the subscription has
 * the asynchronous type.
 *
 * @param[in] subscription subscription to add.
 */
static void add_subscription(subscription_t *subscription)
{
    if (subscription == NULL) {

        return;
    }

    if (subscription->is_asynchronous == false) {
        list_add_data(subscription, &g_sync_subscriptions);
        return;
    }

#ifdef MTENABLE

    pthread_mutex_lock(&g_async_subscription_mutex);

    list_add_data(subscription, &g_async_subscriptions);

    start_async_sbrc_process();

    pthread_mutex_unlock(&g_async_subscription_mutex);
#endif
}

/**
 * Removes subscription from the list.
 * 
 * If the subscription is asynchronous, then it lock mutex 
 * #g_async_subscription_mutex and remove the subscription. 
 * If the subscription is removed it sets #g_to_first_async_subscription to
 * true.
 *
 * @param[in] subscription subscription to remove.
 */
static void remove_subscription(subscription_t *subscription)
{
    if (subscription == NULL) {
        return;
    }

    // Remove synchronous subscription.
    if (subscription->is_asynchronous == false) {
        list_del_and_free_nodes_with_data(&g_sync_subscriptions, subscription, NULL);
        KPLIB_DEBUG_PRINT("\n%s remove_subscription END: sync list is empty = %i", \
            KPLIB_DEBUG_SS_SBCR_PREFIX, list_is_empty(&g_sync_subscriptions));

        return;
    }


#ifdef MTENABLE // Remove asynchronous subscription.
    pthread_mutex_lock(&g_async_subscription_mutex);

    list_t *del_node = NULL;
    list_head_t *list_walker = NULL;

    // Try to find given subscription and remove it from the asynchronous list.
    // Set a global variable to start processing from the first element if the
    // subscription is removed.

    list_for_each(list_walker, &g_async_subscriptions.links)
    {
        list_t *node = list_entry(list_walker, list_t, links);

        if (node->data == subscription) {
            del_node = node;
        }

        if (del_node != NULL) {
            list_del_and_free_node(del_node, NULL);

            KPLIB_DEBUG_PRINT("\n%s remove_subscription: async list is empty = %i",                                    \
                        KPLIB_DEBUG_SS_SBCR_PREFIX, list_is_empty(&g_async_subscriptions));
            g_to_first_async_subscription = true;

            break;
        }
    }
    pthread_mutex_unlock(&g_async_subscription_mutex);
#endif
}

/**
 * @brief Checks notification for the subscription.
 *
 * The function calls KPI function to check notification from the smart space.
 * The timeout sets maximum time that will be used for waiting notifications.
 *
 * If the notification received then subscribed data will be updated.
 *
 * If unsubscription message received then the current subscription will be
 * removed from the subscriptions list and it state will be changed to inactive.
 * In this case unsubscription callback will be called.
 *
 * @param[in] subscription subscription to process.
 * @param[in] timeout maximum time for waiting notifications.
 *
 * @return 0 - time is over, 1 - indication, 2 - unsubscription message, -1 - error.
 */
static int process_subscription(subscription_t *subscription, int timeout)
{
    if (subscription == NULL) {
        return -1;
    }

    ss_triple_t * n_val = NULL;
    ss_triple_t * o_val = NULL;

    int status = ss_subscribe_indication(&subscription->ss_info,
            &subscription->info, &n_val, &o_val, timeout);

    switch (status) {
        case 1: // Indication
            update_subscription(subscription, o_val, n_val);
            break;

        case 2: // Unsubscription
#ifdef KPI_LOW
                subscription->info.socket = -1;
                subscription->info.id[0] = '\0';
#endif
           remove_subscription(subscription);

            
            subscription->is_active = false;

            KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED)("\n%s process_subscription:" \
                "unsubscription message.",                       \
                KPLIB_DEBUG_SS_SBCR_PREFIX);

            if (subscription->unsubscription_handler != NULL) {
                subscription->unsubscription_handler(subscription);
            }

            break;

        case 0: // Timeout
            break;

        default: // Error: -1
            KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED)("\n%s process_subscription:" \
                "indication error for the subscription '%s'",                       \
                KPLIB_DEBUG_SS_SBCR_PREFIX, subscription->info.id);
            break;
    }

    ss_delete_triples(n_val);
    ss_delete_triples(o_val);

    n_val = NULL;
    n_val = NULL;

    return status;
}

/**
 * @brief Waits unsubscription message from the smart space.
 * 
 * To confirm unsubscription, it is needed to get notification from
 * the smart space. This function try to get this notification.
 * It possible that the indication will be received before
 * the unsubscription message, in this case this notification will be processed
 * as usual.
 *
 * It tries to wait notification only a few times.
 *
 * @param[in] subscription subscription to wait notification.
 * @param[in] attempts_number number to attempts to try get notification.
 *
 * @return SSLOG_ERROR_NO on success or error_code otherwise.
 */
static int wait_unsubscription(subscription_t *subscription,
        int attempts_number)
{
    if (subscription == NULL) {
        return SSLOG_ERROR_INCORRECT_SUBSCRIPTION;
    }

    bool is_unsubscribed = false;

    while (attempts_number > 0) {

        ss_triple_t * n_val = NULL;
        ss_triple_t * o_val = NULL;
				
		int status = ss_subscribe_indication(&subscription->ss_info,
                &subscription->info, &n_val, &o_val,
                KPLIB_SBCR_UNSBCR_WAITING_TIMEOUT);
		
        switch (status) {
            case 1: // Indication
                update_subscription(subscription, o_val, n_val);

                break;
            case 2: // Unsubscribde indication
                attempts_number = -1;
                is_unsubscribed = true;
                break;
            case 0: // Timeout
                break;
            default: // Error
                KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_HIGH) ("\n%s wait_unsubscribe END: " \
                        "indication operation fails for subscription: %s",
                        KPLIB_DEBUG_SS_SBCR_PREFIX, subscription->info.id);
                break;
        }

        ss_delete_triples(n_val);
        ss_delete_triples(o_val);

        n_val = NULL;
        n_val = NULL;

        --attempts_number;
    }

    if (is_unsubscribed == false) {
        KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_HIGH) ("\n%s wait_unsubscribe END: "\
                    "fail for subsciption: %s",
                KPLIB_DEBUG_SS_SBCR_PREFIX, subscription->info.id);

#ifdef KPI_LOW
		// KPI_Low doesn't export close function, we can't use it in the Windows.
#if !(defined(WIN32) || defined (WINCE))
        ss_close(subscription->info.socket);
#endif
        subscription->info.socket = -1;
        subscription->info.id[0] = '\0';
#else                 
        ss_close_subscription(&subscription->info);
#endif        

        return SSLOG_ERROR_CANT_UNSUBSCRIBE;
    }

    KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_HIGH) ("\n%s wait_unsubscribe END: "\
                    "success for subsciption: %s",
            KPLIB_DEBUG_SS_SBCR_PREFIX, subscription->info.id);

    return SSLOG_ERROR_NO;

}

/**
 * @brief Updates a subscription using triples.
 *
 * Updates local classes and then all individuals.
 * Calls changed handler of the subscription.
 *
 * @param[in] subscription subscription to update.
 * @param[in] old_triples triples that were removed from the smart space.
 * @param[in] new_triples triples that were inserted to the smart space.
 */
static void update_subscription(subscription_t *subscription,
        ss_triple_t *old_triples, ss_triple_t *new_triples)
{
    if (subscription == NULL) {
        return;
    }

    KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED)("\n%s update_container START",                   \
            KPLIB_DEBUG_SS_SBCR_PREFIX);

    // Create new changes if it is needed or clean all data and increment
    // the sequence number of the changes.
    if (subscription->last_changes == NULL) {
        subscription->last_changes = new_subscription_changes();
    } else {

        clean_changes_data(subscription->last_changes);
        inc_sequence_number(subscription->last_changes);
    }

    update_classes(old_triples, new_triples, subscription->last_changes);

    list_head_t *list_walker = NULL;

    // Get subscribed data (pairs: individual - properties) and update
    // individuals.

    list_for_each(list_walker, &subscription->sbrc_data.links)
    {
        list_t *node = list_entry(list_walker, list_t, links);
        subscription_data_t *sbcr_data = (subscription_data_t *) node->data;

        if (sbcr_data->ind->uuid == NULL) {
            continue;
        }

        update_individual(sbcr_data->ind, old_triples, new_triples,
                subscription->last_changes);
    }

	// Now the callback is called from propcess_async_subscription
	// It is need to unsubscribe from handler 
   /* if (subscription->changed_handler != NULL) {
        subscription->changed_handler(subscription);
    }*/

    KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED)("\n%s update_container END", \
            KPLIB_DEBUG_SS_SBCR_PREFIX);
}

/**
 * @brief Updates local classes/individuals.
 * 
 * This functions creates new individuals if they are were inserted to
 * the smart space.
 * It doesn't removes individuals if they are were removed from the smart space.
 * 
 * It also update information about changes that were done.
 * 
 * @param[in] old_triples triples that were removed from the smart space.
 * @param[in] new_triples triples that were inserted to the smart space.
 * @param[in] changes information about changes.
 */
static void update_classes(ss_triple_t *old_triples, ss_triple_t *new_triples,
        subscription_changes_data_t *changes)
{
    // Get triples that are used to define typed: <uuid> - rdf:type - class_uri
    list_t *new_triples_list = convert_triples_to_list_by_filter(new_triples,
            NULL, RDF_TYPE, NULL);
    list_t *old_triples_list = convert_triples_to_list_by_filter(old_triples,
            NULL, RDF_TYPE, NULL);

    if (new_triples_list == NULL || old_triples_list == NULL) {
        KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED)("\n%s update_classes END: " \
                 "not enough memory.", KPLIB_DEBUG_SS_SBCR_PREFIX);

        list_free_with_nodes(new_triples_list, NULL);
        list_free_with_nodes(old_triples_list, NULL);

        return;
    }

    list_head_t *list_walker = NULL;
    // Update individuals using triples about inserted(new) individuals.

    list_for_each(list_walker, &new_triples_list->links)
    {
        list_t *node = list_entry(list_walker, list_t, links);
        ss_triple_t *triple = (ss_triple_t *) node->data;

        // Try to find local individual with the same UUID.
        individual_t *ind = (individual_t *)
                sslog_repo_get_individual_by_uuid(triple->subject);

        if (ind != NULL) {
            KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED)("\n%s update_classes END: " \
                 "individual with uuid %s was found in the repository.",                 \
                 KPLIB_DEBUG_SS_SBCR_PREFIX, ind->uuid);
            add_individual_to_changes(changes, ind, ACTION_INSERT);
            continue;
        }

        // Create new individual using information about class from the triple.
        if (ind == NULL) {
            const class_t * ont_class =
                    sslog_repo_get_class_by_classtype(triple->object);
            ind = sslog_new_individual(ont_class);
        }

        // Set UUID from the triple.
        if (ind != NULL) {
            sslog_set_individual_uuid(ind, triple->subject);
            KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED)("\n%s update_classes: " \
                 "new individual with uuid %s was created.",                     \
                KPLIB_DEBUG_SS_SBCR_PREFIX, ind->uuid);
            add_individual_to_changes(changes, ind, ACTION_INSERT);
        } else {

            KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED)("\n%s update_classes: " \
                 "can't create individual with uuid %s and classtype %s.",                    \
                KPLIB_DEBUG_SS_SBCR_PREFIX, triple->subject, triple->object);
        }
    }

    list_walker = NULL;
    // Update infromation about removed individuals:
    // no real actions, local individuals will not be removed or modified.

    list_for_each(list_walker, &old_triples_list->links)
    {
        list_t *node = list_entry(list_walker, list_t, links);
        ss_triple_t *triple = (ss_triple_t *) node->data;

        add_individual_uuid_to_changes(changes, triple->subject, ACTION_REMOVE);
    }

    list_free_with_nodes(new_triples_list, NULL);
    list_free_with_nodes(old_triples_list, NULL);
}

/**
 * @brief Updates an individual.
 *
 * This function try to found triples for current individuals by UUID and
 * updates individual's properties using this triples.
 *
 * It also update information about changes that were done.
 *
 * @param[in] individual individual to update.
 * @param[in] old_triples triples that were removed from the smart space.
 * @param[in] new_triples triples that were inserted to the smart space.
 * @param[in] changes information about changes.
 */
static void update_individual(individual_t *individual,
        ss_triple_t *old_triples, ss_triple_t *new_triples,
        subscription_changes_data_t *changes)
{
    KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED)("\n%s update_individual_by_triples START",                                                           \
            KPLIB_DEBUG_SS_SBCR_PREFIX);

    if (individual == NULL) {
        KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED)("%s update_individual_by_triples END: individual = NULL", KPLIB_DEBUG_SS_SBCR_PREFIX);
        return;
    }

    // Gets triples by individual's UUID.
    list_t *new_triples_list = convert_triples_to_list_by_filter(new_triples, individual->uuid, NULL, NULL);
    list_t *old_triples_list = convert_triples_to_list_by_filter(old_triples, individual->uuid, NULL, NULL);

    while (list_is_null_or_empty(new_triples_list) != 1 || list_is_null_or_empty(old_triples_list) != 1) {

        ss_triple_t *new_triple = NULL;
        ss_triple_t *old_triple = NULL;

        // Get action type for triples (inserted, removed, updated), depends on
        // this set active triples and ork  with them.
        action_type current_action = next_triples(new_triples_list, old_triples_list, &new_triple, &old_triple);
        ss_triple_t *active_triple = (current_action == ACTION_REMOVE) ? old_triple : new_triple;

        // Get property using uri from the smart space.
        char *prop_name = active_triple->predicate;
        property_t *prop_type = (property_t *)
                sslog_get_property_type(individual->parent_class, (const char *) prop_name);

        if (prop_type == NULL) {
            KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED)("%s update_individual_by_triples: no property - %s for individual %s",
                    KPLIB_DEBUG_SS_SBCR_PREFIX, prop_name, individual->uuid);
            continue;
        }

        void *object = NULL;

        // Prepare a value for the property depending of the property type.
        if (prop_type->type == OBJECTPROPERTY) {
            char *uuid = active_triple->object;
            object = get_or_create_individual_with_uuid(uuid);
        } else {
            object = active_triple->object;
        }

        // Change property. Triple was removed.
        if (current_action == ACTION_REMOVE && object != NULL) {

            sslog_remove_property_with_value(individual, prop_type, object);
            add_property_to_changes(changes, individual,
                    prop_type, NULL, object, ACTION_REMOVE);

            // New triple was added.
        } else if (current_action == ACTION_INSERT && object != NULL) {

            sslog_add_property((void *) individual, prop_type, object);
            add_property_to_changes(changes, individual,
                    prop_type, object, NULL, ACTION_INSERT);

        } else if (object != NULL) { // Triple was chanched.
            // To update we need to get an old value of the property.
            void *old_data = old_triples->object;

            if (prop_type->type == OBJECTPROPERTY) {
                old_data = (individual_t *)
                        sslog_repo_get_individual_by_uuid(old_triples->object);
            }

            sslog_update_property_with_value(individual,
                    old_triples->predicate, old_data, (void *) object);
            add_property_to_changes(changes, individual, prop_type,
                    object, old_data, ACTION_UPDATE);
        } else {

            KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED)( \
                "\n%s update_individual_by_triples: cant process property(%s)",                                   \
                KPLIB_DEBUG_SS_SBCR_PREFIX, prop_type->name);
        }
    }

    list_free_with_nodes(new_triples_list, NULL);
    list_free_with_nodes(old_triples_list, NULL);

    KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED)("\n%s update_individual_by_triples END",                                                           \
            KPLIB_DEBUG_SS_SBCR_PREFIX);
}

/**
 * @brief Converts list with classes to triples.
 *
 * This triples are used to subscribe to classes.
 * The result list contains triples: '* - rdf:type - class_uri'
 *
 * @param[in] classes list with classess.
 *
 * @return list with triples.
 */
static ss_triple_t *classes_to_triples(list_t* classes)
{
    if (classes == NULL) {
        return NULL;
    }

    KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_HIGH)("\n%s classes_to_triples START.",                \
            KPLIB_DEBUG_SS_SBCR_PREFIX);

    ss_triple_t *triples = NULL;
    list_head_t *list_walker = NULL;

    list_for_each(list_walker, &classes->links)
    {
        list_t *node = list_entry(list_walker, list_t, links);
        class_t *ont_class = (class_t *) node->data;

        // * - rdf:type - class_uri
        ss_add_triple(&triples,
                SS_RDF_SIB_ANY, RDF_TYPE, ont_class->classtype,
                SS_RDF_TYPE_URI, SS_RDF_TYPE_URI);

        KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_HIGH)("\n\t%s classes_to_triples - " \
            "Triple: %s %s %s", KPLIB_DEBUG_SS_SBCR_PREFIX,                \
            triples->subject, triples->predicate, triples->object);
    }

    KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_HIGH)("\n%s classes_to_triples END.",                \
            KPLIB_DEBUG_SS_SBCR_PREFIX);

    return triples;
}

/**
 * @brief Convers triple to list with triples using filter.
 *
 * Do not free data in the result list,
 * the function doesn't creates new triples.
 *
 * Filter for triples element: subject AND predicate AND object <br>
 * NULL is used as any value.
 *
 * @param[in] triples triples to convert.
 * @param[in] subject filter for the subject of triples.
 * @param[in] predicate filter for the predicate of triples.
 * @param[in] object filter for the object of triples.
 *
 * @return new list with triples on success, or NULL otherwise.
 */
static list_t *convert_triples_to_list_by_filter(ss_triple_t *triples,
        char *subject, char *predicate, char *object)
{
    list_t *triples_list = list_get_new_list();

    while (triples != NULL) {

        if ((subject == NULL || strncmp(triples->subject,
                subject, SS_SUBJECT_MAX_LEN) == 0)

                && (predicate == NULL || strncmp(triples->predicate,
                predicate, SS_PREDICATE_MAX_LEN) == 0)

                && (object == NULL || strncmp(triples->object,
                object, SS_OBJECT_MAX_LEN) == 0)) {

            list_add_data(triples, triples_list);
        }

        triples = triples->next;
    }

    return triples_list;
}

/**
 * @brief Gets nodes and remove them from the lists.
 *
 * @param[in] old_triples list with old triples, that were removed.
 * @param[in] new_triples list with new triples, that were inserted.
 * @param[out] new_triple next new triple.
 * @param[out] old_triple newxt old triple.
 *
 * @return action that was performed (insert, remove, update) under triples.
 */
static action_type next_triples(list_t *new_triples, list_t *old_triples,
        ss_triple_t **new_triple, ss_triple_t **old_triple)
{
    // Get first nodes
    list_t *new_first_triple = list_get_first_prev_node(new_triples);
    list_t *old_first_triple = list_get_first_prev_node(old_triples);

    // Remove one of the node.
    if (new_first_triple != NULL) {

        *new_triple = (ss_triple_t *) new_first_triple->data;
        list_del_and_free_node(new_first_triple, NULL);

    } else if (old_first_triple != NULL) {

        *old_triple = (ss_triple_t *) old_first_triple->data;
        list_del_and_free_node(old_first_triple, NULL);

    }

    // If there are only old triples, then get the action and return it.
    // Old triple was removed from the smart space.
    if (*old_triple != NULL) {
        return get_action_type(NULL, *old_triple);
    }

    list_head_t *list_walker = NULL;
    list_head_t *cur_pos = NULL;

    // If there is new triple, then try to find triple with the
    // same predicate in old triples - triples were updated.

    list_for_each_safe(list_walker, cur_pos, &old_triples->links)
    {
        list_t *node = list_entry(list_walker, list_t, links);
        ss_triple_t *triple = (ss_triple_t *) node->data;

        if (strncmp(triple->predicate, (*new_triple)->predicate, SS_PREDICATE_MAX_LEN) == 0) {
            *old_triple = triple;
            list_del_and_free_node(node, NULL);

            break;
        }
    }

    return get_action_type(*new_triple, *old_triple);
}

/**
 * @brief Determines an action.
 *
 * When triples has come from the smart space we can get a status of operation
 * (insert, delete, update) checking triples: <br />
 * - there are new and old values - triple was updated;  <br />
 * - there is only new value - triple was inserted;  <br />
 * - there is only old value - triple was removed.  
 *
 * @param new_triple triple that was inserted to the smart space.
 * @param old_triple triple that was removed from the smart space.
 *
 * @return return type of the action.
 */
static action_type get_action_type(ss_triple_t *new_triple, ss_triple_t *old_triple)
{
    if (new_triple != NULL && old_triple != NULL) {
        return ACTION_UPDATE;
    } else if (new_triple == NULL && old_triple != NULL) {
        return ACTION_REMOVE;
    } else if (new_triple != NULL && old_triple == NULL) {
        return ACTION_INSERT;
    }

    return ACTION_UNKNOWN;
}

/**
 * @brief Tries to find individual with given UUID or creates new individual.
 *
 * @param uuid UUID of the individual.
 *
 * @return individual (founded or new) on success, or NULL otherwise.
 */
static individual_t *get_or_create_individual_with_uuid(char *uuid)
{
    KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED)("\n%s " \
            "get_or_create_individual_with_uuid START: uuid = %s",      \
            KPLIB_DEBUG_SS_SBCR_PREFIX, uuid);

    individual_t *ind = (individual_t *)
            sslog_repo_get_individual_by_uuid(uuid);

    if (ind == NULL) {

        ss_triple_t * triple = NULL;
        ss_triple_t * triple_req = NULL;
        ss_add_triple(&triple_req, uuid, RDF_TYPE, SS_RDF_SIB_ANY,
                SS_RDF_TYPE_URI, SS_RDF_TYPE_URI);

        // Try to get triple with rdf:type: uuid - rdf:type - *
        if (ss_query(sslog_get_ss_info(), triple_req, &triple) < 0) {
            KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_HIGH)("\n%s " \
                "get_or_create_individual_with_uuid END: kpi_low failed",       \
                KPLIB_DEBUG_SS_SBCR_PREFIX);
            return NULL;
        }

        ss_delete_triples(triple_req);

        // There is no information about type in the smart space.
        if (triple == NULL) {
            KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED)("\n%s " \
                "get_or_create_individual_with_uuid END: no rdf:type",      \
                KPLIB_DEBUG_SS_SBCR_PREFIX);
            return NULL;
        }

        // Get class from repository using class type from samrt space.
        const class_t *ont_class =
                sslog_repo_get_class_by_classtype(triple->object);

        ss_delete_triples(triple);

        if (ont_class == NULL) {
            KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED)("\n%s " \
                "get_or_create_individual_with_uuid END: " \
                "parent class not found", KPLIB_DEBUG_SS_SBCR_PREFIX);

            return NULL;
        }

        ind = sslog_new_individual(ont_class);
        sslog_set_individual_uuid(ind, uuid);
    }

    KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED)("\n%s " \
            "get_or_create_individual_with_uuid END",
            KPLIB_DEBUG_SS_SBCR_PREFIX);

    return ind;
}

static void copy_ss_info( ss_info_t *destination, const ss_info_t *source)
{
    destination->free = source->free;
    destination->transaction_id = source->transaction_id;
    destination->socket = source->socket;

    destination->ssap_msg[0] = '\0';
    destination->ss_errno = 0;
    
    strncpy(destination->node_id, source->node_id, SS_NODE_ID_MAX_LEN);
    strncpy(destination->space_id, source->space_id, SS_SPACE_ID_MAX_LEN);

#ifdef ACCESS_NOTA
    strncpy(destination->address.sid, source->address.sid, MAX_SID_LEN);
#else
    strncpy(destination->address.ip, source->address.ip, MAX_IP_LEN);
#endif

    destination->address.port = source->address.port;
}


#ifdef MTENABLE

/** @brief Starts (if it is needed) asynchronous subscription process. */
static void start_async_sbrc_process()
{
	fflush(stdout);

    if (async_sbcr_thread_info.status == THREAD_RUNNING) {
		g_is_async_process_need_to_stopped = false;
        return;
    }

    if (async_sbcr_thread_info.status == THREAD_ENDED) {
        // Join to relese resources, that were used for the thread.
        pthread_join(async_sbcr_thread_info.thread, NULL);
    }

    // Set flag to get permission for asynchronous subscription to work.
    g_is_async_process_need_to_stopped = false;

    if (pthread_create(&async_sbcr_thread_info.thread, NULL,
            propcess_async_subscription, (void *) &g_async_subscriptions) == 0) {
        async_sbcr_thread_info.status = THREAD_RUNNING;
    } else {
        async_sbcr_thread_info.status = THREAD_UNINITIALIZED;
    }

    KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_HIGH)("\n%s Start async propcess, code = %i",
            KPLIB_DEBUG_SS_SBCR_PREFIX, async_sbcr_thread_info.status );
}

/** @brief Stops asynchronous subscription process. */
static void stop_async_sbrc_process()
{
    if (async_sbcr_thread_info.status == THREAD_UNINITIALIZED) {
        return;
    }

    // Set flag to stop asynchronous function.
    g_is_async_process_need_to_stopped = true;

    // Wait while asynchronous subscription working (only from other threads).
	if (pthread_equal(pthread_self(), async_sbcr_thread_info.thread) == 0) {
		pthread_join(async_sbcr_thread_info.thread, NULL);
		async_sbcr_thread_info.status = THREAD_UNINITIALIZED;
	}

    KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_HIGH)("\n%s Async propcess stopped", \
            KPLIB_DEBUG_SS_SBCR_PREFIX);
}

/**
 * @brief Checks: asynchronous subscription process
 * need to be stopped or not.
 *
 * @return true if process need to be stopped.
 */
static bool is_async_sbrc_propcess_need_to_stoped()
{
    return g_is_async_process_need_to_stopped;
}

/** @brief Process asynchronous subscriptions..
 *
 * @param data information for process.
 *
 * @return NULL, it is for pthread.
 */
static void *propcess_async_subscription(void *data)
{
    KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_HIGH)("\n%s Subscribe process START.",
            KPLIB_DEBUG_SS_SBCR_PREFIX);

    list_t *subscriptions = (list_t*) data;
    list_t *list_walker = NULL;

    // Checking flag, sometimes it usefull to stop subscription...
    while (true) {
        pthread_mutex_lock(&g_async_subscription_mutex);

        if (is_async_sbrc_propcess_need_to_stoped() == true) {
            async_sbcr_thread_info.status = 0;
            pthread_mutex_unlock(&g_async_subscription_mutex);
            break;
        }

        if (g_to_first_async_subscription == true) {
            g_to_first_async_subscription = false;
            list_walker = NULL;
        }

        subscription_t *sbcr = NULL;

        // Get next subscription from the global list.
        list_walker = list_get_next_node(subscriptions, list_walker,
                (void **) &sbcr);

        // When there are no containers: break
        if (list_walker == NULL) {
            async_sbcr_thread_info.status = 0;
            pthread_mutex_unlock(&g_async_subscription_mutex);
            break;
        }

        int status = process_subscription(sbcr, KPLIB_SBCR_ASYNC_WAITING_TIMEOUT);

        pthread_mutex_unlock(&g_async_subscription_mutex);

		// Error
		if (status == -1 && sbcr->onerror_handler != NULL) {
			sbcr->onerror_handler(sbcr, sslog_get_error_code());
		} else if (status == 1) { // Indication
			if (sbcr->changed_handler != NULL) {
				sbcr->changed_handler(sbcr);
			}
		}

        usleep(KPLIB_SBCR_ASYNC_PROCESS_SLEEP);
    }

    KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_HIGH)("\n%s Subscribe process END.",
            KPLIB_DEBUG_SS_SBCR_PREFIX);

	pthread_exit(NULL);

	return NULL;
}

#endif











