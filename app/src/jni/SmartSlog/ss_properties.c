/**
* @file ss_properties.c
* @Author Vanag Pavel <vanag@cs.karelia.ru>
* @date   05 April, 2010
* @brief  Contains functions for work with properties throw SS.
* @version 0.2alpha
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
* @section DESCRIPTION
*
* Contains functions for work with properties throw SS.
* This file is part of SmartSlog KP library.
*
*/

#include "ss_properties.h"
#include "repository.h"

#ifndef _SS_PROPERTIES_C_
#define _SS_PROPERTIES_C_

/** @cond EXTERNAL_FUNCTIONS */

/**
* @fn int sslog_ss_add_property(void *entity, property_t *prop, void *data)
*
* @brief Sets property for entity in SS and localy by name.
*
* Checks either entity is individual or class
* Checks entity correctness: cardinality, property values limitation
* and sets property for entity in SS and localy.
* Function sets global SmartSlog KP Library's errno.
*
* @param[in] void *entity. Pointer to the individual struct.
* @param[in] char *propname. Pointer to the char array with name.
* @param[in] void *data. Pointer to the value.
* @return int. Status of the operation when completed (0 if successfull, otherwise -1).
*/
SSLOG_EXTERN int sslog_ss_add_property(void *entity, property_t *prop, void *data)
{
    KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED)
		("\n%s sslog_ss_add_property START\n", KPLIB_DEBUG_SS_PROP_PREFIX);

	int status;
	int error_code = 0;

	status = sslog_add_property(entity, prop, data);
	if (status < 0) {
		KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED)
			("\n%s sslog_ss_add_property END: ss_query returns NULL\n", KPLIB_DEBUG_SS_PROP_PREFIX);
		return status;
	}

	switch (sslog_get_rtti_type(entity)) {
		case RTTI_CLASS: {
			error_code = ss_add_property_for_class((class_t *) entity, prop->name, data);
			break;
		}
		case RTTI_INDIVIDUAL: {
			error_code = ss_add_property_for_individual((individual_t *) entity, prop->name, data);
			break;
		}
		default: {
			error_code = -1;
		}
	}

	if (error_code < 0) {
		KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED)
			("\n%s sslog_ss_add_property END: error occurs\n", KPLIB_DEBUG_SS_PROP_PREFIX);
	}
	else {
		KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED)
			("\n%s sslog_ss_add_property END: property is added to SS\n", KPLIB_DEBUG_SS_PROP_PREFIX);

	}

	return error_code;
}

/**
* @fn int sslog_ss_set_property_with_value_struct(void *entity, prop_val_t *prop_val)
*
* @brief Sets property for entity in SS and localy by name.
*
* Checks either entity is individual or class
* Checks entity correctness: cardinality, property values limitation
* and sets property for entity in SS and localy.
* Function sets global SmartSlog KP Library's errno.
*
* @param[in] void *entity. Pointer to the individual struct.
* @param[in] prop_val_t *prop_val. Pointer to the value struct.
* @return int. Status of the operation when completed (0 if successfull, otherwise -1).
*/
SSLOG_EXTERN int sslog_ss_set_property_with_value_struct(void *entity, prop_val_t *prop_val)
{
	return sslog_ss_add_property(entity, prop_val->property, prop_val->prop_value);
}

/**
* @fn prop_val_t* sslog_ss_get_property(individual_t *ind, property_t *prop)
*
* @brief Gets property for individual from SS by name.
*
* Gets the property from the SS by name and returns it.
* Function sets global SmartSlog KP Library's errno.
*
* @param[in] individual_t * ind. Pointer to the individual struct.
* @param[in] property_t *prop. Pointer to the char array with name.
* @return prop_val_t*. NULL if properties with given name doesn't exists,
*                      pointer to value struct otherwise.
*/
SSLOG_EXTERN prop_val_t* sslog_ss_get_property(individual_t *ind, property_t *prop)
{
    KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED)
		("\n%s sslog_ss_get_property START\n", KPLIB_DEBUG_SS_PROP_PREFIX);

	ss_triple_t * request = NULL;
	ss_triple_t * first_triple = NULL;
	void * data = NULL;
	char * uuid = NULL;

	int error_code = verify_individual(ind);

	if (error_code != SSLOG_ERROR_NO) {
		set_error(error_code);
		KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED)
			("\n%s sslog_ss_get_property END: error while verify_individual\n", KPLIB_DEBUG_SS_PROP_PREFIX);
		return NULL;
	}

	if (prop == NULL || prop->name == NULL) {
		set_error(SSLOG_ERROR_INCORRECT_PROPERTY_NAME);
		KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED)
			("\n%s sslog_ss_get_property END: error while checking second parameter property_t\n", KPLIB_DEBUG_SS_PROP_PREFIX);
		return NULL;
	}

	ss_add_triple(&request, ind->uuid, prop->name, SS_RDF_SIB_ANY, SS_RDF_TYPE_URI, SS_RDF_TYPE_URI);
	if(ss_query(sslog_get_ss_info(), request, &first_triple) < 0) {
		set_error(SSLOG_ERROR_UNKNOWN);
		ss_delete_triples(request);
		KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED)
			("\n%s sslog_ss_get_property END: error while ss_query\n", KPLIB_DEBUG_SS_PROP_PREFIX);
		return NULL;
	}
	ss_delete_triples(request);
	if (first_triple == NULL) {
		KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED)
			("\n%s sslog_ss_get_property END: ss_query returns NULL\n", KPLIB_DEBUG_SS_PROP_PREFIX);
		return NULL;
	}

	if (prop->type == OBJECTPROPERTY) {
		uuid = (char *) first_triple->object;
		const individual_t *ind_tmp = sslog_repo_get_individual_by_uuid(uuid);
		if (ind_tmp == NULL)
		{

			ss_triple_t *request_temp = NULL;
			ss_triple_t *first_triple_temp = NULL;

			ss_add_triple(&request_temp, uuid, RDF_TYPE, SS_RDF_SIB_ANY, SS_RDF_TYPE_URI, SS_RDF_TYPE_URI);

			if(ss_query(sslog_get_ss_info(), request_temp, &first_triple_temp) < 0) {
				set_error(SSLOG_ERROR_UNKNOWN);
				ss_delete_triples(request_temp);
				KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED)
					("\n%s sslog_ss_get_property END: error while ss_query for OBJECTPROPERTY\n", KPLIB_DEBUG_SS_PROP_PREFIX);
				return NULL;
			}
			ss_delete_triples(request_temp);
			if (first_triple_temp == NULL) {
				set_error(SSLOG_ERROR_UNKNOWN);
				KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED)
					("\n%s sslog_ss_get_property END: ss_query for OBJECTPROPERTY returns NULL\n", KPLIB_DEBUG_SS_PROP_PREFIX);
				return NULL;
			}

			const class_t* ont_class = sslog_repo_get_class_by_classtype(first_triple_temp->object);
			individual_t *ind_tmp2 = sslog_new_individual(ont_class);
			if (ind_tmp2 != NULL)
			{
                sslog_set_individual_uuid(ind_tmp2, uuid);
                //ind_tmp2->uuid = uuid;
				data = (void *) ind_tmp2;
			}
			else data = NULL;
			ss_delete_triples(first_triple_temp);
		}
		else data = (void *) ind_tmp;
	}
	else if (prop->type == DATATYPEPROPERTY) {
		data = (void *) strdup(first_triple->object);
	}
	ss_delete_triples(first_triple);

	prop_val_t* value = sslog_new_value_struct(prop, data);

	KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED)
		("\n%s sslog_ss_get_property END: property value is ready\n", KPLIB_DEBUG_SS_PROP_PREFIX);

	return value;
}

/**
* \fn list_t* sslog_ss_get_property_max(individual_t *ind, property_t *prop, int max)
*
* \brief Gets list of properties for individual from SS by name and value.
*
* Gets the property from the SS by name and value and returns list of properties.
* Returns all founded but not more then max.
* Function sets global SmartSlog KP Library's errno.
*
* \param[in] individual_t * ind. Pointer to the individual struct.
* \param[in] property_t *prop. Pointer to the char array with name.
* \param[in] int max. Maximum returns properties.
* \return list_t*. NULL if properties with given name doesn't exists,
*                      pointer to list of value structs otherwise.
*/
SSLOG_EXTERN list_t* sslog_ss_get_property_max(individual_t *ind, property_t *prop, int max)
{
    KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED)
		("\n%s sslog_ss_get_property_max START\n", KPLIB_DEBUG_SS_PROP_PREFIX);

	ss_triple_t * request = NULL;
	ss_triple_t * first_triple = NULL;
	ss_triple_t * triple = NULL;
	list_t *properties;
	void * data = NULL;
	int i;
	char *uuid;

	int error_code = verify_individual(ind);

	if (error_code != SSLOG_ERROR_NO) {
		set_error(error_code);
		KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED)
			("\n%s sslog_ss_get_property_max END: error while verify_individual\n", KPLIB_DEBUG_SS_PROP_PREFIX);
		return NULL;
	}

	if (prop == NULL || prop->name == NULL) {
		set_error(SSLOG_ERROR_INCORRECT_PROPERTY_NAME);
		KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED)
			("\n%s sslog_ss_get_property_max END: error while checking second parameter property_t\n", KPLIB_DEBUG_SS_PROP_PREFIX);
		return NULL;
	}


	ss_add_triple(&request, ind->uuid, prop->name, SS_RDF_SIB_ANY, SS_RDF_TYPE_URI, SS_RDF_TYPE_URI);
	if(ss_query(sslog_get_ss_info(), request, &first_triple) < 0) {
		set_error(SSLOG_ERROR_UNKNOWN);
		ss_delete_triples(request);
		KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED)
			("\n%s sslog_ss_get_property_max END: error while ss_query\n", KPLIB_DEBUG_SS_PROP_PREFIX);
		return NULL;
	}
	ss_delete_triples(request);
	if (first_triple == NULL) {
		KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED)
			("\n%s sslog_ss_get_property_max END: ss_query returns NULL\n", KPLIB_DEBUG_SS_PROP_PREFIX);
		return NULL;
		return NULL;
	}

	properties = list_get_new_list();

	triple = first_triple;
	for(i = 0; i < max && triple; i++)
	{
		if (prop->type == OBJECTPROPERTY) {
			uuid = (char *) triple->object;
			const individual_t *ind_tmp = sslog_repo_get_individual_by_uuid(uuid);
			if (ind_tmp == NULL)
			{

				ss_triple_t *request_temp = NULL;
				ss_triple_t *first_triple_temp = NULL;

				ss_add_triple(&request_temp, uuid, RDF_TYPE, SS_RDF_SIB_ANY, SS_RDF_TYPE_URI, SS_RDF_TYPE_URI);

				if(ss_query(sslog_get_ss_info(), request_temp, &first_triple_temp) < 0) {
					set_error(SSLOG_ERROR_UNKNOWN);
					ss_delete_triples(request_temp);
					triple = triple -> next;
					continue;
				}
				ss_delete_triples(request_temp);
				if (first_triple_temp == NULL) {
					set_error(SSLOG_ERROR_UNKNOWN);
					triple = triple -> next;
					continue;
				}

				const class_t* ont_class = sslog_repo_get_class_by_classtype(first_triple_temp->object);
				individual_t *ind_tmp2 = sslog_new_individual(ont_class);
				if (ind_tmp2 != NULL)
				{
					ind_tmp2->uuid = uuid;
					data = (void *) ind_tmp2;
				}
				else data = NULL;
				ss_delete_triples(first_triple_temp);
			}
			else data = (void *) ind_tmp;
		}
		else if (prop->type == DATATYPEPROPERTY) {
			data = (void *) triple->object;
		}
		prop_val_t* value = sslog_new_value_struct(prop, (void *) data);
		list_t *node = list_get_new_node(value);

		list_add(&node->links, &properties->links);

		triple = triple->next;
	}

	ss_delete_triples(first_triple);

	KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED)
		("\n%s sslog_ss_get_property_max END: property value is ready\n", KPLIB_DEBUG_SS_PROP_PREFIX);

	return properties;

}


/**
* @fn prop_val_t* sslog_ss_get_property_by_name(individual_t *ind, char *propname)
*
* @brief Gets property for individual from SS by name.
*
* Gets the property from the SS by name and returns it.
* Function sets global SmartSlog KP Library's errno.
*
* @param[in] individual_t * ind. Pointer to the individual struct.
* @param[in] char *propname. Pointer to the char array with name.
* @return prop_val_t*. NULL if properties with given name doesn't exists,
*                      pointer to value struct otherwise.
*/
SSLOG_EXTERN prop_val_t* sslog_ss_get_property_by_name(individual_t *ind, char *propname)
{
    KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED)
		("\n%s sslog_ss_get_property_by_name START\n", KPLIB_DEBUG_SS_PROP_PREFIX);

	ss_triple_t * request = NULL;
	ss_triple_t * first_triple = NULL;
	void * data = NULL;
	char * uuid = NULL;

	int error_code = verify_individual(ind);

	if (error_code != SSLOG_ERROR_NO) {
		set_error(error_code);
		KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED)
			("\n%s sslog_ss_get_property_by_name END: error while verify_individual\n", KPLIB_DEBUG_SS_PROP_PREFIX);
		return NULL;
	}

	if (propname == NULL) {
		set_error(SSLOG_ERROR_INCORRECT_PROPERTY_NAME);
		KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED)
			("\n%s sslog_ss_get_property_by_name END: error while checking second parameter char\n", KPLIB_DEBUG_SS_PROP_PREFIX);
		return NULL;
	}

	property_t *prop = (property_t *) sslog_get_property_type(ind->parent_class, propname);
	if (!prop) {
		KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED)
			("\n%s sslog_ss_get_property_by_name END: error while checking second parameter property_t\n", KPLIB_DEBUG_SS_PROP_PREFIX);
		return NULL;
	}

	ss_add_triple(&request, ind->uuid, prop->name, SS_RDF_SIB_ANY, SS_RDF_TYPE_URI, SS_RDF_TYPE_URI);
	if(ss_query(sslog_get_ss_info(), request, &first_triple) < 0) {
		set_error(SSLOG_ERROR_UNKNOWN);
		ss_delete_triples(request);
		KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED)
			("\n%s sslog_ss_get_property_by_name END: error while ss_query\n", KPLIB_DEBUG_SS_PROP_PREFIX);
		return NULL;
	}
	ss_delete_triples(request);
	if (first_triple == NULL) {
		KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED)
			("\n%s sslog_ss_get_property_by_name END: ss_query returns NULL\n", KPLIB_DEBUG_SS_PROP_PREFIX);
		return NULL;
	}

	if (prop->type == OBJECTPROPERTY) {
		uuid = (char *) first_triple->object;
		const individual_t *ind_tmp = sslog_repo_get_individual_by_uuid(uuid);
		if (ind_tmp == NULL)
		{

			ss_triple_t *request_temp = NULL;
			ss_triple_t *first_triple_temp = NULL;

			ss_add_triple(&request_temp, uuid, RDF_TYPE, SS_RDF_SIB_ANY, SS_RDF_TYPE_URI, SS_RDF_TYPE_URI);

			if(ss_query(sslog_get_ss_info(), request_temp, &first_triple_temp) < 0) {
				set_error(SSLOG_ERROR_UNKNOWN);
				ss_delete_triples(request_temp);
				KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED)
					("\n%s sslog_ss_get_property_by_name END: error while ss_query for OBJECTPROPERTY\n", KPLIB_DEBUG_SS_PROP_PREFIX);
				return NULL;
			}
			ss_delete_triples(request_temp);
			if (first_triple_temp == NULL) {
				set_error(SSLOG_ERROR_UNKNOWN);
				KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED)
					("\n%s sslog_ss_get_property_by_name END: ss_query for OBJECTPROPERTY returns NULL\n", KPLIB_DEBUG_SS_PROP_PREFIX);
				return NULL;
			}

			const class_t* ont_class = sslog_repo_get_class_by_classtype(first_triple_temp->object);
			individual_t *ind_tmp2 = sslog_new_individual(ont_class);
			if (ind_tmp2 != NULL)
			{
                sslog_set_individual_uuid(ind_tmp2, uuid);
                //ind_tmp2->uuid = uuid;
				data = (void *) ind_tmp2;
			}
			else data = NULL;
			ss_delete_triples(first_triple_temp);
		}
		else data = (void *) ind_tmp;
	}
	else if (prop->type == DATATYPEPROPERTY) {
		data = (void *) strdup(first_triple->object);
	}
	ss_delete_triples(first_triple);

	prop_val_t* value = sslog_new_value_struct(prop, data);

	KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED)
		("\n%s sslog_ss_get_property_by_name END: property value is ready\n", KPLIB_DEBUG_SS_PROP_PREFIX);

	return value;
}

/**
* @fn prop_val_t* sslog_ss_get_property_by_value(individual_t *ind, property_t *prop, void *data)
*
* @brief Gets first founded property for individual from SS by name and value.
*
* Gets the property from the SS by name and value and returns first founded.
* Function sets global SmartSlog KP Library's errno.
*
* @param[in] individual_t * ind. Pointer to the individual struct.
* @param[in] char *propname. Pointer to the char array with name.
* @param[in] void *data. Pointer to the value.
* @return prop_val_t*. NULL if properties with given name doesn't exists,
*                      pointer to value struct otherwise.
*/
SSLOG_EXTERN prop_val_t* sslog_ss_get_property_by_value(individual_t *ind, property_t *prop, void *data)
{
    KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED)
		("\n%s sslog_ss_get_property_by_value START\n", KPLIB_DEBUG_SS_PROP_PREFIX);

	ss_triple_t * request = NULL;
	ss_triple_t * first_triple = NULL;
	char *uuid;

	if (data == NULL)
		return sslog_ss_get_property(ind, prop);

	int error_code = verify_individual(ind);

	if (error_code != SSLOG_ERROR_NO) {
		set_error(error_code);
		KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED)
			("\n%s sslog_ss_get_property_by_value END: error while verify_individual\n", KPLIB_DEBUG_SS_PROP_PREFIX);
		return NULL;
	}

	if (prop == NULL || prop->name == NULL) {
		set_error(SSLOG_ERROR_INCORRECT_PROPERTY_NAME);
		KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED)
			("\n%s sslog_ss_get_property_by_value END: error while checking second parameter property_t\n", KPLIB_DEBUG_SS_PROP_PREFIX);
		return NULL;
	}

	if (prop->type == OBJECTPROPERTY) {
		if (sslog_get_rtti_type(data) != RTTI_INDIVIDUAL) {
			set_error(SSLOG_ERROR_INCORRECT_PROPERTY_VALUE);
			KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED)
				("\n%s sslog_ss_get_property_by_value END: error while checking third parameter data for OBJECTPROPERTY\n", KPLIB_DEBUG_SS_PROP_PREFIX);
			return NULL;
		}
		individual_t *object = (individual_t *) data;
		ss_add_triple(&request, ind->uuid, prop->name, object->uuid, SS_RDF_TYPE_URI, SS_RDF_TYPE_URI);
	}
	else if (prop->type == DATATYPEPROPERTY) {
		ss_add_triple(&request, ind->uuid, prop->name, (char *) data, SS_RDF_TYPE_URI, SS_RDF_TYPE_URI);
	}


	if(ss_query(sslog_get_ss_info(), request, &first_triple) < 0) {
		set_error(SSLOG_ERROR_UNKNOWN);
		ss_delete_triples(request);
		KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED)
			("\n%s sslog_ss_get_property_by_value END: error while ss_query\n", KPLIB_DEBUG_SS_PROP_PREFIX);
		return NULL;
	}

	ss_delete_triples(request);

	if (!first_triple) {
		KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED)
			("\n%s sslog_ss_get_property_by_value END: ss_query returns NULL\n", KPLIB_DEBUG_SS_PROP_PREFIX);
		return NULL;
	}

	if (prop->type == OBJECTPROPERTY) {
		uuid = (char *) first_triple->object;
		const individual_t *ind_tmp = sslog_repo_get_individual_by_uuid(uuid);
		if (ind_tmp == NULL)
		{

			ss_triple_t *request_temp = NULL;
			ss_triple_t *first_triple_temp = NULL;

			ss_add_triple(&request_temp, uuid, RDF_TYPE, SS_RDF_SIB_ANY, SS_RDF_TYPE_URI, SS_RDF_TYPE_URI);

			if(ss_query(sslog_get_ss_info(), request_temp, &first_triple_temp) < 0) {
				set_error(SSLOG_ERROR_UNKNOWN);
				ss_delete_triples(request_temp);
				KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED)
					("\n%s sslog_ss_get_property_by_value END: error while ss_query for OBJECTPROPERTY\n", KPLIB_DEBUG_SS_PROP_PREFIX);
				return NULL;
			}
			ss_delete_triples(request_temp);
			if (first_triple_temp == NULL) {
				set_error(SSLOG_ERROR_UNKNOWN);
				KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED)
					("\n%s sslog_ss_get_property_by_value END: ss_query for OBJECTPROPERTY returns NULL\n", KPLIB_DEBUG_SS_PROP_PREFIX);
				return NULL;
			}

			const class_t* ont_class = sslog_repo_get_class_by_classtype(first_triple_temp->object);
			individual_t *ind_tmp2 = sslog_new_individual(ont_class);
			if (ind_tmp2 != NULL)
			{
				ind_tmp2->uuid = uuid;
				data = (void *) ind_tmp2;
			}
			else data = NULL;
			ss_delete_triples(first_triple_temp);
		}
		else data = (void *) ind_tmp;
	}
	else if (prop->type == DATATYPEPROPERTY) {
		data = (void *) first_triple->object;
	}
	ss_delete_triples(first_triple);

	prop_val_t* value = sslog_new_value_struct(prop, (void *) data);

	KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED)
		("\n%s sslog_ss_get_property_by_value END: property value is ready\n", KPLIB_DEBUG_SS_PROP_PREFIX);

	return value;
}

/**
* @fn bool sslog_ss_exists_property_value(individual_t* ind, property_t *prop, void *data)
*
* @brief Checks if individual has property with given value.
*
* Checks if individual has property with given name and value.
* Function sets global SmartSlog KP Library's errno.
*
* @param[in] individual_t * ind. Pointer to the individual struct.
* @param[in] property_t *prop. Pointer to the char array with name.
* @param[in] void *data. Pointer to the value.
* @return int. 0 if property doesn't exists, 1 otherwise.
*/
SSLOG_EXTERN bool sslog_ss_exists_property_value(individual_t* ind, property_t *prop, void *data)
{
    KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED)
		("\n%s sslog_ss_exists_property_value START\n", KPLIB_DEBUG_SS_PROP_PREFIX);

	prop_val_t* value;

	value = sslog_ss_get_property_by_value(ind, prop, data);

	if (value == NULL)
	{
		KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED)
			("\n%s sslog_ss_exists_property_value END: property value is not exists\n", KPLIB_DEBUG_SS_PROP_PREFIX);
		return false;
	}
	else
	{
		/*sslog_free_value_struct(value, free);*/
		KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED)
			("\n%s sslog_ss_exists_property_value END: property value is exists\n", KPLIB_DEBUG_SS_PROP_PREFIX);
		return true;
	}

}

/**
* @fn int sslog_ss_update_property(individual_t *ind, char *propname, void *old_data, void *new_data)
*
* @brief Updates property of individual in SS and localy.
*
* Checks individual correctness: cardinality, prop-
* erty values limitation, and converts the individual’s given
* properties to triplets, which will be updated in SS, other
* properties not changed in SS. Same localy.
* Function sets global SmartSlog KP Library's errno.
*
* @param[in] individual_t * ind. Pointer to the individual struct.
* @param[in] char *propname. Char array with name of property.
* @param[in] void *old_data. Pointer to the old value (DATATYPE or OBJECTTYPE).
* @param[in] void *new_data. Pointer to the new value (DATATYPE or OBJECTTYPE).
* @return int. Status of the operation when completed (0 if successfull, otherwise -1).
*/
SSLOG_EXTERN int sslog_ss_update_property(individual_t *ind, char *propname, void *old_data, void *new_data)
{
    KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED)
		("\n%s sslog_ss_update_property START\n", KPLIB_DEBUG_SS_PROP_PREFIX);

	int error_code = verify_individual(ind);

	if (error_code != SSLOG_ERROR_NO) {
		set_error(error_code);
		KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED)
			("\n%s sslog_ss_update_property END: error while verify_individual\n", KPLIB_DEBUG_SS_PROP_PREFIX);
		return -1;
	}

	if (propname == NULL) {
		set_error(SSLOG_ERROR_INCORRECT_PROPERTY_NAME);
		KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED)
			("\n%s sslog_ss_update_property END: error while checking second parameter\n", KPLIB_DEBUG_SS_PROP_PREFIX);
		return -1;
	}

	property_t *prop = (property_t *) sslog_get_property_type(ind->parent_class, propname);
	if (!prop) {
		set_error(SSLOG_ERROR_INCORRECT_ARGUMENT);
		KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED)
			("\n%s sslog_ss_update_property END: error while checking second parameter\n", KPLIB_DEBUG_SS_PROP_PREFIX);
		return -1;
	}

	if (old_data == NULL) {
		set_error(SSLOG_ERROR_INCORRECT_ARGUMENT);
		KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED)
			("\n%s sslog_ss_update_property END: error while checking third parameter\n", KPLIB_DEBUG_SS_PROP_PREFIX);
		return -1;
	}

	if (new_data == NULL) {
		set_error(SSLOG_ERROR_INCORRECT_PROPERTY_NAME);
		KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED)
			("\n%s sslog_ss_update_property END: error while checking fourth parameter\n", KPLIB_DEBUG_SS_PROP_PREFIX);
		return -1;
	}


	ss_triple_t *old_triple = NULL;
	ss_triple_t *new_triple = NULL;

	sslog_reset_errno();

	ss_add_triple(&old_triple, ind->uuid, prop->name, (char *) old_data, SS_RDF_TYPE_URI, SS_RDF_TYPE_LIT);
	ss_add_triple(&new_triple, ind->uuid, prop->name, (char *) new_data, SS_RDF_TYPE_URI, SS_RDF_TYPE_LIT);

	ss_update(sslog_get_ss_info(), new_triple, old_triple, NULL);

	ss_delete_triples(old_triple);
	ss_delete_triples(new_triple);

	if (sslog_get_ss_info()->ss_errno != 0) {
		KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED)
			("\n%s sslog_ss_update_property END: errir while updating remote property\n", KPLIB_DEBUG_SS_PROP_PREFIX);
		return -1;
	}

	if (sslog_update_property_with_value(ind, propname, old_data, new_data) != 0) {
		KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED)
			("\n%s sslog_ss_update_property END: errir while updating local property\n", KPLIB_DEBUG_SS_PROP_PREFIX);
		return -1;
	}

	KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED)
		("\n%s sslog_ss_update_property END: property is updated\n", KPLIB_DEBUG_SS_PROP_PREFIX);

	return 0;
}

int ss_update_properties(individual_t *individual, list_t *properties)
{
	return 0;
}

/**
* @fn int sslog_ss_remove_property_by_name_all(individual_t *ind, char *propname)
*
* @brief Removes all properties with given name for individual in SS and localy.
*
* Finds properties with given name for individual and removes it from SS.
* Then try find all local property with given name and remove it.
* After all you have a struct without removed properties.
* Function sets global SmartSlog KP Library's errno.
*
* @param[in] individual_t * ind. Pointer to the individual struct.
* @param[in] char *propname. Char array with name of property.
* @return int. Status of the operation when completed (0 if successfull, otherwise -1).
*/
SSLOG_EXTERN int sslog_ss_remove_property_by_name_all(individual_t *ind, char *propname)
{
    KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED)
		("\n%s sslog_ss_remove_property_by_name_all START\n", KPLIB_DEBUG_SS_PROP_PREFIX);

	ss_triple_t * request = NULL;

	int error_code = verify_individual(ind);

	if (error_code != SSLOG_ERROR_NO) {
		set_error(error_code);
		KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED)
			("\n%s sslog_ss_remove_property_by_name_all END: error while verify_individual\n", KPLIB_DEBUG_SS_PROP_PREFIX);
		return -1;
	}

	if (propname == NULL) {
		set_error(SSLOG_ERROR_INCORRECT_PROPERTY_NAME);
		KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED)
			("\n%s sslog_ss_remove_property_by_name_all END: error while checking second parameter\n", KPLIB_DEBUG_SS_PROP_PREFIX);
		return -1;
	}

	ss_add_triple(&request, ind->uuid, propname, SS_RDF_SIB_ANY, SS_RDF_TYPE_URI, SS_RDF_TYPE_URI);
	if(ss_remove(sslog_get_ss_info(), request) < 0) {
		set_error(SSLOG_ERROR_UNKNOWN);
		ss_delete_triples(request);
		KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED)
			("\n%s sslog_ss_remove_property_by_name_all END: error while ss_remove\n", KPLIB_DEBUG_SS_PROP_PREFIX);
		return -1;
	}
	ss_delete_triples(request);

	KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED)
		("\n%s sslog_ss_remove_property_by_name_all END: property is removed\n", KPLIB_DEBUG_SS_PROP_PREFIX);

	return 0;
}

/**
* @fn int sslog_ss_remove_property_all(individual_t *ind, property_t *prop)
*
* @brief Removes all properties with given structure for individual in SS and localy.
*
* Finds properties with given structure for individual and removes it from SS.
* Then try find all local property with given structure and remove it.
* After all you have a struct without removed properties.
* Function sets global SmartSlog KP Library's errno.
*
* @param[in] individual_t * ind. Pointer to the individual struct.
* @param[in] property_t *prop. Property structure with name of property.
* @return int. Status of the operation when completed (0 if successfull, otherwise -1).
*/
SSLOG_EXTERN int sslog_ss_remove_property_all(individual_t *ind, property_t *prop)
{
    KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED)
		("\n%s sslog_ss_remove_property_all START\n", KPLIB_DEBUG_SS_PROP_PREFIX);

	ss_triple_t * request = NULL;

	int error_code = verify_individual(ind);

	if (error_code != SSLOG_ERROR_NO) {
		set_error(error_code);
		KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED)
			("\n%s sslog_ss_remove_property_all END: error while verify_individual\n", KPLIB_DEBUG_SS_PROP_PREFIX);
		return -1;
	}

	if (prop == NULL || prop->name == NULL) {
		set_error(SSLOG_ERROR_INCORRECT_PROPERTY_NAME);
		KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED)
			("\n%s sslog_ss_remove_property_all END: error while checking second parameter\n", KPLIB_DEBUG_SS_PROP_PREFIX);
		return -1;
	}

	ss_add_triple(&request, ind->uuid, prop->name, SS_RDF_SIB_ANY, SS_RDF_TYPE_URI, SS_RDF_TYPE_URI);
	if(ss_remove(sslog_get_ss_info(), request) < 0) {
		set_error(SSLOG_ERROR_UNKNOWN);
		ss_delete_triples(request);
		KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED)
			("\n%s sslog_ss_remove_property_all END: error while ss_remove\n", KPLIB_DEBUG_SS_PROP_PREFIX);
		return -1;
	}
	ss_delete_triples(request);

	KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED)
		("\n%s sslog_ss_remove_property_all END: property is removed\n", KPLIB_DEBUG_SS_PROP_PREFIX);

	return 0;
}

/**
* @fn int sslog_ss_remove_property_by_value_struct(individual_t *ind, prop_val_t *prop_val)
*
* @brief Removes all properties with given name and value for individual in SS and localy.
*
* Get name and value from value struct, finds properties with given
* name and value for individual and removes it from SS.
* Then try find all local property with given name and value and remove it.
* After all you have a struct without removed properties.
* Function sets global SmartSlog KP Library's errno.
*
* @param[in] individual_t * ind. Pointer to the individual struct.
* @param[in] prop_val_t *prop_val. Pointer to the value struct.
* @return int. Status of the operation when completed (0 if successfull, otherwise -1).
*/
SSLOG_EXTERN int sslog_ss_remove_property_by_value_struct(individual_t *ind, prop_val_t *prop_val)
{
    KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED)
		("\n%s sslog_ss_remove_property_by_value_struct START\n", KPLIB_DEBUG_SS_PROP_PREFIX);

	ss_triple_t * request = NULL;

	reset_error();

	int error_code = verify_individual(ind);

	if (error_code != SSLOG_ERROR_NO) {
		set_error(error_code);
		KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED)
			("\n%s sslog_ss_remove_property_by_value_struct END: error while verify_individual\n", KPLIB_DEBUG_SS_PROP_PREFIX);
		return -1;
	}

	if (prop_val == NULL || prop_val->property==NULL) {
		set_error(SSLOG_ERROR_INCORRECT_PROPERTY_NAME);
		KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED)
			("\n%s sslog_ss_remove_property_by_value_struct END: error while checking second parameter\n", KPLIB_DEBUG_SS_PROP_PREFIX);
		return -1;
	}

	property_t *prop = (property_t *) sslog_get_property_type(ind->parent_class, prop_val->property->name);
	if (!prop) {
		KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED)
			("\n%s sslog_ss_remove_property_by_value_struct END: error while checking second parameter\n", KPLIB_DEBUG_SS_PROP_PREFIX);
		return -1;
	}

	if (prop_val->prop_value == NULL)
	{
		ss_add_triple(&request, ind->uuid, prop_val->property->name, SS_RDF_SIB_ANY, SS_RDF_TYPE_URI, SS_RDF_TYPE_URI);
	}
	else if (prop->type == OBJECTPROPERTY) {
		if (sslog_get_rtti_type(prop_val->prop_value) != RTTI_INDIVIDUAL) {
			set_error(SSLOG_ERROR_INCORRECT_PROPERTY_VALUE);
			KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED)
				("\n%s sslog_ss_remove_property_by_value_struct END: error while checking second parameter\n", KPLIB_DEBUG_SS_PROP_PREFIX);
			return -1;
		}
		individual_t *object = (individual_t *) prop_val->prop_value;
		ss_add_triple(&request, ind->uuid, prop_val->property->name, object->uuid, SS_RDF_TYPE_URI, SS_RDF_TYPE_URI);
	}
	else if (prop->type == DATATYPEPROPERTY) {
		ss_add_triple(&request, ind->uuid, prop_val->property->name, (char *) prop_val->prop_value, SS_RDF_TYPE_URI, SS_RDF_TYPE_URI);
	}

	if(ss_remove(sslog_get_ss_info(), request) < 0) {
		set_error(SSLOG_ERROR_UNKNOWN);
		ss_delete_triples(request);
		KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED)
			("\n%s sslog_ss_remove_property_by_value_struct END: error while ss_remove\n", KPLIB_DEBUG_SS_PROP_PREFIX);
		return -1;
	}
	ss_delete_triples(request);

	KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED)
		("\n%s sslog_ss_remove_property_by_value_struct END: property is removed\n", KPLIB_DEBUG_SS_PROP_PREFIX);

	return 0;
}

/** @endcond */

/** @cond INTERNAL_FUNCTIONS */

/**
* @fn int ss_set_property_for_individual(individual_t *ind, char *propname, void *data)
*
* @brief Sets property for individual in SS and localy by name.
*
* Checks individual correctness: cardinality, property values limitation
* and sets property for entity in SS and localy.
* Function sets global SmartSlog KP Library's errno.
*
* @param[in] individual_t *ind. Pointer to the individual struct.
* @param[in] char *propname. Pointer to the char array with name.
* @param[in] void *data. Pointer to the value.
* @return int. Status of the operation when completed (0 if successfull, otherwise -1).
*/
int ss_add_property_for_individual(individual_t *ind, char *propname, void *data)
{
    KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED)
		("\n%s ss_add_property_for_individual START\n", KPLIB_DEBUG_SS_PROP_PREFIX);

	ss_triple_t * triples = NULL;

	int error_code = verify_individual(ind);

	if (error_code != SSLOG_ERROR_NO) {
		set_error(error_code);
		KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED)
			("\n%s ss_add_property_for_individual END: error while verify_individual\n", KPLIB_DEBUG_SS_PROP_PREFIX);
		return -1;
	}

	if (propname == NULL) {
		KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED)
			("\n%s ss_add_property_for_individual END: error while checking second parameter\n", KPLIB_DEBUG_SS_PROP_PREFIX);
		return -1;
	}

	property_t *prop = (property_t *) sslog_get_property_type(ind->parent_class, propname);
	if (prop == NULL) {
		KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED)
			("\n%s ss_add_property_for_individual END: error while checking second parameter\n", KPLIB_DEBUG_SS_PROP_PREFIX);
		return -1;
	}
	
	if (data == NULL) {
		KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED)
			("\n%s ss_add_property_for_individual END: error while checking third parameter\n", KPLIB_DEBUG_SS_PROP_PREFIX);
		return -1;
	}

	if (prop->type == OBJECTPROPERTY) {
		if (sslog_get_rtti_type(data) != RTTI_INDIVIDUAL) {
			set_error(SSLOG_ERROR_INCORRECT_PROPERTY_VALUE);
			KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED)
				("\n%s ss_add_property_for_individual END: error while checking second parameter\n", KPLIB_DEBUG_SS_PROP_PREFIX);
			return -1;
		}
		individual_t *object = (individual_t *) data;
		ss_add_triple(&triples, ind->uuid, propname, object->uuid, SS_RDF_TYPE_URI, SS_RDF_TYPE_URI);
		ss_add_triple(&triples, object->uuid, RDF_TYPE, object->classtype, SS_RDF_TYPE_URI, SS_RDF_TYPE_URI);
	}
	else if (prop->type == DATATYPEPROPERTY) {
		ss_add_triple(&triples, ind->uuid, propname, (char *) data, SS_RDF_TYPE_URI, SS_RDF_TYPE_LIT);
	}

	ss_insert(sslog_get_ss_info(), triples, NULL);

	ss_delete_triples(triples);

	KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED)
		("\n%s ss_set_property_for_individual END: property is added\n", KPLIB_DEBUG_SS_PROP_PREFIX);

	return 0;
}

/**
* @fn int ss_set_property_for_class(class_t *ind, char *propname, void *data)
*
* @brief Sets property for class in SS and localy by name.
*
* Checks class correctness: cardinality, property values limitation
* and sets property for entity in SS and localy.
* Function sets global SmartSlog KP Library's errno.
*
* @param[in] individual_t *ind. Pointer to the individual struct.
* @param[in] char *propname. Pointer to the char array with name.
* @param[in] void *data. Pointer to the value.
* @return int. Status of the operation when completed (0 if successfull, otherwise -1).
*/
int ss_add_property_for_class(class_t *ind, char *propname, void *data)
{
	return 0;
}

/** @endcond */

#endif	/* _SS_PROPERTIES_C_ */
