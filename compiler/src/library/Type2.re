/**
 Definition of types and interfaces that exist in the Knot language.
 */
open Infix;
open Reference;

type t =
  | Nil
  | Boolean
  | Integer
  | Float
  | String
  | Element
  | Iterable(t)
  | Structural(list((string, t)))
  | Function(list((string, t)), t)
  | Abstract(trait_t)

and trait_t =
  /* used to represent a type with no traits */
  | Unknown
  /* used to describe floats and integers */
  | Numeric;
