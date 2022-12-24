#include "tensor.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

// NOTE: for now using static inline over macro for type safety
// macro doesn't feel right here

static inline tensor_size_t _tensor_get_size(tensor_t* tensor){
    return tensor->num_rows * tensor->num_columns;
}

static inline size_t _tensor_get_size_in_bytes(tensor_t* tensor){
    return _tensor_get_size(tensor) * sizeof(tensor_entry_t);
}

// create new tensor
// entries are set to zero by default
tensor_t* _new_tensor(tensor_size_t num_rows, tensor_size_t num_columns){
    tensor_size_t num_entries = num_rows * num_columns;
    tensor_entry_t* raw_data = (tensor_entry_t*) calloc(num_entries, sizeof(tensor_entry_t));
    return &(tensor_t) {.data = raw_data, .num_rows = num_rows, .num_columns = num_columns};
}

// TODO - ensure this is inlined
tensor_t* _new_tensor_like(tensor_t* old_tensor){
    return _new_tensor(old_tensor->num_rows, old_tensor->num_columns);
}

// the same as _new_tensor_like (for now)
tensor_t* _new_tensor_zeros_like(tensor_t* old_tensor){
    return _new_tensor(old_tensor->num_rows, old_tensor->num_columns);
}


tensor_t* _copy_tensor(tensor_t* old_tensor){
    tensor_t* new_tensor = _new_tensor_like(old_tensor);
    memcpy(new_tensor->data, old_tensor->data, _tensor_get_size_in_bytes(old_tensor));
    return new_tensor;
}

void _populate_tensor(tensor_t* tensor, tensor_row_column_fn_t row_column_fn){
    for(tensor_size_t row = 0; row < tensor->num_rows; row++){
        for(tensor_size_t column = 0; column < tensor->num_columns; column++){
            tensor_entry_t new_entry = (*row_column_fn)(row, column);
            _tensor_set_entry_row_column(tensor, row, column, new_entry);
        }
    }
}

/**
 * PRINTING
*/

void _display_tensor(tensor_t* tensor){
    printf("Tensor:\n");
    for(tensor_size_t row = 0; row < tensor->num_rows; row++){
        for(tensor_size_t column = 0; column < tensor->num_columns; column++){
            printf("%f ", _tensor_get_entry_row_column(tensor, row, column));
        }
        printf("\n");
    }
    printf("num_rows: %llu\n", tensor->num_rows);
    printf("num_columns: %llu\n", tensor->num_columns);
}

/**
 * VIEWS - TODO
*/

/**
 *  FUNCTIONS
*/

// returns true if and only if tensor dimensions match exactly
// used as a pre-check for component-wise operations
static inline bool _tensor_exact_compatible(tensor_t* left_tensor, tensor_t* right_tensor){
    return (left_tensor->num_columns == right_tensor->num_columns) && (left_tensor->num_rows == right_tensor->num_rows);
}

// TODO: returns true iff tensors can be broadcasted in the same manner as in numpy/PyTorch
static inline bool _tensor_broadcast_compatible(tensor_t* left_tensor, tensor_t* right_tensor){
    return _tensor_exact_compatible(left_tensor, right_tensor);
}

// returns true if and only if internal dimensions match
// so that left_tensor @ right_tensor is a valid matrix multiplication
// static inline bool _tensor_internal_compatible(tensor_t* left_tensor, tensor_t* right_tensor){
//     return (left_tensor->num_columns == right_tensor->num_rows);
// }

static inline tensor_entry_t _scalar_add(tensor_entry_t left_scalar, tensor_entry_t right_scalar) {
    return left_scalar + right_scalar;
}

static inline tensor_entry_t _scalar_multiply(tensor_entry_t left_scalar, tensor_entry_t right_scalar) {
    return left_scalar * right_scalar;
}

static inline tensor_entry_t _scalar_subtract(tensor_entry_t left_scalar, tensor_entry_t right_scalar) {
    return left_scalar - right_scalar;
}

// TODO : allow for broadcasting of different sizes
static inline tensor_t* _tensor_broadcast_scalar_fn(tensor_t* left_tensor, tensor_t* right_tensor, tensor_entry_fn_t tensor_scalar_fn){
    DEBUG_ASSERT(_tensor_broadcast_compatible(left_tensor, right_tensor), "Tensors are not broadcast compatible!\n");
    tensor_t* new_tensor = _new_tensor_like(left_tensor);
    for(tensor_size_t index = 0; index < _tensor_get_size(left_tensor); index++){
        tensor_entry_t left_entry = _tensor_get_entry(left_tensor, index);
        tensor_entry_t right_entry = _tensor_get_entry(right_tensor, index);
        tensor_entry_t new_entry = (*tensor_scalar_fn)(left_entry, right_entry);
        _tensor_set_entry(new_tensor, new_entry, index);
    }
    return new_tensor;
}

// return new tensor which is the result of component-wise addition 
// of left_tensor and right_tensor
// assumes that left_tensor and right_tensor are compatible
tensor_t* _tensor_add(tensor_t* left_tensor, tensor_t* right_tensor){
    return _tensor_broadcast_scalar_fn(left_tensor, right_tensor, &_scalar_add);
}

tensor_t* _tensor_subtract(tensor_t* left_tensor, tensor_t* right_tensor){
    return _tensor_broadcast_scalar_fn(left_tensor, right_tensor, &_scalar_subtract);
}

tensor_t* _tensor_multiply(tensor_t* left_tensor, tensor_t* right_tensor){
    return _tensor_broadcast_scalar_fn(left_tensor, right_tensor, &_scalar_multiply);
}
