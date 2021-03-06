/**
 * Utitlities for converting module AST into formatted Knot code.
 */
include Knot.Kore;

module Target = {
  type module_t =
    | Common
    | ES6;

  type t =
    | JavaScript(module_t)
    | Knot;
};
