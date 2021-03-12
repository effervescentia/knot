open Kore;

module ImportGraph = Resolve.ImportGraph;

let __foo_id = AST.Internal("foo");
let __bar_id = AST.External("bar");
let __fizz_id = AST.Internal("fizz");
let __buzz_id = AST.External("buzz");

let _setup = get_imports => {
  let import_graph = ImportGraph.create(get_imports);

  import_graph |> ImportGraph.init(__foo_id);

  import_graph;
};

let suite =
  "Resolve.ImportGraph"
  >::: [
    "create()"
    >: (
      () => {
        let get_imports = id => id == __foo_id ? [__bar_id] : [];
        let import_graph = ImportGraph.create(get_imports);

        Assert.import_graph(
          {imports: Graph.empty(), get_imports},
          import_graph,
        );
      }
    ),
    "init()"
    >: (
      () => {
        let get_imports = id => id == __foo_id ? [__bar_id] : [];
        let import_graph = _setup(get_imports);

        Assert.import_graph(
          {
            imports:
              Graph.create([__bar_id, __foo_id], [(__foo_id, __bar_id)]),
            get_imports,
          },
          import_graph,
        );
      }
    ),
    "add_module()"
    >: (
      () => {
        let get_imports = id =>
          id == __foo_id
            ? [__bar_id] : id == __fizz_id ? [__buzz_id, __bar_id] : [];
        let import_graph = _setup(get_imports);

        let added = import_graph |> ImportGraph.add_module(__fizz_id);

        Assert.list_namespace([__buzz_id, __fizz_id], added);
        Assert.import_graph(
          {
            imports:
              Graph.create(
                [__buzz_id, __fizz_id, __bar_id, __foo_id],
                [
                  (__fizz_id, __bar_id),
                  (__fizz_id, __buzz_id),
                  (__foo_id, __bar_id),
                ],
              ),
            get_imports,
          },
          import_graph,
        );
      }
    ),
    "prune_subtree()"
    >: (
      () => {
        let get_imports = id =>
          id == __foo_id ? [__bar_id] : id == __bar_id ? [__fizz_id] : [];
        let import_graph = _setup(get_imports);

        Assert.import_graph(
          {
            imports:
              Graph.create(
                [__fizz_id, __bar_id, __foo_id],
                [(__foo_id, __bar_id), (__bar_id, __fizz_id)],
              ),
            get_imports,
          },
          import_graph,
        );

        let removed = import_graph |> ImportGraph.prune_subtree(__bar_id);

        Assert.list_namespace([__fizz_id, __bar_id], removed);
        Assert.import_graph(
          {
            imports: Graph.create([__foo_id], [(__foo_id, __bar_id)]),
            get_imports,
          },
          import_graph,
        );
      }
    ),
    "find_missing()"
    >: (
      () => {
        let get_imports = id =>
          id == __foo_id ? [__bar_id] : id == __bar_id ? [__fizz_id] : [];
        let import_graph = _setup(get_imports);

        import_graph |> ImportGraph.prune_subtree(__bar_id) |> ignore;

        Assert.list_namespace(
          [__bar_id],
          import_graph |> ImportGraph.find_missing,
        );
      }
    ),
    "refresh_subtree()"
    >: (
      () => {
        let get_imports = id =>
          id == __foo_id ? [__bar_id] : id == __bar_id ? [__fizz_id] : [];
        let new_get_imports = id => id == __buzz_id ? [] : [__buzz_id];
        let import_graph = {
          ..._setup(get_imports),
          get_imports: new_get_imports,
        };

        let (removed, added) =
          import_graph |> ImportGraph.refresh_subtree(__bar_id);

        Assert.list_namespace([__fizz_id], removed);
        Assert.list_namespace([__buzz_id, __bar_id], added);
        Assert.import_graph(
          {
            imports:
              Graph.create(
                [__buzz_id, __bar_id, __foo_id],
                [(__bar_id, __buzz_id), (__foo_id, __bar_id)],
              ),
            get_imports: new_get_imports,
          },
          import_graph,
        );
      }
    ),
  ];
