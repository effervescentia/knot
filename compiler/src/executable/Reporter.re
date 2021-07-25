open Kore;
open Pretty;

module Resolver = Resolve.Resolver;
module Module = Resolve.Module;
module Writer = File.Writer;

let _print_code_examples =
  List.intersperse([Newline, string("// or"), Newline] |> concat)
  % newline
  % indent(2);

let _print_resolution = ((description, examples)) =>
  [
    [description |> Print.fmt("• %s") |> string] |> newline,
    Newline,
    switch (examples) {
    | Some(examples) => [examples |> _print_code_examples] |> newline
    | None => Nil
    },
  ]
  |> concat;

let _print_err = (~index, path, title, content) =>
  [
    Print.fmt("%d) %s", index + 1, title) |> Print.bad |> string,
    switch (path) {
    | Some((Module.{relative, full}, cursor)) =>
      let cursor_suffix =
        Cursor.(
          switch (cursor) {
          | Range({line, column}, _)
          | Point({line, column}) => Print.fmt(":%d:%d", line, column)
          }
        );

      [
        [
          string(" : "),
          relative |> Print.cyan |> string,
          cursor_suffix |> Print.grey |> string,
        ]
        |> newline,
        [Print.fmt("(%s%s)", full, cursor_suffix) |> Print.grey |> string]
        |> newline,
      ]
      |> concat
      |> indent(2);
    | None => Newline
    },
    Newline,
    [content |> indent(2)] |> concat,
  ]
  |> concat;

let _type_trait_to_string = print_target =>
  Type2.Trait.(
    fun
    | Unknown =>
      "Unknown"
      |> print_target
      |> Print.fmt("%s which can represent any type")

    | Number =>
      Print.fmt(
        "%s which is shared by the types %s and %s",
        print_target("number"),
        Print.bold("int"),
        Print.bold("float"),
      )
  );

let _extract_type_err =
  Type2.Error.(
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
               |> Print.fmt(
                    "• the left-hand-side argument has the trait %s",
                  )
               |> string,
             ]
             |> newline,
             [
               rhs
               |> _type_trait_to_string(Print.bad)
               |> Print.fmt(
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
        [
          [
            Print.fmt(
              "expected a type that implements the trait %s but found the type %s instead",
              trait |> _type_trait_to_string(Print.good),
              t |> Type2.Raw.to_string |> Print.bad,
            )
            |> string,
          ]
          |> newline,
          Newline,
        ]
        |> concat,
        None,
      )
    | TypeMismatch(expected, actual) => (
        "Types Do Not Match",
        [
          [
            Print.fmt(
              "expected the type %s but found the type %s instead",
              expected |> Type2.Raw.to_string |> Print.good,
              actual |> Type2.Raw.to_string |> Print.bad,
            )
            |> string,
          ]
          |> newline,
          Newline,
        ]
        |> concat,
        None,
      )
    | NotFound(id) => (
        "Identifier Not Found",
        [
          [
            id
            |> Identifier.to_string
            |> Print.bad
            |> Print.fmt(
                 "unable to resolve an identifier %s in the local scope or any inherited scope",
               )
            |> string,
          ]
          |> newline,
        ]
        |> newline,
        Some(
          [
            _print_resolution((
              id
              |> Identifier.to_string
              |> Print.bad
              |> Print.fmt(
                   "check that the identifier %s is spelled correctly",
                 ),
              None,
            )),
            _print_resolution((
              "define the value yourself",
              Some(
                [Print.fmt("const %s = …;"), Print.fmt("let %s = …;")]
                |> List.map(fmt =>
                     id |> Identifier.to_string |> Print.bold |> fmt |> string
                   ),
              ),
            )),
            _print_resolution((
              "import the value from another module",
              Some(
                [Print.fmt("import { %s } from \"…\";")]
                |> List.map(fmt =>
                     id |> Identifier.to_string |> Print.bold |> fmt |> string
                   ),
              ),
            )),
          ]
          |> concat,
        ),
      )
    | ExternalNotFound(namespace, id) => (
        "External Not Found",
        [
          (
            switch (id) {
            | Named(id) =>
              Print.fmt(
                "an export with the identifier %s could not be found in module %s",
                id |> Identifier.to_string |> Print.bad,
                namespace |> Namespace.to_string |> Print.bad,
              )
            | Main =>
              Print.fmt(
                "a main export could not be found in module %s",
                namespace |> Namespace.to_string |> Print.bad,
              )
            }
          )
          |> string,
          Newline,
        ]
        |> newline,
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
      |> Print.bad
      |> Print.fmt("the reserved keyword %s was used as an identifier")
      |> string,
      Some(
        [
          _print_resolution((
            name
            |> Print.bad
            |> Print.fmt("check that the identifier %s is spelled correctly"),
            None,
          )),
          _print_resolution((
            Print.fmt(
              "rename %s so that there is no conflict with reserved keywords (%s)",
              name |> Print.bad,
              Constants.Keyword.reserved |> String.join(~separator=", "),
            ),
            None,
          )),
        ]
        |> concat,
      ),
    );

let _extract_compile_err = resolver =>
  fun
  | ImportCycle(cycles) => (
      None,
      "Import Cycle Found",
      cycles
      |> Print.many(~separator=" -> ", Functional.identity)
      |> Print.fmt("import cycle between the following modules: %s")
      |> string,
    )

  | UnresolvedModule(name) => (
      None,
      "Unresolved Module",
      name |> Print.fmt("could not resolve module: %s") |> string,
    )

  | FileNotFound(path) => (
      None,
      "File Not Found",
      path |> Print.fmt("could not find file with path: %s") |> string,
    )

  | InvalidModule(namespace) => (
      resolver
      |> Resolver.resolve_module(~skip_cache=true, namespace)
      |> Module.get_path
      |?> (x => (x, Cursor.zero)),
      "Invalid Module",
      Print.fmt("failed to parse module") |> string,
    )

  | ParseError(err, namespace, cursor) =>
    _extract_parse_err(err)
    |> Tuple.reduce3((title, description, resolutions) => {
         let module_ =
           Resolver.resolve_module(namespace, resolver)
           |> Module.read_to_string;

         (
           resolver
           |> Resolver.resolve_module(~skip_cache=true, namespace)
           |> Module.get_path
           |?> (x => (x, cursor)),
           title,
           [
             description,
             switch (module_) {
             | Ok(x) => File.CodeFrame.print(x, cursor) |> string
             | Error(_) => [string("code frame not available")] |> newline
             },
             switch (resolutions) {
             | Some(resolutions) =>
               [
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

let report = (resolver: Resolver.t, errors: list(compile_err)) => {
  let header =
    Print.fmt("%sFAILED%s", String.repeat(20, " "), String.repeat(20, " "));
  let summary =
    [
      Print.fmt(
        "finished with %s and %s",
        errors
        |> List.length
        |> string_of_int
        |> Print.fmt("%s error(s)")
        |> Print.red,
        0 |> string_of_int |> Print.fmt("%s warning(s)") |> Print.yellow,
      )
      |> string,
    ]
    |> newline;
  let horiz_border = String.repeat(String.length(header), "═");

  [
    Newline,
    [horiz_border |> Print.fmt("╔%s╗") |> Print.bad |> string] |> newline,
    [header |> Print.fmt("║%s║") |> Print.bad |> string] |> newline,
    [horiz_border |> Print.fmt("╚%s╝") |> Print.bad |> string] |> newline,
    Newline,
    summary,
    Newline,
    errors
    |> List.mapi((index, err) =>
         err
         |> _extract_compile_err(resolver)
         |> Tuple.reduce3((path, title, content) =>
              _print_err(~index, path, title, content)
            )
       )
    |> List.intersperse(Newline)
    |> concat,
    summary,
  ]
  |> concat;
};

let panic = (resolver: Resolver.t, errors: list(compile_err)) => {
  report(resolver, errors) |> Writer.write(stderr);

  exit(2);
};
