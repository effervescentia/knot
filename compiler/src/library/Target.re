let javascript_es6 = "javascript-es6";
let javascript_common = "javascript-common";
let knot = "knot";

type module_t =
  | Common
  | ES6;

type t =
  | JavaScript(module_t)
  | Knot;

let extension_of =
  fun
  | JavaScript(_) => ".js"
  | Knot => Constants.file_extension;

let of_string =
  fun
  | x when x == javascript_es6 => Some(JavaScript(ES6))
  | x when x == javascript_common => Some(JavaScript(Common))
  | x when x == knot => Some(Knot)
  | _ => None;

let to_string =
  fun
  | JavaScript(ES6) => javascript_es6
  | JavaScript(Common) => javascript_common
  | Knot => knot;