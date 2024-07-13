
#ifdef EVIL_LAMBDA
  // This requires nested functions to be allowed.
  // Only GCC supports it.
  // ... Unconfirmed if Clang does. It might.
  #if defined(__clang__) || !defined(__GNUC__)
    #error "Lambda requires a GNU compiler."
  #endif
  // A cleaner, but slightly more cumbersome lambda:
  #define lambda(ret_type, _body) ({ ret_type _ _body _; })
  // e.g. int (*max)(int, int) = lambda (int, (int x, int y) { return x > y ? x : y; });
  // Pros:
  // * Woot, easier to pass, as the user has to know the signature anyway.
  // * Name not part of lambda definition. More lambda-y.
  // * Body of function inside macro, feels more like a lambda.
  // * Uses expression disgnator (GCC-only), which creates a properly constructed function pointer.
  // * It *may* work under Clang too!
  // Cons:
  // * The signature isn't constructed for the user, they have to both know and understand it.
#endif