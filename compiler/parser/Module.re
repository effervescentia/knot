open Core;

module M = Matchers;

let import =
  M.import
  >> M.identifier
  >>= (
    main =>
      M.from
      >> M.string
      >>= (s => return(Import(s, [MainExport(main)])))
      << optional(M.semicolon)
  );
let decl = input =>
  (
    Const.decl
    <|> State.decl
    <|> Function.decl
    <|> View.decl
    ==> (d => Declaration(d))
  )(
    input,
  );
let stmt = input => (import <|> decl)(input);

let stmts = input => (many(stmt) ==> (l => Statements(l)))(input);