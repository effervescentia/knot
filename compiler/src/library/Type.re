/**
 * Definition of types that exist in the Knot language.
 */

type t =
  | K_Nil
  | K_Boolean
  | K_Integer
  | K_Float
  | K_String
  | K_Element
  /* used to indicate types which have failed to resolve due to a compile-time error */
  | K_Invalid
  /* used as a placeholder while developing */
  | K_Unknown;
