#if !defined(FETCH_CONTEXT) || !defined(PREFIX) || !defined(ARRAY) || !defined(POST_CALL)
#error "required macros not defined"
#endif

#if !defined(ORDER) && (!defined(HANDLE_ORDER1))
#error "you need to handle order somehow"
#endif

#ifndef ORDER
#define ORDER
#endif

#ifndef PREP_ORDER1
#define PREP_ORDER1(transA, M, N, A, lda)
#endif

#ifndef HANDLE_ORDER1
#define HANDLE_ORDER1(order, transA, M, N, A, lda)
#endif

#ifndef INIT_ARGS
#define INIT_ARGS
#endif

#ifndef TRAIL_ARGS
#define TRAIL_ARGS
#endif

#ifndef SZ
#define SZ(a) a
#endif

#ifndef TRANS
#define TRANS(t) t
#endif

#ifndef SCAL
#define SCAL(s) s
#endif

#ifndef FUNC_INIT
#define FUNC_INIT
#endif

#ifndef FUNC_FINI
#define FUNC_FINI
#endif

#define __GLUE(part1, part2) __GLUE_INT(part1, part2)
#define __GLUE_INT(part1, part2) part1 ## part2

#define GEMV(dtype, typec, TYPEC)					\
  static int typec ## gemv(const cb_order order,			\
			   const cb_transpose transA,			\
			   const size_t M,				\
			   const size_t N,				\
			   const dtype alpha,				\
			   gpudata *A,					\
			   const size_t offA,				\
			   const size_t lda,				\
			   gpudata *X,					\
			   const size_t offX,				\
			   const int incX,				\
			   const dtype beta,				\
			   gpudata *Y,					\
			   const size_t offY,				\
			   const int incY) {				\
    FETCH_CONTEXT(A);							\
    FUNC_DECLS;								\
    PREP_ORDER1(transA, M, N, A, lda);					\
									\
    HANDLE_ORDER1(order, transA, M, N, A, lda);				\
									\
    FUNC_INIT;								\
									\
    ARRAY_INIT(A);							\
    ARRAY_INIT(X);							\
    ARRAY_INIT(Y);							\
									\
    PRE_CALL __GLUE(PREFIX(typec, TYPEC), gemv)(INIT_ARGS ORDER TRANS(transA), SZ(M), SZ(N), SCAL(alpha), ARRAY(A, dtype), SZ(lda), ARRAY(X, dtype), incX, SCAL(beta), ARRAY(Y, dtype), incY TRAIL_ARGS); \
    POST_CALL;								\
									\
    ARRAY_FINI(A);							\
    ARRAY_FINI(X);							\
    ARRAY_FINI(Y);							\
									\
    FUNC_FINI;								\
									\
    return GA_NO_ERROR;							\
  }

GEMV(float, s, S)

GEMV(double, d, D)

COMPYTE_LOCAL compyte_blas_ops __GLUE(NAME, _ops) = {
  setup,
  teardown,
  sgemv,
  dgemv,
};
