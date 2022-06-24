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
  type module_entry_t('a) =
    | Type('a)
    | Value('a);

  type t('a) = [
    | `List('a)
    | `Struct(list((string, 'a)))
    | `Enumerated(list((string, list('a))))
    | `Function(list('a), 'a)
    /* entities */
    | `View(list((string, 'a)), 'a)
    | `Style(list('a), list(string), list(string))
    | `Module(list((string, module_entry_t('a))))
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

  let pp_enumerated =
      (pp_type: Fmt.t('a)): Fmt.t(list((string, list('a)))) =>
    Fmt.(
      (ppf, variants) =>
        List.is_empty(variants)
          ? string(ppf, "|")
          : pf(
              ppf,
              "@[<h>%a@]",
              list(~sep=Sep.newline, (ppf, (id, args)) =>
                List.is_empty(args)
                  ? string(ppf, id)
                  : pf(ppf, "| %s(%a)", id, list(pp_type), args)
              ),
              variants,
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

  let pp_style =
      (pp_type: Fmt.t('a))
      : Fmt.t((list('a), list(string), list(string))) =>
    Fmt.(
      (ppf, (args, ids, classes)) =>
        pf(
          ppf,
          "@[<h>Style<(%a), %a, %a>@]",
          list(~sep=Sep.comma, pp_type),
          args,
          list(string),
          ids,
          list(string),
          classes,
        )
    );

  let pp_module =
      (pp_type: Fmt.t('a)): Fmt.t(list((string, module_entry_t('a)))) =>
    Fmt.(
      (ppf, entries) =>
        pf(
          ppf,
          "@[<h>Module<%a>@]",
          record(string, ppf =>
            fun
            | Type(t) => pf(ppf, "type %a", pp_type, t)
            | Value(t) => pp_type(ppf, t)
          ),
          entries,
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

      | `Enumerated(variants) => Container.pp_enumerated(pp, ppf, variants)

      | `Function(args, res) => Container.pp_function(pp, ppf, (args, res))

      | `View(props, res) => Container.pp_view(pp, ppf, (props, res))

      | `Style(args, ids, classes) =>
        Container.pp_style(pp, ppf, (args, ids, classes))

      | `Module(entries) => Container.pp_module(pp, ppf, entries)

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
  | NotFound(string)
  | ExternalNotFound(Namespace.t, Export.t)
  /* FIXME: not reported */
  | DuplicateIdentifier(string)
  /* FIXME: not reported */
  | TypeMismatch(t, t)
  | InvalidUnaryOperation(AST_Operator.unary_t, t)
  | InvalidBinaryOperation(AST_Operator.binary_t, t, t)
  | InvalidJSXPrimitiveExpression(t)
  | InvalidJSXClassExpression(t)
  | InvalidJSXTag(string, t, list((string, t)))
  | UnexpectedJSXAttribute(string, t)
  | InvalidJSXAttribute(string, t, t)
  | MissingJSXAttributes(string, list((string, t)))
  | InvalidDotAccess(t, string)
  | InvalidFunctionCall(t, list(t))
  /* FIXME: not reported */
  | UntypedFunctionArgument(string)
  /* FIXME: not reported */
  | DefaultArgumentMissing(string);

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
    | `Enumerated(variants) => Container.pp_enumerated(pp, ppf, variants)
    | `Function(args, res) => Container.pp_function(pp, ppf, (args, res))
    | `View(args, res) => Container.pp_view(pp, ppf, (args, res))
    | `Style(args, ids, classes) =>
      Container.pp_style(pp, ppf, (args, ids, classes))
    | `Module(entries) => Container.pp_module(pp, ppf, entries)

and pp_invalid: Fmt.t(invalid_t) =
  ppf =>
    fun
    | NotInferrable => Fmt.pf(ppf, "NotInferrable");

let pp_error: Fmt.t(error_t) =
  Fmt.(
    ppf =>
      fun
      | NotFound(id) => pf(ppf, "NotFound<%s>", id)

      | DuplicateIdentifier(id) => pf(ppf, "DuplicateIdentifier<%s>", id)

      | UntypedFunctionArgument(id) =>
        pf(ppf, "UntypedFunctionArgument<%s>", id)

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

      | InvalidDotAccess(type_, prop) =>
        pf(ppf, "InvalidDotAccess<%a, %s>", pp, type_, prop)

      | InvalidFunctionCall(type_, expected_args) =>
        pf(
          ppf,
          "InvalidFunctionCall<%a, %a>",
          pp,
          type_,
          list(pp),
          expected_args,
        )

      | InvalidJSXTag(id, type_, expected_attrs) =>
        pf(
          ppf,
          "InvalidJSXTag<%s, %a, %a>",
          id,
          pp,
          type_,
          record(string, pp),
          expected_attrs,
        )

      | UnexpectedJSXAttribute(name, type_) =>
        pf(ppf, "UnexpectedJSXAttribute<%s, %a>", name, pp, type_)

      | InvalidJSXAttribute(name, expected_type, actual_type) =>
        pf(
          ppf,
          "InvalidJSXAttribute<%s, %a, %a>",
          name,
          pp,
          expected_type,
          pp,
          actual_type,
        )

      | MissingJSXAttributes(id, attrs) =>
        pf(
          ppf,
          "MissingJSXAttributes<%s, @[<hv>%a@]>",
          id,
          record(string, pp),
          attrs,
        )

      | DefaultArgumentMissing(id) =>
        pf(ppf, "DefaultArgumentMissing<%s>", id)
  );

let rec of_raw = (raw_type: Raw.t): t =>
  switch (raw_type) {
  | (`Nil | `Integer | `Float | `Boolean | `String | `Element) as t =>
    Valid(t)

  | `List(t) => Valid(`List(of_raw(t)))

  | `Struct(ts) => Valid(`Struct(ts |> List.map(Tuple.map_snd2(of_raw))))

  | `Enumerated(ts) =>
    Valid(`Enumerated(ts |> List.map(Tuple.map_snd2(List.map(of_raw)))))

  | `Function(args, res) =>
    Valid(`Function((args |> List.map(of_raw), of_raw(res))))

  | `View(props, res) =>
    Valid(`View((props |> List.map(Tuple.map_snd2(of_raw)), of_raw(res))))

  | `Style(args, ids, classes) =>
    Valid(`Style((args |> List.map(of_raw), ids, classes)))

  | `Module(entries) =>
    Valid(
      `Module(
        entries
        |> List.map(
             Tuple.map_snd2(
               Container.(
                 fun
                 | Type(t) => Type(of_raw(t))
                 | Value(t) => Value(of_raw(t))
               ),
             ),
           ),
      ),
    )

  | `Unknown => raise(UnknownTypeEncountered)
  };
