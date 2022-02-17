open Infix;
open Reference;

module Error = {
  type t('a) =
    | NotFound(Identifier.t)
    | TypeMismatch('a, 'a)
    | NotNarrowable('a, 'a)
    | ExternalNotFound(Namespace.t, Export.t)
    | DuplicateIdentifier(Identifier.t);

  /* pretty printing */

  let pp = (pp_type: Fmt.t('a)): Fmt.t(t('a)) =>
    Fmt.(
      ppf =>
        fun
        | NotFound(id) => pf(ppf, "NotFound<%a>", Identifier.pp, id)
        | DuplicateIdentifier(id) =>
          pf(ppf, "DuplicateIdentifier<%a>", Identifier.pp, id)
        | NotNarrowable(lhs, rhs) =>
          pf(ppf, "NotNarrowable<%a, %a>", pp_type, lhs, pp_type, rhs)
        | TypeMismatch(lhs, rhs) =>
          pf(ppf, "TypeMismatch<%a, %a>", pp_type, lhs, pp_type, rhs)
        | ExternalNotFound(namespace, id) =>
          pf(
            ppf,
            "ExternalNotFound<%a#%a>",
            Namespace.pp,
            namespace,
            Export.pp,
            id,
          )
    );
};

module Primitive = {
  type t = [ | `Nil | `Boolean | `Integer | `Float | `String | `Element];

  /* pretty printing */

  let pp: Fmt.t(t) =
    Fmt.(
      (ppf, type_) =>
        Constants.(
          switch (type_) {
          | `Nil => Keyword.nil
          | `Boolean => "bool"
          | `Integer => "int"
          | `Float => "float"
          | `String => "string"
          | `Element => "element"
          }
        )
        |> string(ppf)
    );
};

module Container = {
  type t('a) = [
    | `List('a)
    | `Struct(list((string, 'a)))
    | `Function(list((string, 'a)), 'a)
  ];

  let pp_list = (pp_type: Fmt.t('a)): Fmt.t('a) =>
    Fmt.(ppf => pf(ppf, "List<%a>", pp_type));

  let pp_props = (pp_type: Fmt.t('a)): Fmt.t((string, 'a)) =>
    (ppf, (key, type_)) => Fmt.pf(ppf, "%s: %a", key, pp_type, type_);

  let pp_struct = (pp_type: Fmt.t('a)): Fmt.t(list((string, 'a))) =>
    Fmt.(
      (ppf, props) =>
        List.is_empty(props)
          ? string(ppf, "{}")
          : pf(
              ppf,
              "@[<h>{ %a }@]",
              list(~sep=comma, pp_props(pp_type)),
              props,
            )
    );

  let pp_function = (pp_type: Fmt.t('a)): Fmt.t((list((string, 'a)), 'a)) =>
    Fmt.(
      (ppf, (args, res)) =>
        pf(
          ppf,
          "@[<h>Function<(%a), %a>@]",
          list(~sep=comma, pp_props(pp_type)),
          args,
          pp_type,
          res,
        )
    );
};

module Raw = {
  type unknown_t = [ | `Unknown];

  /**
   this type is used during the initial parsing phase to allow early type inference
   this also avoid needing more than 2 type-safe AST variants to throughout compilation
   */
  type t =
    | Valid(valid_t)
    | Invalid(invalid_t)

  and valid_t = [ Primitive.t | Container.t(t) | unknown_t]

  and invalid_t = Error.t(t);

  let rec pp: Fmt.t(t) =
    Fmt.(
      (ppf, type_) =>
        switch (type_) {
        | Valid(t) => pp_valid(ppf, t)
        | Invalid(err) => Error.pp(pp, ppf, err)
        }
    )

  and pp_valid: Fmt.t(valid_t) =
    (ppf, type_) =>
      switch (type_) {
      | (`Nil | `Boolean | `Integer | `Float | `String | `Element) as x =>
        Primitive.pp(ppf, x)

      | `List(t) => Container.pp_list(pp, ppf, t)

      | `Struct(props) => Container.pp_struct(pp, ppf, props)

      | `Function(args, res) => Container.pp_function(pp, ppf, (args, res))

      | `Unknown => Fmt.string(ppf, "Unknown")
      };
};

/**
 the final type attributed to elements within the fully-typed AST
 */
type t =
  | Valid(valid_t)
  | Invalid(invalid_t)

and valid_t = [ Primitive.t | Container.t(t)]

and invalid_t = Error.t(t);

/* methods */

let rec pp: Fmt.t(t) =
  ppf =>
    fun
    | Valid(t) => pp_valid(ppf, t)
    | Invalid(err) => Error.pp(pp, ppf, err)

and pp_valid: Fmt.t(valid_t) =
  ppf =>
    fun
    | (`Nil | `Boolean | `Integer | `Float | `String | `Element) as t =>
      Primitive.pp(ppf, t)
    | `List(t) => Container.pp_list(pp, ppf, t)
    | `Struct(props) => Container.pp_struct(pp, ppf, props)
    | `Function(args, res) => Container.pp_function(pp, ppf, (args, res));
