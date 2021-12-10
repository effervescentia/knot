open Kore;

module Resolver = Resolve.Resolver;
module Module = Resolve.Module;
module Writer = File.Writer;

let _example_sep =
  Fmt.Sep.(create(~trail=Trail.nop, (ppf, ()) => Fmt.pf(ppf, "@,// or@,")));

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
      option((Module.path_t, Range.t)),
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
          | Some((Module.{relative, full}, range)) => {
              pf(
                ppf,
                " : %a:%a%a",
                cyan_str,
                relative,
                grey(text_loc),
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

let _pp_type_trait = (pp_target: Fmt.t(string)): Fmt.t(Type.Trait.t) =>
  ppf =>
    Type.Trait.(
      fun
      | Unknown =>
        Fmt.pf(ppf, "%a which can represent any type", pp_target, "Unknown")

      | Number =>
        Fmt.(
          pf(
            ppf,
            "%a which is shared by the types %a and %a",
            pp_target,
            "number",
            bold_str,
            "int",
            bold_str,
            "float",
          )
        )
    );

let _extract_type_err =
  Type.Error.(
    fun
    | NotAssignable(t, trait) => (
        "Type Cannot Be Assigned",
        Fmt.(
          (
            ppf =>
              pf(
                ppf,
                "expected a type that implements the trait %a but found the type %a instead",
                _pp_type_trait(good_str),
                trait,
                bad(Type.Raw.pp),
                t,
              )
          )
        ),
        [],
      )

    | TypeMismatch(expected, actual) => (
        "Types Do Not Match",
        Fmt.(
          (
            ppf =>
              pf(
                ppf,
                "expected the type %a but found the type %a instead",
                good(Type.Raw.pp),
                expected,
                bad(Type.Raw.pp),
                actual,
              )
          )
        ),
        [],
      )

    | NotFound(id) => (
        "Identifier Not Found",
        (
          ppf =>
            Fmt.pf(
              ppf,
              "unable to resolve an identifier %a in the local scope or any inherited scope",
              Fmt.bad(Identifier.pp),
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
                  Fmt.bad(Identifier.pp),
                  id,
                )
            ),
            [],
          ),
          (
            (ppf => Fmt.string(ppf, "define the value yourself")),
            Fmt.[str("const %s = …;"), str("let %s = …;")]
            |> List.map(fmt => id |> ~@Fmt.bold(Identifier.pp) |> fmt),
          ),
          (
            (ppf => Fmt.string(ppf, "import the value from another module")),
            Fmt.[str("import { %s } from \"…\";")]
            |> List.map(fmt => id |> ~@Fmt.bold(Identifier.pp) |> fmt),
          ),
        ],
      )

    | DuplicateIdentifier(id) => (
        "Identifier Already Defined",
        (
          ppf =>
            Fmt.pf(
              ppf,
              "a variable with the same name (%a) already exists in the local scope or an inherited scope",
              Fmt.bad(Identifier.pp),
              id,
            )
        ),
        [
          ((ppf => Fmt.string(ppf, "change the name of this variable")), []),
        ],
      )

    | ExternalNotFound(namespace, id) => (
        "External Not Found",
        switch (id) {
        | Named(id) => (
            ppf =>
              Fmt.pf(
                ppf,
                "an export with the identifier %a could not be found in module %a",
                Fmt.bad(Identifier.pp),
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
                  "a main export could not be found in module %a",
                  bad(Namespace.pp),
                  namespace,
                )
            )
          )
        },
        [],
      )

    | TypeResolutionFailed => (
        "Type Resolution Failed",
        (
          ppf =>
            Fmt.pf(
              ppf,
              "failed to resolve the type of an expression, this is likely an internal issue with the compiler",
            )
        ),
        [],
      )
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
                  Constants.Keyword.reserved |> String.join(~separator=", "),
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
      |> Module.get_path
      |?> (x => (x, Range.zero)),
      (ppf => err |> pp_compile_err(ppf)),
    )

  | ParseError(err, namespace, range) =>
    _extract_parse_err(err)
    |> Tuple.join3((title, description, resolutions) => {
         let module_ =
           Resolver.resolve_module(namespace, resolver)
           |> Module.read_to_string;

         (
           title,
           resolver
           |> Resolver.resolve_module(~skip_cache=true, namespace)
           |> Module.get_path
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
                     | Ok(x) => File.CodeFrame.print(x, range) |> string(ppf)
                     | Error(_) => pf(ppf, "@,@,[code frame not available]"),
                   module_,
                   ppf =>
                     fun
                     | [] => nop(ppf, ())
                     | _ =>
                       pf(
                         ppf,
                         "@,@,try one of the following to resolve this issue:@,%a",
                         block(
                           ~layout=Vertical,
                           ~sep=Sep.double_newline,
                           _pp_resolution,
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
        border |> Fmt.str("╔%s╗"),
        header |> Fmt.str("║%s║"),
        border |> Fmt.str("╚%s╝"),
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
        Fmt.str("%i error(s)", error_count),
        yellow_str,
        Fmt.str("%i warning(s)", warning_count),
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
          list(~layout=Vertical, ~sep=Sep.nop, (ppf, (index, err)) =>
            err
            |> _extract_compile_err(resolver)
            |> Tuple.join3((path, title, content) =>
                 (index, path, title, content)
               )
            |> Fmt.pf(ppf, "%a@,@,", _pp_err)
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
