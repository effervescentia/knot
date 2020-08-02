open Globals;

let spacemaker = () =>
  List.init(Random.int(20), _ =>
    switch (Random.int(3)) {
    | 0 => Knot.Token.Space
    | 1 => Knot.Token.Tab
    | _ => Knot.Token.Newline
    }
  );
let rec drift =
  fun
  | [x] => [x]
  | [x, ...xs] => [x] @ spacemaker() @ drift(xs)
  | [] => [];

let load_resource = file => open_in(Config.resource_dir ++ "/" ++ file);

let analyze_resource = (scope, file) =>
  load_resource(file)
  |> FileStream.of_channel
  |> TokenStream.of_file_stream(Lexer.next_token)
  |> Parser.parse(Parser.prog)
  |> Analyzer.analyze(~scope);

let to_token_stream = tkns => {
  let remaining = ref(tkns);
  let next = ts =>
    if (List.length(ts) == 0) {
      None;
    } else {
      let t = List.nth(ts, 0);
      remaining := List.tl(ts);
      Some(t);
    };

  LazyStream.of_function(() => next(remaining^));
};

let test_parse_ast = (prog, (tkns, ast)) =>
  switch (Parser.parse(prog, to_token_stream(tkns))) {
  | Some(res) => Assert.assert_ast_eql(ast, res)
  | None => assert_failure("no AST found")
  };

let test_parse_import = (prog, (tkns, import)) =>
  switch (Parser.parse(prog, to_token_stream(tkns))) {
  | Some(res) => Assert.assert_import_eql(import, res)
  | None => assert_failure("no import found")
  };

let test_parse_stmt = (prog, (tkns, stmt)) =>
  switch (Parser.parse(prog, to_token_stream(tkns))) {
  | Some(res) => Assert.assert_stmt_eql(stmt, res)
  | None => assert_failure("no statment found")
  };

let test_parse_decl = (decl, (tkns, ast)) =>
  switch (Parser.parse(decl, to_token_stream(tkns))) {
  | Some(res) => Assert.assert_decl_eql(ast, res)
  | None => assert_failure("no declaration found")
  };

let test_many = (test, xs) => List.iter(test, xs);
