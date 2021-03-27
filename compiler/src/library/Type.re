/**
 Definition of types and interfaces that exist in the Knot language.
 */
open Infix;

type strong_t =
  | K_Nil
  | K_Boolean
  | K_Integer
  | K_Float
  | K_String
  | K_Element
  | K_Anonymous(int, trait_t)

/**
 describes constraints on a type during type resolution
 */
and trait_t =
  | K_Unknown
  /* used to describe functions */
  | K_Callable(list(trait_t), trait_t)
  /* used to describe iterable structures */
  | K_Iterable(trait_t)
  /* used to describe floats and integers */
  | K_Numeric
  /* used to describe structural interfaces */
  | K_Structural(list((string, trait_t)))
  /* used to describe exact types, should only be used while parsing and be hardened to the wrapped type */
  | K_Exactly(strong_t);

type t =
  | K_Strong(strong_t)
  | K_Weak(int)
  /* used to indicate types which have failed to resolve due to a compile-time error */
  | K_Invalid(type_err)
and type_err =
  | TraitConflict(trait_t, trait_t)
  | NotAssignable(t, trait_t)
  | TypeMismatch(t, t)
  | NotFound(Reference.Identifier.t)
  | ExternalNotFound(Reference.Namespace.t, Reference.Identifier.t);

let restrict = (lhs: trait_t, rhs: trait_t): option(trait_t) =>
  switch (lhs, rhs) {
  /* placeholder for other first-class types */
  | (K_Callable(_), K_Callable(_))
  | (K_Iterable(_), K_Iterable(_))
  | (K_Structural(_), K_Structural(_)) => raise(Error.NotImplemented)

  | (l, r) when l == r => Some(l)

  | (K_Exactly(l), K_Exactly(r)) when l == r => Some(K_Exactly(l))

  | (K_Numeric, K_Exactly(K_Integer))
  | (K_Exactly(K_Integer), K_Numeric) => Some(K_Exactly(K_Integer))

  | (K_Numeric, K_Exactly(K_Float))
  | (K_Exactly(K_Float), K_Numeric) => Some(K_Exactly(K_Float))

  | _ => None
  };

let generalize = (lhs: trait_t, rhs: trait_t): option(trait_t) =>
  switch (lhs, rhs) {
  /* placeholder for other first-class types */
  | (K_Callable(_), K_Callable(_))
  | (K_Iterable(_), K_Iterable(_))
  | (K_Structural(_), K_Structural(_)) => raise(Error.NotImplemented)

  | (l, r) when l == r => Some(l)

  | (K_Exactly(l), K_Exactly(r)) when l == r => Some(K_Exactly(l))

  | (K_Numeric, K_Exactly(K_Integer | K_Float))
  | (K_Exactly(K_Integer | K_Float), K_Numeric)
  | (K_Exactly(K_Integer | K_Float), K_Exactly(K_Integer | K_Float)) =>
    Some(K_Numeric)

  | _ => None
  };

let rec _strong_to_string =
  fun
  | K_Nil => "nil" |> Pretty.string
  | K_Boolean => "bool" |> Pretty.string
  | K_Integer => "int" |> Pretty.string
  | K_Float => "float" |> Pretty.string
  | K_String => "string" |> Pretty.string
  | K_Element => "element" |> Pretty.string
  | K_Anonymous(id, trait) =>
    [
      "Anonymous<" |> Pretty.string,
      id |> string_of_int |> Pretty.string,
      trait |> _trait_to_string,
      ">" |> Pretty.string,
    ]
    |> Pretty.concat
and _trait_to_string =
  fun
  | K_Numeric => "Numeric" |> Pretty.string
  | K_Callable(arguments, result) =>
    [
      "Callable<(" |> Pretty.string,
      arguments
      |> List.map(_trait_to_string)
      |> List.intersperse(", " |> Pretty.string)
      |> Pretty.concat,
      "), " |> Pretty.string,
      result |> _trait_to_string,
      ">" |> Pretty.string,
    ]
    |> Pretty.concat
  | K_Iterable(t) =>
    [
      "Iterable<" |> Pretty.string,
      t |> _trait_to_string,
      ">" |> Pretty.string,
    ]
    |> Pretty.concat
  | K_Structural(_) => "Structural" |> Pretty.string
  | K_Exactly(t) => t |> _strong_to_string
  | K_Unknown => "unknown" |> Pretty.string;

let rec _type_to_string =
  fun
  | K_Invalid(err) =>
    ["Invalid<" |> Pretty.string, err |> _err_to_string, ">" |> Pretty.string]
    |> Pretty.concat

  | K_Weak(id) =>
    [
      "Weak<" |> Pretty.string,
      id |> string_of_int |> Pretty.string,
      ">" |> Pretty.string,
    ]
    |> Pretty.concat
  | K_Strong(t) => t |> _strong_to_string

and _err_to_string =
  fun
  | TraitConflict(x, y) =>
    [
      "TraitConflict<" |> Pretty.string,
      x |> _trait_to_string,
      ", " |> Pretty.string,
      y |> _trait_to_string,
      ">" |> Pretty.string,
    ]
    |> Pretty.concat
  | NotAssignable(x, y) =>
    [
      "NotAssignable<" |> Pretty.string,
      x |> _type_to_string,
      ", " |> Pretty.string,
      y |> _trait_to_string,
      ">" |> Pretty.string,
    ]
    |> Pretty.concat
  | TypeMismatch(x, y) =>
    [
      "TypeMismatch<" |> Pretty.string,
      x |> _type_to_string,
      ", " |> Pretty.string,
      y |> _type_to_string,
      ">" |> Pretty.string,
    ]
    |> Pretty.concat
  | NotFound(x) =>
    [
      "NotFound<" |> Pretty.string,
      x |> Reference.Identifier.to_string |> Pretty.string,
      ">" |> Pretty.string,
    ]
    |> Pretty.concat
  | ExternalNotFound(namespace, id) =>
    [
      "ExternalNotFound<" |> Pretty.string,
      namespace |> Reference.Namespace.to_string |> Pretty.string,
      id |> Reference.Identifier.to_string |> Pretty.string,
      ">" |> Pretty.string,
    ]
    |> Pretty.concat;

let to_string = _type_to_string % Pretty.to_string;
let trait_to_string = _trait_to_string % Pretty.to_string;
