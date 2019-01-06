open Knot;
open OUnit2;

let load_resource = file => open_in(Config.resource_dir ++ "/" ++ file);

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

  Opal.LazyStream.of_function(() => next(remaining^));
};

let test_parse_ast = (prog, (tkns, ast)) =>
  switch (KnotParse.Parser.parse(prog, to_token_stream(tkns))) {
  | Some(res) => Assert.assert_ast_eql(ast, res)
  | None => assert_failure("no AST found")
  };

let test_parse_decl = (decl, (tkns, ast)) =>
  switch (KnotParse.Parser.parse(decl, to_token_stream(tkns))) {
  | Some(res) => Assert.assert_decl_eql(ast, res)
  | None => assert_failure("no declaration found")
  };

let test_many = (test, xs) => List.iter(test, xs);