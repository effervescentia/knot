open Kore;

let _import_named = (ctx, import) =>
  fun
  | (((id, _), None), range) =>
    ctx |> import((Reference.Export.Named(id), range), id)

  | (((id, _), Some(label)), range) =>
    ctx |> import((Reference.Export.Named(id), range), NR.get_value(label));

let _import_module = (ctx, imports, import) =>
  imports
  |> List.iter(
       fun
       | (A.MainImport((alias, _)), range) =>
         ctx |> import((Reference.Export.Main, range), alias)

       | (A.NamedImport(id, alias), range) =>
         _import_named(ctx, import, ((id, alias), range)),
     );

let namespace = imports =>
  M.string
  >|= NR.map_value(Reference.Namespace.of_string)
  >|= (namespace => (namespace, imports));

let main_import =
  M.identifier
  >|= NR.map_value(A.of_public)
  >|= (import => [import |> NR.wrap(A.of_main_import)]);

let named_imports = (ctx: ModuleContext.t) =>
  Identifier.parser(ctx)
  >>= (
    id =>
      Keyword.as_ >> Identifier.parser(ctx) >|= (label => (id, Some(label)))
  )
  <|> (Identifier.parser(ctx) >|= (id => (id, None)))
  |> M.comma_sep
  |> M.between(Symbol.open_closure, Symbol.close_closure)
  >|= NR.map_value(
        List.map(((name, label) as import) =>
          (
            import,
            NR.(
              Range.join(
                get_range(name),
                label |?> get_range |?: get_range(name),
              )
            ),
          )
        ),
      );

let module_import = (ctx: ModuleContext.t) =>
  Keyword.import
  >>= NR.get_range
  % (
    start =>
      choice([
        main_import,
        named_imports(ctx)
        >|= NR.get_value
        >|= List.map(NR.map_value(A.of_named_import)),
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
        ((namespace, imports)) => (
          (NR.get_value(namespace), imports) |> A.of_import,
          Range.join(start, NR.get_range(namespace)),
        )
      )
  );

let standard_import = (ctx: ModuleContext.t) =>
  Keyword.import
  >>= NR.get_range
  % (
    start =>
      named_imports(ctx)
      >@= NR.get_value
      % List.iter(_import_named(ctx, ModuleContext.import(Stdlib)))
      >|= (
        ((imports, imports_range)) => (
          A.of_standard_import(imports),
          Range.join(start, imports_range),
        )
      )
  );

let parser = (ctx: ModuleContext.t) =>
  choice([module_import(ctx), standard_import(ctx)]) |> M.terminated;
