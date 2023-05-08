open Knot.Kore;

exception UnknownTypeEncountered;

module DecoratorTarget = {
  type t =
    | Module
    | Style;

  let pp: Fmt.t(t) =
    ppf =>
      (
        fun
        | Module => Constants.Keyword.module_
        | Style => Constants.Keyword.style
      )
      % Fmt.string(ppf);
};

module ModuleEntryKind = {
  type t =
    | Type
    | Value;

  let to_string =
    fun
    | Type => "Type"
    | Value => "Value";
};

/**
 the final type attributed to elements within the fully-typed AST
 */
type t =
  | Valid(valid_t)
  | Invalid(invalid_t)

and valid_t =
  | Nil
  | Boolean
  | Integer
  | Float
  | String
  | Element
  | Style
  | List(t)
  | Object(list((string, (t, bool))))
  | Enumerated(list((string, list(t))))
  | Function(list(t), t)
  | Decorator(list(t), DecoratorTarget.t)
  | View(list((string, (t, bool))), t)
  | Module(list((ModuleEntryKind.t, string, t)))

and invalid_t =
  | NotInferrable;

type error_t =
  | NotFound(string)
  | ExternalNotFound(Reference.Namespace.t, Reference.Export.t)
  /* FIXME: not reported */
  | DuplicateIdentifier(string)
  /* FIXME: not reported */
  | TypeMismatch(t, t)
  | InvalidUnaryOperation(Operator.Unary.t, t)
  | InvalidBinaryOperation(Operator.Binary.t, t, t)
  | InvalidKSXPrimitiveExpression(t)
  | InvalidKSXTag(string, t, list((string, t)))
  | UnexpectedKSXAttribute(string, t)
  | InvalidKSXAttribute(string, t, t)
  | MissingKSXAttributes(string, list((string, t)))
  | InvalidDotAccess(t, string)
  | InvalidStyleBinding(t, t)
  | InvalidFunctionCall(t, list(t))
  /* FIXME: not reported */
  | UntypedFunctionArgument(string)
  /* FIXME: not reported */
  | DefaultArgumentMissing(string)
  | InvalidDecoratorInvocation(t, list(t))
  | DecoratorTargetMismatch(DecoratorTarget.t, DecoratorTarget.t)
  | UnknownStyleRule(string)
  | InvalidStyleRule(string, t, t)
  | InvalidViewMixin(t)
  | MustUseExplicitChildren(t);

/* pretty printing */

let pp_list = (pp_type: Fmt.t('a)): Fmt.t('a) =>
  Fmt.(ppf => pf(ppf, "%a[]", pp_type));

let pp_props = (pp_type: Fmt.t('a)): Fmt.t((string, ('a, bool))) =>
  (ppf, (key, (type_, required))) =>
    Fmt.pf(ppf, "%s%s %a", key, required ? ":" : "?:", pp_type, type_);

let pp_object = (pp_type: Fmt.t('a)): Fmt.t(list((string, ('a, bool)))) =>
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

let pp_enumerated = (pp_type: Fmt.t('a)): Fmt.t(list((string, list('a)))) =>
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

let pp_decorator =
    (pp_type: Fmt.t('a)): Fmt.t((list('a), DecoratorTarget.t)) =>
  Fmt.(
    (ppf, (args, target)) =>
      pf(
        ppf,
        "@[<h>(%a) on %a@]",
        list(~sep=Sep.comma, pp_type),
        args,
        DecoratorTarget.pp,
        target,
      )
  );

let pp_view =
    (pp_type: Fmt.t('a)): Fmt.t((list((string, ('a, bool))), 'a)) =>
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
    (pp_type: Fmt.t('a)): Fmt.t((list('a), list(string), list(string))) =>
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
    (pp_type: Fmt.t('a)): Fmt.t(list((ModuleEntryKind.t, string, 'a))) =>
  Fmt.(
    (ppf, entries) =>
      pf(
        ppf,
        "@[<h>Module<%a>@]",
        closure((ppf, (kind, name, value)) =>
          pf(
            ppf,
            "%s(%s: %a)",
            ModuleEntryKind.to_string(kind),
            name,
            pp_type,
            value,
          )
        ),
        entries,
      )
  );

let rec pp: Fmt.t(t) =
  ppf =>
    fun
    | Valid(t) => pp_valid(ppf, t)
    | Invalid(err) => pp_invalid(ppf, err)

and pp_valid: Fmt.t(valid_t) =
  ppf =>
    Constants.(
      fun
      | Nil => Keyword.nil |> Fmt.string(ppf)
      | Boolean => Keyword.boolean |> Fmt.string(ppf)
      | Integer => Keyword.integer |> Fmt.string(ppf)
      | Float => Keyword.float |> Fmt.string(ppf)
      | String => Keyword.string |> Fmt.string(ppf)
      | Element => Keyword.element |> Fmt.string(ppf)
      | Style => Keyword.style |> Fmt.string(ppf)
      | List(t) => t |> pp_list(pp, ppf)
      | Object(properties) => properties |> pp_object(pp, ppf)
      | Enumerated(variants) => variants |> pp_enumerated(pp, ppf)
      | Function(parameters, result) =>
        (parameters, result) |> pp_function(pp, ppf)
      | Decorator(parameters, target) =>
        (parameters, target) |> pp_decorator(pp, ppf)
      | View(parameters, result) => (parameters, result) |> pp_view(pp, ppf)
      | Module(entries) => entries |> pp_module(pp, ppf)
    )

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
          Reference.Namespace.pp,
          namespace,
          Reference.Export.pp,
          id,
        )

      | TypeMismatch(lhs, rhs) =>
        pf(ppf, "TypeMismatch<%a, %a>", pp, lhs, pp, rhs)

      | InvalidUnaryOperation(op, type_) =>
        pf(
          ppf,
          "InvalidUnaryOperation<%s, %a>",
          Operator.Unary.to_string(op),
          pp,
          type_,
        )

      | InvalidBinaryOperation(op, lhs, rhs) =>
        pf(
          ppf,
          "InvalidBinaryOperation<%s, %a, %a>",
          Operator.Binary.to_string(op),
          pp,
          lhs,
          pp,
          rhs,
        )

      | InvalidKSXPrimitiveExpression(type_) =>
        pf(ppf, "InvalidKSXPrimitiveExpression<%a>", pp, type_)

      | InvalidDotAccess(type_, prop) =>
        pf(ppf, "InvalidDotAccess<%a, %s>", pp, type_, prop)

      | InvalidStyleBinding(view, style) =>
        pf(ppf, "InvalidStyleBinding<%a, %a>", pp, view, pp, style)

      | InvalidFunctionCall(type_, expected_args) =>
        pf(
          ppf,
          "InvalidFunctionCall<%a, %a>",
          pp,
          type_,
          list(pp),
          expected_args,
        )

      | InvalidKSXTag(id, type_, expected_attrs) =>
        pf(
          ppf,
          "InvalidKSXTag<%s, %a, %a>",
          id,
          pp,
          type_,
          record(string, pp),
          expected_attrs,
        )

      | UnexpectedKSXAttribute(name, type_) =>
        pf(ppf, "UnexpectedKSXAttribute<%s, %a>", name, pp, type_)

      | InvalidKSXAttribute(name, expected_type, actual_type) =>
        pf(
          ppf,
          "InvalidKSXAttribute<%s, %a, %a>",
          name,
          pp,
          expected_type,
          pp,
          actual_type,
        )

      | MissingKSXAttributes(id, attrs) =>
        pf(
          ppf,
          "MissingKSXAttributes<%s, @[<hv>%a@]>",
          id,
          record(string, pp),
          attrs,
        )

      | DefaultArgumentMissing(id) =>
        pf(ppf, "DefaultArgumentMissing<%s>", id)

      | InvalidDecoratorInvocation(type_, expected_args) =>
        pf(
          ppf,
          "InvalidDecoratorInvocation<%a, %a>",
          pp,
          type_,
          list(pp),
          expected_args,
        )

      | DecoratorTargetMismatch(lhs, rhs) =>
        pf(
          ppf,
          "DecoratorTargetMismatch<%a, %a>",
          DecoratorTarget.pp,
          lhs,
          DecoratorTarget.pp,
          rhs,
        )

      | UnknownStyleRule(rule) => pf(ppf, "UnknownStyleRule<%s>", rule)

      | InvalidStyleRule(rule, expected_type, actual_type) =>
        pf(
          ppf,
          "InvalidStyleRule<%s, %a, %a>",
          rule,
          pp,
          expected_type,
          pp,
          actual_type,
        )

      | InvalidViewMixin(type_) => pf(ppf, "InvalidViewMixin<%a>", pp, type_)

      | MustUseExplicitChildren(type_) =>
        pf(ppf, "MustUseExplicitChildren<%a>", pp, type_)
  );
