#ifndef __STDARG_H__
#define __STDARG_H__

#ifndef __GNUC__

typedef void* va_list;

#define __va_size( type )           \
  ( ( sizeof( type ) + 3 ) & ~0x3 ) \

#define va_start( va_l, last )                            \
  ( ( va_l ) = ( void * )&( last ) + __va_size( last ) )  \

#define va_end( va_l )

#define va_arg( va_l, type )                        \
  ( ( va_l ) += __va_size( type ),                  \
  *( ( type * )( ( va_l ) - __va_size( type ) ) ) ) \

#else

typedef __builtin_va_list va_list;
#define va_start __builtin_va_start
#define va_end __builtin_va_end
#define va_arg __builtin_va_arg

#endif

#endif