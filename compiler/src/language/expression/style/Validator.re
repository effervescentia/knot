open Knot.Kore;
open AST;

let validate_style_rule =
    (name): (((Type.t, Type.t)) => option(Type.error_t)) =>
  fun
  /* assume this has been reported already and ignore */
  | (_, Invalid(_))
  | (Invalid(_), _) => None

  | (arg_type, value_type) when arg_type == value_type => None

  /* special override for raw string styles */
  | (_, Valid(`String)) => None

  | (expected_type, actual_type) =>
    Some(InvalidStyleRule(name, expected_type, actual_type));
