
#ifdef EVIL_COROUTINE
  // This lovely hack makes use of switch statements,
  // And the __LINE__ C macro
  // It tracks the current state, and switches case.
  // But... I imagine awful things may happen with an extra semi-colon.
  // Which would be hard to debug.
  #if defined(EVIL_LAMBDA) && !defined(EVIL_NO_WARN)
    // And bad things happen with expression statements.
    #warning "Lambda's don't play well with Coroutines. Avoid using them in the body of a coroutine."
  #endif
  #ifndef EVIL_NO_WARN
    #warning "Coroutine's don't support nesting. It may work sometimes, other times it may explode."
  #endif

  // Original macro hack by Robert Elder (c) 2016. Used against their advice, but with their permission.
  #define coroutine() static int state=0; switch(state) { case 0:
  #define co_return(x) { state=__LINE__; return x; case __LINE__:; }
  #define co_end() }
#endif