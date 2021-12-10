open Kore;
open Pretty;

module Resolver = Resolve.Resolver;
module Module = Resolve.Module;
module Writer = File.Writer;

let _print_code_examples =
  List.intersperse([Newline, string("// or"), Newline] |> concat)
  % concat
  % indent(2);

let _print_resolution = ((description, examples)) =>
  [
    [description |> Fmt.str("• %s") |> string] |> concat,
    switch (examples) {
    | Some(examples) =>
      [Newline, Newline, examples |> _print_code_examples] |> concat
    | None => Nil
    },
  ]
  |> concat;

let _print_err = (~index, path, title, content) =>
  [
    Fmt.str("%d) %s", index + 1, title) |> ~@Fmt.bad_str |> string,
    switch (path) {
    | Some((Module.{relative, full}, range)) =>
      let cursor_suffix =
        range
        |> Range.get_start
        |> (
          start =>
            Fmt.str(
              ":%d:%d",
              Point.get_line(start),
              Point.get_column(start),
            )
        );

      [
        [
          string(" : "),
          relative |> ~@Fmt.cyan_str |> string,
          cursor_suffix |> ~@Fmt.grey_str |> string,
        ]
        |> newline,
        [Fmt.str("(%s%s)", full, cursor_suffix) |> ~@Fmt.grey_str |> string]
        |> newline,
      ]
      |> concat
      |> indent(2);
    | None => Nil
    },
    Newline,
    [content |> indent(2)] |> concat,
  ]
  |> concat;

let _pp_type_trait = (pp_target: Fmt.t(string)): Fmt.t(Type.Trait.t) =>
  ppf =>
    Type.Trait.(
      fun
      | Unknown =>
        "Unknown" |> Fmt.pf(ppf, "%a which can represent any type", pp_target)

      | Number =>
        Fmt.pf(
          ppf,
          "%a which is shared by the types %a and %a",
          pp_target,
          "number",
          Fmt.bold_str,
          "int",
          Fmt.bold_str,
          "float",
        )
    );

let _type_trait_to_string = print_target =>
  Type.Trait.(
    fun
    | Unknown =>
      "Unknown" |> print_target |> Fmt.str("%s which can represent any type")

    | Number =>
      Fmt.str(
        "%s which is shared by the types %a and %a",
        print_target("number"),
        Fmt.bold_str,
        "int",
        Fmt.bold_str,
        "float",
      )
  );

let _extract_type_err =
  Type.Error.(
    fun
    /* | TraitConflict(lhs, rhs) => (
         "Types Have Conflicting Traits",
         [
           [
             string(
               "the types of the arguments in this operation are not compatible with each other:",
             ),
           ]
           |> newline,
           Newline,
           [
             [
               lhs
               |> _type_trait_to_string(Print.bad)
               |> Fmt.str(
                    "• the left-hand-side argument has the trait %s",
                  )
               |> string,
             ]
             |> newline,
             [
               rhs
               |> _type_trait_to_string(Print.bad)
               |> Fmt.str(
                    "• the right-hand-side argument has the trait %s",
                  )
               |> string,
             ]
             |> newline,
           ]
           |> concat,
           Newline,
         ]
         |> concat,
         None,
       ) */
    | NotAssignable(t, trait) => (
        "Type Cannot Be Assigned",
        Fmt.str(
          "expected a type that implements the trait %a but found the type %a instead",
          _pp_type_trait(Fmt.good_str),
          trait,
          Fmt.bad(Type.Raw.pp),
          t,
        )
        |> string,
        None,
      )
    | TypeMismatch(expected, actual) => (
        "Types Do Not Match",
        Fmt.str(
          "expected the type %a but found the type %a instead",
          Fmt.good(Type.Raw.pp),
          expected,
          Fmt.bad(Type.Raw.pp),
          actual,
        )
        |> string,
        None,
      )
    | NotFound(id) => (
        "Identifier Not Found",
        id
        |> Fmt.str(
             "unable to resolve an identifier %a in the local scope or any inherited scope",
             Fmt.bad(Identifier.pp),
           )
        |> string,
        Some(
          [
            _print_resolution((
              id
              |> Fmt.str(
                   "check that the identifier %a is spelled correctly",
                   Fmt.bad(Identifier.pp),
                 ),
              None,
            )),
            _print_resolution((
              "define the value yourself",
              Some(
                [Fmt.str("const %s = …;"), Fmt.str("let %s = …;")]
                |> List.map(fmt =>
                     id |> ~@Fmt.bold(Identifier.pp) |> fmt |> string
                   ),
              ),
            )),
            _print_resolution((
              "import the value from another module",
              Some(
                [Fmt.str("import { %s } from \"…\";")]
                |> List.map(fmt =>
                     id |> ~@Fmt.bold(Identifier.pp) |> fmt |> string
                   ),
              ),
            )),
          ]
          |> List.intersperse([Newline, Newline] |> concat)
          |> newline,
        ),
      )
    | DuplicateIdentifier(id) => (
        "Identifier Already Defined",
        id
        |> Fmt.str(
             "a variable with the same name (%a) already exists in the local scope or an inherited scope",
             Fmt.bad(Identifier.pp),
           )
        |> string,
        Some(
          [_print_resolution(("change the name of this variable", None))]
          |> concat,
        ),
      )
    | ExternalNotFound(namespace, id) => (
        "External Not Found",
        (
          switch (id) {
          | Named(id) =>
            Fmt.str(
              "an export with the identifier %a could not be found in module %a",
              Fmt.bad(Identifier.pp),
              id,
              Fmt.bad(Namespace.pp),
              namespace,
            )
          | Main =>
            Fmt.str(
              "a main export could not be found in module %a",
              Fmt.bad(Namespace.pp),
              namespace,
            )
          }
        )
        |> string,
        None,
      )
    | TypeResolutionFailed => ("Type Resolution Failed", Pretty.Nil, None)
  );

let _extract_parse_err =
  fun
  | TypeError(err) => _extract_type_err(err)
  | ReservedKeyword(name) => (
      "Reserved Keyword",
      name
      |> Fmt.str(
           "the reserved keyword %a was used as an identifier",
           Fmt.bad_str,
         )
      |> string,
      Some(
        [
          _print_resolution((
            Fmt.str(
              "check that the identifier %a is spelled correctly",
              Fmt.bad_str,
              name,
            ),
            None,
          )),
          _print_resolution((
            Fmt.str(
              "rename %a so that there is no conflict with reserved keywords (%s)",
              Fmt.bad_str,
              name,
              Constants.Keyword.reserved |> String.join(~separator=", "),
            ),
            None,
          )),
        ]
        |> List.intersperse([Newline, Newline] |> concat)
        |> concat,
      ),
    );

let _extract_compile_err = resolver =>
  fun
  | ImportCycle(cycles) as err => (
      None,
      "Import Cycle Found",
      err |> ~@pp_compile_err |> string,
    )

  | UnresolvedModule(name) as err => (
      None,
      "Unresolved Module",
      err |> ~@pp_compile_err |> string,
    )

  | FileNotFound(path) as err => (
      None,
      "File Not Found",
      err |> ~@pp_compile_err |> string,
    )

  | InvalidModule(namespace) as err => (
      resolver
      |> Resolver.resolve_module(~skip_cache=true, namespace)
      |> Module.get_path
      |?> (x => (x, Range.zero)),
      "Invalid Module",
      err |> ~@pp_compile_err |> string,
    )

  | ParseError(err, namespace, range) =>
    _extract_parse_err(err)
    |> Tuple.join3((title, description, resolutions) => {
         let module_ =
           Resolver.resolve_module(namespace, resolver)
           |> Module.read_to_string;

         (
           resolver
           |> Resolver.resolve_module(~skip_cache=true, namespace)
           |> Module.get_path
           |?> (x => (x, range)),
           title,
           [
             description,
             switch (module_) {
             | Ok(x) => File.CodeFrame.print(x, range) |> string
             | Error(_) =>
               [Newline, Newline, string("[code frame not available]")]
               |> concat
             },
             switch (resolutions) {
             | Some(resolutions) =>
               [
                 Newline,
                 Newline,
                 [string("try one of the following to resolve this issue:")]
                 |> newline,
                 Newline,
                 resolutions |> indent(2),
               ]
               |> concat
             | None => Nil
             },
           ]
           |> concat,
         );
       });

let report =
    (
      resolver: Resolver.t,
      errors: list(compile_err),
      ppf: Stdlib.Format.formatter,
    ) => {
  let header =
    Fmt.str("%sFAILED%s", String.repeat(20, " "), String.repeat(20, " "));
  let summary =
    Fmt.str(
      "finished with %a and %a",
      Fmt.red(ppf => Fmt.pf(ppf, "%i error(s)")),
      List.length(errors),
      Fmt.yellow(ppf => Fmt.pf(ppf, "%i warning(s)")),
      0,
    );

  let horiz_border = String.(repeat(length(header), "═"));

  Fmt.(
    page(
      (ppf, ()) =>
        pf(
          ppf,
          "\n%s\n%s\n%s\n\n%s\n%a\n%s",
          horiz_border |> Fmt.str("╔%s╗") |> ~@Fmt.bad_str,
          header |> Fmt.str("║%s║") |> ~@Fmt.bad_str,
          horiz_border |> Fmt.str("╚%s╝") |> ~@Fmt.bad_str,
          summary,
          list(~layout=Vertical, ~sep=Sep.newline, (ppf, (index, err)) =>
            [
              err
              |> _extract_compile_err(resolver)
              |> Tuple.join3((path, title, content) =>
                   _print_err(~index, path, title, content)
                 ),
            ]
            |> Pretty.newline
            |> Pretty.to_string
            |> string(ppf)
          ),
          errors |> List.mapi(Tuple.with_fst2),
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
