open Infix;
open Reference;

exception UnknownTypeEncountered;

module Primitive = {
  type t = [ | `Nil | `Boolean | `Integer | `Float | `String | `Element];

  /* pretty printing */

  let pp: Fmt.t(t) =
    Fmt.(
      (ppf, type_) =>
        Constants.(
          switch (type_) {
          | `Nil => Keyword.nil
          | `Boolean => Keyword.boolean
          | `Integer => Keyword.integer
          | `Float => Keyword.float
          | `String => Keyword.string
          | `Element => Keyword.element
          }
        )
        |> string(ppf)
    );
};

module Container = {
  type t('a) = [
    | `List('a)
    | `Struct(list((string, 'a)))
    | `Function(list('a), 'a)
    | `View(list((string, 'a)), 'a)
  ];

  let pp_list = (pp_type: Fmt.t('a)): Fmt.t('a) =>
    Fmt.(ppf => pf(ppf, "%a[]", pp_type));

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
              list(~sep=Sep.comma, pp_props(pp_type)),
              props,
            )
    );

  let pp_function = (pp_type: Fmt.t('a)): Fmt.t((list('a), 'a)) =>
    Fmt.(
      (ppf, (args, res)) =>
        pf(
          ppf,
          "@[<h>(%a) -> %a@]",
          list(~sep=Sep.comma, pp_type),
          args,
          pp_type,
          res,
        )
    );

  let pp_view = (pp_type: Fmt.t('a)): Fmt.t((list((string, 'a)), 'a)) =>
    Fmt.(
      (ppf, (props, res)) =>
        pf(
          ppf,
          "@[<h>View<(%a), %a>@]",
          list(~sep=Sep.comma, pp_props(pp_type)),
          props,
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
  type t = [ Primitive.t | Container.t(t) | unknown_t];

  let rec pp: Fmt.t(t) =
    (ppf, type_) =>
      switch (type_) {
      | (`Nil | `Boolean | `Integer | `Float | `String | `Element) as x =>
        Primitive.pp(ppf, x)

      | `List(t) => Container.pp_list(pp, ppf, t)

      | `Struct(props) => Container.pp_struct(pp, ppf, props)

      | `Function(args, res) => Container.pp_function(pp, ppf, (args, res))

      | `View(props, res) => Container.pp_view(pp, ppf, (props, res))

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

and invalid_t =
  | NotInferrable;

type error_t =
  | NotFound(Identifier.t)
  | ExternalNotFound(Namespace.t, Export.t)
  | DuplicateIdentifier(Identifier.t)
  | TypeMismatch(t, t)
  | InvalidUnaryOperation(AST_Operator.unary_t, t)
  | InvalidBinaryOperation(AST_Operator.binary_t, t, t)
  | InvalidJSXPrimitiveExpression(t)
  | InvalidJSXClassExpression(t)
  | UntypedFunctionArgument(Identifier.t)
  | DefaultArgumentMissing(Identifier.t);

/* pretty printing */

let rec pp: Fmt.t(t) =
  ppf =>
    fun
    | Valid(t) => pp_valid(ppf, t)
    | Invalid(err) => pp_invalid(ppf, err)

and pp_valid: Fmt.t(valid_t) =
  ppf =>
    fun
    | (`Nil | `Boolean | `Integer | `Float | `String | `Element) as t =>
      Primitive.pp(ppf, t)
    | `List(t) => Container.pp_list(pp, ppf, t)
    | `Struct(props) => Container.pp_struct(pp, ppf, props)
    | `Function(args, res) => Container.pp_function(pp, ppf, (args, res))
    | `View(args, res) => Container.pp_view(pp, ppf, (args, res))

and pp_invalid: Fmt.t(invalid_t) =
  ppf =>
    fun
    | NotInferrable => Fmt.pf(ppf, "NotInferrable");

let pp_error: Fmt.t(error_t) =
  Fmt.(
    ppf =>
      fun
      | NotFound(id) => pf(ppf, "NotFound<%a>", Identifier.pp, id)

      | DuplicateIdentifier(id) =>
        pf(ppf, "DuplicateIdentifier<%a>", Identifier.pp, id)

      | UntypedFunctionArgument(id) =>
        pf(ppf, "UntypedFunctionArgument<%a>", Identifier.pp, id)

      | ExternalNotFound(namespace, id) =>
        pf(
          ppf,
          "ExternalNotFound<%a#%a>",
          Namespace.pp,
          namespace,
          Export.pp,
          id,
        )

      | TypeMismatch(lhs, rhs) =>
        pf(ppf, "TypeMismatch<%a, %a>", pp, lhs, pp, rhs)

      | InvalidUnaryOperation(op, type_) =>
        pf(
          ppf,
          "InvalidUnaryOperation<%s, %a>",
          AST_Operator.Dump.unary_to_string(op),
          pp,
          type_,
        )

      | InvalidBinaryOperation(op, lhs, rhs) =>
        pf(
          ppf,
          "InvalidBinaryOperation<%s, %a, %a>",
          AST_Operator.Dump.binary_to_string(op),
          pp,
          lhs,
          pp,
          rhs,
        )

      | InvalidJSXPrimitiveExpression(type_) =>
        pf(ppf, "InvalidJSXPrimitiveExpression<%a>", pp, type_)

      | InvalidJSXClassExpression(type_) =>
        pf(ppf, "InvalidJSXClassExpression<%a>", pp, type_)

      | DefaultArgumentMissing(id) =>
        pf(ppf, "DefaultArgumentMissing<%a>", Identifier.pp, id)
  );

let rec of_raw = (raw_type: Raw.t): t =>
  switch (raw_type) {
  | (`Nil | `Integer | `Float | `Boolean | `String | `Element) as t =>
    Valid(t)

  | `List(t) => Valid(`List(of_raw(t)))

  | `Struct(ts) => Valid(`Struct(ts |> List.map(Tuple.map_snd2(of_raw))))

  | `Function(args, res) =>
    Valid(`Function((args |> List.map(of_raw), of_raw(res))))

  | `View(props, res) =>
    Valid(`View((props |> List.map(Tuple.map_snd2(of_raw)), of_raw(res))))

  | `Unknown => raise(UnknownTypeEncountered)
  };
