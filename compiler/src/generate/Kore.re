include Knot.Kore;
include AST;

type resolve_t = Reference.Namespace.t => string;

module Target = {
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
};
