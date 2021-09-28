open Infix;
open Reference;

module Trait = {
  type t =
    | Number
    | Unknown;

  /* pretty printing */

  let pp: Fmt.t(t) =
    ppf =>
      (
        fun
        | Number => "Number"
        | Unknown => "Unknown"
      )
      % Fmt.string(ppf);
};

module Error = {
  type t('a) =
    | NotFound(Identifier.t)
    | TypeMismatch('a, 'a)
    | NotAssignable('a, Trait.t)
    | ExternalNotFound(Namespace.t, Export.t)
    | TypeResolutionFailed
    | DuplicateIdentifier(Identifier.t);

  /* pretty printing */

  let pp = (pp_type: Fmt.t('a)): Fmt.t(t('a)) =>
    ppf =>
      fun
      | NotFound(id) => Fmt.pf(ppf, "NotFound<%a>", Identifier.pp, id)
      | DuplicateIdentifier(id) =>
        Fmt.pf(ppf, "DuplicateIdentifier<%a>", Identifier.pp, id)
      | NotAssignable(type_, trait) =>
        Fmt.pf(ppf, "NotAssignable<%a, %a>", pp_type, type_, Trait.pp, trait)
      | TypeMismatch(lhs, rhs) =>
        Fmt.pf(ppf, "TypeMismatch<%a, %a>", pp_type, lhs, pp_type, rhs)
      | TypeResolutionFailed => Fmt.string(ppf, "TypeResolutionFailed")
      | ExternalNotFound(namespace, id) =>
        Fmt.pf(
          ppf,
          "ExternalNotFound<%a#%a>",
          Namespace.pp,
          namespace,
          Export.pp,
          id,
        );
};

type abstract_t('a) = [ | `Abstract('a)];

type container_t('a) = [
  | `List('a)
  | `Struct(list((string, 'a)))
  | `Function(list((string, 'a)), 'a)
];

type primitive_t = [
  | `Nil
  | `Boolean
  | `Integer
  | `Float
  | `String
  | `Element
];

module Raw = {
  type t =
    | Strong(strong_t)
    | Weak(ref(result(weak_t, error_t)))
    | Invalid(Error.t(t))

  and error_t = Error.t(t)

  and strong_t = [ primitive_t | container_t(t)]

  and weak_t = [ primitive_t | container_t(t) | abstract_t(Trait.t)];

  /* pretty printing */

  let pp_primitive: Fmt.t(primitive_t) =
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
      |> Fmt.string(ppf);

  let pp_list = (pp_type: Fmt.t('a)): Fmt.t('a) =>
    ppf => Fmt.pf(ppf, "List<%a>", pp_type);

  let pp_abstract: Fmt.t(Trait.t) =
    ppf => Fmt.pf(ppf, "Abstract<%a>", Trait.pp);

  let pp_props = (pp_type: Fmt.t('a)): Fmt.t((string, 'a)) =>
    (ppf, (key, type_)) => Fmt.pf(ppf, "%s: %a", key, pp_type, type_);

  let pp_struct = (pp_type: Fmt.t('a)): Fmt.t(list((string, 'a))) =>
    (ppf, props) =>
      List.is_empty(props)
        ? Fmt.string(ppf, "{}")
        : Fmt.pf(
            ppf,
            "{ %a }",
            Fmt.list(
              ~sep=(ppf, ()) => Fmt.string(ppf, ", "),
              pp_props(pp_type),
            ),
            props,
          );

  let pp_function = (pp_type: Fmt.t('a)): Fmt.t((list((string, 'a)), 'a)) =>
    (ppf, (args, res)) =>
      Fmt.pf(
        ppf,
        "Function<(%a), %a>",
        Fmt.list(
          ~sep=(ppf, ()) => Fmt.string(ppf, ", "),
          pp_props(pp_type),
        ),
        args,
        pp_type,
        res,
      );

  let rec pp: Fmt.t(t) =
    (ppf, type_) =>
      switch (type_) {
      | Strong(t) => pp_strong(ppf, t)
      | Invalid(err) => Error.pp(pp, ppf, err)
      | Weak({contents: weak_type}) =>
        Fmt.pf(ppf, "Weak<%a>", pp_weak, weak_type)
      }

  and pp_weak: Fmt.t(result(weak_t, error_t)) =
    ppf =>
      fun
      | Ok(ok_type) =>
        switch (ok_type) {
        | (`Nil | `Boolean | `Integer | `Float | `String | `Element) as t =>
          pp_primitive(ppf, t)
        | `List(t) => pp_list(pp, ppf, t)
        | `Struct(props) => pp_struct(pp, ppf, props)
        | `Function(args, res) => pp_function(pp, ppf, (args, res))
        | `Abstract(trait) => pp_abstract(ppf, trait)
        }
      | Error(err) => Error.pp(pp, ppf, err)

  and pp_strong: Fmt.t(strong_t) =
    (ppf, type_) =>
      switch (type_) {
      | (`Nil | `Boolean | `Integer | `Float | `String | `Element) as x =>
        pp_primitive(ppf, x)

      | `List(t) => pp_list(pp, ppf, t)
      | `Struct(props) => pp_struct(pp, ppf, props)
      | `Function(args, res) => pp_function(pp, ppf, (args, res))
      };
};

type t =
  | Valid(valid_t)
  | Invalid(Error.t(t))

and valid_t = [ primitive_t | container_t(t) | abstract_t(Trait.t)]

and error_t = Error.t(t);

let of_result = (res: result(valid_t, error_t)) =>
  switch (res) {
  | Ok(x) => Valid(x)
  | Error(err) => Invalid(err)
  };

/* methods */

let to_result = (res: t) =>
  switch (res) {
  | Valid(x) => Ok(x)
  | Invalid(err) => Error(err)
  };

let rec _valid_to_raw = (type_: valid_t): Raw.t =>
  switch (type_) {
  | (`Nil | `Boolean | `Integer | `Float | `String | `Element) as t =>
    Strong(t)

  | `List(x) => Strong(`List(to_raw(x)))
  | `Struct(xs) => Strong(`Struct(xs |> List.map(Tuple.map_snd2(to_raw))))
  | `Function(args, res) =>
    Strong(
      `Function((args |> List.map(Tuple.map_snd2(to_raw)), to_raw(res))),
    )
  | `Abstract(_) as x => Weak(ref(Ok(x)))
  }

and to_raw = (type_: t): Raw.t =>
  switch (type_) {
  | Valid(t) => _valid_to_raw(t)
  | Invalid(NotAssignable(t, trait)) =>
    Invalid(NotAssignable(to_raw(t), trait))
  | Invalid(TypeMismatch(lhs, rhs)) =>
    Invalid(TypeMismatch(to_raw(lhs), to_raw(rhs)))
  | Invalid(
      (
        ExternalNotFound(_) | DuplicateIdentifier(_) | NotFound(_) |
        TypeResolutionFailed
      ) as err,
    ) =>
    Invalid(err)
  }

and err_to_strong_err = (err: error_t): Raw.error_t =>
  switch (err) {
  | NotAssignable(t, trait) => NotAssignable(to_raw(t), trait)
  | TypeMismatch(lhs, rhs) => TypeMismatch(to_raw(lhs), to_raw(rhs))
  | (
      ExternalNotFound(_) | DuplicateIdentifier(_) | NotFound(_) |
      TypeResolutionFailed
    ) as err => err
  };

let rec pp: Fmt.t(t) =
  ppf =>
    fun
    | Valid(t) => pp_valid(ppf, t)
    | Invalid(err) => Error.pp(pp, ppf, err)

and pp_valid: Fmt.t(valid_t) =
  ppf =>
    fun
    | (`Nil | `Boolean | `Integer | `Float | `String | `Element) as t =>
      Raw.pp_primitive(ppf, t)
    | `List(t) => Raw.pp_list(pp, ppf, t)
    | `Struct(props) => Raw.pp_struct(pp, ppf, props)
    | `Function(args, res) => Raw.pp_function(pp, ppf, (args, res))
    | `Abstract(trait) => Raw.pp_abstract(ppf, trait);

let rec of_raw = (type_: Raw.t): t =>
  switch (type_) {
  | Strong(t) => Valid(valid_of_strong(t))
  | Weak({contents: Ok(t)}) => Valid(valid_of_weak(t))
  | Weak({contents: Error(err)}) => Invalid(err_of_raw_err(err))
  | Invalid(err) => Invalid(err_of_raw_err(err))
  }

and valid_of_strong = (type_: Raw.strong_t): valid_t =>
  switch (type_) {
  | (`Nil | `Boolean | `Integer | `Float | `String | `Element) as t => t
  | `List(x) => `List(of_raw(x))
  | `Struct(xs) => `Struct(xs |> List.map(Tuple.map_snd2(of_raw)))
  | `Function(args, res) =>
    `Function((args |> List.map(Tuple.map_snd2(of_raw)), of_raw(res)))
  }

and valid_of_weak = (type_: Raw.weak_t): valid_t =>
  switch (type_) {
  | (`Nil | `Boolean | `Integer | `Float | `String | `Element) as t => t
  | `List(x) => `List(of_raw(x))
  | `Struct(xs) => `Struct(xs |> List.map(Tuple.map_snd2(of_raw)))
  | `Function(args, res) =>
    `Function((args |> List.map(Tuple.map_snd2(of_raw)), of_raw(res)))
  | `Abstract(t) => `Abstract(t)
  }

and err_of_raw_err = (err: Raw.error_t): error_t =>
  switch (err) {
  | TypeMismatch(lhs, rhs) => TypeMismatch(of_raw(lhs), of_raw(rhs))
  | NotAssignable(x, y) => NotAssignable(of_raw(x), y)
  | (
      ExternalNotFound(_) | DuplicateIdentifier(_) | NotFound(_) |
      TypeResolutionFailed
    ) as err => err
  };
