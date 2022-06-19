open Kore;

let _import_named = (ctx: ModuleContext.t, import) =>
  fun
  | (((id, _), None), _) as no_alias =>
    ctx |> import((Reference.Export.Named(id), N.get_range(no_alias)), id)

  | (((id, _), Some((alias, _))), _) as with_alias =>
    ctx
    |> import((Reference.Export.Named(id), N.get_range(with_alias)), alias);

let _import_module = (ctx, imports, import) =>
  imports
  |> List.iter(
       fun
       | (A.MainImport((alias, _)), _) as main_import =>
         ctx
         |> import((Reference.Export.Main, N.get_range(main_import)), alias)

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

let named_imports = (ctx: ModuleContext.t) =>
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

let module_import = (ctx: ModuleContext.t) =>
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
          namespace |> ModuleContext.import |> _import_module(ctx, imports)
      )
      >|= (
        ((namespace, imports)) =>
          N.untyped(
            (fst(namespace), imports) |> A.of_import,
            N.join_ranges(kwd, namespace),
          )
      )
  );

let standard_import = (ctx: ModuleContext.t) =>
  Keyword.import
  >>= (
    kwd =>
      named_imports(ctx)
      >@= fst
      % List.iter(_import_named(ctx, ModuleContext.import(Stdlib)))
      >|= (
        imports =>
          N.untyped(
            A.of_standard_import(fst(imports)),
            N.join_ranges(kwd, imports),
          )
      )
  );

let parser = (ctx: ModuleContext.t) =>
  choice([module_import(ctx), standard_import(ctx)]) |> M.terminated;
