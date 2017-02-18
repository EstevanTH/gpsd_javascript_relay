// Configuration (comment / uncomment):
#define SUPPORT_SNPRINTF

// Prepare macros:
#ifdef SUPPORT_SNPRINTF
	#define COMPAT_SNPRINTF( s, n, format ) snprintf( ( s ), ( n ), ( format ),
#else
	#define COMPAT_SNPRINTF( s, n, format ) sprintf( ( s ), ( format ),
#endif
