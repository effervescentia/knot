include Knot.Kore;
include AST;

type print_t = string => unit;

type output_t = {
  print: string => unit,
  resolve: namespace_t => string,
};

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
