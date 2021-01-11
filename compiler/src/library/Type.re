type t =
  | K_Nil
  | K_Boolean
  | K_Integer
  | K_Float
  | K_String
  /* used to indicate types which have failed to resolve due to a compile-time error */
  | K_Invalid;
