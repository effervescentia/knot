open Globals;

let () = {
  let failed = ref(false);

  Random.self_init();

  run_test_tt_main(
    ~exit=
      _ => {
        failed := true;

        ANSITerminal.(
          Printf.sprintf("\n[%s] suite failed!\n", sprintf([red], "x"))
        )
        |> print_endline;

        exit(-1);
      },
    "Knot"
    >::: [
      FileStreamTest.tests,
      LexerTest.tests,
      TokenStreamTest.tests,
      ParserTest.tests,
      ResolverTest.tests,
      GeneratorTest.tests,
    ],
  );

  failed^
    ? ()
    : ANSITerminal.(sprintf([green], "✓"))
      |> Printf.sprintf("\n[%s] Test suite successful!")
      |> print_endline;
};
