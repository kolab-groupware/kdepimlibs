/* -*- Mode: C -*- */
/*======================================================================
  FILE: icalvalue.h
  CREATOR: eric 20 March 1999



 (C) COPYRIGHT 2000, Eric Busboom <eric@softwarestudio.org>
     http://www.softwarestudio.org

 This program is free software; you can redistribute it and/or modify
 it under the terms of either: 

    The LGPL as published by the Free Software Foundation, version
    2.1, available at: http://www.fsf.org/copyleft/lesser.html

  Or:

    The Mozilla Public License Version 1.0. You may obtain a copy of
    the License at http://www.mozilla.org/MPL/

  The original code is icalvalue.h

  ======================================================================*/

#ifndef ICALVALUE_H
#define ICALVALUE_H

#include <time.h>
#include "icalenums.h"
#include "icaltypes.h"
#include "icalrecur.h"
#include "icalduration.h"
#include "icalperiod.h"
#include "icalderivedproperty.h" /* For icalproperty_method, etc. */
#include "icalderivedparameter.h"
#include "icalderivedvalue.h"
                          
/* Defined in icalderivedvalue.h */
/*typedef struct icalvalue_impl icalvalue;*/

icalvalue* icalvalue_new(icalvalue_kind kind);

icalvalue* icalvalue_new_clone(const icalvalue* value);

icalvalue* icalvalue_new_from_string(icalvalue_kind kind, const char* str);

void icalvalue_free(icalvalue* value);

int icalvalue_is_valid(const icalvalue* value);

const char* icalvalue_as_ical_string(const icalvalue* value);

icalvalue_kind icalvalue_isa(const icalvalue* value);

int icalvalue_isa_value(void*);

icalparameter_xliccomparetype icalvalue_compare(const icalvalue* a, const icalvalue *b);


/* Special, non autogenerated value accessors */

icalvalue* icalvalue_new_recur (struct icalrecurrencetype v);
void icalvalue_set_recur(icalvalue* value, struct icalrecurrencetype v);
struct icalrecurrencetype icalvalue_get_recur(const icalvalue* value);

icalvalue* icalvalue_new_trigger (struct icaltriggertype v);
void icalvalue_set_trigger(icalvalue* value, struct icaltriggertype v);
struct icaltriggertype icalvalue_get_trigger(const icalvalue* value);

icalvalue* icalvalue_new_datetimeperiod (struct icaldatetimeperiodtype v);
void icalvalue_set_datetimeperiod(icalvalue* value, 
				  struct icaldatetimeperiodtype v);
struct icaldatetimeperiodtype icalvalue_get_datetimeperiod(const icalvalue* value);

/* Convert enumerations */

icalvalue_kind icalvalue_string_to_kind(const char* str);
const char* icalvalue_kind_to_string(const icalvalue_kind kind);

/** Check validity of a specific icalvalue_kind **/
int icalvalue_kind_is_valid(const icalvalue_kind kind);

/** Encode a character string in ical format, esacpe certain characters, etc. */
int icalvalue_encode_ical_string(const char *szText, char *szEncText, int MaxBufferLen);

#endif /*ICALVALUE_H*/
