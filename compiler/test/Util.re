open Core;

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

let to_file_stream = s => {
  let remaining = ref(s);
  let next = s =>
    if (String.length(s) == 0) {
      None;
    } else {
      let ch = s.[0];
      remaining := String.sub(s, 1, String.length(s) - 1);
      Some((ch, (0, 0)));
    };

  LazyStream.of_function(() => next(remaining^));
};

let to_token_stream = tkns => {
  let remaining = ref(tkns);
  let next = ts =>
    if (List.length(ts) == 0) {
      None;
    } else {
      let t = List.nth(ts, 0);
      remaining := List.tl(ts);
      KnotLex.Debug.print_tkn(t)
      |> Printf.sprintf("grabbed next token: %s")
      |> print_endline;
      Some((
        t,
        KnotLex.Debug.print_action
        % Printf.sprintf("context action: %s")
        % print_endline,
      ));
    };

  LazyStream.of_function(() => next(remaining^));
};

let test_parse_ast = (prog, (tkns, ast)) =>
  switch (Parser.parse(prog, to_token_stream(tkns))) {
  | Some(res) => Assert.assert_ast_eql(ast, res)
  | None => assert_failure("no AST found")
  };

let test_parse_decl = (decl, (tkns, ast)) =>
  switch (Parser.parse(decl, to_token_stream(tkns))) {
  | Some(res) => Assert.assert_decl_eql(ast, res)
  | None => assert_failure("no declaration found")
  };

let test_parse_expr = (expr, (tkns, ast)) =>
  switch (Parser.parse(expr, to_token_stream(tkns))) {
  | Some(res) => Assert.assert_expr_eql(ast, res)
  | None => assert_failure("no expression found")
  };

let test_parse_jsx = (jsx, (tkns, ast)) =>
  switch (Parser.parse(jsx, to_token_stream(tkns))) {
  | Some(res) => Assert.assert_jsx_eql(ast, res)
  | None => assert_failure("no JSX found")
  };

let test_many = (test, xs) => List.iter(test, xs);
