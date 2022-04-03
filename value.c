#include "value.h"

bool values_equal(Value value1, Value value2) {
    if (value1.type != value2.type)
        return false;

    if (value1.type == VAL_NUMBER && 
        value2.type == VAL_NUMBER) {
        return AS_NUMBER(value1) == AS_NUMBER(value2);
    } else if (value1.type == VAL_BOOLEAN &&
               value2.type == VAL_BOOLEAN) {
        return AS_BOOLEAN(value1) == AS_BOOLEAN(value2);
    } else { // TODO : This should have a proper return
        return false;
    }
}
