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
    Fmt.(
      ppf =>
        fun
        | NotFound(id) => pf(ppf, "NotFound<%a>", Identifier.pp, id)
        | DuplicateIdentifier(id) =>
          pf(ppf, "DuplicateIdentifier<%a>", Identifier.pp, id)
        | NotAssignable(type_, trait) =>
          pf(ppf, "NotAssignable<%a, %a>", pp_type, type_, Trait.pp, trait)
        | TypeMismatch(lhs, rhs) =>
          pf(ppf, "TypeMismatch<%a, %a>", pp_type, lhs, pp_type, rhs)
        | TypeResolutionFailed => string(ppf, "TypeResolutionFailed")
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

type abstract_t('a) = [ | `Abstract('a)];

type generic_t = [ | `Generic(int, int)];

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
    | Invalid(error_t)

  and error_t = Error.t(t)

  and strong_t = [ primitive_t | container_t(t) | generic_t]

  and weak_t = [ primitive_t | container_t(t) | generic_t];

  /* static */

  /* let unknown = Weak(ref(Ok(`Abstract(Trait.Unknown)))); */

  /* pretty printing */

  let pp_primitive: Fmt.t(primitive_t) =
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

  let pp_list = (pp_type: Fmt.t('a)): Fmt.t('a) =>
    Fmt.(ppf => pf(ppf, "List<%a>", pp_type));

  let pp_abstract: Fmt.t(Trait.t) =
    Fmt.(ppf => pf(ppf, "Abstract<%a>", Trait.pp));

  let pp_generic: Fmt.t((int, int)) =
    Fmt.(
      (ppf, (scope_id, weak_id)) =>
        pf(ppf, "Generic<%d, %d>", scope_id, weak_id)
    );

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

  let rec pp: Fmt.t(t) =
    Fmt.(
      (ppf, type_) =>
        switch (type_) {
        | Strong(t) => pp_strong(ppf, t)
        | Invalid(err) => Error.pp(pp, ppf, err)
        | Weak({contents: weak_type}) =>
          pf(ppf, "Weak<%a>", pp_weak, weak_type)
        }
    )

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
        | `Generic(scope_id, weak_id) =>
          pp_generic(ppf, (scope_id, weak_id))
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
      | `Generic(id) => pp_generic(ppf, id)
      };
};

type t =
  | Valid(valid_t)
  | Invalid(error_t)

and valid_t = [ primitive_t | container_t(t) | generic_t]

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
  | `Generic(_) as x => Strong(x)
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
    | `Generic(id) => Raw.pp_generic(ppf, id);

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
  | `Generic(t) => `Generic(t)
  }

and valid_of_weak = (type_: Raw.weak_t): valid_t =>
  switch (type_) {
  | (`Nil | `Boolean | `Integer | `Float | `String | `Element) as t => t
  | `List(x) => `List(of_raw(x))
  | `Struct(xs) => `Struct(xs |> List.map(Tuple.map_snd2(of_raw)))
  | `Function(args, res) =>
    `Function((args |> List.map(Tuple.map_snd2(of_raw)), of_raw(res)))
  | `Generic(t) => `Generic(t)
  }

/* and strong_of_weak = (type_: Raw.weak_t): Raw.strong_t =>
   switch (type_) {
   | (`Nil | `Boolean | `Integer | `Float | `String | `Element) as t => t
   | `List(Weak({current: Ok(x)})) => `List(Strong(strong_of_weak(x)))
   | `List(Weak({current: Error(x)})) => `List(Invalid(x))
   | `List((Strong(_) | Invalid(_)) as x) => `List(x)
   | `Struct(xs) => `Struct(xs |> List.map(Tuple.map_snd2(of_raw)))
   | `Function(args, res) =>
     `Function((args |> List.map(Tuple.map_snd2(of_raw)), of_raw(res)))
   | `Abstract(t) => `Abstract(t)
   | t => t
   } */

and err_of_raw_err = (err: Raw.error_t): error_t =>
  switch (err) {
  | TypeMismatch(lhs, rhs) => TypeMismatch(of_raw(lhs), of_raw(rhs))
  | NotAssignable(x, y) => NotAssignable(of_raw(x), y)
  | (
      ExternalNotFound(_) | DuplicateIdentifier(_) | NotFound(_) |
      TypeResolutionFailed
    ) as err => err
  };
