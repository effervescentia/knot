include Knot.Core;

module NestedHashtbl = Knot.NestedHashtbl;

type linked_module =
  | NotLoaded(list(unit => unit))
  | Loaded(Knot.AST.ast_module);

let analyze_list = analyze => List.map(analyze % await_ctx);
