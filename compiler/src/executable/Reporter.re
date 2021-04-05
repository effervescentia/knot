open Kore;

module Resolver = Resolve.Resolver;
module Module = Resolve.Module;
module Writer = File.Writer;

let _print_code_examples =
  List.intersperse(
    [Pretty.Newline, "// or" |> Pretty.string, Pretty.Newline]
    |> Pretty.concat,
  )
  % Pretty.newline
  % Pretty.indent(2);

let _print_resolution = ((description, examples)) =>
  [
    [description |> Print.fmt("• %s") |> Pretty.string] |> Pretty.newline,
    Pretty.Newline,
    switch (examples) {
    | Some(examples) => [examples |> _print_code_examples] |> Pretty.newline
    | None => Pretty.Nil
    },
  ]
  |> Pretty.concat;

let _print_err = (~index, path, title, content) =>
  [
    Print.fmt("%d) %s", index + 1, title) |> Print.bad |> Pretty.string,
    switch (path) {
    | Some(path) =>
      [" : " |> Pretty.string, path |> Print.cyan |> Pretty.string]
      |> Pretty.newline
      |> Pretty.indent(2)
    | None => Pretty.Newline
    },
    Pretty.Newline,
    [content |> Pretty.indent(2)] |> Pretty.concat,
  ]
  |> Pretty.concat;

let _type_trait_to_string = print_target =>
  Type.(
    fun
    | K_Unknown =>
      "Unknown"
      |> print_target
      |> Print.fmt("%s which can represent any type")

    | K_Exactly(t) =>
      t |> Type.strong_to_string |> Print.fmt("Exactly<%s>") |> print_target

    | K_Numeric =>
      Print.fmt(
        "%s which is shared by the types %s and %s",
        "Numeric" |> print_target,
        "int" |> Print.bold,
        "float" |> Print.bold,
      )

    | K_Iterable(t) =>
      t
      |> Type.trait_to_string
      |> Print.fmt("Iterable<%s>")
      |> print_target
      |> Print.fmt(
           "%s which is used for containers of sequential data such as a list",
         )

    | K_Structural(traits) =>
      traits
      |> Print.many(~separator=", ", ((name, trait)) =>
           Print.fmt("%s: %s", name, trait |> Type.trait_to_string)
         )
      |> Print.fmt("Structural<{ %s }>")
      |> print_target
      |> Print.fmt(
           "'%s' which is used for containers of data keyed by strings",
         )

    | K_Callable(args, result) =>
      Print.fmt(
        "Callable<[ %s ], %s>",
        args |> Print.many(~separator=", ", Type.trait_to_string),
        result |> Type.trait_to_string,
      )
      |> print_target
      |> Print.fmt("'%s' which is used for functions")
  );

let _extract_type_err =
  Type.(
    fun
    | TraitConflict(lhs, rhs) => (
        "Types Have Conflicting Traits",
        [
          [
            "the types of the arguments in this operation are not compatible with each other:"
            |> Pretty.string,
          ]
          |> Pretty.newline,
          Pretty.Newline,
          [
            [
              lhs
              |> _type_trait_to_string(Print.bad)
              |> Print.fmt(
                   "• the left-hand-side argument has the trait %s",
                 )
              |> Pretty.string,
            ]
            |> Pretty.newline,
            [
              rhs
              |> _type_trait_to_string(Print.bad)
              |> Print.fmt(
                   "• the right-hand-side argument has the trait %s",
                 )
              |> Pretty.string,
            ]
            |> Pretty.newline,
          ]
          |> Pretty.concat,
          Pretty.Newline,
        ]
        |> Pretty.concat,
        None,
      )
    | NotAssignable(t, trait) => (
        "Type Cannot Be Assigned",
        [
          [
            Print.fmt(
              "expected a type that implements the trait %s but found the type %s instead",
              trait |> _type_trait_to_string(Print.good),
              t |> Type.to_string |> Print.bad,
            )
            |> Pretty.string,
          ]
          |> Pretty.newline,
          Pretty.Newline,
        ]
        |> Pretty.concat,
        None,
      )
    | TypeMismatch(expected, actual) => (
        "Types Do Not Match",
        [
          [
            Print.fmt(
              "expected the type %s but found the type %s instead",
              expected |> Type.to_string |> Print.good,
              actual |> Type.to_string |> Print.bad,
            )
            |> Pretty.string,
          ]
          |> Pretty.newline,
          Pretty.Newline,
        ]
        |> Pretty.concat,
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
            |> Pretty.string,
          ]
          |> Pretty.newline,
        ]
        |> Pretty.newline,
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
                     id
                     |> Identifier.to_string
                     |> Print.bold
                     |> fmt
                     |> Pretty.string
                   ),
              ),
            )),
            _print_resolution((
              "import the value from another module",
              Some(
                [Print.fmt("import { %s } from \"…\";")]
                |> List.map(fmt =>
                     id
                     |> Identifier.to_string
                     |> Print.bold
                     |> fmt
                     |> Pretty.string
                   ),
              ),
            )),
          ]
          |> Pretty.concat,
        ),
      )
    | ExternalNotFound(namespace, id) => (
        "External Not Found",
        [
          Print.fmt(
            "an export with the identifier %s could not be found in module %s",
            id |> Identifier.to_string |> Print.bad,
            namespace |> Namespace.to_string |> Print.bad,
          )
          |> Pretty.string,
          Pretty.Newline,
        ]
        |> Pretty.newline,
        None,
      )
  );

let _extract_parse_err =
  fun
  | TypeError(err) => _extract_type_err(err);

let _extract_compile_err = resolver =>
  fun
  | ImportCycle(cycles) => (
      None,
      "Import Cycle Found",
      cycles
      |> Print.many(~separator=" -> ", Functional.identity)
      |> Print.fmt("import cycle between the following modules: %s")
      |> Pretty.string,
    )

  | UnresolvedModule(name) => (
      None,
      "Unresolved Module",
      name |> Print.fmt("could not resolve module: %s") |> Pretty.string,
    )

  | FileNotFound(path) => (
      None,
      "File Not Found",
      path |> Print.fmt("could not find file with path: %s") |> Pretty.string,
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
           |?> (x => x.relative),
           title,
           [
             description,
             File.CodeFrame.print(module_, cursor) |> Pretty.string,
             switch (resolutions) {
             | Some(resolutions) =>
               [
                 Pretty.Newline,
                 [
                   "try one of the following to resolve this issue:"
                   |> Pretty.string,
                 ]
                 |> Pretty.newline,
                 Pretty.Newline,
                 resolutions |> Pretty.indent(2),
               ]
               |> Pretty.concat
             | None => Pretty.Nil
             },
           ]
           |> Pretty.concat,
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
      |> Pretty.string,
    ]
    |> Pretty.newline;
  let horiz_border = String.repeat(header |> String.length, "═");

  [
    Pretty.Newline,
    [horiz_border |> Print.fmt("╔%s╗") |> Print.bad |> Pretty.string]
    |> Pretty.newline,
    [header |> Print.fmt("║%s║") |> Print.bad |> Pretty.string]
    |> Pretty.newline,
    [horiz_border |> Print.fmt("╚%s╝") |> Print.bad |> Pretty.string]
    |> Pretty.newline,
    Pretty.Newline,
    summary,
    Pretty.Newline,
    errors
    |> List.mapi((index, err) =>
         err
         |> _extract_compile_err(resolver)
         |> Tuple.reduce3((path, title, content) =>
              _print_err(~index, path, title, content)
            )
       )
    |> List.intersperse(Pretty.Newline)
    |> Pretty.concat,
    summary,
  ]
  |> Pretty.concat;
};

let panic = (resolver: Resolver.t, errors: list(compile_err)) => {
  report(resolver, errors) |> Writer.write(stderr);

  exit(2);
};
