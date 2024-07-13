
#ifndef EVIL_NO_PROC
  // Included by default

  #define declare(_name, _ret, ...) _ret _name(__VA_ARGS__)
  #define proc(_name, _ret, ...) _ret _name(__VA_ARGS__){
#endif