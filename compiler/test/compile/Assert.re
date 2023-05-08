include TestResolve.Assert;

let compiler =
  Alcotest.(
    check(
      testable(
        (ppf, _) => "" |> Format.pp_print_string(ppf),
        Compile.Compiler.(
          (l, r) => {
            Alcotest.check(
              Compare.module_table,
              "module_table matches",
              l.modules,
              r.modules,
            );

            l.graph.imports == r.graph.imports
            && l.resolver.root_dir == r.resolver.root_dir
            && l.config == r.config;
          }
        ),
      ),
      "compiler matches",
    )
  );

let file_contents = (expected, file) => {
  let actual = File.IO.read_to_string(file);

  string(expected, actual);
};
