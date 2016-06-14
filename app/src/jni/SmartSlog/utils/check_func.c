/**
 * @file check_func.c
 * @Author Alaxandr A. Lomov <lomov@cs.karelia.ru>
 * @date   11 January, 2010
 * @brief  Functions for different checks.
 *
 *
 * @section LICENSE
 *
 * PetrSU KP Library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * PetrSU KP Library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with PetrSU KP Library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA  02110-1301  USA
 *
 *
 * @section DESCRIPTION
 *
 * Contains functions for different checks.
 * This file is part of SmartSlog KP Library.
 *
 * Copyright (C) 2009 - SmartSlog. All rights reserved.
 *
 */

#include "check_func.h"


/******************************************************************************/
/***************************** External functions *****************************/
/// @cond EXTERNAL_FUNCTIONS

/**
 * @brief Checks string to null or empty value.
 *
 * @param string string for check.
 *
 * @return true if string is not null or empty, false otherwise.
 */
SSLOG_EXTERN bool is_str_null_or_empty(const char* string)
{
    if (string == NULL) {
        return true;
    }

    if (*string == '\0') {
        return true;
    }

    return false;
}
/// @endcond



/******************************************************************************/
/***************************** INTERNAL functions *****************************/
/// @cond INTERNAL_FUNCTIONS

/**
 * @brief Checks class structure.
 *
 * Checks: class struct not equals NULL; RTTI equals class; classtype set;
 *
 * @param ont_class class for check.
 *
 * @return SSLOG_ERROR_NO if checks not fails or error code otherwise.
 */
int verify_class(const class_t *ont_class)
{
	KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED) \
        ("\n%s verify_class START\n", KPLIB_DEBUG_CHECKS_PREFIX);

	int error_code = SSLOG_ERROR_NO;

    if (ont_class == NULL) {
        error_code = SSLOG_ERROR_INCORRECT_CLASS;
    } else if (ont_class->rtti != RTTI_CLASS) {
        error_code = SSLOG_ERROR_INCORRECT_CLASS;
    } else if (is_str_null_or_empty(ont_class->classtype) == 1) {
        error_code = SSLOG_ERROR_INCORRECT_CLASSTYPE;
    }

    KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_LOW)
		("\n%s verify_class END: error_code = %i\n",  KPLIB_DEBUG_CHECKS_PREFIX, error_code);

    return error_code;
}

/**
 * @brief Checks individual structure.
 *
 * Checks: individual structure not equals NULL; RTTI equals individual;
 * class set; class type; classtype set and equals parent class type;
 *
 * @param individual individual for check.
 *
 * @return SSLOG_ERROR_NO if checks not fails or error code otherwise.
 */
int verify_individual(const individual_t *individual)
{
	KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED) \
        ("\n%s verify_individual START\n", KPLIB_DEBUG_CHECKS_PREFIX);

	if (individual == NULL) {
        KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED) ("\n%s verify_individual END: " \
                    "individual is null\n", KPLIB_DEBUG_CHECKS_PREFIX);
        return SSLOG_ERROR_INCORRECT_INDIVIDUAL;
    }

    int error_code = SSLOG_ERROR_NO;

    if (individual->rtti != RTTI_INDIVIDUAL) {
        error_code = SSLOG_ERROR_INCORRECT_RTTI;
    } else if (is_str_null_or_empty(individual->classtype) == 1) {
        error_code = SSLOG_ERROR_INCORRECT_CLASSTYPE;
    } else if (individual->parent_class == NULL) {
        error_code = SSLOG_ERROR_INCORRECT_INDIVIDUAL_PARENTCLASS;
    } else if (is_str_null_or_empty(individual->parent_class->classtype) == 1) {
        error_code = SSLOG_ERROR_INCORRECT_INDIVIDUAL_PARENTCLASS;
    } else if (strcmp(individual->parent_class->classtype,
            individual->classtype) != 0) {
        error_code = SSLOG_ERROR_INCORRECT_INDIVIDUAL_PARENTCLASS;
        //    } else if (is_str_null_or_empty(individual->uuid) == true) {
        //        error_code = SSLOG_ERROR_INCORRECT_UUID;
    }

    KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED) ("\n%s verify_individual END: " \
            "UUID = %s, error_code = %i\n",        \
            KPLIB_DEBUG_CHECKS_PREFIX, individual->uuid, error_code);

    return error_code;
}

/**
 * @brief Checks entity(class, individual, property) structure.
 *
 * @param entity one of structures: class, individual, property.
 *
 * @return SSLOG_ERROR_NO if checks not fails or error code otherwise.
 */
int verify_entity(const void *entity)
{
	KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED) \
        ("\n%s verify_entity START\n", KPLIB_DEBUG_CHECKS_PREFIX);

	if (entity == NULL) {
        KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED) ("\n%s verify_entity END: " \
                    "entity is null\n", KPLIB_DEBUG_CHECKS_PREFIX);
        return SSLOG_ERROR_INCORRECT_ENTITY;
    }

    int rtti_type = sslog_get_rtti_type(entity);

    if ((rtti_type >= RTTI_MAX_VALUE) || (rtti_type <= RTTI_MIN_VALUE)) {
        KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED) ("\n%s verify_entity END: " \
                    "unknown RTTI\n", KPLIB_DEBUG_CHECKS_PREFIX);
        return SSLOG_ERROR_INCORRECT_ENTITY;
    }

    int error_code = SSLOG_ERROR_NO;
    switch (rtti_type) {
        case RTTI_CLASS:
        {
            error_code = verify_class((class_t *) entity);
            break;
        }
        case RTTI_INDIVIDUAL:
        {
            error_code = verify_individual((individual_t *) entity);
            break;
        }
        case RTTI_PROPERTY:
        {
            error_code = verify_property((property_t *) entity);
            break;
        }
        case RTTI_SUBSCRIPTION:
        {
            error_code = verify_subscription((subscription_t *) entity);
        }
    }

	KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED)
		("\n%s verify_individual END: error_code = %i\n", KPLIB_DEBUG_CHECKS_PREFIX, error_code);

	return error_code;
}

/**
 * @brief Checks property structure.
 *
 * Checks: class struct not equals NULL, RTTI, property name, type and about field.
 *
 * @param prop property for check.
 *
 * @return 0 if checks not fails or error code otherwise.
 */
int verify_property(const property_t *property)
{
	KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED) \
        ("\n%s verify_property START\n", KPLIB_DEBUG_CHECKS_PREFIX);

	if (property == NULL) {
        KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED) ("\n%s verify_property END: " \
            "property is null\n", KPLIB_DEBUG_CHECKS_PREFIX);
        return SSLOG_ERROR_INCORRECT_PROPERTY;
    }

    if (sslog_get_rtti_type(property) != RTTI_PROPERTY) {
        KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED) ("\n%s verify_property END: " \
            "incorrect RTTI\n", KPLIB_DEBUG_CHECKS_PREFIX);
        return SSLOG_ERROR_INCORRECT_RTTI;
    }

    int error_code = SSLOG_ERROR_NO;

    if (is_str_null_or_empty(property->name) == true) {
        error_code = SSLOG_ERROR_INCORRECT_PROPERTY_NAME;
    } else if (is_str_null_or_empty(property->about) == true) {
        error_code = SSLOG_ERROR_INCORRECT_PROPERTY_ABOUT;
    } else if (property->type != OBJECTPROPERTY
            && property->type != DATATYPEPROPERTY) {
        error_code = SSLOG_ERROR_INCORRECT_PROPERTY_TYPE;
    }

    KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED) ("\n%s verify_property END: " \
            "About = %s, error_code = %i\n",        \
            KPLIB_DEBUG_CHECKS_PREFIX, property->name, error_code);

    return error_code;
}


//INFO: No checks for container data.

/**
 * @brief Checks subscription container structure.
 *
 * Checks: class struct not equals NULL, RTTI and list of subscription data.
 *
 * @param container subscription container for check.
 *
 * @return 0 if checks not fails or error code otherwise.
 */
int verify_subscription(const subscription_t *subscription)
{
	KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED) \
        ("\n%s verify_subscription START\n", KPLIB_DEBUG_CHECKS_PREFIX);

	int error_code = SSLOG_ERROR_NO;

    if (subscription == NULL) {
        error_code = SSLOG_ERROR_NULL_ARGUMENT;
    }
	else if (sslog_get_rtti_type(subscription) != RTTI_SUBSCRIPTION) {
        error_code = SSLOG_ERROR_INCORRECT_SUBSCRIPTION;
    }

	KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED)
		("\n%s verify_subscription END: error_code = %i\n", KPLIB_DEBUG_CHECKS_PREFIX, error_code);

    return error_code;
}

/**
 * @brief Checks individual and property list.
 *
 * Checks: individual and try to find properties for given individual.
 *
 * @param individual individual for check.
 * @param prop_list properties list for chek for given individual.
 *
 * @return 0 if checks not fails or error code otherwise.
 */
int verify_individual_prop_list(const individual_t *individual, list_t *prop_list)
{
	KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED) \
        ("\n%s verify_individual_prop_list START\n", KPLIB_DEBUG_CHECKS_PREFIX);

	int error_code = verify_individual(individual);

    if (error_code != SSLOG_ERROR_NO) {
        KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED)
			("\n%s verify_individual_prop_list END: invalid individual\n", KPLIB_DEBUG_CHECKS_PREFIX);
        return error_code;
    }

    list_head_t *list_walker = NULL;

    list_for_each(list_walker, &prop_list->links)
    {
        list_t *node = list_entry(list_walker, list_t, links);
        property_t *test_prop = (property_t *) node->data;

        if (verify_property(test_prop) != SSLOG_ERROR_NO) {
            error_code = SSLOG_ERROR_INCORRECT_PROPLIST;
			break;
        }

        if (sslog_get_property_type(individual->parent_class, test_prop->name) == NULL) {
            error_code = SSLOG_ERROR_INCORRECT_PROPLIST;
			break;
        }

    }

	KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED)
		("\n%s verify_individual_prop_list END: error_code = %i\n", KPLIB_DEBUG_CHECKS_PREFIX, error_code);

    return error_code;
}

/**
 * @brief Checks pattern structure.
 *
 * Checks: pattern structure not equals NULL, RTTI.
 *
 * @param pattern pattern to check.
 *
 * @return SSLOG_ERROR_NO if checks not fails or error code otherwise.
 */
int verify_pattern(const pattern_t *pattern)
{
    KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED)
		("\n%s verify_pattern START\n", KPLIB_DEBUG_CHECKS_PREFIX);

	int error_code = SSLOG_ERROR_NO;

    if (pattern == NULL) {
        error_code = SSLOG_ERROR_INCORRECT_PATTERN;
    } else if (pattern->rtti != RTTI_PATTERN) {
        error_code = SSLOG_ERROR_INCORRECT_PATTERN;
    }

    KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED)
		("\n%s verify_pattern END: error_code = %i\n", KPLIB_DEBUG_CHECKS_PREFIX, error_code);

    return error_code;
}

/**
 * @brief Checks subscription container structure.
 *
 * Checks: class struct not equals NULL, RTTI and list of subscription data.
 *
 * @param container subscription container for check.
 *
 * @return 0 if checks not fails or error code otherwise.
 */
int verify_subscription_changes(const subscription_changes_data_t *changes)
{
    KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED)
		("\n%s verify_subscription_changes START\n", KPLIB_DEBUG_CHECKS_PREFIX);

	int error_code = SSLOG_ERROR_NO;

	if (changes == NULL) {
        error_code = SSLOG_ERROR_NULL_ARGUMENT;
    }
	else if (sslog_get_rtti_type(changes) != RTTI_SUBSCRIPTION_CHANGES) {
        error_code = SSLOG_ERROR_INCORRECT_ENTITY;
    }

	KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED)
		("\n%s verify_subscription_changes END: error_code = %i\n", KPLIB_DEBUG_CHECKS_PREFIX, error_code);

    return error_code;
}


/// @endcond
