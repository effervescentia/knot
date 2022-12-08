open Knot.Kore;

module Namespace = Reference.Namespace;

type type_lookup_t = Hashtbl.t(string, Type.t);

type t = {
  range: Range.t,
  parent: option(t),
  mutable children: list(t),
  types: type_lookup_t,
  context: ParseContext.t,
};

/* static */

let rec _with_root = (f: t => 'a, scope: t): 'a =>
  switch (scope.parent) {
  | Some(parent) => _with_root(f, parent)
  | None => f(scope)
  };

let create =
    (context: ParseContext.t, ~parent: option(t)=?, range: Range.t): t => {
  context,
  range,
  parent,
  children: [],
  types: Hashtbl.create(0),
};

/**
 convert to a scope for use within a function or other closure
 */
let of_parse_context = (range: Range.t, ctx: ParseContext.t): t => {
  let types =
    ctx.symbols.imported.values
    @ ctx.symbols.declared.values
    |> List.to_seq
    |> Hashtbl.of_seq;

  {...create(ctx, range), types};
};

/* methods */

/**
 create a new child scope from a parent scope and register them with each other
 */
let create_child = (range: Range.t, parent: t): t => {
  let child = create(~parent, parent.context, range);

  parent.children = parent.children @ [child];

  child;
};

/**
 find a type in this or any parent scope
 */
let rec lookup = (id: string, scope: t): option(Type.t) => {
  switch (scope.parent, Hashtbl.find_opt(scope.types, id)) {
  | (_, Some(type_)) => Some(type_)

  | (Some(parent), _) => parent |> lookup(id)

  | _ => None
  };
};

/**
 define a new type in this scope
 */
let define = (id: string, type_: Type.t, scope: t): option(Type.error_t) => {
  let result = scope |> lookup(id) |?> (_ => Type.DuplicateIdentifier(id));

  Hashtbl.add(scope.types, id, type_);

  result;
};

let inject_plugin_types = (~prefix="$", plugin: Reference.Plugin.t, scope: t) =>
  scope.context.modules.plugins
  |> List.assoc_opt(plugin)
  |> Option.iter(
       List.iter(
         fun
         | (id, Type.Container.Value(type_)) =>
           scope |> define(id |> Fmt.str("%s%s", prefix), type_) |> ignore
         | _ => (),
       ),
     );

let report_type_err = (scope: t, range: Range.t, err: Type.error_t) =>
  scope.context |> ParseContext.report(TypeError(err), range);