open Kore;

let _import_named = (ctx: ParseContext.t, import) =>
  fun
  | (((id, _), None), _) as no_alias =>
    ctx
    |> import(Reference.Export.Named(id), id)
    |> Result.map_error(Tuple.with_snd2(N.get_range(no_alias)))

  | (((id, _), Some((alias, _))), _) as with_alias =>
    ctx
    |> import(Reference.Export.Named(id), alias)
    |> Result.map_error(Tuple.with_snd2(N.get_range(with_alias)));

let _import_module = (ctx, imports, import) =>
  imports
  |> List.map(
       fun
       | (A.MainImport((alias, _)), _) as main_import =>
         ctx
         |> import(Reference.Export.Main, alias)
         |> Result.map_error(Tuple.with_snd2(N.get_range(main_import)))

       | (A.NamedImport(id, alias), _) as named_import =>
         _import_named(
           ctx,
           import,
           N.untyped((id, alias), N.get_range(named_import)),
         ),
     );

let namespace = imports =>
  M.string
  >|= N.map(Reference.Namespace.of_string)
  >|= (namespace => (namespace, imports));

let main_import =
  M.identifier >|= (import => [import |> N.wrap(A.of_main_import)]);

let named_imports = (ctx: ParseContext.t) =>
  Identifier.parser(ctx)
  >>= (
    id =>
      Keyword.as_ >> Identifier.parser(ctx) >|= (alias => (id, Some(alias)))
  )
  <|> (Identifier.parser(ctx) >|= (id => (id, None)))
  |> M.comma_sep
  |> M.between(Symbol.open_closure, Symbol.close_closure)
  >|= N.map(
        List.map(((name, alias) as import) =>
          N.untyped(import, N.join_ranges(name, alias |?: name))
        ),
      );

let module_import = (ctx: ParseContext.t) =>
  Keyword.import
  >>= (
    kwd =>
      choice([
        main_import,
        named_imports(ctx) >|= fst >|= List.map(N.map(A.of_named_import)),
      ])
      |> M.comma_sep
      >|= List.flatten
      << Keyword.from
      >>= namespace
      >@= (
        (((namespace, _), imports)) =>
          namespace
          |> ParseContext.import
          |> _import_module(ctx, imports)
          |> List.iter(
               Result.iter_error(((err, range)) =>
                 ctx |> ParseContext.report(TypeError(err), range)
               ),
             )
      )
      >|= (
        ((namespace, imports)) =>
          N.untyped(
            (fst(namespace), imports) |> A.of_import,
            N.join_ranges(kwd, namespace),
          )
      )
  );

let standard_import = (ctx: ParseContext.t) =>
  Keyword.import
  >>= (
    kwd =>
      named_imports(ctx)
      >@= fst
      % List.iter(
          _import_named(ctx, ParseContext.import(Stdlib))
          % Result.iter_error(x => ()),
        )
      >|= (
        imports =>
          N.untyped(
            A.of_standard_import(fst(imports)),
            N.join_ranges(kwd, imports),
          )
      )
  );

let parser = (ctx: ParseContext.t) =>
  choice([module_import(ctx), standard_import(ctx)]) |> M.terminated;
