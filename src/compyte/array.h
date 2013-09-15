#ifndef COMPYTE_ARRAY_H
#define COMPYTE_ARRAY_H
/**
 * \file array.h
 * \brief Array functions.
 */

#include <compyte/buffer.h>

#ifdef __cplusplus
extern "C" {
#endif
#ifdef CONFUSE_EMACS
}
#endif

/**
 * Main array structure.
 */
typedef struct _GpuArray {
  /**
   * Device data buffer.
   */
  gpudata *data;
  /**
   * Backend operations vector.
   */
  const compyte_buffer_ops *ops;
  /**
   * Size of each dimension.  The number of elements is #nd.
   */
  size_t *dimensions;
  /**
   * Stride for each dimension.  The number of elements is #nd.
   */
  ssize_t *strides;
  /**
   * Offset to the first array element into the device data buffer.
   */
  size_t offset;
  /**
   * Number of dimensions.
   */
  unsigned int nd;
  /**
   * Flags for this array (see \ref aflags).
   */
  int flags;
  /**
   * Type of the array elements.
   */
  int typecode;

/**
 * \defgroup aflags Array Flags
 * @{
 */
  /* Try to keep in sync with numpy values for now */
  /**
   * Array is C-contiguous.
   */
#define GA_C_CONTIGUOUS   0x0001
  /**
   * Array is Fortran-contiguous.
   */
#define GA_F_CONTIGUOUS   0x0002
  /**
   * Buffer data is properly aligned for the type (currently this is
   * always assumed to be true).
   */
#define GA_ALIGNED        0x0100
  /**
   * Can write to the data buffer.  (This is always true for array
   * allocated through this library).
   */
#define GA_WRITEABLE      0x0400
  /**
   * Array data is behaved (properly aligned and writable).
   */
#define GA_BEHAVED        (GA_ALIGNED|GA_WRITEABLE)
  /**
   * Array layout is that of a C array.
   */
#define GA_CARRAY         (GA_C_CONTIGUOUS|GA_BEHAVED)
  /**
   * Array layout is that of a Fortran array.
   */
#define GA_FARRAY         (GA_F_CONTIGUOUS|GA_BEHAVED)
  /**
   * @}
   */
  /* Numpy flags that will not be supported at this level (and why):

     NPY_NOTSWAPPED: data is alway native endian
     NPY_FORCECAST: no casts
     NPY_ENSUREARRAY: no inherited classes
     NPY_UPDATEIFCOPY: cannot support without refcount (or somesuch)

     Maybe will define other flags later */
} GpuArray;

/**
 * Type used to specify the desired order to some functions
 */
typedef enum _ga_order {
  /**
   * Any order is fine.
   */
  GA_ANY_ORDER=-1,
  /**
   * C order is desired.
   */
  GA_C_ORDER=0,
  /**
   * Fortran order is desired.
   */
  GA_F_ORDER=1
} ga_order;

/**
 * Checks if all the specified flags are set.
 *
 * \param a array
 * \param flags flags to check
 *
 * \returns true if all flags in `flags` are set and false otherwise.
 */
static inline int GpuArray_CHKFLAGS(const GpuArray *a, int flags) {
  return (a->flags & flags) == flags;
}
/* Add tests here when you need them */

/**
 * Checks if the array data is writable.
 *
 * \param a array
 *
 * \returns true if the data area of `a` is writable
 */
#define GpuArray_ISWRITEABLE(a) GpuArray_CHKFLAGS(a, GA_WRITEABLE)
/**
 * Checks if the array elements are aligned.
 *
 * \param a array
 *
 * \returns true if the elements of `a` are aligned.
 */
#define GpuArray_ISALIGNED(a) GpuArray_CHKFLAGS(a, GA_ALIGNED)
/**
 * Checks if the array elements are contiguous in memory.
 *
 * \param a array
 *
 * \returns true if the data area of `a` is contiguous
 */
#define GpuArray_ISONESEGMENT(a) ((a)->flags & (GA_C_CONTIGUOUS|GA_F_CONTIGUOUS))
/**
 * Checks if the array elements are laid out if Fortran order.
 *
 * \param a array
 *
 * \returns true if the data area of `a` is Fortran-contiguous
 */
#define GpuArray_ISFORTRAN(a) GpuArray_CHKFLAGS(a, GA_F_CONTIGUOUS)
/**
 * Retrive the size of the elements in the array.
 *
 * \param a array
 *
 * \returns the size of the array elements.
 */
#define GpuArray_ITEMSIZE(a) compyte_get_elsize((a)->typecode)

/**
 * Initialize and allocate a new empty (uninitialized data) array.
 *
 * \param a the GpuArray structure to initialize.  Content will be
 * ignored so make sure to deallocate any previous array first.
 * \param ops backend operations to use.
 * \param ctx context in which to allocate array data. Must come from
 * the same backend as the operations vector.
 * \param typecode type of the elements in the array
 * \param nd desired order (number of dimensions)
 * \param dims size for each dimension.
 * \param ord desired layout of data.
 *
 * \returns A return of GA_NO_ERROR means that the structure is
 * properly initialized and that the memory requested is reserved on
 * the device.  Any other error code means that the structure is
 * left uninitialized.
 */
COMPYTE_PUBLIC int GpuArray_empty(GpuArray *a, const compyte_buffer_ops *ops,
                                  void *ctx, int typecode, unsigned int nd,
                                  const size_t *dims, ga_order ord);

/**
 * Initialize and allocate a new zero-initialized array.
 *
 * \param a the GpuArray structure to initialize.  Content will be
 * ignored so make sure to deallocate any previous array first.
 * \param ops backend operations to use.
 * \param ctx context in which to allocate array data. Must come from
 * the same backend as the operations vector.
 * \param typecode type of the elements in the array
 * \param nd desired order (number of dimensions)
 * \param dims size for each dimension.
 * \param ord desired layout of data.
 *
 * \returns A return of GA_NO_ERROR means that the structure is
 * properly initialized and that the memory requested is reserved on
 * the device.  Any other error code means that the structure is
 * left uninitialized.
 */
COMPYTE_PUBLIC int GpuArray_zeros(GpuArray *a, const compyte_buffer_ops *ops,
                                  void *ctx, int typecode, unsigned int nd,
                                  const size_t *dims, ga_order ord);

/**
 * Initialize and allocate a new array structure from a pre-existing buffer.
 *
 * The array will be considered to own the gpudata structure after the
 * call is made and will free it when deallocated.  An error return
 * from this function will deallocate `data`.
 *
 * \param a the GpuArray structure to initialize.  Content will be
 * ignored so make sure to deallocate any previous array first.
 * \param ops backend that corresponds to the buffer.
 * \param data buffer to user.
 * \param offset position of the first data element of the array in the buffer.
 * \param typecode type of the elements in the array
 * \param nd order of the data (number of dimensions).
 * \param dims size for each dimension.
 * \param strides stride for each dimension.
 * \param writeable true if the buffer is writable false otherwise.
 *
 * \returns A return of GA_NO_ERROR means that the structure is
 * properly initialized. Any other error code means that the structure
 * is left uninitialized and the provided buffer is deallocated.
 */
COMPYTE_PUBLIC int GpuArray_fromdata(GpuArray *a,
                                     const compyte_buffer_ops *ops,
                                     gpudata *data, size_t offset,
                                     int typecode, unsigned int nd,
                                     const size_t *dims,
                                     const ssize_t *strides, int writeable);

/**
 * Initialize an array structure to provide a view of another.
 *
 * The new structure will point to the same data area and have the
 * same values of properties as the source one.  The data area is
 * shared and writes from one array will be reflected in the other.
 * The properties are copied and not shared and can be modified
 * independantly.
 *
 * \param v the result array
 * \param a the source array
 *
 * \return GA_NO_ERROR if the operation was succesful.
 * \return an error code otherwise
 */
COMPYTE_PUBLIC int GpuArray_view(GpuArray *v, const GpuArray *a);

/**
 * Blocks until all operations (kernels, copies) involving `a` are finished.
 *
 * \param a the array to synchronize
 *
 * \return GA_NO_ERROR if the operation was succesful.
 * \return an error code otherwise
 */
COMPYTE_PUBLIC int GpuArray_sync(GpuArray *a);

/**
 * Returns a sub-view of a source array.
 *
 * The indexing follows simple basic model where each dimension is
 * indexed separately.  For a single dimension the indexing selects
 * from the start index (included) to the end index (excluded) while
 * selecting one over step elements. As an example for the array `[ 0
 * 1 2 3 4 5 6 7 8 9 ]` indexed with start index 1 stop index 8 and
 * step 2 the result would be `[ 1 3 5 7 ]`.
 *
 * The special value 0 for step means that only one element
 * corresponding to the start index and the resulting array order will
 * be one smaller.
 *
 * \param r the result array
 * \param a the source array
 * \param starts the start of the subsection for each dimension (length must be a->nd)
 * \param stops the end of the subsection for each dimension (length must be a->nd)
 * \param steps the steps for the subsection for each dimension (length must be a->nd)
 *
 * \return GA_NO_ERROR if the operation was succesful.
 * \return an error code otherwise
 */
COMPYTE_PUBLIC int GpuArray_index(GpuArray *r, const GpuArray *a,
                                  const ssize_t *starts, const ssize_t *stops,
                                  const ssize_t *steps);

/**
 * Sets the content of an array to the content of another array.
 *
 * The value array must be smaller or equal in number of dimensions to
 * the destination array.  Each of its dimensions' size must be either
 * exactly equal to the destination array's corresponding dimensions
 * or 1.  Dimensions of size 1 will be repeated to fill the full size
 * of the destination array. Extra size 1 dimensions will be added at
 * the end to make the two arrays shape-equivalent.
 *
 * \param a the destination array
 * \param v the value array
 *
 * \return GA_NO_ERROR if the operation was succesful.
 * \return an error code otherwise
 */
COMPYTE_PUBLIC int GpuArray_setarray(GpuArray *a, const GpuArray *v);

/**
 * Change the dimensions of an array.
 *
 * Return a new array with the desired dimensions. The new dimensions
 * must have the same total size as the old ones. A copy of the
 * underlying data may be performed if necessary, unless `nocopy` is
 * 0.
 *
 * \param res the result array
 * \param a the source array
 * \param nd new dimensions order
 * \param newdims new dimensions (length is nd)
 * \param ord the desired resulting order
 * \param nocopy if 0 error out if a data copy is required.
 *
 * \return GA_NO_ERROR if the operation was succesful.
 * \return an error code otherwise
 */
COMPYTE_PUBLIC int GpuArray_reshape(GpuArray *res, const GpuArray *a,
                                    unsigned int nd, const size_t *newdims,
                                    ga_order ord, int nocopy);

/**
 * Rearrange the axes of an array.
 *
 * Return a new array with its shape and strides swapped accordingly
 * to the `new_axes` parameter.  If `new_axes` is NULL then the order
 * is reversed.  The returned array is a view on the data of the old
 * one.
 *
 * \param res the result array
 * \param a the source array
 * \param new_axes either NULL or a list of a->nd elements
 *
 * \return GA_NO_ERROR if the operation was successful.
 * \return an error code otherwise
 */
COMPYTE_PUBLIC int GpuArray_transpose(GpuArray *res, const GpuArray *a,
                                      const unsigned int *new_axes);

/**
 * Relase all device and host memory associated with `a`.
 *
 * This function frees all host memory, and releases the device memory
 * if it is the owner. In case an array has views it is the
 * responsability of the caller to ensure a base array is not cleared
 * before its views.
 *
 * This function will also zero out the structure to prevent
 * accidental reuse.
 *
 * \param a the array to clear
 */
COMPYTE_PUBLIC void GpuArray_clear(GpuArray *a);

/**
 * Checks if two arrays may share device memory.
 *
 * \param a an array
 * \param b an array
 *
 * \returns 1 if `a` and `b` may share a portion of their data.
 */
COMPYTE_PUBLIC int GpuArray_share(const GpuArray *a, const GpuArray *b);

/**
 * Retursns the context of an array.
 *
 * \param a an array
 *
 * \returns the context in which `a` was allocated.
 */
COMPYTE_PUBLIC void *GpuArray_context(const GpuArray *a);

/**
 * Copies all the elements of and array to another.
 *
 * The arrays `src` and `dst` must have the same size (total number of
 * elements) and be in the same context.
 *
 * \param dst destination array
 * \param src source array
 *
 * \return GA_NO_ERROR if the operation was succesful.
 * \return an error code otherwise
 */
COMPYTE_PUBLIC int GpuArray_move(GpuArray *dst, const GpuArray *src);

/**
 * Copy data from the host memory to the device memory.
 *
 * \param dst destination array (must be contiguous)
 * \param src source host memory (contiguous block)
 * \param src_sz size of data to copy (in bytes)
 *
 * \return GA_NO_ERROR if the operation was succesful.
 * \return an error code otherwise
 */
COMPYTE_PUBLIC int GpuArray_write(GpuArray *dst, const void *src,
                                  size_t src_sz);

/**
 * Copy data from the device memory to the host memory.
 *
 * \param dst dstination host memory (contiguous block)
 * \param dst_sz size of data to copy (in bytes)
 * \param src source array (must be contiguous)
 *
 * \return GA_NO_ERROR if the operation was succesful.
 * \return an error code otherwise
 */
COMPYTE_PUBLIC int GpuArray_read(void *dst, size_t dst_sz,
                                 const GpuArray *src);

/**
 * Set all of an array's data to a byte pattern.
 *
 * \param a an array (must be contiguous)
 * \param data the byte to repeat
 *
 * \return GA_NO_ERROR if the operation was succesful.
 * \return an error code otherwise
 */
COMPYTE_PUBLIC int GpuArray_memset(GpuArray *a, int data);

/**
 * Make a copy of an array.
 *
 * This is analogue to GpuArray_view() except it copies the device
 * memory and no data is shared.
 *
 * \return GA_NO_ERROR if the operation was succesful.
 * \return an error code otherwise
 */
COMPYTE_PUBLIC int GpuArray_copy(GpuArray *res, const GpuArray *a,
                                 ga_order order);

/**
 * Get a description of the last error in the context of `a`.
 *
 * The description may reflect operations with other arrays in the
 * same context if other operations were performed between the
 * occurence of the error and the call to this function.
 *
 * Operations in other contexts, however have no incidence on the
 * return value.
 *
 * \param a an array
 * \param err the error code returned
 *
 * \returns A user-readable string describing the nature of the error.
 */
COMPYTE_PUBLIC const char *GpuArray_error(const GpuArray *a, int err);

/**
 * Print a textual description of `a` to the specified file
 * descriptor.
 *
 * \param fd a file descriptior open for writing
 * \param a an array
 */
COMPYTE_PUBLIC void GpuArray_fprintf(FILE *fd, const GpuArray *a);

#ifdef __cplusplus
}
#endif

#endif
