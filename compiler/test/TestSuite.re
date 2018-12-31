open OUnit2;

let () = {
  let failed = ref(false);

  run_test_tt_main(
    ~exit=
      _ => {
        failed := true;

        ANSITerminal.(
          Printf.sprintf("\n[%s] suite failed!\n", sprintf([red], "x"))
          |> print_endline
        );
      },
    "Knot"
    >::: [
      FileStreamTest.tests,
      LexerTest.tests,
      TokenStreamTest.tests,
      ParserTest.tests,
    ],
  );

  failed^ ?
    () :
    ANSITerminal.(
      Printf.sprintf(
        "\n[%s] Test suite successful!",
        sprintf([green], "✓"),
      )
      |> print_endline
    );
};