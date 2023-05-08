open Knot.Kore;

module Namespace = Reference.Namespace;

type type_lookup_t = Hashtbl.t(string, Type.t);

type event_t =
  | Lookup;
type handler_t = unit => option(Type.error_t);
type handler_lookup_t = Hashtbl.t(string, list((event_t, handler_t)));

type t('ast) = {
  range: Range.t,
  parent: option(t('ast)),
  mutable children: list(t('ast)),
  types: type_lookup_t,
  context: ParseContext.t('ast),
  handlers: handler_lookup_t,
};

/* static */

let rec _with_root = (f: t('a) => 'a, scope: t('a)): 'a =>
  switch (scope.parent) {
  | Some(parent) => _with_root(f, parent)
  | None => f(scope)
  };

let _get_handlers = (id: string, target_event: event_t, scope: t('a)) =>
  Hashtbl.find_opt(scope.handlers, id)
  |?: []
  |> List.filter_map(((event, handler)) =>
       event == target_event ? Some(handler) : None
     );

let add_handler =
    (id: string, event: event_t, handler: handler_t, scope: t('a)) => {
  let id_handlers = Hashtbl.find_opt(scope.handlers, id) |?: [];

  Hashtbl.replace(scope.handlers, id, [(event, handler), ...id_handlers]);
};

let create =
    (context: ParseContext.t('a), ~parent: option(t('a))=?, range: Range.t)
    : t('a) => {
  context,
  range,
  parent,
  children: [],
  types: Hashtbl.create(0),
  handlers:
    parent
    |> Option.map(parent' => Hashtbl.copy(parent'.handlers))
    |?: Hashtbl.create(0),
};

/**
 convert to a scope for use within a function or other closure
 */
let of_parse_context = (range: Range.t, ctx: ParseContext.t('a)): t('a) => {
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
let create_child = (range: Range.t, parent: t('a)): t('a) => {
  let child = create(~parent, parent.context, range);

  parent.children = parent.children @ [child];

  child;
};

/**
 find a type in this or any parent scope
 */
let rec lookup =
        (id: string, scope: t('a)): option(result(Type.t, Type.error_t)) => {
  switch (scope.parent, Hashtbl.find_opt(scope.types, id)) {
  | (_, Some(type_)) =>
    List.nth_opt(
      scope
      |> _get_handlers(id, Lookup)
      |> List.filter_map(handler => handler()),
      0,
    )
    |> (
      fun
      | Some(error') => Some(Error(error'))
      | None => Some(Ok(type_))
    )

  | (Some(parent), _) => parent |> lookup(id)

  | _ => None
  };
};

/**
 define a new type in this scope
 */
let define = (id: string, type_: Type.t, scope: t('a)): option(Type.error_t) => {
  let result = scope |> lookup(id) |?> (_ => Type.DuplicateIdentifier(id));

  Hashtbl.add(scope.types, id, type_);

  result;
};

let inject_plugin_types =
    (~prefix="$", plugin: Reference.Plugin.t, scope: t('a)) =>
  scope.context.modules.plugins
  |> List.assoc_opt(plugin)
  |> Option.iter(
       List.iter(
         fun
         | (Type.ModuleEntryKind.Value, id, type_) =>
           scope |> define(id |> Fmt.str("%s%s", prefix), type_) |> ignore
         | _ => (),
       ),
     );

/**
 create a new child scope augmented with the members of a plugin
 */
let create_augmented =
    (~prefix="$", plugin: Reference.Plugin.t, range: Range.t, parent: t('a))
    : t('a) => {
  let child = parent |> create_child(range);
  child |> inject_plugin_types(~prefix, plugin);

  child;
};

let report_type_err = (scope: t('a), range: Range.t, err: Type.error_t) =>
  scope.context |> ParseContext.report(TypeError(err), range);
