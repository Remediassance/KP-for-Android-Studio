/**
 * @file subscription_internal.h
 * @author Aleksandr A. Lomov <lomov@cs.karelia.ru>
 * @date   19 April, 2012
 * @brief Internal part of the interface for subscription.
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
 * subscription_internal.h - 
 * <b>Internal part of the interface for subscription.</b>
 *
 * <b>Copyright (C) SmartSlog Team (Aleksandr A. Lomov).
 * All rights reserved.</b> <br />
 * Mail-list: smartslog@cs.karelia.ru
 */


#ifndef SS_SUBSCRIPTION_INTERNAL_H
#define	SS_SUBSCRIPTION_INTERNAL_H


#include "structures.h"
#include "subscription.h"


#ifdef	__cplusplus
extern "C" {
#endif


/**
 * @brief Count of attemts to try receiving unsubscription notification.
 */
#define KPLIB_SBCR_UNSUBSCRIBE_ATTEMPTS 10

/** @defgroup SubscriptionTimeouts Timeouts that are used for the subscription.
 *
 * All timeouts in milliseconds.
 */
/*@{*/
/**
 * @brief Timeout to check notification for asynchronous subscriptions.
 */
#define KPLIB_SBCR_ASYNC_WAITING_TIMEOUT 100

/**
 * @brief Timeout to check notification for synchronous subscriptions.
 */
#define KPLIB_SBCR_SYNC_WAITING_TIMEOUT  1000

/**
 * @brief Timeout to wait unsubscription notification.
 */
#define KPLIB_SBCR_UNSBCR_WAITING_TIMEOUT  500

 /**
  * Timeout between checking asynchronous subscriptions.
  */
#define KPLIB_SBCR_ASYNC_PROCESS_SLEEP 100 
/*@}*/

/******************************************************************************/
/****************************** Structures list *******************************/

/// @cond INTERNAL_STRUCTURES

/**
 * @brief  Subscription data for subscription container
 * 
 * @see subscription_s.
 */
typedef struct subscription_data_s {
    individual_t *ind;  /**< individual for subscribe. */
    list_t *properties; /**< individual properties for subscribe. */
} subscription_data_t;


/**
 * @brief  Contains data for the subscription.
 *
 * @see subscription_t
 */
struct subscription_s {
    int rtti;             /**< Run-time type information. */

    /**
     * Subscription data list.
     * It is pairs of 'individual - properties' (#subscription_data_s).
     */
    list_t sbrc_data;
    list_t sbrc_classes;  /**< Classes that are included to the subscription. */

    bool is_active;       /**< Flag: active means subscribed. */
    bool is_asynchronous; /**< Flag: subscription synchronous or asynchrnous. */
    bool is_stopped;      /**< Flag; use to stop synchronous subcription waiting. */

    ss_subs_info_t info;  /**< KPI information about subscription. */
    ss_info_t ss_info;   /**< KPI information about SIB to support multiSIB. */

    /**
     * Information about last changes (#subscription_changes_data_t)
     */
    subscription_changes_data_t *last_changes; 

    /**
     * Callback function, it is called after subscription data changes.
     */
    void (*changed_handler)(subscription_t *sender);

    /**
     * Callback function, it is called then the subscription is unsubscribed.
     */
    void (*unsubscription_handler)(subscription_t *sender);
    
    /**
     * Callback function, it is called when an error occurred 
     * while checking indication.
     */
    void (*onerror_handler)(subscription_t *sender, int sslog_errno);
};

/// @endcond



/******************************************************************************/
/****************************** Functions list ********************************/
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
subscription_data_t* new_subscription_data(individual_t *individual,
        list_t *properties);

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
void free_subscription_data(subscription_data_t *data, bool free_prop_list);

/// @endcond


#ifdef	__cplusplus
}
#endif

#endif	/* SS_SUBSCRIPTION_INTERNAL_H */

