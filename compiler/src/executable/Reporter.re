open Kore;

module Resolver = Resolve.Resolver;
module Source = Resolve.Source;
module Writer = File.Writer;

let __numeric_types = [Type.Valid(`Float), Type.Valid(`Integer)];

let _example_sep =
  Fmt.Sep.(create(~trail=Trail.nop, (ppf, ()) => Fmt.pf(ppf, "@,// or@,")));
let _or_sep =
  Fmt.Sep.(create(~trail=Trail.nop, (ppf, ()) => Fmt.pf(ppf, " or ")));

let _pp_resolution: Fmt.t((Stdlib.Format.formatter => unit, list(string))) =
  (ppf, (description, examples)) =>
    Fmt.(
      pf(
        ppf,
        "• %t%a",
        description,
        ppf =>
          fun
          | [] => nop(ppf, ())
          | _ =>
            pf(
              ppf,
              "@,%a",
              block(~layout=Vertical, ~sep=_example_sep, string),
              examples,
            ),
        examples,
      )
    );

let _pp_err:
  Fmt.t(
    (
      int,
      string,
      option((Source.path_t, Range.t)),
      Stdlib.Format.formatter => unit,
    ),
  ) =
  (ppf, (index, title, path, content)) => {
    Fmt.(
      pf(
        ppf,
        "%a%a@,%a",
        bad_str,
        str("%d) %s", index + 1, title),
        ppf =>
          fun
          | None => nop(ppf, ())
          | Some((Source.{relative, full}, range)) => {
              pf(
                ppf,
                " : %a%a%a",
                cyan_str,
                relative,
                grey(ppf => pf(ppf, ":%a", text_loc)),
                range,
                indented(
                  vbox(
                    grey((ppf, ()) =>
                      pf(ppf, "(%s:%a)", full, text_loc, range)
                    ),
                  ),
                ),
                (),
              );
            },
        path,
        indented(vbox(ppf => pf(ppf, "%t"))),
        content,
      )
    );
  };

let _extract_type_err =
  fun
  | Type.TypeMismatch(expected, actual) => (
      "Types Do Not Match",
      Fmt.(
        (
          ppf =>
            pf(
              ppf,
              "expected the type %a but received %a",
              good(Type.pp),
              expected,
              bad(Type.pp),
              actual,
            )
        )
      ),
      [],
    )

  | Type.NotFound(id) => (
      "Identifier Not Found",
      (
        ppf =>
          Fmt.pf(
            ppf,
            "unable to resolve an identifier %a in the local scope or any inherited scope",
            Fmt.bad_str,
            id,
          )
      ),
      [
        (
          (
            ppf =>
              Fmt.pf(
                ppf,
                "check that the identifier %a is spelled correctly",
                Fmt.bad_str,
                id,
              )
          ),
          [],
        ),
        (
          (ppf => Fmt.string(ppf, "define the value yourself")),
          Fmt.[str("const %s = …;"), str("let %s = …;")]
          |> List.map(fmt => id |> ~@Fmt.bold_str |> fmt),
        ),
        (
          (ppf => Fmt.string(ppf, "import the value from another module")),
          Fmt.[str("import { %s } from \"…\";")]
          |> List.map(fmt => id |> ~@Fmt.bold_str |> fmt),
        ),
      ],
    )

  /* FIXME: this isn't being reported */
  | Type.DuplicateIdentifier(id) => (
      "Identifier Already Defined",
      (
        ppf =>
          Fmt.pf(
            ppf,
            "a variable with the same name (%a) already exists in the local scope or an inherited scope",
            Fmt.bad_str,
            id,
          )
      ),
      [((ppf => Fmt.string(ppf, "change the name of this variable")), [])],
    )

  /* FIXME: this isn't being reported */
  | Type.ExternalNotFound(namespace, id) => (
      "External Not Found",
      switch (id) {
      | Named(id) => (
          ppf =>
            Fmt.pf(
              ppf,
              "an export with the identifier %a could not be found in module %a",
              Fmt.bad_str,
              id,
              Fmt.bad(Namespace.pp),
              namespace,
            )
        )

      | Main =>
        Fmt.(
          (
            ppf =>
              pf(
                ppf,
                "a %a export could not be found in module %a",
                bad_str,
                Constants.Keyword.main,
                bad(Namespace.pp),
                namespace,
              )
          )
        )
      },
      [],
    )

  | Type.InvalidUnaryOperation(operator, type_) => (
      "Invalid Unary Operation",
      (
        switch (operator) {
        | Not => ("NOT (!)", [Type.Valid(`Boolean)])

        | Positive => ("ABSOLUTE (+)", __numeric_types)

        | Negative => ("NEGATE (-)", __numeric_types)
        }
      )
      |> (
        ((operator, expected)) =>
          Fmt.(
            ppf =>
              pf(
                ppf,
                "the %a unary operator expects an expression of type %a but received %a",
                Fmt.bold_str,
                operator,
                list(~sep=_or_sep, good(Type.pp)),
                expected,
                bad(Type.pp),
                type_,
              )
          )
      ),
      [],
    )

  | Type.InvalidBinaryOperation(operator, lhs_type, rhs_type) => (
      "Invalid Binary Operation",
      {
        let print_static_error = ((operator, expected)) => {
          let pp_expected = (ppf, type_) =>
            (expected |> List.mem(type_) ? Fmt.good : Fmt.bad)(
              Type.pp,
              ppf,
              type_,
            );

          Fmt.(
            (
              ppf =>
                pf(
                  ppf,
                  "the %a binary operator expects both arguments to be of type %a but received %a and %a",
                  Fmt.bold_str,
                  operator,
                  list(~sep=_or_sep, good(Type.pp)),
                  expected,
                  pp_expected,
                  lhs_type,
                  pp_expected,
                  rhs_type,
                )
            )
          );
        };
        let print_equality_error = operator =>
          Fmt.(
            (
              ppf =>
                pf(
                  ppf,
                  "the %a binary operator expects both arguments to be of the same type but received %a and %a",
                  Fmt.bold_str,
                  "EQUAL (==)",
                  bad(Type.pp),
                  lhs_type,
                  bad(Type.pp),
                  rhs_type,
                )
            )
          );

        switch (operator) {
        | LogicalAnd =>
          ("AND (&&)", [Type.Valid(`Boolean)]) |> print_static_error
        | LogicalOr =>
          ("OR (||)", [Type.Valid(`Boolean)]) |> print_static_error

        | LessOrEqual =>
          ("LESS THAN OR EQUAL (<=)", __numeric_types) |> print_static_error
        | LessThan => ("LESS THAN (<)", __numeric_types) |> print_static_error
        | GreaterOrEqual =>
          ("GREATER THAN OR EQUAL (>=)", __numeric_types)
          |> print_static_error
        | GreaterThan =>
          ("GREATER THAN (>)", __numeric_types) |> print_static_error

        | Equal => print_equality_error("EQUAL (==)")
        | Unequal => print_equality_error("NOT EQUAL (!=)")

        | Add => ("ADD (+)", __numeric_types) |> print_static_error
        | Subtract => ("SUBTRACT (-)", __numeric_types) |> print_static_error
        | Divide => ("DIVIDE (/)", __numeric_types) |> print_static_error
        | Multiply => ("MULTIPLY (*)", __numeric_types) |> print_static_error
        | Exponent => ("EXPONENT (^)", __numeric_types) |> print_static_error
        };
      },
      [],
    )

  | Type.InvalidJSXPrimitiveExpression(type_) => (
      "Invalid JSX Primitive Expression",
      Fmt.(
        (
          ppf =>
            pf(
              ppf,
              "jsx only supports rendering primitive values inline but received %a",
              bad(Type.pp),
              type_,
            )
        )
      ),
      [
        (
          Fmt.(
            (
              ppf =>
                pf(
                  ppf,
                  "@[<hv>convert the value to have a primitive type@,@[<h>ie. %a@]@]",
                  list(~layout=Horizontal, Type.pp),
                  [
                    Type.Valid(`Nil),
                    Type.Valid(`Boolean),
                    Type.Valid(`Integer),
                    Type.Valid(`Float),
                    Type.Valid(`String),
                    Type.Valid(`Element),
                  ],
                )
            )
          ),
          [],
        ),
      ],
    )

  | Type.InvalidJSXClassExpression(type_) => (
      "Invalid JSX Class Expression",
      Fmt.(
        (
          ppf =>
            pf(
              ppf,
              "jsx classes can only be controlled with arguments of type %a but received %a",
              good(Type.pp),
              Type.Valid(`Boolean),
              bad(Type.pp),
              type_,
            )
        )
      ),
      [],
    )

  | Type.InvalidJSXTag(id, type_, props) => (
      "Invalid JSX Tag",
      Fmt.(
        (
          ppf =>
            pf(
              ppf,
              "this jsx tag was expected to be of type %a with props %a but received %a",
              good_str,
              Constants.Keyword.view,
              good(ppf =>
                pf(
                  ppf,
                  "@[<hv>(%a)@]",
                  list(~layout=Horizontal, attribute(string, Type.pp)),
                )
              ),
              props,
              bad(Type.pp),
              type_,
            )
        )
      ),
      [],
    )

  | Type.InvalidJSXAttribute(key, expected, actual) => (
      "Invalid JSX Attribute",
      Fmt.(
        (
          ppf =>
            pf(
              ppf,
              "this jsx tag expects the attribute %a to be of type %a but received %a",
              bad_str,
              key,
              good(Type.pp),
              expected,
              bad(Type.pp),
              actual,
            )
        )
      ),
      [],
    )

  | Type.UnexpectedJSXAttribute(key, type_) => (
      "Unexpected JSX Attribute",
      Fmt.(
        (
          ppf =>
            pf(
              ppf,
              "found an unexpected attribute %a with type %a",
              bad_str,
              key,
              good(Type.pp),
              type_,
            )
        )
      ),
      [],
    )

  | Type.MissingJSXAttributes(id, missing) => (
      "Missing JSX Attributes",
      (
        ppf =>
          Fmt.pf(
            ppf,
            "jsx tag %s is missing the attributes %a",
            id,
            Fmt.(
              bad(ppf =>
                pf(
                  ppf,
                  "@[<hv>(%a)@]",
                  list(~layout=Horizontal, attribute(string, Type.pp)),
                )
              )
            ),
            missing,
          )
      ),
      [],
    )

  | Type.InvalidDotAccess(type_, prop) => (
      "Invalid Dot Access",
      Fmt.(
        (
          ppf =>
            pf(
              ppf,
              "@[<hv>dot access can only be performed on values with %a types@,expected a value matching the type %a but received %a@]",
              good_str,
              "struct",
              good_str,
              Fmt.str("{ %s: any }", prop),
              bad(Type.pp),
              type_,
            )
        )
      ),
      [],
    )

  | Type.InvalidFunctionCall(type_, args) => (
      "Invalid Function Call",
      Fmt.(
        (
          ppf =>
            pf(
              ppf,
              "@[<hv>function calls can only be performed on values with %a types@,expected a value matching the type %a but received %a@]",
              good_str,
              "function",
              good(ppf =>
                pf(
                  ppf,
                  "@[<hv>(%a) -> any@]",
                  list(~layout=Horizontal, Type.pp),
                )
              ),
              args,
              bad(Type.pp),
              type_,
            )
        )
      ),
      [],
    )

  | Type.UntypedFunctionArgument(id) => (
      "Untyped Function Argument",
      (
        ppf =>
          Fmt.pf(
            ppf,
            "the function argument %a must define a type",
            Fmt.bad_str,
            id,
          )
      ),
      [],
    )

  | Type.DefaultArgumentMissing(id) => (
      "Default Argument Missing",
      (
        ppf =>
          Fmt.pf(
            ppf,
            "the function argument %a must define a default value",
            Fmt.bad_str,
            id,
          )
      ),
      [
        (
          Fmt.(
            (
              ppf =>
                pf(ppf, "remove default values from all preceding arguments")
            )
          ),
          [],
        ),
      ],
    )

  | Type.InvalidDecoratorInvocation(type_, args) => (
      "Invalid Decorator Invocation",
      Fmt.(
        (
          ppf =>
            pf(
              ppf,
              "@[<hv>decorator invocations can only be performed on values with %a types@,expected a value matching the type %a but received %a@]",
              good_str,
              "decorator",
              good(ppf =>
                pf(
                  ppf,
                  "@[<hv>(%a) on target@]",
                  list(~layout=Horizontal, Type.pp),
                )
              ),
              args,
              bad(Type.pp),
              type_,
            )
        )
      ),
      [],
    )

  | Type.DecoratorTargetMismatch(lhs, rhs) => (
      "Decorator Target Mismatch",
      Fmt.(
        (
          ppf =>
            pf(
              ppf,
              "@[<hv>this decorator can only target a %a but found %a@]",
              good(Type.DecoratorTarget.pp),
              lhs,
              bad(Type.DecoratorTarget.pp),
              rhs,
            )
        )
      ),
      [],
    )

  | Type.UnknownStyleRule(rule) => (
      "Unknown Style Rule",
      Fmt.(
        (
          ppf =>
            pf(
              ppf,
              "@[<hv>the style rule %a was not recognized for the target platform@]",
              bad_str,
              rule,
            )
        )
      ),
      [],
    );

let _extract_parse_err =
  fun
  | TypeError(err) => _extract_type_err(err)

  | ReservedKeyword(name) => (
      "Reserved Keyword",
      (
        ppf =>
          Fmt.(
            pf(
              ppf,
              "the reserved keyword %a was used as an identifier",
              bad_str,
              name,
            )
          )
      ),
      [
        (
          Fmt.(
            (
              ppf =>
                pf(
                  ppf,
                  "check that the identifier %a is spelled correctly",
                  bad_str,
                  name,
                )
            )
          ),
          [],
        ),
        (
          Fmt.(
            (
              ppf =>
                pf(
                  ppf,
                  "rename %a so that there is no conflict with reserved keywords (%s)",
                  bad_str,
                  name,
                  Constants.Keyword.reserved
                  |> List.map(~@bold_str)
                  |> String.join(~separator=", "),
                )
            )
          ),
          [],
        ),
      ],
    );

let _extract_compile_err = resolver =>
  fun
  | ImportCycle(cycles) as err => (
      "Import Cycle Found",
      None,
      (ppf => err |> pp_compile_err(ppf)),
    )

  | UnresolvedModule(name) as err => (
      "Unresolved Module",
      None,
      (ppf => err |> pp_compile_err(ppf)),
    )

  | FileNotFound(path) as err => (
      "File Not Found",
      None,
      (ppf => err |> pp_compile_err(ppf)),
    )

  | InvalidModule(namespace) as err => (
      "Invalid Module",
      resolver
      |> Resolver.resolve_module(~skip_cache=true, namespace)
      |> Source.get_path
      |?> (x => (x, Range.zero)),
      (ppf => err |> pp_compile_err(ppf)),
    )

  | ParseError(err, namespace, range) =>
    _extract_parse_err(err)
    |> Tuple.join3((title, description, resolutions) => {
         let module_ =
           Resolver.resolve_module(namespace, resolver)
           |> Source.read_to_string;

         (
           title,
           resolver
           |> Resolver.resolve_module(~skip_cache=true, namespace)
           |> Source.get_path
           |?> (x => (x, range)),
           (
             ppf =>
               Fmt.(
                 pf(
                   ppf,
                   "%t%a%a",
                   description,
                   ppf =>
                     fun
                     | Ok(x) =>
                       pf(
                         ppf,
                         "@,@.@[<v 0>%a@]",
                         ppf => File.CodeFrame.pp(ppf),
                         (x, range),
                       )
                     | Error(_) => pf(ppf, "@,@,[code frame not available]"),
                   module_,
                   ppf =>
                     fun
                     | [] => nop(ppf, ())
                     | _ =>
                       pf(
                         ppf,
                         "@,@,@[<hv>try one of the following to resolve this issue:@,%a@]",
                         indented(
                           list(
                             ~layout=Vertical,
                             ~sep=Sep.double_newline,
                             _pp_resolution,
                           ),
                         ),
                         resolutions,
                       ),
                   resolutions,
                 )
               )
           ),
         );
       });

let _pp_header: Fmt.t(string) =
  (ppf, title) => {
    let header =
      Fmt.str(
        "%s%s%s",
        String.repeat(20, " "),
        title,
        String.repeat(20, " "),
      );
    let border = String.(repeat(length(header), "═"));

    Fmt.(
      pf(
        ppf,
        "%s@,%s@,%s",
        border |> str("╔%s╗"),
        header |> str("║%s║"),
        border |> str("╚%s╝"),
      )
    );
  };

let _pp_summary: Fmt.t((int, int)) =
  (ppf, (error_count, warning_count)) =>
    Fmt.(
      pf(
        ppf,
        "finished with %a and %a",
        red_str,
        str("%i error(s)", error_count),
        yellow_str,
        str("%i warning(s)", warning_count),
      )
    );

let report =
    (
      resolver: Resolver.t,
      errors: list(compile_err),
      ppf: Stdlib.Format.formatter,
    ) => {
  let summary = (List.length(errors), 0) |> ~@_pp_summary;

  Fmt.(
    page(
      (ppf, ()) =>
        pf(
          ppf,
          "@,%a@,@,%s@,@,%a%s",
          bad(_pp_header),
          "FAILED",
          summary,
          list(
            ~layout=Vertical,
            ~sep=Sep.trailing_double_newline,
            page(_pp_err),
          ),
          errors
          |> List.mapi(index =>
               _extract_compile_err(resolver)
               % Tuple.join3((path, title, content) =>
                   (index, path, title, content)
                 )
             ),
          summary,
        ),
      ppf,
      (),
    )
  );
};

let panic = (resolver: Resolver.t, errors: list(compile_err)) => {
  report(resolver, errors) |> Writer.write(stderr);

  exit(2);
};
