open Kore;

module Compiler = Compile.Compiler;

let __root_dir = "fizz";
let __config =
  Compiler.{
    name: "foo",
    entry: Internal("bar"),
    root_dir: __root_dir,
    source_dir: "buzz",
  };

let suite =
  "Compile.Compiler"
  >::: [
    "create() - use global error handler"
    >: (
      () =>
        Assert.compiler(
          {
            config: __config,
            graph: Resolve.ImportGraph.create(_ => []),
            modules: Compile.ModuleTable.create(1),
            resolver: {
              cache: "",
              root_dir: __root_dir,
            },
            errors: ref([]),
            throw,
          },
          Compiler.create(__config),
        )
    ),
    "create() - use custom error handler"
    >: (
      () => {
        let throw = _ => ();

        Assert.compiler(
          {
            config: __config,
            graph: Resolve.ImportGraph.create(_ => []),
            modules: Compile.ModuleTable.create(1),
            resolver: {
              cache: "",
              root_dir: __root_dir,
            },
            errors: ref([]),
            throw,
          },
          Compiler.create(~catch=throw, __config),
        );
      }
    ),
  ];
