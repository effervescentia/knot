open Infix;
open Reference;

type factory_t = {
  seed: ref(int),
  create: (~parent: t=?, Range.t) => t,
}
and t = {
  factory: factory_t,
  weak: unit => Type.Raw.t,
  namespace: Namespace.t,
  range: Range.t,
  parent: option(t),
  types: Hashtbl.t(string, Type.Raw.t),
  /* error reporting callback */
  report: Error.compile_err => unit,
  mutable children: list(t),
};

/* static */

let factory = (namespace: Namespace.t, report: Error.compile_err => unit) => {
  let seed = ref(0);
  let weak_seed = ref(0);

  let create_memo =
    Fun.memo(
      ((id, _, _, _)) => id,
      ((_, factory, parent, range): (int, factory_t, option(t), Range.t)) => {
        let weak = () => {
          let type_ = Type.Raw.Weak(ref(Ok(`Generic(weak_seed^))));

          weak_seed := weak_seed^ + 1;

          type_;
        };

        let scope = {
          factory,
          weak,
          namespace,
          range,
          parent,
          report,
          types: Hashtbl.create(0),
          children: [],
        };

        seed := seed^ + 1;

        scope;
      },
    );
  let rec create_ = (~parent: option(t)=?, range: Range.t) =>
    create_memo((seed^, {seed, create: create_}, parent, range));

  {seed, create: create_};
};

/* methods */

/**
 check that no weak types are scheduled with this or any child scope
 */
let is_resolved = (scope: t) =>
  scope.types
  |> Hashtbl.to_seq_values
  |> List.of_seq
  |> List.for_all(
       fun
       | Type.Raw.Weak(_) => false
       | _ => true,
     );

let peek = (resolve: unit => 'a, scope: t) => {
  let seed = scope.factory.seed^;
  let result = resolve();

  if (is_resolved(scope)) {
    Some(result);
  } else {
    scope.factory.seed := seed;

    None;
  };
};

let child = (parent: t, range: Range.t): t => {
  let scope = parent.factory.create(~parent, range);

  parent.children = parent.children @ [scope];

  scope;
};

let rec exists = (name: string, scope: t): bool =>
  Hashtbl.mem(scope.types, name)
  || (
    switch (scope.parent) {
    | Some(parent) => parent |> exists(name)
    | _ => false
    }
  );

let rec resolve =
        ((name, range) as id: Node.Raw.t(Identifier.t), scope: t): Type.t =>
  switch (
    Hashtbl.find_opt(
      scope.types,
      id |> Node.Raw.get_value |> Identifier.to_string,
    ),
    scope.parent,
  ) {
  | (Some(result), _) => Type.of_raw(result)
  | (_, Some(parent)) => parent |> resolve(id)
  | _ =>
    let type_err = Type.Error.NotFound(name);

    scope.report(ParseError(TypeError(type_err), scope.namespace, range));

    Invalid(type_err);
  };

let _safe_define =
    ((name, range): Node.Raw.t(Identifier.t), type_: Type.Raw.t, scope: t) => {
  let key = Identifier.to_string(name);

  (
    if (scope |> exists(key)) {
      let type_err = Type.Error.DuplicateIdentifier(name);

      scope.report(ParseError(TypeError(type_err), scope.namespace, range));

      Type.Raw.Invalid(type_err);
    } else {
      type_;
    }
  )
  |> Hashtbl.replace(scope.types, key);
};

let define = (id: Node.Raw.t(Identifier.t), type_: Type.Raw.t, scope: t) =>
  _safe_define(id, type_, scope);

/* let define_weak =
     (~trait=Type.Trait.Unknown, id: Node.Raw.t(Identifier.t), scope: t) =>
   _safe_define(id, Type.Raw.Weak(ref(Ok(`Abstract(trait)))), scope); */

let finalize = (scope: t) =>
  scope.types
  |> Hashtbl.to_seq
  |> Seq.map(((key, value)) => (key, value))
  |> Seq.map(
       fun
       /* | (key, Type.Raw.Weak(weak)) => (key, Type.Raw.) */
       | x => x,
     )
  |> Hashtbl.replace_seq(scope.types);
