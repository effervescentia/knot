open Kore;

module Resolver = Resolve.Resolver;
module Module = Resolve.Module;
module Writer = File.Writer;

let _print_red = (~color) => color ? Print.red : Print.bold;
let _print_green = (~color) => color ? Print.green : Print.bold;
let _print_yellow = (~color) => color ? Print.yellow : Print.bold;
let _print_bad = (~color) => color ? Print.bad : Print.bold;
let _print_good = (~color) => color ? Print.good : Print.bold;
let _print_warn = (~color) => color ? Print.warn : Print.bold;

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

let _print_err = (~index, ~color, title, content) =>
  [
    [
      Print.fmt("%d) %s", index + 1, title)
      |> _print_bad(~color)
      |> Pretty.string,
    ]
    |> Pretty.newline,
    Pretty.Newline,
    [content |> Pretty.indent(2)] |> Pretty.concat,
  ]
  |> Pretty.concat;

let _type_trait_to_string = (~color) =>
  Type.(
    fun
    | K_Unknown =>
      "Unknown"
      |> _print_bad(~color)
      |> Print.fmt("%s which can represent any type")

    | K_Exactly(t) =>
      t
      |> Type.strong_to_string
      |> Print.fmt("Exactly<%s>")
      |> _print_bad(~color)

    | K_Numeric =>
      Print.fmt(
        "%s which is shared by the types %s and %s",
        "Numeric" |> _print_bad(~color),
        "int" |> _print_good(~color),
        "float" |> _print_good(~color),
      )

    | K_Iterable(t) =>
      t
      |> Type.trait_to_string
      |> Print.fmt("Iterable<%s>")
      |> _print_bad(~color)
      |> Print.fmt(
           "%s which is used for containers of sequential data such as a list",
         )

    | K_Structural(traits) =>
      traits
      |> Print.many(~separator=", ", ((name, trait)) =>
           Print.fmt("%s: %s", name, trait |> Type.trait_to_string)
         )
      |> Print.fmt("Structural<{ %s }>")
      |> _print_bad(~color)
      |> Print.fmt(
           "'%s' which is used for containers of data keyed by strings",
         )

    | K_Callable(args, result) =>
      Print.fmt(
        "Callable<[ %s ], %s>",
        args |> Print.many(~separator=", ", Type.trait_to_string),
        result |> Type.trait_to_string,
      )
      |> _print_bad(~color)
      |> Print.fmt("'%s' which is used for functions")
  );

let _extract_type_err = (~color) =>
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
              |> _type_trait_to_string(~color)
              |> Print.fmt(
                   "• the left-hand-side argument has the trait %s",
                 )
              |> Pretty.string,
            ]
            |> Pretty.newline,
            [
              rhs
              |> _type_trait_to_string(~color)
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
            trait
            |> _type_trait_to_string(~color)
            |> Print.fmt("expected a type that implements the trait %s,")
            |> Pretty.string,
          ]
          |> Pretty.newline,
          [
            t
            |> Type.to_string
            |> _print_bad(~color)
            |> Print.fmt("but found the type %s instead")
            |> Pretty.string,
          ]
          |> Pretty.newline,
          Pretty.Newline,
        ]
        |> Pretty.concat,
        None,
      )
    | TypeMismatch(lhs, rhs) => (
        "Types Do Not Match",
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
              |> Type.to_string
              |> _print_bad(~color)
              |> Print.fmt("• the left-hand-side argument has the type %s")
              |> Pretty.string,
            ]
            |> Pretty.newline,
            [
              rhs
              |> Type.to_string
              |> _print_bad(~color)
              |> Print.fmt(
                   "• the right-hand-side argument has the type %s",
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
    | NotFound(id) => (
        "Identifier Not Found",
        [
          [
            id
            |> Identifier.to_string
            |> _print_bad(~color)
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
              |> _print_bad(~color)
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
                     |> _print_good(~color)
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
                     |> _print_good(~color)
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
            id |> Identifier.to_string |> _print_bad(~color),
            namespace |> Namespace.to_string |> _print_bad(~color),
          )
          |> Pretty.string,
          Pretty.Newline,
        ]
        |> Pretty.newline,
        None,
      )
  );

let _extract_parse_err = (~color) =>
  fun
  | TypeError(err) => _extract_type_err(~color, err);

let _extract_compile_err = (~color, resolver) =>
  fun
  | ImportCycle(cycles) => (
      "Import Cycle Found",
      cycles
      |> Print.many(~separator=" -> ", Functional.identity)
      |> Print.fmt("import cycle between the following modules: %s")
      |> Pretty.string,
    )

  | UnresolvedModule(name) => (
      "Unresolved Module",
      name |> Print.fmt("could not resolve module: %s") |> Pretty.string,
    )

  | FileNotFound(path) => (
      "File Not Found",
      path |> Print.fmt("could not find file with path: %s") |> Pretty.string,
    )

  | ParseError(err, namespace, cursor) =>
    _extract_parse_err(~color, err)
    |> Tuple.reduce3((title, description, resolutions) => {
         let module_ =
           Resolver.resolve_module(namespace, resolver)
           |> Module.read_to_string;

         (
           title,
           [
             description,
             File.CodeFrame.print(~color, module_, cursor) |> Pretty.string,
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

let report = (~color, resolver: Resolver.t, errors: list(compile_err)) => {
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
        |> _print_red(~color),
        0
        |> string_of_int
        |> Print.fmt("%s warning(s)")
        |> _print_yellow(~color),
      )
      |> Pretty.string,
    ]
    |> Pretty.newline;
  let horiz_border = String.repeat(header |> String.length, "═");

  [
    Pretty.Newline,
    [
      horiz_border
      |> Print.fmt("╔%s╗")
      |> _print_bad(~color)
      |> Pretty.string,
    ]
    |> Pretty.newline,
    [header |> Print.fmt("║%s║") |> _print_bad(~color) |> Pretty.string]
    |> Pretty.newline,
    [
      horiz_border
      |> Print.fmt("╚%s╝")
      |> _print_bad(~color)
      |> Pretty.string,
    ]
    |> Pretty.newline,
    Pretty.Newline,
    summary,
    Pretty.Newline,
    errors
    |> List.mapi((index, err) =>
         err
         |> _extract_compile_err(~color, resolver)
         |> Tuple.reduce2((title, content) =>
              _print_err(~index, ~color, title, content)
            )
       )
    |> List.intersperse(Pretty.Newline)
    |> Pretty.concat,
    summary,
  ]
  |> Pretty.concat;
};

let panic = (~color, resolver: Resolver.t, errors: list(compile_err)) => {
  report(~color, resolver, errors) |> Writer.write(stderr);

  exit(2);
};
