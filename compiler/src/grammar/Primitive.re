open Kore;

let nil =
  Keyword.nil
  >|= Block.cursor
  >|= (cursor => (AST.nil, `Strong(`Nil), cursor));

let boolean =
  Keyword.true_
  >|= Block.cursor
  >|= (cursor => (true |> AST.of_bool, `Strong(`Boolean), cursor))
  <|> (
    Keyword.false_
    >|= Block.cursor
    >|= (cursor => (false |> AST.of_bool, `Strong(`Boolean), cursor))
  );

let number = Number.parser >|= Tuple.map_fst3(AST.of_num);

let string =
  M.string
  >|= (
    block => (
      block |> Block.value |> AST.of_string,
      `Strong(`String),
      block |> Block.cursor,
    )
  );

let parser = choice([nil, boolean, number, string]);
