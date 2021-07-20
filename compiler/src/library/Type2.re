/**
 Definition of types and interfaces that exist in the Knot language.
 */
open Infix;
open Reference;

type trait_t =
  /* used to represent a type with no traits */
  | Unknown
  /* used to describe floats and integers */
  | Numeric;

type container_t('a) = [
  | `Iterable('a)
  | `Structural(list((string, 'a)))
  | `Function(list((string, 'a)), 'a)
  | `Abstract(trait_t)
];

type primitive_t = [
  | `Nil
  | `Boolean
  | `Integer
  | `Float
  | `String
  | `Element
];

type t = [ container_t(t) | primitive_t];

let _trait_to_string =
  fun
  | Numeric => "Numeric" |> Pretty.string
  | Unknown => "unknown" |> Pretty.string;

let rec _type_to_string: t => Pretty.t =
  fun
  | `Nil => "nil" |> Pretty.string
  | `Boolean => "bool" |> Pretty.string
  | `Integer => "int" |> Pretty.string
  | `Float => "float" |> Pretty.string
  | `String => "string" |> Pretty.string
  | `Element => "element" |> Pretty.string
  | `Iterable(t) =>
    ["Iterable<" |> Pretty.string, t |> _type_to_string, ">" |> Pretty.string]
    |> Pretty.concat
  | `Structural(_) => "Structural" |> Pretty.string
  | `Function(args, result) =>
    [
      "(" |> Pretty.string,
      args
      |> List.map(((name, type_)) =>
           [
             name |> Pretty.string,
             ": " |> Pretty.string,
             type_ |> _type_to_string,
           ]
           |> Pretty.concat
         )
      |> List.intersperse(", " |> Pretty.string)
      |> Pretty.concat,
      ") -> " |> Pretty.string,
      result |> _type_to_string,
    ]
    |> Pretty.concat
  | `Abstract(trait) =>
    [
      "Anonymous<" |> Pretty.string,
      trait |> _trait_to_string,
      ">" |> Pretty.string,
    ]
    |> Pretty.concat;

let to_string = _type_to_string % Pretty.to_string;
